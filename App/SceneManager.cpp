#include "SceneManager.h"
#include "SoundBox.h"

SceneManager::SceneManager()
{
	viewer = new osgViewer::Viewer;
	root = new osg::Group;

	InitalBoxes();

	// **透明渲染设置**
	osg::ref_ptr<osg::Camera> camera = viewer->getCamera();
	camera->setClearColor(osg::Vec4(0, 0, 0, 0));  // 设置透明背景
	camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 启用 Alpha 透明混合
	osg::StateSet* stateSet = camera->getOrCreateStateSet();
	stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
	stateSet->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	// **创建窗口**
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	viewer->setUpViewInWindow(0, 0, screenWidth, screenHeight, 0);

	viewer->setSceneData(root);
	ChangeSoundBoxHeight* changeCallBack = new ChangeSoundBoxHeight(boxes);
	root->setUpdateCallback(changeCallBack);
	viewer->setCameraManipulator(new osgGA::TrackballManipulator());
}

HWND SceneManager::getHWND()
{
	osgViewer::Viewer::Windows windows;
	viewer->getWindows(windows);
	if (!windows.empty()) {
		osgViewer::GraphicsWindowWin32* gw = dynamic_cast<osgViewer::GraphicsWindowWin32*>(windows.front());
		if (gw) {
			return gw->getHWND();
		}
	}
	return nullptr;
}

void SceneManager::InitalBoxes()
{
	//初始化前 先初始化波谱
	ChangeSoundBoxHeight::spectum = new int[100];
	for (int i = 0; i < 100; i++)
	{
		*(ChangeSoundBoxHeight::spectum + i) = 1;
	}

	//初始化box
	osg::Vec3 centerPos(0.0f, 0.0f, 0.0f);
	float interval = 15.0f;
	float xSize = 30.0f;
	float ySize = 30.0f;
	float zSize = 220.f;

	for (int i = 0; i < 100; i++)
	{
		osg::ref_ptr<SoundBox> box1 = new SoundBox(xSize, ySize, zSize, centerPos + osg::Vec3(interval + xSize, centerPos.y(), centerPos.z()));
		centerPos = centerPos + osg::Vec3(interval + xSize, centerPos.y(), centerPos.z());
		box1->setDataVariance(osg::Object::DYNAMIC);
		root->addChild(box1);
		boxes.push_back(box1);
	}
}
