#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <locale>
#include <codecvt>
#include <set>


#include "Utils.h"

struct Player
{
    std::wstring name;
    int pos;
    int price;
    int initial_price;
    int points;
    int prev_points;
    int effective_points;
    int team;
    bool skip;
    short gwprice[50];
    short gwpoints[50];
    short gwxpts[50];
    bool gwgames[50];

    bool AddToSquadTotals(int& gk, int& df, int& md, int& fw, int& lp, int delta);
};


class Season
{
public:
    static const int COUNT = 700;
    static const int LIVERPOOL = 12;

    Player* player = new Player[COUNT];
    std::set<int> excluded;
    int pcount = 0;
    int max_gw = 0;

    int max_wildcards = 2;
    int fixed_wcweek = -1;

    std::wostream* pout = &std::wcout;
    std::wofstream filestream;
    const std::string seasonName;
    
    Season(const std::string season_name);

    int FindPlayer(const std::wstring& player_name, int minMatchTokens = 2);

    int max_reval_weeks = 0;
    int bad_form_weeks = 3;
    void ResetPlayers(int gw);

    int Load(bool xpts);

    void ApplyXpts(int gw, int weeks);

};

