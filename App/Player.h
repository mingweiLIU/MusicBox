#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <bass.h>
#include <basswasapi.h>
#include <functional>

enum class  State
{
	NONE,
	HOVERED,
	SELECTED,
	PRESSED
};

struct SongInfo
{
public:
	SongInfo(std::wstring name, std::wstring artisit, std::wstring filePath) {
		this->name = name;
		this->artist = artisit;
		this->filePath = filePath;
	}
	std::wstring name;
	std::wstring artist;
	std::wstring filePath;
	State state = State::NONE;
};

class Player
{
public:
	Player(HWND hwnd,bool playMode,std::vector<SongInfo>& songList);
	~Player();
	void PlaySong(std::wstring filePath);
	bool PlaySong(int index, float& trackTimeLong);
	void PauseSong();
	void PauseResumSong();
	void StopSongAndFree();
	//void NextSong();
	//void PreSong();
	void UpdateSpectum();
	bool isPlaying();
	void PlayAtTime(float timeStamp);
	void FlushData();//清空数据

	void RecordSystemSound();

	//设置回调函数
	//设置播放时间更新函数
	void setPlayTimeUpdateCallback(std::function<void(double,double)> callback);
	//设置播放曲目变化更新函数
	void setPlayIndexChangeCallback(std::function<void(int,float)> callback);

	void initBASS();

public:
	int playingIndex=-1;
	std::vector<SongInfo>& songList;
	bool playMode = true;//是否是自己也在播放 true表示音频来源自己 false表示音频来源于系统声音
protected:
private:
	HSTREAM hs;
	HWND hwnd;
	void Error(const char* es);
	std::wstring stringToWideString(const std::string& str);
	const WCHAR* stringToWCHARPtr(const std::string& str);
	float currentTrackLengthSecond;//当前播放曲目的长度（秒）
	static void CALLBACK endNextSong(HSYNC handle1, DWORD channel, DWORD data, void* user);//一首歌播完后下一首
	static DWORD CALLBACK DuffRecording(void* buffer, DWORD length, void* user);//一首歌播完后下一首
	std::function<void(double,double)> playTimeUpdateCall=nullptr; //播放时时间更新的函数 参数是时间和比例
	std::function<void(int,float)> playIndexUpdateCall=nullptr;//播放的曲目变化时函数 参数是曲目index和曲目总时长
};