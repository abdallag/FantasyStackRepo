#include <vector>

#include "ExpectedPointsStrategy.h"

int DoXptsSeasonLoop(Season& season, Team& team, Solver& solver) {
    std::wostream& fout = *season.pout;
    int points = 0;
    int prevw = 1;
    int nwc = 1;

    bool secondwc = false;
    for (int w = 3; w <= season.max_gw; w += 2) {
        if (w > 19 && !secondwc) {
            secondwc = true;
            nwc++;
        }
        
        fout << "\n\nTEAM at gw " << w << " :" << std::endl;
        points += team.print_sol(prevw, w, false, false);
        fout << "Points so far = " << points << std::endl;
        fout << "*******************************\n";
        solver.Init();
        season.ResetPlayers(w);
        season.ApplyXpts(w, 6);

        prevw = w;
        std::multimap<int, int> blanker;
        int gk = Solver::GK;
        int df = Solver::DF;
        int md = Solver::MD;
        int fw = Solver::FW;
        int lp = 0;

        fout << std::endl;
        for (int i = 0; i < 11; i++) {
            Player* p = season.player + team.team[i];
            p->skip = true;
            p->AddToSquadTotals(gk, df, md, fw, lp, 1);
        }

        // Adjust budget after selling transers
        const int MAX_PLAYERS_TO_TRANSFER = 2;
        std::vector<int> old_players(MAX_PLAYERS_TO_TRANSFER);
        std::vector<int> new_players(MAX_PLAYERS_TO_TRANSFER);

        short max_delta = SHRT_MIN;
        std::vector<int> max_old(2);
        std::vector<int> max_new(2);
        
        for (int i = 0; i < 11; i++) {
            Player* p = season.player + team.team[i];
            p->AddToSquadTotals(gk, df, md, fw, lp, -1);
            old_players[0] = i;
            for (int j = 0; j < 11; j++) {
                if (i == j)
                    continue;
            
                Player* p2 = season.player + team.team[j];
                p2->AddToSquadTotals(gk, df, md, fw, lp, -1);
                old_players[1] = j;
                
                int npbudget = team.bank;
                npbudget += team.GetPlayerValue(w, old_players[0]);
                npbudget += team.GetPlayerValue(w, old_players[1]);

                solver.Solve(0, gk, df, md, fw, lp, npbudget, 2);
                if (solver.GetSolution(new_players.data(), gk, df, md, fw, lp, npbudget)) {
                    short gain = season.player[new_players[0]].effective_points
                        + season.player[new_players[1]].effective_points
                        - season.player[team.team[old_players[0]]].effective_points
                        - season.player[team.team[old_players[1]]].effective_points;

                    if (gain > max_delta) {
                        max_delta = gain;
                        max_old = old_players;
                        max_new = new_players;
                    }
                }
                p2->AddToSquadTotals(gk, df, md, fw, lp, 1);
            }
            p->AddToSquadTotals(gk, df, md, fw, lp, 1);
        }
        fout << "***********TRANSFERS**********\n";
        if (max_delta == SHRT_MIN) 
        {
            fout << " NO SOLUTION-----------------\n";
            w--;
        }
        else {
            fout << std::endl;
            int i = 0;
            for (int i = 0; i < MAX_PLAYERS_TO_TRANSFER; i++) {
                fout << "\nTransfer out " << season.player[team.team[max_old[i]]].name << std::endl;
                fout << "Transfer in " << season.player[max_new[i]].name << std::endl;
                team.UpdatePlayer(w, max_old[i], max_new[i]);
            }
            fout << "Expected gain = " << max_delta << std::endl;
        }
    }

    fout << "\n\nTEAM Season End " << std::endl;
    points += team.print_sol(season.max_gw, season.max_gw +1, false, false);
    fout << "Points so far = " << points << std::endl;
    fout << "*******************************\n";

    return points;
}
