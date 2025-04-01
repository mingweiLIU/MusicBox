#include "Player.h"
#include <algorithm>
#include <locale>
#include <codecvt>
#include "SoundBox.h"

Player::Player(HWND hwnd, std::vector<SongInfo>& songList):songList(songList)
{
	if (!BASS_Init(-1, 44100, 0, hwnd, NULL)) {
		Error("无法初始化播放设备！");
		return;
	}
	// BASS的Unicode版本需要额外设置
	BASS_SetConfig(BASS_CONFIG_UNICODE, 1);
	this->playingIndex = 0;
	this->hs = NULL;
}

Player::~Player()
{
	BASS_StreamFree(hs);
	BASS_Free();
}

void Player::PlaySong(std::wstring filePath)
{
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
void Player::PlaySong(int index)
{
	//如果是正在播放的 就不处理	
	if (index==this->playingIndex) return;

	//如果在播 就停止
	if (hs != NULL)
	{
		BASS_ChannelStop(hs);
		playingIndex = -1;
	}

	//std::wstring filePathW = stringToWideString(songList[index].filePath);
	//const char* te = songList[index].filePath.c_str();
	//std::wstring utf8 =L"D:\\KwDownload\\song\\许巍-时光.mp3";
	hs = BASS_StreamCreateFile(FALSE, songList[index].filePath.c_str(), 0, 0, 0);
	if (!hs)
	{
		Error("播放错误！");
		return;
	}
	BASS_ChannelPlay(hs, FALSE);
	BASS_ChannelSetSync(hs, BASS_SYNC_END, 0, Player::endNextSong, this);

	playingIndex = index;
}

void Player::PauseSong() {
	BASS_ChannelPause(hs);
}

void Player::PauseResumSong() {
	BASS_ChannelPlay(hs, FALSE);
}

//void Player::NextSong()
//{
//	this->playingIndex++;
//	if (this->playingIndex == this->songList.size()) {
//		this->playingIndex = 0;
//	}
//	this->PlaySong(this->playingIndex);
//}

//void Player::PreSong()
//{
//	this->playingIndex--;
//	if (this->playingIndex < 0) {
//		this->playingIndex = this->songList.size()-1;
//	}
//	this->PlaySong(this->playingIndex);
//}

void Player::UpdateSpectum()
{
	if (!this->hs) return;

	int x; 
	float times = 4.0;
	float fft[1024];
	BASS_ChannelGetData(this->hs, fft, BASS_DATA_FFT2048); // get the FFT data

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
}

bool Player::isPlaying() {
	return this->hs != NULL;
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
}
