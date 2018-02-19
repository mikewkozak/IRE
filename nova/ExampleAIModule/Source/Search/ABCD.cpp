#include "ABCD.h"

ABCD::ABCD(int maxDepth, EvaluationFunction* ef)
{
    _maxDepth = maxDepth;
    _ef = ef;
    _timeout = false;
    _downSamplingTimes = 0;
    TIME_LIMIT = LoadConfigInt("ABCD", "time_limit");
    MAX_SAMPLING = LoadConfigInt("ABCD", "downsampling");
}

playerActions_t ABCD::start(bool player, GameState gs)
{
    float alpha = std::numeric_limits<float>::min();
    float beta = std::numeric_limits<float>::max();
    bool maxplayer = player;
    bool minplayer = !player;

	_timer.start();
    gameNode_t bestMove = loop(gs, maxplayer, minplayer, alpha, beta, _maxDepth, maxplayer);
	double timeABCD = _timer.stopAndGetTime();
    //LOG("Start: " << _start << " end: " << clock() << " totalTime: " << searchTime << " NodesExpanded: " << _nodesExpanded);
    //LOG("ABCD: " << bestMove.evaluation << " in " << double(clock()-start)/CLOCKS_PER_SEC);
    
	// save stats
	size_t numberOfGroups = gs._army.friendly.size() + gs._army.enemy.size();
	stats.groupTime[numberOfGroups].add(timeABCD);
	stats.groupBranching[numberOfGroups].add(_branching.getMean());
	stats.groupFrequency[numberOfGroups]++;
    stats.groupTimeouts[numberOfGroups] += _timeout;
    stats.groupDownSamplings[numberOfGroups] += _downSamplingTimes;

	auto oldPrecision = fileLog.precision(6);
	auto oldFlags = fileLog.flags();
	fileLog.setf(std::ios::fixed, std::ios::floatfield);
	LOG("Groups: " << std::setw(2) << numberOfGroups 
		<< " branchingMin: " << static_cast<size_t>(_branching.getMin())
		<< " branchingMax: " << std::setw(7) << static_cast<size_t>(_branching.getMax())
		<< " branchingAvg: " << std::setw(10) << std::setprecision(2) << _branching.getMean()
		<< " seconds: " << std::setw(10) << std::setprecision(6) << timeABCD
		<< " tiemout: " << _timeout << " downSamplings: " << _downSamplingTimes);
	fileLog.precision(oldPrecision);
	fileLog.flags(oldFlags);

    return bestMove.action;
}

ABCD::gameNode_t ABCD::loop(const GameState &gs, bool maxplayer, bool minplayer, float alpha, float beta, int depthLeft, int nextPlayerInSimultaneousNode)
{
    // check timeout
	if (!_timeout) _timeout = _timer.getElapsedTime() > TIME_LIMIT;

	if (depthLeft <= 0 || gs.gameover() || _timeout) {
		// Evaluate the current state
		float evaluation = _ef->evaluate(gs);
		gameNode_t bestNode = { (playerActions_t)NULL, evaluation };
        return bestNode;
    }
    
	int nextPlayer = gs.getNextPlayerToMove(nextPlayerInSimultaneousNode);

	int sampling = 0;
	if (nextPlayer == (int)maxplayer) {
        ActionGenerator actions = ActionGenerator(&gs, maxplayer);
		if (actions._size > 1) _branching.add(actions._size);
// 		LOG("Depth: " << depthLeft << " branching: " << actions._size);
        gameNode_t best = {(playerActions_t)NULL, NULL};
        playerActions_t next;
        do {
            next = actions.getNextAction();
            if (!next.empty()) {
                sampling++;
                if (sampling > MAX_SAMPLING) {
                    _downSamplingTimes++;
                    return best;
                }
                GameState gs2 = gs.cloneIssue(next, maxplayer);
                gameNode_t tmp = loop(gs2, maxplayer, minplayer, alpha, beta, depthLeft-1, nextPlayerInSimultaneousNode);
                alpha = std::max(alpha,tmp.evaluation);
                if (best.action.empty() || tmp.evaluation>best.evaluation) {
                    best = tmp;
                    best.action = next;
                }
                if (beta<=alpha) {
                    return best;
                }
            }
        } while(!next.empty());
        return best;
	} else if (nextPlayer == (int)minplayer) {
        ActionGenerator actions = ActionGenerator(&gs, minplayer);
		if (actions._size > 1) _branching.add(actions._size);
        gameNode_t best = {(playerActions_t)NULL, NULL};
        playerActions_t next;
        do {
            next = actions.getNextAction();
            if (!next.empty()) {
                sampling++;
                if (sampling > MAX_SAMPLING) {
                    _downSamplingTimes++;
                    return best;
                }
                GameState gs2 = gs.cloneIssue(next, minplayer);
                gameNode_t tmp = loop(gs2, maxplayer, minplayer, alpha, beta, depthLeft-1, nextPlayerInSimultaneousNode);
                beta = std::min(beta,tmp.evaluation);
                if (best.action.empty() || tmp.evaluation<best.evaluation) {
                    best = tmp;
                    best.action = next;
                }
                if (beta<=alpha) {
                    return best;
                }
            }
        } while(!next.empty());
        return best;
    } else {
		DEBUG("ABCD error (none of the players can move)");
        GameState gs2 = gs;
        while(!gs2.gameover() && !gs2.canExecuteAnyAction(maxplayer) && !gs2.canExecuteAnyAction(minplayer)) 
			gs2.moveForward();
        //LOG(spaces << "Next loop after moving forward");
        return loop(gs2, maxplayer, minplayer, alpha, beta, depthLeft, nextPlayerInSimultaneousNode);
    }
}

std::string ABCD::gameNodeString(std::string spaces, gameNode_t node, GameState gs)
{
	return ActionGenerator::toString(node.action);
}