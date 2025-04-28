#pragma once
#include <osg/Geometry>
#include <osg/Geode>

class SoundWave : public osg::Geode {
public:
	SoundWave(bool slave=false);
	void Update();
	osg::ref_ptr<osg::Geometry> GetGeomtry() { return waveGeometry; }
private:
	osg::ref_ptr<osg::Vec3Array> vertex;
	osg::ref_ptr<osg::Vec4Array> colors;
	osg::ref_ptr<osg::Geometry> waveGeometry;
	std::vector<osg::Vec2> originPoints;
	//���Ƿ��Ǹ��� �����Ļ� ��ɫ��һ�� ����߶���Ҫ����
	bool slave = false;
	float dropHeight = -2;//����Ǵ�wave ��ôÿһ�θ���ʱ�� ����ǰ���׸߶�û�и��ڸõ�߶�ʱ ��Ӧ��ʱ���͵� ÿ�ν���dropHeight�߶�
	float offsetHeight = 20;//��waveӦ����͸�����wave�ĸ߶�
};

class ChangeSoundWave :public osg::NodeCallback
{
public:
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		SoundWave* soundWave = dynamic_cast<SoundWave*>(node);
		if (soundWave&& soundWave->GetGeomtry()) {
			soundWave->Update();
			osg::ref_ptr<osg::Geometry> waveGeom = soundWave->GetGeomtry();
			waveGeom->dirtyDisplayList();
			waveGeom->dirtyBound();
			soundWave->dirtyBound();
		}
	}
};