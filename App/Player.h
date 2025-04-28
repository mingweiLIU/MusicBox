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
	void FlushData();//�������

	void RecordSystemSound();

	//���ûص�����
	//���ò���ʱ����º���
	void setPlayTimeUpdateCallback(std::function<void(double,double)> callback);
	//���ò�����Ŀ�仯���º���
	void setPlayIndexChangeCallback(std::function<void(int,float)> callback);

	void initBASS();

public:
	int playingIndex=-1;
	std::vector<SongInfo>& songList;
	bool playMode = true;//�Ƿ����Լ�Ҳ�ڲ��� true��ʾ��Ƶ��Դ�Լ� false��ʾ��Ƶ��Դ��ϵͳ����
protected:
private:
	HSTREAM hs;
	HWND hwnd;
	void Error(const char* es);
	std::wstring stringToWideString(const std::string& str);
	const WCHAR* stringToWCHARPtr(const std::string& str);
	float currentTrackLengthSecond;//��ǰ������Ŀ�ĳ��ȣ��룩
	static void CALLBACK endNextSong(HSYNC handle1, DWORD channel, DWORD data, void* user);//һ�׸貥�����һ��
	static DWORD CALLBACK DuffRecording(void* buffer, DWORD length, void* user);//һ�׸貥�����һ��
	std::function<void(double,double)> playTimeUpdateCall=nullptr; //����ʱʱ����µĺ��� ������ʱ��ͱ���
	std::function<void(int,float)> playIndexUpdateCall=nullptr;//���ŵ���Ŀ�仯ʱ���� ��������Ŀindex����Ŀ��ʱ��
};