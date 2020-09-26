#pragma once
#include <map>

#include "Season.h"

class Team
{
public:
    int team[11] = { 0 };
    int bench[3] = { 0 };
    int bcount;
    short cost[11] = { 0 };
    short bcost[3] = { 0 };
    short bank;
    Season* season;
    Team(Season* s);

    bool verbose = false;
    int print_sol(int startgw, int endgw, bool use_effective = false, bool captain = true, int factor = 1);
    void RefreshCost(int w, int budget);
    short GetTeamValue(int w);
    short GetPlayerValue(int w, int i);
    void UpdatePlayer(int w, int i, int pi);
    inline short GetAllAvailableFunds(int w) {
        return GetTeamValue(w) + bank;
    }
    short GetEffectivePoints();
    bool Seed(std::string seedfile, int w, short budget);
    inline void Sub(int teami, int benchi) {
        std::swap(team[teami], bench[benchi]);
        std::swap(cost[teami], bcost[benchi]);
    }
    int MinEffectivePlayer();
};

