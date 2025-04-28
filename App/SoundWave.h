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
	//这是否是副波 副波的话 颜色不一样 另外高度需要降低
	bool slave = false;
	float dropHeight = -2;//如果是从wave 那么每一次更新时它 当当前音谱高度没有高于该点高度时 其应该时降低的 每次降低dropHeight高度
	float offsetHeight = 20;//从wave应该最低高于主wave的高度
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