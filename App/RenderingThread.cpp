#include "RenderingThread.h"

RenderingThread::RenderingThread(osg::ref_ptr<osgViewer::Viewer> viewer)
{
	this->viewer = viewer;
	this->_done = false;
}

RenderingThread::~RenderingThread()
{
	_done = true;
	if (isRunning()) {
		cancel();
		join();
	}
}

void RenderingThread::run()
{
	if (!viewer) {
		_done = true;
		return;
	}

	do
	{
		viewer->frame();
	} while (!testCancel());
	//} while (true);
}

