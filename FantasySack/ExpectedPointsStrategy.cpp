#include <vector>

#include <absl/flags/flag.h>

#include "ExpectedPointsStrategy.h"

using namespace std;

ABSL_FLAG(bool, wceval, 0, "Evalute playing a wildcard at each week");
ABSL_FLAG(int, gw, -1, "Specify a gw to analyze");

#define XP100(P)  double(P)/100

int DoXptsSeasonLoop(Season& season, Team& team, Solver& solver) {
    int startgw = 3, endgw = season.max_gw;
    int gwflag = absl::GetFlag(FLAGS_gw);
    if (gwflag != -1) {
        startgw = gwflag;
        endgw = gwflag;
    }
    wostream& fout = *season.pout;
    int points = 0;
    int prevw = 1;
    int nwc = 1;

    bool secondwc = false;
    for (int w = startgw; w <= endgw; w += 2) {
        if (w > 19 && !secondwc) {
            secondwc = true;
            nwc++;
        }
        
        fout << "\n\nTEAM at gw " << w << " :" << endl;
        points += team.print_sol(prevw, w, false, false);
        fout << "Points so far = " << points << endl;
        fout << "*******************************\n";

        if (absl::GetFlag(FLAGS_wceval)
            || gwflag != -1) {
            Team wct(team);
            solver.Init();
            season.ResetPlayers(w);
            season.ApplyXpts(w, 6);
            solver.Solve(wct, w);
            wct.print_sol(prevw, w, false, false);

            fout << "Wild card estimated points in the next few weeks = " << XP100(wct.GetEffectivePoints()) << endl;
        }
        solver.Init();
        season.ResetPlayers(w);
        season.ApplyXpts(w, 6);

        int bench = team.bcount;
        int minp = team.MinEffectivePlayer();

        do {
            prevw = w;
            multimap<int, int> blanker;
            int gk = 0;
            int df = 0;
            int md = 0;
            int fw = 0;
            int lp = 0;

            fout << endl;
            for (int i = 0; i < 11; i++) {
                Player* p = season.player + team.team[i];
                p->skip = true;
            }

            // Adjust budget after selling transers
            const int MAX_PLAYERS_TO_TRANSFER = 2;
            vector<int> old_players(MAX_PLAYERS_TO_TRANSFER);
            vector<int> new_players(MAX_PLAYERS_TO_TRANSFER);

            short max_delta = SHRT_MIN;
            short max_effective = SHRT_MIN;
            vector<int> max_old(2);
            vector<int> max_new(2);

            short max_single_delta = SHRT_MIN;
            int max_single_old = team.team[0];
            int max_single_new = team.team[0];

            for (int i = 0; i < 11; i++) {
                Player* p = season.player + team.team[i];
                p->AddToSquadTotals(gk, df, md, fw, lp, 1);
                old_players[0] = i;
                int spbudget = team.bank + team.GetPlayerValue(w, old_players[0]);
                for (int j = 0; j < season.pcount; j++) {
                    Player* p2 = season.player + j;
                    if (p2->skip)
                        continue;
                    if (p2->price > spbudget)
                        continue;
                    short delta2 = p2->effective_points - p->effective_points;
                    if (p2->pos == p->pos && delta2 > max_single_delta) {
                        max_single_old = i;
                        max_single_new = j;
                        max_single_delta = delta2;
                    }
                }
                for (int j = 0; j < 11; j++) {
                    if (i == j)
                        continue;

                    Player* p2 = season.player + team.team[j];
                    p2->AddToSquadTotals(gk, df, md, fw, lp, 1);
                    old_players[1] = j;

                    int npbudget = team.bank;
                    npbudget += team.GetPlayerValue(w, old_players[0]);
                    npbudget += team.GetPlayerValue(w, old_players[1]);

                    solver.Init();
                    solver.SetMax(gk, df, md, fw, lp);
                    int sol = solver.Solve(0, 0, 0, 0, 0, 0, npbudget, 2);
                    if (solver.GetSolution(new_players.data(), 0, 0, 0, 0, 0, npbudget)) {
                        auto& p1in = season.player[new_players[0]];
                        auto& p2in = season.player[new_players[1]];
                        auto& p1out = season.player[team.team[old_players[0]]];
                        auto& p2out = season.player[team.team[old_players[1]]];;

                        int gain = p1in.effective_points + p2in.effective_points
                            - p1out.effective_points - p2out.effective_points;

                        if (gain > max_delta) {
                            max_effective = sol;
                            max_delta = gain;
                            max_old = old_players;
                            max_new = new_players;

                        }
                        else if (gain == max_delta) {
                            fout << p1out.name << '+' << p2out.name << " >< " 
                                << p1in.name << '+' << p2in.name 
                                << " = " << XP100(gain)<< endl;
                        }
                    }
                    else {
                        fout << "No solution\n";
                    }
                    p2->AddToSquadTotals(gk, df, md, fw, lp, -1);
                }
                p->AddToSquadTotals(gk, df, md, fw, lp, -1);
            }
            fout << "***********TRANSFERS**********\n";
            if (max_delta == SHRT_MIN)
            {
                fout << " NO SOLUTION-----------------\n";
                w--;
            }
            else {
                fout << endl;
                int i = 0;
                fout << "Max single transfer gain = " << endl;
                fout << season.player[team.team[max_single_old]].name << " >< "
                    << season.player[max_single_new].name
                    << " = " << XP100(max_single_delta) << endl;
                
                fout << "\nExpected double transfer gain = " << XP100(max_delta) << endl;
                fout << "Expected next weeks points = " << XP100(team.GetEffectivePoints())
                     << " + " << XP100(max_delta) << " = " << XP100(team.GetEffectivePoints() +max_delta) << endl;

                for (int i = 0; i < MAX_PLAYERS_TO_TRANSFER; i++) {
                    fout << "\nTransfer out " << season.player[team.team[max_old[i]]].name << endl;
                    fout << "Transfer in " << season.player[max_new[i]].name << endl;
                    team.UpdatePlayer(w, max_old[i], max_new[i]);
                }
            }

            if (bench) {
                Player& p1 = season.player[team.team[minp]];
                Player& p2 = season.player[team.bench[bench-1]];
                fout << "Substituting out " << p1.name << " in " << p2.name <<endl;
                team.Sub(minp, bench-1);
            }
        } while (bench--);
    }

    if (gwflag == -1) {
        fout << "\n\nTEAM Season End " << endl;
        points += team.print_sol(season.max_gw, season.max_gw + 1, false, false);
        fout << "Points so far = " << points << endl;
        fout << "*******************************\n";
    }

    return points;
}
