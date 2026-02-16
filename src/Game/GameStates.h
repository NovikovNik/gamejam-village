#pragma once
#include "../Utils/Singleton.h"

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

	public:
	bool w;
	bool a;
	bool s;
	bool d;
};
