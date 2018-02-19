#pragma once

#include "Evaluationfunction.h"

class EvaluationFunctionBasic : public EvaluationFunction
{
public:
	float evaluate(const GameState &gs);
};