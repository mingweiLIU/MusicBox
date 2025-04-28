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

    //���ø��¶���
	operationThread = new osg::OperationsThread();
	operationQueue = new osg::OperationQueue();
    operationThread->setOperationQueue(operationQueue);
    operationThread->start();

	setDisplayMode(SpectumMode::Cube);

    // ͸����Ⱦ����
    osg::ref_ptr<osg::Camera> camera = viewer->getCamera();
    camera->setClearColor(osg::Vec4(0, 0, 0, 0));  // ����͸������
    camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // �ڴ�������֮ǰ������Alpha���
    osg::StateSet* stateSet = camera->getOrCreateStateSet();
    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateSet->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // �޸Ĵ��ڴ���˳���ȴ���������ʾ
    viewer->setUpViewInWindow(0, 0, 1, 1, 0);
    viewer->setSceneData(root);
    soundBoxCallBack = new ChangeSoundBoxHeight(boxes);
    soundBoxRoot->setUpdateCallback(soundBoxCallBack);

    dropSoundBoxCallback = new DropSoundBoxCallback(dropBoxes); 
    dropBoxRoot->setUpdateCallback(dropSoundBoxCallback);

    //viewer->setCameraManipulator(new osgGA::TrackballManipulator());

    // ��realize��ȡ���ھ��
    viewer->realize(); 

    // ��ȡ�����ô���Ϊ�ޱ߿��͸��
    HWND hwnd = getHWND();
    if (hwnd) {
		makeWindowBorderless(hwnd);
    }

    //Ϊ�˱������� ������Ĳ��� ��ʼ��ʱʹ�ú�С�ķ�Χ ������ȫ��
    //�޸Ĵ����С
    osgViewer::Viewer::Windows windows;
    viewer->getWindows(windows);
    if (!windows.empty()) {
        osgViewer::GraphicsWindow* gw = windows.front();

        // �޸Ĵ��ڴ�С
        gw->setWindowRectangle(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    }
    if (hwnd)
	{
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    }

}

SceneManager::~SceneManager()
{
	// ֹͣ�߳�
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
    // �ڴ�����ʾ֮ǰ������ʽ
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    style |= WS_POPUP; // ע�������Ȳ�����WS_VISIBLE
    SetWindowLong(hwnd, GWL_STYLE, style);

    // ������չ��ʽ
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_LAYERED | WS_EX_TOPMOST;
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

    // ����͸����ɫ
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // ȷ�������ö�
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

    // ����������ú����ʾ����
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
		//ɾ��
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
	//��ʼ��ǰ �ȳ�ʼ������
	ChangeSoundBoxHeight::spectum = new int[100];
	for (int i = 0; i < 100; i++)
	{
		*(ChangeSoundBoxHeight::spectum + i) = 50;
	}

	//��ʼ��box
	osg::Vec3 centerPos(0.0f, 0.0f, 0.0f);
	float interval = 15.0f;
	float xSize = 30.0f;
	float ySize = 30.0f;
	float zSize = 220.f;

	float zSizeDrop = 10.0f;

	for (int i = 0; i < 100; i++)
	{
		//��ʼ��������box
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
		//ɾ��
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
	//��ʼ��box
	osg::Vec3 centerPos(0.0f, 0.0f, 0.0f);
	float interval = 15.0f;
	float xSize = 30.0f;
	float ySize = 30.0f;
	float zSize = 220.f;

	float zSizeDrop = 10.0f;

	for (int i = 0; i < 100; i++)
	{
		//��ʼ������Ҫ��׹��box
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
		//ɾ��
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
		//ɾ��
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
	// ����͸��ͶӰ
	viewer->getCamera()->setProjectionMatrixAsPerspective(
		30.0, // ��Ұ�Ƕ�
		static_cast<double>(GetSystemMetrics(SM_CXSCREEN)) / GetSystemMetrics(SM_CYSCREEN), // ��߱�
		0.1,  // ��ƽ��
		10000.0 // Զƽ��
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
