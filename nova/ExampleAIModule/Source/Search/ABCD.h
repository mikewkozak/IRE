#pragma once

#include "GameState.h"
#include "ActionGenerator.h"
#include "EvaluationFunction.h"

class ABCD
{
public:
	struct gameNode_t {
		playerActions_t action;
		float evaluation;
	};

	ABCD(int maxDepth, EvaluationFunction* ef);
	playerActions_t start(bool player, GameState gs);
	gameNode_t loop(const GameState &gs, bool maxplayer, bool minplayer, float alpha, float beta, int depthLeft, int nextPlayerInSimultaneousNode);
    std::string gameNodeString(std::string spaces, gameNode_t node, GameState gs);

private:
	Timer _timer;
	int _maxDepth;
	EvaluationFunction* _ef;
	Statistic _branching;
    bool _timeout;
    long _downSamplingTimes;
    int TIME_LIMIT; // in seconds
    int MAX_SAMPLING;
};
