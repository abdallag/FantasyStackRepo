// FantasySack.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <cstdio>
#include <io.h>
#include <fcntl.h>
using namespace std;

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>

#include "WildCardStrategy.h"
#include "PointsStrategy.h"
#include "ExpectedPointsStrategy.h"

ABSL_FLAG(std::string, season, "", "Season");
ABSL_FLAG(std::string, mode, "", "mode [step][xpts][wc]");
ABSL_FLAG(std::string, seed, "", "Name of a file containing seed squad. Player names line by line.");

int main(int argc, char** argv)
{
    _setmode(_fileno(stdout), _O_WTEXT); // <=== Windows madness

    if (argc < 2) {
        wcout << "Please specify season .\n";
        return 7;
    }
    
    auto extra = absl::ParseCommandLine(argc, argv);
    if (extra.size() > 1) {
        wcout << "Too many argumetns " << extra.size() << endl;
        return 8;
    }

    Season season(absl::GetFlag(FLAGS_season));
    auto mode = absl::GetFlag(FLAGS_mode);
    int err = season.Load(mode == "xpts");
    if (err != 0) {
        return err;
    }

    std::wostream& fout = *season.pout;
    
    fout << "Game weeks =" << season.max_gw << endl;
    
    Team team(&season);
    if (mode == "xpts") {
        season.ApplyXpts(1, 6);

    }
    else {
        for (int i = 0; i < season.pcount; i++) {
            season.player[i].effective_points = season.player[i].prev_points;
        }
    }

    fout << "Solving for initial team..." << endl;
    Solver* solver = new Solver(&season);
    auto seedFile = absl::GetFlag(FLAGS_seed);
    if (seedFile != "") {
        if (!team.Seed(seedFile, 1, Solver::BUDGET)) {
            return 9;
        }
    }
    else {
        solver->Solve(team, 1);
    }
    int highest_points = 0;
   
    if (mode == "step") {
        highest_points = DoSeasonLoop(season, team, *solver);
    }
    else if (mode == "xpts") {
        highest_points = DoXptsSeasonLoop(season, team, *solver);
    }
    else if (mode == "wc") {
        Wildcard(season, team,*solver, 0, 0, 3);
    }
    else {
        fout << "\n\nInitial team based on last year points:" << endl;
        team.print_sol(1, season.max_gw+1, true);
        return 0;
    }

    // Optimum TEAM
    solver->Init();
    season.ResetPlayers(1);
    for (int i = 0; i < season.pcount; i++) {
        season.player[i].effective_points = season.player[i].points;
    }

    solver->Solve(team, 1);
    fout << "\n\nOptimal team:" << endl;
    int optimal_points = team.print_sol(1, season.max_gw + 1, false, false);

    fout << endl << "Delta =" << optimal_points - highest_points << " points" << endl;

    delete solver;
    return 0;
}
