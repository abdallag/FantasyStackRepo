#include "Solver.h"
#include "absl/flags/flag.h"

ABSL_FLAG(int, max_gk_budget, 47, "Maximum goal keeper budget.");

void Solver::Init() {
    std::wostream& fout = *season->pout;
    int total = (Season::COUNT + 1) * (GK + 1) * (DF + 1) * (MD + 1) * (FW + 1) * (LP + 1);
    if (!alloc_pool) {
        MAX_GK_BUDGET = absl::GetFlag(FLAGS_max_gk_budget);
        fout << "Maximum goalkeeper budget = " << MAX_GK_BUDGET << std::endl;
        fout << "Allocating Memory...\n";
        fout.flush();
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

Solver::Solver(Season* s) :
    season(s)
{
    Init();
}

int Solver::SolveInternal(int i, int gk, int df, int md, int fw, int lp, int budget)
{
    int attack = md + fw;
    int tot = attack + gk + df;
    if (tot == SQUAD_TARGET)
        return 0;

    if (i >= season->pcount) {
        return 0;
    }

    if (sol[i][gk][df][md][fw][lp][budget] != -1)
        return cache[i][gk][df][md][fw][lp][budget];
    sol[i][gk][df][md][fw][lp][budget] = 0;

    int res = SolveInternal(i + 1, gk, df, md, fw, lp, budget);
    cache[i][gk][df][md][fw][lp][budget] = res;
    Player& p = season->player[i];
    int price = p.price;
    if (p.skip ||
        price == 0 ||
        budget < price ||
        p.pos == 1 && price > MAX_GK_BUDGET ||
        tot == SQUAD_TARGET - 1 && MAX_GK > 0 && p.pos != 1 && gk == 0)
        return res;

    if (p.team == Season::LIVERPOOL)
        if (lp == LP)
            return res;

    bool isok = false;
    int gk2 = gk, df2 = df, md2 = md, fw2 = fw;
    switch (p.pos) {
    case 1:
        if (gk != MAX_GK) {
            gk2++;
            isok = true;
        }
        break;
    case 2:
        if (df != MAX_DF) {
            df2++;
            isok = true;
        }
        break;
    case 3:
        if (attack < MAX_ATTACK && md != MAX_MD) {
            md2++;
            isok = true;
        }
        break;
    case 4:
        if (attack < MAX_ATTACK && fw != MAX_FW) {
            fw2++;
            isok = true;
        }
        break;
    }
    if (isok) {
        int lp2 = lp;
        if (p.team == Season::LIVERPOOL)
            lp2++;
        int my_gain = p.effective_points;
        int next_players_gain = SolveInternal(i + 1, gk2, df2, md2, fw2, lp2, budget - price);
        int take = next_players_gain + my_gain;

        bool islast = false;
        if (tot == SQUAD_TARGET - 1)
            islast = true;

        //If taking this player will not allow adding any other players,
        //then don't take it unless we need no more players
        if (next_players_gain == 0 && !islast)
            return res;

        if (take > res) {
            res = take;
            sol[i][gk][df][md][fw][lp][budget] = 1;
            cache[i][gk][df][md][fw][lp][budget] = res;
        }
    }

    return res;
}

bool Solver::GetSolution(int* team, int gk, int df, int md, int fw, int lp, int budget) {
    std::wostream& fout = *season->pout;
    int p = 0;
    bool found = false;
    for (int i = 0; i < season->pcount; i++) {
        int attack = md + fw;
        int tot = attack + gk + df;
        if (tot == SQUAD_TARGET) {
            found = true;
            break;
        }
        if (sol[i][gk][df][md][fw][lp][budget] == 0)
            continue;

        team[p++] = i;
        switch (season->player[i].pos) {
        case 1:
            gk++;
            break;
        case 2:
            df++;
            break;
        case 3:
            md++;
            break;
        case 4:
            fw++;
            break;
        }
        budget -= season->player[i].price;
        if (season->player[i].team == Season::LIVERPOOL)
            lp++;
    }
    return found;
}
