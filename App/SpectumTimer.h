#pragma once
#include <windows.h>
#include "Player.h"

class SpectumTimer {
public:
	SpectumTimer(Player* player):outPlayer(player){}
	void Start();
	void Stop();

private:
	UINT timerID = 0;
	Player* outPlayer;
	static void CALLBACK TimerCall(UINT uTimerID, UINT uMsg,
		DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
	void OnTimer();
};