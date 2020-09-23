#pragma once
#include <map>

#include "Season.h"

class Team
{
public:
    int team[11] = { 0 };
    short cost[11] = { 0 };
    short bank;
    Season* season;
    Team(Season* s);

    bool verbose = false;
    int print_sol(int startgw, int endgw, bool use_effective = false, bool captain = true);
    void RefreshCost(int w, int budget);
    short GetTeamValue(int w);
    short GetPlayerValue(int w, int i);
    void UpdatePlayer(int w, int i, int pi);
    inline short GetAllAvailableFunds(int w) {
        return GetTeamValue(w) + bank;
    }
    bool Seed(std::string seedfile, int w, short budget);;
};

