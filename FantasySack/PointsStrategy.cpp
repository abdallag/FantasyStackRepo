#include "PointsStrategy.h"

int DoSeasonLoop(Season& season, Team& team, Solver& solver,int wcafter) {
    std::wostream& fout = *season.pout;
    int points = 0;
    int prevw = 0;
    int nwc = 1;

    bool secondwc = false;
    for (int w = 2; w <= season.max_gw; w += 2) {
        if (w > 19 && !secondwc) {
            secondwc = true;
            nwc++;
        }
        if (w >= 3) {
            season.bad_form_weeks = 3;
        }

        fout << "\n\nTEAM at gw " << w << " :" << std::endl;
        points += team.print_sol(prevw, w, false, false);
        fout << "Points so far = " << points << std::endl;
        fout << "*******************************\n";
        solver.Init();
        season.ResetPlayers(w);

        int skipped_players = 0;
        for (int i = 0; i < 11; i++) {
            Player* p = season.player + team.team[i];
            if (p->skip) {
                skipped_players++;
            }
        }
        fout << "Skipped Players = " << skipped_players << std::endl;

        if (wcafter != 0 && skipped_players > wcafter && w >= 3 && nwc > 0
            //|| wcafter != 0 && w >=8 && nwc > 0 
            //|| wcafter != 0 && w >= 28 && nwc > 0 
            || w == 39) {
            fout << "Wildcard at w " << w << "\n";
            solver.Solve(team, w);
            prevw = w;
            w--;
            nwc--;
            continue;
        }
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
            if (p->skip) {
                blanker.emplace(p->effective_points, i);
                if (blanker.size() > 2) {
                    p = season.player + team.team[blanker.begin()->second];
                    blanker.erase(blanker.begin());
                }
                else {
                    continue;
                }
            }

            // This is a player we are not skipping
            // Let's not select again and make sure
            // it is execluded from the lines
            // and not account its budget
            p->skip = true;

            switch (p->pos) {
            case 1:
                gk--;
                break;
            case 2:
                df--;
                break;
            case 3:
                md--;
                break;
            case 4:
                fw--;
                break;
            default:
                fout << "Unknnown Position: Should never happen\n";
                break;
            }
            if (p->team == Season::LIVERPOOL)
                lp++;
        }

        fout << "***********TRANSFERS**********\n";
        if (blanker.size() < 2) {
            fout << "NO TRANSFERS in gw " << w << std::endl;
            w--;
        }
        else {

            int newplayer[2];
            int npbudget = team.bank;
            auto it = blanker.begin();
            npbudget += team.GetPlayerValue(w, (it++)->second);
            npbudget += team.GetPlayerValue(w, it->second);

            solver.Solve(0, gk, df, md, fw, lp, npbudget, 2);
            if (!solver.GetSolution(newplayer, gk, df, md, fw, lp, npbudget)) {
                fout << " NO SOLUTION-----------------\n";
                w--;
            }
            else {
                fout << std::endl;
                int i = 0;
                for (auto it = blanker.begin(); it != blanker.end(); it++) {
                    fout << "\nTransfer out " << season.player[team.team[it->second]].name << std::endl;
                    fout << "Transfer in " << season.player[newplayer[i]].name << std::endl;
                    team.UpdatePlayer(w, it->second, newplayer[i]);
                    i++;
                }
            }
        }
    }

    fout << "\n\nTEAM Season End " << std::endl;
    points += team.print_sol(38, 39, false, false);
    fout << "Points so far = " << points << std::endl;
    fout << "*******************************\n";

    return points;
}
