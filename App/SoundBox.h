#pragma once

#include <osg/Geometry>
#include <osg/Geode>
#include <bass.h>

#define MaxValue (255.0f);


class SoundBox :public osg::Geode
{
private:
	osg::ref_ptr<osg::Vec3Array> boxVertex;
	osg::ref_ptr<osg::Vec4Array> boxVertexColor;
	osg::ref_ptr<osg::Geometry> box;
	osg::Vec3 boxPos;
	float times;

	float boxLength, boxWidth, boxHeight;

public:
	SoundBox(float length = 120.0f, float width = 120.0f, float hight = 200.0f, osg::Vec3 pos = osg::Vec3(0.0f, 0.0f, 0.0f));
	void UpdateBoxColor();
	void UpdateHeight(int newHeight);
	osg::ref_ptr<osg::Geometry> GetBox();

	~SoundBox(void);
};

class ChangeSoundBoxHeight :public osg::NodeCallback
{
private:
	float boxHeight;
	std::vector<osg::Geode*> boxesVec;
public:

	ChangeSoundBoxHeight(std::vector<osg::Geode*> boxesVec) :boxHeight(100.0f), boxesVec(boxesVec) {};
	~ChangeSoundBoxHeight() {
		if (ChangeSoundBoxHeight::spectum)
		{
			delete[] ChangeSoundBoxHeight::spectum;
		}
	}
	virtual void operator()(osg::Node* nd, osg::NodeVisitor* nv)
	{
		for (int i = 0; i < boxesVec.size(); i++)
		{
			SoundBox* soundbox = dynamic_cast<SoundBox*>(boxesVec[i]);
			osg::ref_ptr<osg::Geometry> box = soundbox->GetBox();
			if (ChangeSoundBoxHeight::spectum)
			{
				soundbox->UpdateHeight(*(ChangeSoundBoxHeight::spectum + i));
			}
			boxHeight += 1;
			if (boxHeight > 250)
			{
				boxHeight = 100.0f;
			}
			box->dirtyDisplayList();
			box->dirtyBound();
			soundbox->dirtyBound();
		}
	}
	static int* spectum;
};