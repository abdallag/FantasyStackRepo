#include "WildCardStrategy.h"

std::multimap<int, int* > all_scores;
int best[3] = { -1, -1, -1 };
int WILDCARD_WEEK[] = { 30, 31 , 26 };

int highest_points = 0;
int last_wcweek = 0;
bool all_weeks_fixed = false;

void Wildcard(Season& season, Team& team, Solver& solver, int i, int points, int start) {
    last_wcweek = season.max_gw - 3;
    std::wostream& fout = *season.pout;
    if (i == season.max_wildcards) {
        fout << "\n\nTEAM " << i << " at gw " << WILDCARD_WEEK[i - 1] << " :" << std::endl;
        points += team.print_sol(WILDCARD_WEEK[i - 1], season.max_gw +1);

        fout << "Final Points=" << points << std::endl;
        if (points > highest_points) {
            highest_points = points;
            for (int j = 0; j < season.max_wildcards; j++) {
                best[j] = WILDCARD_WEEK[j];
            }
        }
        fout << "\nHighest Points = " << highest_points;
        int* record = new int[season.max_wildcards];
        for (int j = 0; j < season.max_wildcards; j++) {
            fout << " GW " << best[j];
            record[j] = WILDCARD_WEEK[j];
        }
        fout << std::endl;

        all_scores.emplace(points, record);
        if (all_scores.size() > 50) {
            delete all_scores.begin()->second;
            all_scores.erase(all_scores.begin());
        }
        return;
    }
    if (start > last_wcweek)
        return;
    int end = last_wcweek;

    if (all_weeks_fixed) {
        start = WILDCARD_WEEK[i];
        end = start;
    }
    else {
        if (season.fixed_wcweek != -1) {
            if (i == 0) {
                end = season.fixed_wcweek - 1;
            }
            if (i == 1) {
                end = season.fixed_wcweek;
            }

            if (i == 2 && start <= season.fixed_wcweek) {
                start = season.fixed_wcweek;
                end = season.fixed_wcweek;
            }
        }
    }

    for (int w = start; w <= end; w++) {
        if (season.skipped_week[w]) {
            continue;
        }

        WILDCARD_WEEK[i] = w;
        int prevw = 1;
        if (i > 0)
            prevw = WILDCARD_WEEK[i - 1];
        fout << "\n\nTEAM " << i << " at gw " << prevw << " :" << std::endl;
        int new_points = points + team.print_sol(prevw, w);
        solver.Init();
        season.ResetPlayers(w);
        if (solver.Solve(team, w) == 0) {
            fout << "-----------------No Solution Found--------------";
            return;
        }

        Wildcard(season, team, solver, i + 1, new_points, w + 1);
    }

    for (auto it = all_scores.begin(); it != all_scores.end(); it++) {
        for (int i = 0; i < season.max_wildcards; i++)
            fout << (it->second)[i] << ' ';
        fout << it->first << std::endl;
    }

}
