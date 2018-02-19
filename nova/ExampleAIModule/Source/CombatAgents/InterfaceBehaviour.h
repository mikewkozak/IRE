#pragma once

class InterfaceBehaviour
{
public:
	// Abstract class because of this pure virtual function
	virtual void byDefault() = 0;
	virtual void onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies) = 0;
	virtual void onGetPosition(BWAPI::Position targetPosition) = 0;
    virtual void onGetNewPosition(BWAPI::Position targetPosition) = 0;
	virtual void onStop() = 0;
	virtual void onHold() = 0;
};
