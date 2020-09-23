#include <vector>

#include <absl/flags/flag.h>

#include "ExpectedPointsStrategy.h"

using namespace std;

ABSL_FLAG(bool, wceval, 0, "Evalute playing a wildcard at each week");

int DoXptsSeasonLoop(Season& season, Team& team, Solver& solver) {
    wostream& fout = *season.pout;
    int points = 0;
    int prevw = 1;
    int nwc = 1;

    bool secondwc = false;
    for (int w = 3; w <= season.max_gw; w += 2) {
        if (w > 19 && !secondwc) {
            secondwc = true;
            nwc++;
        }
        
        fout << "\n\nTEAM at gw " << w << " :" << endl;
        points += team.print_sol(prevw, w, false, false);
        fout << "Points so far = " << points << endl;
        fout << "*******************************\n";

        if (absl::GetFlag(FLAGS_wceval)) {
            Team wct(team);
            solver.Init();
            season.ResetPlayers(w);
            season.ApplyXpts(w, 6);
            solver.Solve(wct, w);
            wct.print_sol(prevw, w);

            fout << "Wild card effective points = " << wct.GetEffectivePoints(prevw, w) << endl;
        }
        solver.Init();
        season.ResetPlayers(w);
        season.ApplyXpts(w, 6);

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

        
        for (int i = 0; i < 11; i++) {
            Player* p = season.player + team.team[i];
            p->AddToSquadTotals(gk, df, md, fw, lp, 1);
            old_players[0] = i;
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
                        fout << p1out.name << '+' << p2out.name << " >< " << p1in.name << '+' << p2in.name << " = " << gain << endl;
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
            for (int i = 0; i < MAX_PLAYERS_TO_TRANSFER; i++) {
                fout << "\nTransfer out " << season.player[team.team[max_old[i]]].name << endl;
                fout << "Transfer in " << season.player[max_new[i]].name << endl;
                team.UpdatePlayer(w, max_old[i], max_new[i]);
            }
            fout << "Expected gain = " << max_delta << endl;
            fout << "Effective points = " << team.GetEffectivePoints(prevw, w) <<endl;
        }
    }

    fout << "\n\nTEAM Season End " << endl;
    points += team.print_sol(season.max_gw, season.max_gw +1, false, false);
    fout << "Points so far = " << points << endl;
    fout << "*******************************\n";

    return points;
}
