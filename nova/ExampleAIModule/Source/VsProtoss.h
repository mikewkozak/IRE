#pragma once

#include "State.h"
#include "InformationManager.h"
#include "StrategyManager.h"
#include "BuildManager.h"
#include "SquadManager.h"
#include "WorkerManager.h"

class StrategyManager;

//------------------------------------------------------------------------
//  Execute 2 Fact Vult/Mines
//------------------------------------------------------------------------
class TwoFactMines : public State<StrategyManager>
{
private:
	TwoFactMines() { _name = "2 Fact Vult/Mines"; }
	TwoFactMines(const TwoFactMines&);
	TwoFactMines& operator=(const TwoFactMines&);
public:
	static TwoFactMines* Instance();
	virtual void Enter(StrategyManager* strategyManager);
	virtual void Execute(StrategyManager* strategyManager);
	virtual void Exit(StrategyManager* strategyManager);
};

//------------------------------------------------------------------------
//  Execute Tank transition
//------------------------------------------------------------------------
class TankTransition : public State<StrategyManager>
{
private:
	TankTransition() { _name = "Tank transition"; }
	TankTransition(const TankTransition&);
	TankTransition& operator=(const TankTransition&);
public:
	static TankTransition* Instance();
	virtual void Enter(StrategyManager* strategyManager);
	virtual void Execute(StrategyManager* strategyManager);
	virtual void Exit(StrategyManager* strategyManager);
};

//------------------------------------------------------------------------
//  Execute Full tank
//------------------------------------------------------------------------
class FullTank : public State<StrategyManager>
{
private:
	FullTank() { _name = "Full tank"; }
	FullTank(const FullTank&);
	FullTank& operator=(const FullTank&);
public:
	static FullTank* Instance();
	virtual void Enter(StrategyManager* strategyManager);
	virtual void Execute(StrategyManager* strategyManager);
	virtual void Exit(StrategyManager* strategyManager);
};

//------------------------------------------------------------------------
//  Execute Sparks
//------------------------------------------------------------------------
class Sparks : public State<StrategyManager>
{
private:
	Sparks() { _name = "Sparks"; }
	Sparks(const Sparks&);
	Sparks& operator=(const Sparks&);
public:
	static Sparks* Instance();
	virtual void Enter(StrategyManager* strategyManager);
	virtual void Execute(StrategyManager* strategyManager);
	virtual void Exit(StrategyManager* strategyManager);
};

//------------------------------------------------------------------------
//  Execute 2 Fact Tanks Phase 1
//------------------------------------------------------------------------
class TwoFactTanks1 : public State<StrategyManager>
{
private:
	TwoFactTanks1() { _name = "2 Fact Tanks Phase 1"; }
	TwoFactTanks1(const TwoFactTanks1&);
	TwoFactTanks1& operator=(const TwoFactTanks1&);
public:
	static TwoFactTanks1* Instance();
	virtual void Enter(StrategyManager* strategyManager);
	virtual void Execute(StrategyManager* strategyManager);
	virtual void Exit(StrategyManager* strategyManager);
};

//------------------------------------------------------------------------
//  Execute 2 Fact Tanks Phase 2
//------------------------------------------------------------------------
class TwoFactTanks2 : public State<StrategyManager>
{
private:
	TwoFactTanks2() { _name = "2 Fact Tanks Phase 2"; }
	TwoFactTanks2(const TwoFactTanks2&);
	TwoFactTanks2& operator=(const TwoFactTanks2&);
public:
	static TwoFactTanks2* Instance();
	virtual void Enter(StrategyManager* strategyManager);
	virtual void Execute(StrategyManager* strategyManager);
	virtual void Exit(StrategyManager* strategyManager);
};