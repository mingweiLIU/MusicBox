#pragma once
#include <osgViewer/Viewer>
#include <osgViewer/GraphicsWindow>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/BlendFunc>
#include <osg/Camera>
#include <osgViewer/api/Win32/GraphicsWindowWin32>
#include <osgGA/TrackballManipulator>
#include <osg/OperationThread>
#include <thread>

#include "SoundBox.h"
#include "SoundWave.h"

//波形模式
enum class SpectumMode
{
	Cube,//纯波谱立体模式
	CubeDrop,//纯波谱立体上方加一个小块 该小方块还会下掉
	Wave,//纯波型
	WaveDrop,//纯波型，波会下掉
	DoubleWave,//纯波型+从波形 从波形是额外的波会下掉
	COUNT//永远放在最后 用于确定枚举数量
};


class SceneManager
{
public:
	SceneManager();
	~SceneManager();
	HWND getHWND();
	void setDisplayMode(SpectumMode mode);
public:
	osg::ref_ptr<osgViewer::Viewer> viewer;
	osg::ref_ptr<osg::Group> root;

	//soundBox相关
	osg::ref_ptr<osg::Group> soundBoxRoot;
	std::vector<osg::Geode*> boxes;
	osg::ref_ptr<ChangeSoundBoxHeight> soundBoxCallBack;
	bool isShowingSoundBox = false;

	//DropBox相关
	osg::ref_ptr<osg::Group> dropBoxRoot;
	std::vector<osg::Geode*> dropBoxes;
	osg::ref_ptr<DropSoundBoxCallback> dropSoundBoxCallback;
	bool isShowingDropSoundBox = false;

	//SoundWave相关
	osg::ref_ptr<osg::Group> soundWaveRoot;
	bool isShowingSoundWave = false;

	//DropSoundWave相关
	osg::ref_ptr<osg::Group> soundDropWaveRoot;
	bool isShowingSoundDropWave = false;
private:

	osg::ref_ptr<osg::OperationsThread> operationThread;
	osg::ref_ptr<osg::OperationQueue> operationQueue;

private:
    //设置半透明
	void makeWindowBorderless(HWND hwnd);
	void addNodeSafe(osg::Group* parent, osg::Node* child);
	void removeNodeSafe(osg::Group* parent, osg::Node* child);
	//控制Cube的可视化与否
	void setSoundBox(bool show);
	void createSoundBox();
	//控制Dropbox的可视化与否
	void setDropBox(bool show);
	void createDropSoundBox();
	//控制主波谱的可视化与否
	void setWave(bool show);
	void createWave();
	//控制从波谱的可视化与否
	void setDropWave(bool show);
	void createDropWave();

public:
	//设置相机位置
	void setCamera();
	//获取相机参数
	void getCamera();
};

//实现安全删除和添加的类
class AddNodeOperation : public osg::Operation
{
public:
	AddNodeOperation(osg::Group* parent, osg::Node* child)
		: _parent(parent), _child(child) {}

	virtual void operator()(osg::Object*)
	{
		if (_parent.valid() && _child.valid())
		{
			_parent->addChild(_child.get());
		}
	}

private:
	osg::ref_ptr<osg::Group> _parent;
	osg::ref_ptr<osg::Node> _child;
};

//删除
class RemoveNodeOperation : public osg::Operation
{
public:
	RemoveNodeOperation(osg::Group* parent, osg::Node* child)
		: _parent(parent), _child(child) {}

	virtual void operator()(osg::Object*)
	{
		if (_parent.valid() && _child.valid())
		{
			_parent->removeChild(_child.get());
		}
	}

private:
	osg::ref_ptr<osg::Group> _parent;
	osg::ref_ptr<osg::Node> _child;
};
