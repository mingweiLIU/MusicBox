#include "Player.h"
#include <algorithm>
#include <locale>
#include <codecvt>
#include "SoundBox.h"

Player::Player(HWND hwnd, bool playMode, std::vector<SongInfo>& songList):songList(songList)
{
	this->hs = NULL;
	this->hwnd = hwnd;
	this->playMode = playMode;

	initBASS();
}

Player::~Player()
{
	StopSongAndFree();
}

void Player::PlaySong(std::wstring filePath)
{
	if(!playMode)
	{
		Error("模式设置错误，请先修改模式！");
		return;
	}
	//如果是正在播放的 就不处理	
	if (songList[playingIndex].filePath == filePath) return;

	//如果在播 就停止
	if (hs!=NULL)
	{
		BASS_ChannelStop(hs);
	}

	hs = BASS_StreamCreateFile(FALSE, filePath.c_str(), 0, 0, 0);
	if (hs<BASS_ERROR_ENDED)
	{
		Error("播放错误！");
		return;
	}
	BASS_ChannelPlay(hs, FALSE);
	BASS_ChannelSetSync(hs, BASS_SYNC_END, 0, Player::endNextSong, this);
	
	auto it = std::find_if(songList.begin(), songList.end(), [&filePath](SongInfo oneInfo) {
		return oneInfo.filePath == filePath;
	});
	if (it!=songList.end()) return ;

	playingIndex = std::distance(songList.begin(),it);
}
bool Player::PlaySong(int index,float& trackTimeLong)
{
	if (!playMode)
	{
		Error("模式设置错误，请先修改模式！");
		return false;
	}
	if (index<0 || index>=songList.size()) return false;

	//如果是正在播放的 就不处理	
	if (index == this->playingIndex) { 
		trackTimeLong = currentTrackLengthSecond;
		return true;
	}

	//如果在播 就停止
	if (hs != NULL)
	{
		BASS_ChannelStop(hs);
		playingIndex = -1;
	}

	hs = BASS_StreamCreateFile(FALSE, songList[index].filePath.c_str(), 0, 0, 0);
	if (!hs)
	{
		Error("播放错误！");
		return false;
	}
	BASS_ChannelPlay(hs, FALSE);
	BASS_ChannelSetSync(hs, BASS_SYNC_END, 0, Player::endNextSong, this);

	playingIndex = index;

	//获取时长
	QWORD pos = BASS_ChannelGetLength(hs, BASS_POS_BYTE);
	int secs=0;
	if (pos != -1) {
		secs = (int)BASS_ChannelBytes2Seconds(hs, pos);
	}
	currentTrackLengthSecond = secs;
	trackTimeLong = currentTrackLengthSecond;
	return true;
}

void Player::PauseSong() {
	if (!playMode) {
		BASS_WASAPI_Stop(false);
	}
	BASS_ChannelPause(hs);
}

void Player::PauseResumSong() {
	if (!playMode) {
		BASS_WASAPI_Start();
	}
	BASS_ChannelPlay(hs, FALSE);
}

void Player::StopSongAndFree() {
	//停止
	if (!playMode) {
		BASS_WASAPI_Stop(true);    // 停止捕获
		BASS_WASAPI_Free();    // 释放 WASAPI 设备（可选）
	}
	BASS_ChannelStop(hs);  // 立即停止播放
	BASS_StreamFree(hs);   // 释放流资源（可选，如需彻底销毁
	BASS_Free();
	hs = NULL;
}


bool Player::isPlaying() {
	return this->hs != NULL;
}

void Player::PlayAtTime(float timeStamp)
{
	//如果在播 就停止
	if (hs == NULL)
	{
		return;
	}

	QWORD pos = BASS_ChannelSeconds2Bytes(hs, timeStamp);
	BASS_ChannelSetPosition(hs, pos, BASS_POS_BYTE);
	BASS_ChannelPlay(hs, false);
}

void Player::FlushData() {
	//if (playMode) {
	//	BYTE tempBuffer[4096];
	//	while (BASS_ChannelGetData(hs, tempBuffer, sizeof(tempBuffer)) > 0);
	//}
	//else {
	//	float fftDummy[1024] = { 0 };
	//	while (BASS_WASAPI_GetData(fftDummy, BASS_DATA_FFT2048) > 0);
	//}
}

void Player::RecordSystemSound()
{
	if(playMode)
	{
		Error("模式设置错误，请先修改模式！");
		return;
	}
	BASS_WASAPI_Start(); // 开始捕获声卡输出
	BASS_Start(); // 开始 Bass 音频处理

	// 创建一个流来接收捕获的数据
	hs = BASS_StreamCreate(44100, 2, 0, STREAMPROC_PUSH, NULL);
	BASS_ChannelPlay(hs, FALSE);
}

void Player::setPlayTimeUpdateCallback(std::function<void(double,double)> callback)
{
	playTimeUpdateCall = callback;
}

void Player::setPlayIndexChangeCallback(std::function<void(int,float)> callback)
{
	playIndexUpdateCall = callback;
}
void Player::initBASS() {
	//检查是否已经初始化
	BASS_INFO info;
	if (!BASS_GetInfo(&info)) {
		DWORD error = BASS_ErrorGetCode();
		if (error != BASS_ERROR_INIT) {
			return;
		}
	}
	else {
		return;
	}

	if (playMode) {
		if (!BASS_Init(-1, 44100, 0, hwnd, NULL)) {
			Error("无法初始化播放设备！");
			return;
		}
		// BASS的Unicode版本需要额外设置
		BASS_SetConfig(BASS_CONFIG_UNICODE, 1);
	}
	else {
		if (!BASS_Init(0, 44100, 0, hwnd, NULL)) {
			Error("Can't initialize BASS");
		}
		BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 10); // 低延迟
		// 初始化 WASAPI 捕获（监听默认输出设备）
		if (!BASS_WASAPI_Init(-3, 0, 0, BASS_WASAPI_BUFFER, 1, 0.1, Player::DuffRecording, NULL)
			&& !BASS_WASAPI_Init(-2, 0, 0, BASS_WASAPI_BUFFER, 1, 0.1, Player::DuffRecording, NULL)) {
			Error("Can't initialize WASAPI device");
		}
	}
}
void Player::Error(const char* es)
{
	char mes[200];
	sprintf(mes, "%s\n(error code: %d)", es, BASS_ErrorGetCode());
	MessageBox(NULL, mes, 0, 0);
}

std::wstring Player::stringToWideString(const std::string& str)
{
	if (str.empty()) return L"";

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
	return wstr;
}

const WCHAR* Player::stringToWCHARPtr(const std::string& str)
{
	// 计算需要的宽字符数量
	int size_needed = MultiByteToWideChar(
		CP_UTF8,            // 使用UTF-8编码
		0,                  // 标志位
		str.c_str(),        // 输入字符串
		(int)str.size(),    // 字符串长度
		NULL,               // 输出缓冲区（NULL表示仅计算所需大小）
		0                   // 输出缓冲区大小
	);

	// 分配缓冲区（+1用于null终止符）
	static WCHAR* buffer = new WCHAR[size_needed + 1];

	// 执行转换
	MultiByteToWideChar(
		CP_UTF8,
		0,
		str.c_str(),
		(int)str.size(),
		buffer,
		size_needed
	);

	// 添加null终止符
	buffer[size_needed] = L'\0';

	return buffer;
}

void CALLBACK Player::endNextSong(HSYNC handle1, DWORD channel, DWORD data, void* user)
{
	Player* player = (Player*)user;
	player->playingIndex++;
	if (player->playingIndex == player->songList.size()) {
		player->playingIndex = 0;
	}

	std::wstring filePathW =player->songList[player->playingIndex].filePath;
	player->hs = BASS_StreamCreateFile(FALSE, filePathW.c_str(), 0, 0, 0);
	if (player->hs < BASS_ERROR_ENDED)
	{
		player->Error("播放错误！");
		return;
	}
	BASS_ChannelPlay(player->hs, FALSE);
	BASS_ChannelSetSync(player->hs, BASS_SYNC_END, 0, Player::endNextSong, user);

	if (player->playIndexUpdateCall)
	{
		QWORD pos = BASS_ChannelGetLength(player->hs, BASS_POS_BYTE);
		int secs = 0;
		if (pos != -1) {
			secs = (int)BASS_ChannelBytes2Seconds(player->hs, pos);
		}
		player->currentTrackLengthSecond = secs;

		player->playIndexUpdateCall(player->playingIndex,secs);
	}
}

//什么也不做
DWORD CALLBACK Player::DuffRecording(void* buffer, DWORD length, void* user)
{
	return TRUE; // continue recording
}

void Player::UpdateSpectum()
{
	if (!this->hs) return;

	int x;
	float times = 4.0;
	float fft[1024] = {};
	if (playMode)
	{
		BASS_ChannelGetData(this->hs, fft, BASS_DATA_FFT2048); // get the FFT data
	}
	else {
		BASS_WASAPI_GetData(fft, BASS_DATA_FFT2048);
	}

	int BANDS = 100;
	int SPECHEIGHT = 255 * times;
	int b0 = 0;
	for (x = 0; x < BANDS; x++)
	{
		float peak = 0;
		int b1 = pow(2, x * 10.0 / (BANDS - 1));
		if (b1 > 1023) b1 = 1023;
		if (b1 <= b0) b1 = b0 + 1; // make sure it uses at least 1 FFT bin
		for (; b0 < b1; b0++)
			if (peak < fft[1 + b0]) peak = fft[1 + b0];
		*(ChangeSoundBoxHeight::spectum + x) = sqrt(peak) * 3 * SPECHEIGHT - 4; // scale it (sqrt to make low values more visible)
		if (*(ChangeSoundBoxHeight::spectum + x) > SPECHEIGHT)
			*(ChangeSoundBoxHeight::spectum + x) = SPECHEIGHT; // cap it
	}

	//下面同步播放时间
	if (playMode)
	{
		QWORD pos = BASS_ChannelGetPosition(hs, BASS_POS_BYTE);
		double time = BASS_ChannelBytes2Seconds(hs, pos);
		if (playTimeUpdateCall) {
			playTimeUpdateCall(time, time / currentTrackLengthSecond);
		}
	}
}
