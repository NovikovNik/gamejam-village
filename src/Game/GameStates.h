#pragma once
#include "../Utils/Singleton.h"
#include <SDL3/SDL.h>

class GameStates : public Singleton<GameStates>
{
	public:
	GameStates()
	{
		w = false;
		a = false;
		s = false;
		d = false;
	}

	void ResetMovement() { w = a = s = d = false; }
	void SetOnlyW() { ResetMovement(); w = true; }
	void SetOnlyA() { ResetMovement(); a = true; }
	void SetOnlyS() { ResetMovement(); s = true; }
	void SetOnlyD() { ResetMovement(); d = true; }

	void SyncMovementFromKeyboard(bool fourDirectionsOnly);

	public:
	bool w;
	bool a;
	bool s;
	bool d;
};
