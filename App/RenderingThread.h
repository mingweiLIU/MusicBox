#pragma once
#include <OpenThreads/Thread>
#include <osgViewer/Viewer>
class RenderingThread:public OpenThreads::Thread
{
public:
	RenderingThread(osg::ref_ptr<osgViewer::Viewer> viewer);
	virtual ~RenderingThread();
	virtual void run();

protected:
	osg::ref_ptr<osgViewer::Viewer> viewer;
	bool _done;
};

