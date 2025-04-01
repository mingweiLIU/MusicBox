#pragma once
#include <osgViewer/Viewer>
#include <osgViewer/GraphicsWindow>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/BlendFunc>
#include <osg/Camera>
#include <windows.h>
#include <osgViewer/api/Win32/GraphicsWindowWin32>
#include <osgGA/TrackballManipulator>

class SceneManager
{
public:
	SceneManager();
	HWND getHWND();
public:
	osg::ref_ptr<osgViewer::Viewer> viewer;
	osg::ref_ptr<osg::Group> root;
private:
	std::vector<osg::Geode*> boxes;
private:
	void InitalBoxes();
};
