#pragma once
#include <set>

#include "Season.h"
#include "Team.h"

class Solver
{
public:
	static const int GK = 1;
	static const int DF = 5;
	static const int MD = 5;
	static const int FW = 3;
	static const int MAX_ATTACK = 7;
	static const int ALL = 11;
	static const int LP = 3;
	static const int BUDGET = 1000 - 165 - (DF - 3) * 5;
  
protected:
	// Dynamic Programming Memoization Cache:
	// Dimension Description:
	// 0# Players
	// 1# Goal keepers  constraint
	// 2# Defender constraint
	// 3# Midfielders constraint
	// 4# Forwards constraint
	// 5# Number of players in team Season::LIVERPOOL. 
	//    This covers the constraint that there should be no more than 3 players in LIVERPOOL team.
	//    This is a relaxation of the actual FPL contstraint that there should be no more than 3 players in any team.
	//    To cover the original constraint, we would need alot more more memory and alot more processing.
	//    However, in practice, usually no more than 1 teams yield more than 3 players. If the algorithm produces
	//	  duplicates in a team differen from LIVERPOOL (e.g Man City or Westham in a doulbe game week :D) , I just
	//    rerun it after changing Season::LIVERPOOL. If more than 1 team  produce duplicates, I use execlustion
	//    lists to work around it. I propably should do it programatically but I am too lazy at the momont.
	short* cache[Season::COUNT + 1][GK + 1][DF + 1][MD + 1][FW + 1][LP + 1];
	short* sol[Season::COUNT + 1][GK + 1][DF + 1][MD + 1][FW + 1][LP + 1];

	int SQUAD_TARGET = ALL;

	int MAX_GK = GK;
	int MAX_DF = DF;
	int MAX_MD = MD;
	int MAX_FW = FW;
	int MAX_LP = LP;

	int MAX_GK_BUDGET = 1000;

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
		for (int i = 0; i < 11; i++) {
			season->player[team.team[i]].price = team.cost[i];
		}
		int sol = Solve(0, 0, 0, 0, 0, 0, budget);
		if (!sol) {
			return sol;
		}
		GetSolution(team.team, 0, 0, 0, 0, 0, budget);
		team.RefreshCost(w, budget);

		return sol;
	}

};

