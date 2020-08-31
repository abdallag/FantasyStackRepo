// FantasySack.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <codecvt>
#include <map>
using namespace std;

struct Player
{
	wstring name;
	int pos;
    int price;
    int initial_price;
	int points;
    int prev_points;
    int effective_points;
    int team;
    bool fix;
    bool skip;
    short gwprice[50];
    short gwpoints[50];
};

const int COUNT = 700;
Player* player = new Player[COUNT];

const int GK = 1;
const int DF = 5;
const int MD = 5;
const int FW = 3;
const int MAX_ATTACK = 7;
const int ALL = 11;
const int LP = 3;
const int BUDGET = 1000 - 175;
short*  cache[COUNT + 1][ GK + 1][ DF + 1][ MD + 1][ FW + 1][LP + 1];
short* sol[COUNT + 1][GK + 1][DF + 1][MD + 1][FW + 1][LP + 1];

int pcount = 0;
int max_gw = 0;
int solve(int i, int gk, int df, int md, int fw, int lp, int budget)
{
    if( i >= pcount)
        return 0;
    
    int attack = MD + FW - md - fw;
    int tot = attack + GK + DF - gk - df;
    if (tot == ALL)
        return 0;
    
    if (sol[i][ gk][ df][ md][ fw][lp][ budget] != -1)
        return cache[i][ gk][ df][ md][fw][ lp][ budget];
    sol[i][ gk][ df][ md][ fw][lp][ budget] = 0;
    
    int res = solve(i + 1, gk, df, md, fw, lp, budget);
    cache[i][ gk][ df][ md][ fw][lp][ budget] = res;
    int price = player[i].price;
    if (price == 0 || budget < price)
        return res;
    
    if (player[i].team == 14 && lp == LP)
        return res;

    bool isok = false;
    int gk2 = gk, df2 = df, md2 = md, fw2 = fw;
    switch (player[i].pos) {
    case 1:
        if (gk != 0) {
            gk2--;
            isok = true;
        }
        break;
    case 2:
        if (df != 0) {
            df2--;
            isok = true;
        }
        break;
    case 3:
        if (attack < MAX_ATTACK && md != 0) {
            md2--;
            isok = true;
        }
        break;
    case 4:
        if (attack < MAX_ATTACK && fw != 0) {
            fw2--;
            isok = true;
        }
        break;
    }
    if (isok) {
        int lp2 = lp;
        if (player[i].team == 14)
            lp2++;
        int gain = player[i].effective_points;
        int take = solve(i + 1, gk2, df2, md2, fw2, lp2, budget - price) + gain;
        if (take > res) {
            res = take;
            sol[i][ gk][ df][ md][ fw][lp][ budget] = 1;
            cache[i][ gk][ df][ md][ fw][lp][ budget] = res;
        }
    }

    return res;
}

bool alloc_pool = false;
void init() {
    int total = (COUNT +1)*( GK +1)*( DF +1)*( MD +1)*( FW +1)*( LP +1);
    if (!alloc_pool) {
        cout << "Allocating Memory...\n";
        short* cache_pool = new short[total * (BUDGET + 1)];
        short* sol_pool = new short[total * (BUDGET + 1)];
        
        for (int i = 0; i < total; i++) {
            ((short**)cache)[i] = cache_pool;
            cache_pool += BUDGET;
            cache_pool += 1;
            ((short**)sol)[i] = sol_pool;
            sol_pool += BUDGET;
            sol_pool += 1;
        }
    }
    for (int i = 0; i < total; i++) {
        for (int j = 0; j <= BUDGET; j++) {
            ((short**)cache)[i][j] = 0;
            ((short**)sol)[i][j] = -1;
        }
    }
    alloc_pool = true;
}

void get_sol(int* team, int budget) {
    int gk = GK;
    int df = DF;
    int md = MD;
    int fw = FW;
    int lp = 0;
    
    int p = 0;
    for (int i = 0; i <= pcount; i++) {
        int attack = MD + FW - md - fw;
        int tot = attack + GK + DF - gk - df;
        if (tot == 11)
            break;
        if (sol[i][gk][df][md][fw][lp][budget] == 0)
            continue;

        team[p++] = i;
        switch (player[i].pos) {
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
        }
        budget -= player[i].price;
        if (player[i].team == 14)
            lp++;
    }
 }


bool verbose = true;
int print_sol(int* team, int startgw, int endgw, int& new_budget) {

    int cost = 0;
    int max = 0;
    int maxi = 0;
    int points = 0;
    new_budget = 0;

    map<int, int> team_count;
    bool broken = false;
    for (int i = 0; i < 11; i++) {
        Player& p = player[team[i]];
        wcout << p.name << ' ';
        if (verbose)
            cout << ' ' << p.price << ' ' << p.points << " " << p.team << endl;
        team_count[p.team] = team_count[p.team] + 1;
        if (team_count[p.team] > 3)
            broken = true;

        cost += p.price;
        new_budget += p.gwprice[endgw];
        int player_points = 0;
        for (int j = startgw; j <= endgw; j++) {
            player_points += p.gwpoints[j];
        }
        points += player_points;
        if (player_points > max) {
            max = player_points;
            maxi = i;
        }
    }
    if(broken) 
        cout << "\n**********BROKEN TEAM COUNT*******************\n";

    points += max;
  
    cout << "\nTotal cost at gw " << startgw -1 << " = " << cost << endl;
    cout << "Total value at gw " << endgw << " = " << new_budget << endl;
    std::wcout << "Captain=" << player[team[maxi]].name << endl;
    cout << "Points for weeks [" << startgw << "," << endgw << "] = " << points << endl;

    return points;
}

bool skipped_week[50];
int max_reval_weeks = 6;
int bad_form_weeks = 3;
void reval_players(int gw) {
    for (int i = 0; i < pcount; i++) {
        Player& p = player[i];
        p.price = p.gwprice[gw];
        p.effective_points = 0;
        p.fix = false;
        int eval_weeks = gw;
        if (max_reval_weeks)
            eval_weeks = max_reval_weeks;
        int nbadweeks = 0;
        for (int j = gw; j > 0 && eval_weeks-- > 0; j--) {
            if (skipped_week[j]) {
                eval_weeks++;
                continue;
            }
            auto gwp = p.gwpoints[j];
            p.effective_points += gwp;
            if (gwp < 3) {
                nbadweeks++;
            }
        }
        if (bad_form_weeks && nbadweeks >= bad_form_weeks) {
            p.skip = true;
        }
        else {
            p.skip = false;
        }
    }
}

int Load(const string& season) {
    map<wstring, int> name_to_index;

    wifstream fin((season + ".txt").c_str());
    if (!fin.is_open())
        cout << "Error reading season file" << endl;

    fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
    int i;

    while (fin >> i)
    {
        cout << i;
        Player& p = player[i];

        fin >> p.name >> p.pos >> p.points >> p.price >> p.prev_points >> p.team;
        cout << " " << p.points << " " << p.prev_points << " " << p.price << " " << p.team << endl;
        name_to_index[p.name] = pcount++;
        for (int j = 0; j < 50; j++) {
            p.gwprice[j] = 0;
            p.gwpoints[j] = 0;
        }
    }
    cout << "loading GW data" << endl;
    wifstream finw((season + "_gw.txt").c_str());
    if (!finw.is_open())
        cout << "Error reading gameweeks file" << endl;
    finw.imbue(std::locale(finw.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
    int gw;
    max_gw = 0;
    while (finw >> gw)
    {
        wstring name;
        finw >> name;
        auto it = name_to_index.find(name);
        if (it == name_to_index.end()) {
            wcout << "Player not found " << gw << " " << name << "\n";
            int x, y;
            finw >> x >> y;
            continue;
        }
        Player& p = player[it->second];
        finw >> p.gwpoints[gw] >> p.gwprice[gw];
        if (gw > max_gw) {
            max_gw = gw;
        }
    }

    // Fill missing prices
    for (int i = 0; i < pcount; i++) {
        Player& p = player[i];
        int last_price = p.price;
        for (int j = 0; j < 50; j++) {
            if (p.gwprice[j] == 0) {
                p.gwprice[j] = last_price;
            }
            last_price = p.gwprice[j];
        }
    }
    return 0;
}


//int nwc = sizeof(WILDCARD_WEEK) / sizeof(int);
//for (int i = 0; i < nwc && WILDCARD_WEEK[i] <= max_gw; i++) {
//    init();
//    reval_players(WILDCARD_WEEK[i]);
//    solve(0, GK, DF, MD, FW, 0, budget);
//    cout << "\n\nWildcard " << i + 1 << " team:" << endl;
//    budget;
//    int nextwc = max_gw;
//    if (i + 1 < nwc) {
//        nextwc = WILDCARD_WEEK[i + 1];
//    }
//    points += print_sol(WILDCARD_WEEK[i], nextwc, budget);
//}
//
//cout << "Final Points=" << points << endl;

map<int, int* > all_scores;
int best[3] = { -1, -1, -1};
int WILDCARD_WEEK[] = { 30, 31 , 26 };
int fixed_wcweek = -1;

int highest_points = 0;
int max_wildcards = 2;
int last_wcweek = 0;
bool all_weeks_fixed = false;

void Wildcard(int i, int points, int start, int* team) {
    if (i == max_wildcards) {
        cout << "\n\nTEAM " << i << " at gw " << WILDCARD_WEEK[i - 1] << " :" << endl;
        int tmp;
        points += print_sol(team, WILDCARD_WEEK[i - 1] + 1, max_gw, tmp);

        cout << "Final Points=" << points << endl;
        if (points > highest_points) {
            highest_points = points;
            for (int j = 0; j < max_wildcards; j++) {
                best[j] = WILDCARD_WEEK[j];
            }
        }
        cout << "\nHighest Points = " << highest_points;
        int* record = new int[max_wildcards];
        for (int j = 0; j < max_wildcards; j++) {
            cout << " GW " << best[j];
            record[j] = WILDCARD_WEEK[j];
        }
        cout << endl;

        all_scores[points] = record;
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
        if (fixed_wcweek != -1) {
            if (i == 0) {
                end = fixed_wcweek - 1;
            }
            if (i == 1) {
                end = fixed_wcweek;
            }

            if (i == 2 && start <= fixed_wcweek) {
                start = fixed_wcweek;
                end = fixed_wcweek;
            }
        }
    }

    for (int w = start; w <= end; w++) {
        if (skipped_week[w]) {
            continue;
        }

        WILDCARD_WEEK[i] = w;
        int budget;
        int prevw = 1;
        if (i > 0)
            prevw = WILDCARD_WEEK[i - 1];
        cout << "\n\nTEAM " << i << " at gw " << prevw << " :" << endl;
        int new_points = points + print_sol(team, prevw, w, budget);
        init();
        reval_players(w);
        solve(0, GK, DF, MD, FW, 0, budget);

        int team[11];
        get_sol(team, budget);
        Wildcard(i + 1, new_points, w + 1, team);
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        cout << "No season specified.\n";
        return 7;
    }
    string season = argv[1];

    int err = Load(season);
    if (err != 0) {
        return err;
    }

    for (int j = 0; j < 50; j++) {
        skipped_week[j] = false;
    }

    if (season == "2019-20") {
        max_wildcards = 3;
        fixed_wcweek = 39;
        for (int i = 30; i < 39; i++) {
            skipped_week[i] = true;
        }
    }
    

    cout << "Game weeks =" << max_gw << endl;
    last_wcweek = max_gw - 3;

    int starting_team[11];
    init();
    for (int i = 0; i < pcount; i++) {
        player[i].effective_points = player[i].prev_points;
    }
    solve(0, GK, DF, MD, FW, 0, BUDGET);
    get_sol(starting_team, BUDGET);
    
    Wildcard(0, 0, 3, starting_team);

    // Optimum TEAM
    init();
    reval_players(1);
    for (int i = 0; i < pcount; i++) {
        player[i].effective_points = player[i].points;
    }

    solve(0, GK, DF, MD, FW, 0, BUDGET);
    cout << "\n\nOptimal team:" << endl;
    int optimal_team[11];
    get_sol(optimal_team, BUDGET);
    int tmp;
    int optimal_points = print_sol(optimal_team, 1, max_gw, tmp);

    cout << endl << "Delta =" << optimal_points - highest_points << " points" << endl;


    for (auto it = all_scores.begin(); it != all_scores.end(); it++) {
        for (int i = 0; i < max_wildcards; i++)
            cout << (it->second)[i] << ' ';
        cout << it->first << endl;
    }
    return 0;
}
