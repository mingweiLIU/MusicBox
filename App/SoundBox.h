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
	osg::Vec3 boxPos;//�ײ�����λ��
	float times;//�Ͳ��չ���������ϵ��
	float maxValue;//�Ͳ������ֵ���

	float boxLength, boxWidth, boxHeight;

public:
	SoundBox(float length = 120.0f, float width = 120.0f, float hight = 200.0f, osg::Vec3 pos = osg::Vec3(0.0f, 0.0f, 0.0f));
	void UpdateBoxColor();
	void UpdateHeight(int newHeight);
	void UpdatePosByHeight(int deltaHeight);//��鳤�Ȳ��� ����height������λ�� ���deltaHeightΪ�����߶�
	osg::ref_ptr<osg::Geometry> GetBox();
	float GetBottomHeight() { return this->boxPos.z(); };

	~SoundBox(void);
	static float OffSetBoxTop;//����Ϊ������ʱ �����²���box���˵ľ���
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
				//���ڵ�ʱ�򲻸���
				if (*(ChangeSoundBoxHeight::spectum + i) < (soundbox->GetBottomHeight() - SoundBox::OffSetBoxTop)) {
					//�����û�б�����Ķ��� ��ô��ֱ�ӻ���
					soundbox->UpdatePosByHeight(dropHeightPerFrame);
				}
				else if(*(ChangeSoundBoxHeight::spectum + i) > (soundbox->GetBottomHeight() - SoundBox::OffSetBoxTop)) {
					//���������Ķ����� ��ô�͸��µ���λ�� ����Ӧ�����ò�ֵ
					soundbox->UpdatePosByHeight(*(ChangeSoundBoxHeight::spectum + i)-soundbox->GetBottomHeight() + SoundBox::OffSetBoxTop);
				}
			}
			box->dirtyDisplayList();
			box->dirtyBound();
			soundbox->dirtyBound();
		}
	}
};
