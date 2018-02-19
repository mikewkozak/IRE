#pragma once

#include "State.h"
#include "InformationManager.h"
#include "StrategyManager.h"
#include "BuildManager.h"
#include "SquadManager.h"
#include "WorkerManager.h"

class StrategyManager;

//------------------------------------------------------------------------
//  Execute 1 Factory ...
//------------------------------------------------------------------------
class OneFactory : public State<StrategyManager>
{
private:
	OneFactory() { _name = "1 Factory ..."; }
	OneFactory(const OneFactory&);
	OneFactory& operator=(const OneFactory&);
public:
	static OneFactory* Instance();
	virtual void Enter(StrategyManager* strategyManager);
	virtual void Execute(StrategyManager* strategyManager);
	virtual void Exit(StrategyManager* strategyManager);
};

//------------------------------------------------------------------------
//  Execute 2 Factories Rush Defense
//------------------------------------------------------------------------
class RushDefense : public State<StrategyManager>
{
private:
	RushDefense(){ _name = "2 Factories Rush Defense"; }
	RushDefense(const RushDefense&);
	RushDefense& operator=(const RushDefense&);
public:
	static RushDefense* Instance();
	virtual void Enter(StrategyManager* strategyManager);
	virtual void Execute(StrategyManager* strategyManager);
	virtual void Exit(StrategyManager* strategyManager);
};

//------------------------------------------------------------------------
//  Execute 1 Factory Fast Expand Opening
//------------------------------------------------------------------------
class OneFactoryFastExpand : public State<StrategyManager>
{
private:
	OneFactoryFastExpand(){ _name = "1 Factory Fast Expand Opening"; }
	OneFactoryFastExpand(const OneFactoryFastExpand&);
	OneFactoryFastExpand& operator=(const OneFactoryFastExpand&);
public:
	static OneFactoryFastExpand* Instance();
	virtual void Enter(StrategyManager* strategyManager);
	virtual void Execute(StrategyManager* strategyManager);
	virtual void Exit(StrategyManager* strategyManager);
};

//------------------------------------------------------------------------
//  Execute 3 Factories
//------------------------------------------------------------------------
class ThreeFactories : public State<StrategyManager>
{
private:
	ThreeFactories(){ _name = "3 Factories"; }
	ThreeFactories(const ThreeFactories&);
	ThreeFactories& operator=(const ThreeFactories&);
public:
	static ThreeFactories* Instance();
	virtual void Enter(StrategyManager* strategyManager);
	virtual void Execute(StrategyManager* strategyManager);
	virtual void Exit(StrategyManager* strategyManager);
};