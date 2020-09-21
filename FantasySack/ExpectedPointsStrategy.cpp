#include "ExpectedPointsStrategy.h"

int DoXptsSeasonLoop(Season& season, Team& team, Solver& solver) {
    std::wostream& fout = *season.pout;
    int points = 0;
    int prevw = 1;
    int nwc = 1;

    bool secondwc = false;
    for (int w = 2; w <= season.max_gw; w += 2) {
        if (w > 19 && !secondwc) {
            secondwc = true;
            nwc++;
        }
        if (season.skipped_week[w]) {
            w--;
            continue;
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
            blanker.emplace(p->effective_points, i);
            p->skip = true;
            if (blanker.size() > 2) {
                auto rit = blanker.rbegin();
                p = season.player + team.team[rit->second];
                rit++;
                blanker.erase(rit.base());
            }
            else {
                continue;
            }

            // This is a player we are not skipping
            // Let's not select again and make sure
            // it is execluded from the lines
            // and not account its budget

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

        // Adjust budget after selling transers
        const int MAX_PLAYERS_TO_TRANSFER = 2;
        int old_players[MAX_PLAYERS_TO_TRANSFER];
        int new_players[MAX_PLAYERS_TO_TRANSFER];

        auto it = blanker.begin();
        int npbudget = team.bank;
        for (int i = 0; i < MAX_PLAYERS_TO_TRANSFER; i++) {
            Player* p = season.player + team.team[it->second];
            old_players[i] = it->second;
            it++;
            npbudget += team.GetPlayerValue(w, old_players[i]);
        }

       
        fout << "***********TRANSFERS**********\n";
        fout << "Within " << npbudget << " budget\n";
        solver.Solve(0, gk, df, md, fw, lp, npbudget, 2);
        if (!solver.GetSolution(new_players, gk, df, md, fw, lp, npbudget)) {
            fout << " NO SOLUTION-----------------\n";
            w--;
        }
        else {
            fout << std::endl;
            int i = 0;
            for (int i = 0; i < MAX_PLAYERS_TO_TRANSFER; i++) {
                fout << "\nTransfer out " << season.player[team.team[old_players[i]]].name << std::endl;
                fout << "Transfer in " << season.player[new_players[i]].name << std::endl;
                team.UpdatePlayer(w, old_players[i], new_players[i]);
            }
        }
    }

    fout << "\n\nTEAM Season End " << std::endl;
    points += team.print_sol(38, 39, false, false);
    fout << "Points so far = " << points << std::endl;
    fout << "*******************************\n";

    return points;
}
