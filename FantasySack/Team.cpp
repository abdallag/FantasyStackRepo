#include "Team.h"

Team::Team(Season* s) : season(s) {

}

int Team::print_sol(int startgw, int endgw, bool use_effective, bool captain) {
    std::wostream& fout = *season->pout;

    int max = 0;
    int maxi = 0;
    int points = 0;
    std::map<int, int> team_count;
    bool broken = false;
    for (int i = 0; i < 11; i++) {
        Player& p = season->player[team[i]];
        fout << p.name << ' ';
        if (verbose) {
            if (startgw != 1) {
                fout << p.gwprice[startgw - 1] << '/';
            }
            fout << p.gwprice[endgw] << '/' << p.price << ' ' << p.points << " " << p.team << std::endl;
        }
        team_count[p.team] = team_count[p.team] + 1;
        if (team_count[p.team] > 3)
            broken = true;

        int player_points = 0;
        if (use_effective) {
            player_points = p.effective_points;
        }
        else {
            for (int j = startgw; j < endgw; j++) {
                player_points += p.gwpoints[j];
            }
        }
        points += player_points;
        if (player_points > max) {
            max = player_points;
            maxi = i;
        }
    }
    if (broken)
        fout << "\n**********BROKEN TEAM COUNT*******************\n";

    if (captain)
        points += max;

    fout << "\nTeam value gw " << startgw << "->" << endgw << " = " << GetTeamValue(startgw) << "->" << GetTeamValue(endgw) << std::endl;
    if (captain) {
        fout << "Captain=" << season->player[team[maxi]].name;
        if (points != 0) {
            fout << " points percentage = " << (100.0 * max) / points << " %\n";
        }
    }
    fout << "Points for weeks [" << startgw << "," << endgw - 1 << "] = " << points << std::endl;

    return points;
}


void Team::RefreshCost(int w, int budget)
{
   int total = 0;
   for (int i = 0; i < 11; i++) {
        cost[i] = season->player[team[i]].gwprice[w];
        total += cost[i];
   }
   bank = budget - total;
}

short Team::GetTeamValue(int w) {
    int total = 0;
    for (int i = 0; i < 11; i++) {
        total += GetPlayerValue(w, i);
    }
    return total;
}

short Team::GetPlayerValue(int w, int i) {
    int nowcost = season->player[team[i]].gwprice[w];
    if (nowcost <= cost[i])
        return nowcost;
    return nowcost + (nowcost - cost[i]) / 2;
}

void Team::UpdatePlayer(int w, int i, int pi) {
    team[i] = pi;
    int oldValue = GetPlayerValue(w, i);
    int newValue = season->player[pi].gwprice[w];
    cost[i] = newValue;
    bank += oldValue - newValue;
}