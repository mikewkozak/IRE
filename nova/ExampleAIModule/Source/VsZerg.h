#pragma once

#include "State.h"
#include "InformationManager.h"
#include "StrategyManager.h"
#include "BuildManager.h"
#include "SquadManager.h"
#include "WorkerManager.h"

class StrategyManager;

//------------------------------------------------------------------------
//  Execute 2 Port Wraith
//------------------------------------------------------------------------
class TwoPortWraith : public State<StrategyManager>
{
private:
	TwoPortWraith() { _name = "2 Port Wraith"; }
	TwoPortWraith(const TwoPortWraith&);
	TwoPortWraith& operator=(const TwoPortWraith&);
public:
	static TwoPortWraith* Instance();
	virtual void Enter(StrategyManager* strategyManager);
	virtual void Execute(StrategyManager* strategyManager);
	virtual void Exit(StrategyManager* strategyManager);
};

//------------------------------------------------------------------------
//  Execute 1 Barracks Fast Expansion
//------------------------------------------------------------------------
class OneRaxFE : public State<StrategyManager>
{
private:
	OneRaxFE() { _name = "1 Barracks Fast Expansion"; }
	OneRaxFE(const OneRaxFE&);
	OneRaxFE& operator=(const OneRaxFE&);
public:
	static OneRaxFE* Instance();
	virtual void Enter(StrategyManager* strategyManager);
	virtual void Execute(StrategyManager* strategyManager);
	virtual void Exit(StrategyManager* strategyManager);
};