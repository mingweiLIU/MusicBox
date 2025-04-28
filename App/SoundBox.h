#pragma once

#include <osg/Geometry>
#include <osg/Geode>
#include <bass.h>
#include <mutex>

class SoundBox :public osg::Geode
{
private:
	osg::ref_ptr<osg::Vec3Array> boxVertex;
	osg::ref_ptr<osg::Vec4Array> boxVertexColor;
	osg::ref_ptr<osg::Geometry> box;
	osg::Vec3 boxPos;//底部中心位置
	float times;//和波普关联的扩大系数
	float maxValue;//和波普最大值相关

	float boxLength, boxWidth, boxHeight;

public:
	SoundBox(float length = 120.0f, float width = 120.0f, float hight = 200.0f, osg::Vec3 pos = osg::Vec3(0.0f, 0.0f, 0.0f));
	void UpdateBoxColor();
	void UpdateHeight(int newHeight);
	void UpdatePosByHeight(int deltaHeight);//体块长度不变 根据height来更新位置 这个deltaHeight为新增高度
	osg::ref_ptr<osg::Geometry> GetBox();
	float GetBottomHeight() { return this->boxPos.z(); };

	~SoundBox(void);
	static float OffSetBoxTop;//当其为顶部块时 其与下部的box顶端的距离
};

class ChangeSoundBoxHeight :public osg::NodeCallback
{
private:
	std::vector<osg::Geode*>& boxesVec;
public:

	ChangeSoundBoxHeight(std::vector<osg::Geode*>& boxesVec) : boxesVec(boxesVec) {};
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
			box->dirtyDisplayList();
			box->dirtyBound();
			soundbox->dirtyBound();
		}
	}
	static int* spectum;
};

class DropSoundBoxCallback :public osg::NodeCallback {
private:
	std::vector<osg::Geode*>& boxesVec;
	float dropHeightPerFrame = -2;
public:
	DropSoundBoxCallback(std::vector<osg::Geode*>& boxesVec) : boxesVec(boxesVec) {};
	virtual void operator()(osg::Node* nd, osg::NodeVisitor* nv)
	{
		for (int i = 0; i < boxesVec.size(); i++)
		{
			SoundBox* soundbox = dynamic_cast<SoundBox*>(boxesVec[i]);
			if (!soundbox) continue;

			osg::ref_ptr<osg::Geometry> box = soundbox->GetBox();
			if (ChangeSoundBoxHeight::spectum)
			{
				//等于的时候不更新
				if (*(ChangeSoundBoxHeight::spectum + i) < (soundbox->GetBottomHeight() - SoundBox::OffSetBoxTop)) {
					//如果还没有被下面的顶到 那么就直接回落
					soundbox->UpdatePosByHeight(dropHeightPerFrame);
				}
				else if(*(ChangeSoundBoxHeight::spectum + i) > (soundbox->GetBottomHeight() - SoundBox::OffSetBoxTop)) {
					//如果被下面的顶到了 那么就更新到新位置 这里应该是用差值
					soundbox->UpdatePosByHeight(*(ChangeSoundBoxHeight::spectum + i)-soundbox->GetBottomHeight() + SoundBox::OffSetBoxTop);
				}
			}
			box->dirtyDisplayList();
			box->dirtyBound();
			soundbox->dirtyBound();
		}
	}
};
