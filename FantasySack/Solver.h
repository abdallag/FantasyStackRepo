#pragma once
#include <set>

#include "Season.h"
#include "Team.h"

class Solver
{
public:
	static const int GK = 1;
	static const int DF = 4;
	static const int MD = 5;
	static const int FW = 3;
	static const int MAX_ATTACK = 7;
	static const int ALL = 11;
	static const int LP = 3;
	static const int BUDGET = 1000 - 165 - (DF - 3) * 5;
  
protected:
	short* cache[Season::COUNT + 1][GK + 1][DF + 1][MD + 1][FW + 1][LP + 1];
	short* sol[Season::COUNT + 1][GK + 1][DF + 1][MD + 1][FW + 1][LP + 1];

	int SQUAD_TARGET = ALL;

	int MAX_GK = GK;
	int MAX_DF = DF;
	int MAX_MD = MD;
	int MAX_FW = FW;
	int MAX_LP = LP;

    Season* season;

	int SolveInternal(int i, int gk, int df, int md, int fw, int lp, int budget);

public:
    bool alloc_pool = false;
    void Init();

    Solver(Season* s);

	inline int Solve(int i, int gk, int df, int md, int fw, int lp, int budget = Solver::BUDGET, int squad_target = ALL) {
		SQUAD_TARGET = squad_target;
		return SolveInternal(i, gk, df, md, fw, lp, budget);
	}

	bool GetSolution(int* team, int gk, int df, int md, int fw, int lp, int budget);

	inline void SetMax(int gk = GK, int df= DF, int md=MD, int fw=FW, int lp=LP) {
		MAX_GK = gk;
		MAX_DF = df;
		MAX_MD = md;
		MAX_FW = fw;
		MAX_LP = lp;
	}

	inline int Solve(Team& team, int w) {
		int budget = BUDGET;
		if (w != 1) {
			budget = team.GetAllAvailableFunds(w);
		}
		SetMax();
		int sol = Solve(0, 0, 0, 0, 0, 0, budget);
		if (!sol) {
			return sol;
		}
		GetSolution(team.team, 0, 0, 0, 0, 0, budget);
		team.RefreshCost(w, budget);

		return sol;
	}

};

