#include "SceneManager.h"
#include <osg/BoundingSphere>
#include "SoundWave.h"

SceneManager::SceneManager()
{
    viewer = new osgViewer::Viewer;
    root = new osg::Group;
	soundBoxRoot = new osg::Group;
	soundBoxRoot->setDataVariance(osg::Object::DYNAMIC);
    dropBoxRoot = new osg::Group;
    dropBoxRoot->setDataVariance(osg::Object::DYNAMIC);
	soundWaveRoot = new osg::Group;
    soundWaveRoot->setDataVariance(osg::Object::DYNAMIC);
    soundDropWaveRoot = new osg::Group;
    soundDropWaveRoot->setDataVariance(osg::Object::DYNAMIC);
    root->addChild(soundBoxRoot);
    root->addChild(dropBoxRoot);
    root->addChild(soundWaveRoot);
	root->addChild(soundDropWaveRoot);

    //设置更新队列
	operationThread = new osg::OperationsThread();
	operationQueue = new osg::OperationQueue();
    operationThread->setOperationQueue(operationQueue);
    operationThread->start();

	setDisplayMode(SpectumMode::Cube);

    // 透明渲染设置
    osg::ref_ptr<osg::Camera> camera = viewer->getCamera();
    camera->setClearColor(osg::Vec4(0, 0, 0, 0));  // 设置透明背景
    camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 在创建窗口之前先设置Alpha混合
    osg::StateSet* stateSet = camera->getOrCreateStateSet();
    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateSet->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // 修改窗口创建顺序，先创建但不显示
    viewer->setUpViewInWindow(0, 0, 1, 1, 0);
    viewer->setSceneData(root);
    soundBoxCallBack = new ChangeSoundBoxHeight(boxes);
    soundBoxRoot->setUpdateCallback(soundBoxCallBack);

    dropSoundBoxCallback = new DropSoundBoxCallback(dropBoxes); 
    dropBoxRoot->setUpdateCallback(dropSoundBoxCallback);

    //viewer->setCameraManipulator(new osgGA::TrackballManipulator());

    // 先realize获取窗口句柄
    viewer->realize(); 

    // 获取并设置窗口为无边框和透明
    HWND hwnd = getHWND();
    if (hwnd) {
		makeWindowBorderless(hwnd);
    }

    //为了避免闪屏 做下面的操作 初始化时使用很小的范围 现在来全屏
    //修改窗体大小
    osgViewer::Viewer::Windows windows;
    viewer->getWindows(windows);
    if (!windows.empty()) {
        osgViewer::GraphicsWindow* gw = windows.front();

        // 修改窗口大小
        gw->setWindowRectangle(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    }
    if (hwnd)
	{
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    }

}

SceneManager::~SceneManager()
{
	// 停止线程
	if (operationThread.valid())
	{
		operationThread->cancel();
		operationThread->join();
	}
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

void SceneManager::setDisplayMode(SpectumMode mode)
{
    switch (mode)
    {
	case SpectumMode::Cube: {
		setSoundBox(true);
		setDropBox(false);
		setWave(false);
		setDropWave(false);
		break;
    }
	case SpectumMode::CubeDrop: {
		setSoundBox(true);
		setDropBox(true);
		setWave(false);
		setDropWave(false);
		break;
    }
	case SpectumMode::Wave: {
		setSoundBox(false);
		setDropBox(false);
		setWave(true);
		setDropWave(false);
		break;
    }
	case SpectumMode::WaveDrop: {
		setSoundBox(false);
		setDropBox(false);
		setWave(false);
		setDropWave(true);
		break;

    }
	case SpectumMode::DoubleWave: {
		setSoundBox(false);
		setDropBox(false);
		setWave(true);
		setDropWave(true);
		break;
    }
	default:
		break;
	}
}

void SceneManager::makeWindowBorderless(HWND hwnd) {
    // 在窗口显示之前设置样式
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    style |= WS_POPUP; // 注意这里先不设置WS_VISIBLE
    SetWindowLong(hwnd, GWL_STYLE, style);

    // 设置扩展样式
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_LAYERED | WS_EX_TOPMOST;
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

    // 设置透明颜色
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // 确保窗口置顶
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

    // 完成所有设置后才显示窗口
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

void SceneManager::addNodeSafe(osg::Group* parent, osg::Node* child)
{
    operationQueue->add(new AddNodeOperation(parent, child));
}

void SceneManager::removeNodeSafe(osg::Group* parent, osg::Node* child)
{
    operationQueue->add(new RemoveNodeOperation(parent, child));
}

void SceneManager::setSoundBox(bool show)
{
	if (show)
	{
		if (!isShowingSoundBox) createSoundBox();
	}
	else {
		//删除
		if (isShowingSoundBox)
		{
			for (int i = 0; i < boxes.size(); ++i)
			{
				removeNodeSafe(soundBoxRoot.get(), boxes[i]);
			}
			boxes.swap(std::vector<osg::Geode*>());
		}
	}
	isShowingSoundBox = show;
}

void SceneManager::createSoundBox()
{
	//初始化前 先初始化波谱
	ChangeSoundBoxHeight::spectum = new int[100];
	for (int i = 0; i < 100; i++)
	{
		*(ChangeSoundBoxHeight::spectum + i) = 50;
	}

	//初始化box
	osg::Vec3 centerPos(0.0f, 0.0f, 0.0f);
	float interval = 15.0f;
	float xSize = 30.0f;
	float ySize = 30.0f;
	float zSize = 220.f;

	float zSizeDrop = 10.0f;

	for (int i = 0; i < 100; i++)
	{
		//初始化跳动的box
		osg::ref_ptr<SoundBox> box1 = new SoundBox(xSize, ySize, zSize, osg::Vec3((interval + xSize) * i, 0, 0));
		box1->setDataVariance(osg::Object::DYNAMIC);
		soundBoxRoot->addChild(box1);
		boxes.push_back(box1);
	}
}



void SceneManager::setDropBox(bool show)
{
	if (show)
	{
		if (!isShowingDropSoundBox) createDropSoundBox();
	}
	else {
		//删除
		if (isShowingDropSoundBox)
		{
			for (int i = 0; i < dropBoxes.size(); ++i)
			{
				removeNodeSafe(dropBoxRoot.get(), dropBoxes[i]);
			}
			dropBoxes.swap(std::vector<osg::Geode*>());
		}
	}
	isShowingDropSoundBox = show;
}

void SceneManager::createDropSoundBox()
{
	//初始化box
	osg::Vec3 centerPos(0.0f, 0.0f, 0.0f);
	float interval = 15.0f;
	float xSize = 30.0f;
	float ySize = 30.0f;
	float zSize = 220.f;

	float zSizeDrop = 10.0f;

	for (int i = 0; i < 100; i++)
	{
		//初始化顶部要下坠的box
		osg::ref_ptr<SoundBox> boxDrop = new SoundBox(xSize, ySize, zSizeDrop, osg::Vec3((interval + xSize) * i, 0, *(ChangeSoundBoxHeight::spectum + i) + SoundBox::OffSetBoxTop));
		boxDrop->setDataVariance(osg::Object::DYNAMIC);
		addNodeSafe(dropBoxRoot.get(), boxDrop.get());
		dropBoxes.push_back(boxDrop);
	}
}


void SceneManager::setWave(bool show)
{
	if (show)
	{
		if (!isShowingSoundWave) createWave();
	}
	else {
		//删除
		if (isShowingSoundWave) removeNodeSafe(soundWaveRoot.get(), soundWaveRoot->getChild(0));
	}
	isShowingSoundWave = show;
}

void SceneManager::createWave()
{
	osg::ref_ptr<SoundWave> soundWave = new SoundWave();
	soundWaveRoot->addChild(soundWave);
	soundWave->addUpdateCallback(new ChangeSoundWave());
}

void SceneManager::setDropWave(bool show)
{
	if (show)
	{
		if(!isShowingSoundDropWave) createDropWave();
	}
	else {
		//删除
		if (isShowingSoundDropWave) removeNodeSafe(soundDropWaveRoot.get(), soundDropWaveRoot->getChild(0));
	}
	isShowingSoundDropWave = show;
}

void SceneManager::createDropWave()
{
	osg::ref_ptr<SoundWave> soundSlaveWave = new SoundWave(true);
	soundDropWaveRoot->addChild(soundSlaveWave);
	soundSlaveWave->addUpdateCallback(new ChangeSoundWave());
}

void SceneManager::setCamera() {
	osg::Matrix viewMat;
	viewer->getCamera()->setViewMatrixAsLookAt(osg::Vec3d(2195, -5380, 1323.75), osg::Vec3d(2195, -5379, 1323.75), osg::Vec3d(0, 0, 1));
	// 设置透视投影
	viewer->getCamera()->setProjectionMatrixAsPerspective(
		30.0, // 视野角度
		static_cast<double>(GetSystemMetrics(SM_CXSCREEN)) / GetSystemMetrics(SM_CYSCREEN), // 宽高比
		0.1,  // 近平面
		10000.0 // 远平面
	);
}

void SceneManager::getCamera()
{
	osg::Vec3d eye, center, up;
	viewer->getCamera()->getViewMatrixAsLookAt(eye, center, up);
	osg::BoundingSphere bs = root->getBound();
osg::Vec3d center1 = bs.center();
float radius = bs.radius();
}
