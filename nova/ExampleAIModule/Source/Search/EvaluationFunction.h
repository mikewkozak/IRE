#pragma once

#include "GameState.h"
#include "InformationManager.h"

class EvaluationFunction
{
public:
	// Try to make this function "zero sum", i.e. everything that can add points to MAX player, can remove points
	// return the reward of a given state for a MAX player
	virtual float evaluate(const GameState &gs) = 0;
};