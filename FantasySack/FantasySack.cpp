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

#include "WildCardStrategy.h"
#include "PointsStrategy.h"
#include "ExpectedPointsStrategy.h"

int main(int argc, char** argv)
{
    _setmode(_fileno(stdout), _O_WTEXT); // <=== Windows madness

    if (argc < 2) {
        wcout << "Please specify season .\n";
        return 7;
    }
    string seasonName = argv[1];
    string mode;
    int wcafter = 0;
    if (argc > 2) {
        mode = argv[2];
        
        if (argc > 3)
            sscanf_s(argv[3], "%d", &wcafter);
    }

    Season season(seasonName);
    
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
    solver->Solve(team, 1);
    int highest_points = 0;
   
    if (mode == "step") {
        highest_points = DoSeasonLoop(season, team, *solver, wcafter);
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

    fout << seasonName.c_str() << " DEF = " << Solver::DF << " WC > " << wcafter << endl;
    fout << endl << "Delta =" << optimal_points - highest_points << " points" << endl;

    delete solver;
    return 0;
}
