#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <bass.h>

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
};

class Player
{
public:
	Player(HWND hwnd,std::vector<SongInfo>& songList);
	~Player();
	void PlaySong(std::wstring filePath);
	void PlaySong(int index);
	void PauseSong();
	void PauseResumSong();
	//void NextSong();
	//void PreSong();
	void UpdateSpectum();
	bool isPlaying();
public:
	int playingIndex;
	std::vector<SongInfo>& songList;
protected:
private:
	HSTREAM hs;
	void Error(const char* es);
	std::wstring stringToWideString(const std::string& str);
	const WCHAR* stringToWCHARPtr(const std::string& str);
	static void CALLBACK endNextSong(HSYNC handle1, DWORD channel, DWORD data, void* user);//一首歌播完后下一首

};