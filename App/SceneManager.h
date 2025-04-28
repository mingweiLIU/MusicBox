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

//����ģʽ
enum class SpectumMode
{
	Cube,//����������ģʽ
	CubeDrop,//�����������Ϸ���һ��С�� ��С���黹���µ�
	Wave,//������
	WaveDrop,//�����ͣ������µ�
	DoubleWave,//������+�Ӳ��� �Ӳ����Ƕ���Ĳ����µ�
	COUNT//��Զ������� ����ȷ��ö������
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

	//soundBox���
	osg::ref_ptr<osg::Group> soundBoxRoot;
	std::vector<osg::Geode*> boxes;
	osg::ref_ptr<ChangeSoundBoxHeight> soundBoxCallBack;
	bool isShowingSoundBox = false;

	//DropBox���
	osg::ref_ptr<osg::Group> dropBoxRoot;
	std::vector<osg::Geode*> dropBoxes;
	osg::ref_ptr<DropSoundBoxCallback> dropSoundBoxCallback;
	bool isShowingDropSoundBox = false;

	//SoundWave���
	osg::ref_ptr<osg::Group> soundWaveRoot;
	bool isShowingSoundWave = false;

	//DropSoundWave���
	osg::ref_ptr<osg::Group> soundDropWaveRoot;
	bool isShowingSoundDropWave = false;
private:

	osg::ref_ptr<osg::OperationsThread> operationThread;
	osg::ref_ptr<osg::OperationQueue> operationQueue;

private:
    //���ð�͸��
	void makeWindowBorderless(HWND hwnd);
	void addNodeSafe(osg::Group* parent, osg::Node* child);
	void removeNodeSafe(osg::Group* parent, osg::Node* child);
	//����Cube�Ŀ��ӻ����
	void setSoundBox(bool show);
	void createSoundBox();
	//����Dropbox�Ŀ��ӻ����
	void setDropBox(bool show);
	void createDropSoundBox();
	//���������׵Ŀ��ӻ����
	void setWave(bool show);
	void createWave();
	//���ƴӲ��׵Ŀ��ӻ����
	void setDropWave(bool show);
	void createDropWave();

public:
	//�������λ��
	void setCamera();
	//��ȡ�������
	void getCamera();
};

//ʵ�ְ�ȫɾ������ӵ���
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

//ɾ��
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
