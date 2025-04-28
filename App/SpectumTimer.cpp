#include "SpectumTimer.h"
void SpectumTimer::Start()
{
	timerID = timeSetEvent(
		25,                   // 间隔
		25,                      // 分辨率
		&SpectumTimer::TimerCall, // 静态回调
		(DWORD_PTR)this,        // 传递对象指针
		TIME_PERIODIC           // 周期性定时器
	);
	if (!timerID) {
		MessageBox(0, "周期事件启动失败", "错误", MB_OK);
	}
}

void SpectumTimer::Stop()
{
	if (timerID) {
		timeKillEvent(timerID);
		timerID = 0;
	}
}

void CALLBACK SpectumTimer::TimerCall(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	SpectumTimer* pThis = reinterpret_cast<SpectumTimer*>(dwUser);
	pThis->OnTimer();
}

void SpectumTimer::OnTimer()
{
	outPlayer->UpdateSpectum();
}

