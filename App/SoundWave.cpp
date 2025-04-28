#include "SoundWave.h"
#include "SoundBox.h"
#include "BSpline.h"
#include "ColorGenerator.h"
#include <osgUtil/Tessellator>
#include <osg/PolygonMode>

SoundWave::SoundWave(bool slave )
{
	//这是否是副波
	this->slave = slave;

	if (!ChangeSoundBoxHeight::spectum) {
		ChangeSoundBoxHeight::spectum = new int[100];
		for (int i = 0; i < 100; i++)
		{
			*(ChangeSoundBoxHeight::spectum + i) = 50;
		}
	}
	
	osg::Vec3 centerPos(0.0f, 0.0f, 0.0f);
	float interval = 15.0f;
	float xSize = 30.0f;
	float ySize = 30.0f;
	float zSize = 220.f;

	float zSizeDrop = 10.0f;

	for (int i = 0; i <100 ; i++)
	{
		originPoints.push_back(osg::Vec2((interval + xSize) * i, *(ChangeSoundBoxHeight::spectum + i)));
	}
	// 创建Catmull-Rom样条
	BSpline  bspline(originPoints, 3); // 0.5是默认张力

	// 生成平滑后的曲线点
	std::vector<osg::Vec2> smoothedPoints = bspline.generateCurvePoints(1000);

	vertex = new osg::Vec3Array;
	colors = new osg::Vec4Array;	

	//设置的彩色条
	GradientType slaveGradientType = slave ? GradientType::BLUEYELLOW2BLUE : GradientType::DEFAULT;
	float externalOffset = slave ? offsetHeight : 0;//主从高度不一样 从wave要高点

	for (int i = smoothedPoints.size()-1; i > -1; i--)
	{
		auto& point = smoothedPoints[i];
		vertex->push_back(osg::Vec3(point.x(), 0, 0));
		RGB_M firstColor = ColorGenerator::getInstance()->getColor(0.0, slaveGradientType);
		colors->push_back(osg::Vec4(firstColor.r / 255.0, firstColor.g / 255.0, firstColor.b / 255.0, 1.0f));

		vertex->push_back(osg::Vec3(point.x(), 0, point.y()+ externalOffset));
		RGB_M oneColor = ColorGenerator::getInstance()->getColor(point.y()/(4*255.0), slaveGradientType);
		colors->push_back(osg::Vec4(oneColor.r / 255.0, oneColor.g / 255.0, oneColor.b / 255.0, 1.0f));
	}

	waveGeometry = new osg::Geometry;
	waveGeometry->setDataVariance(osg::Object::DYNAMIC);
	waveGeometry->setVertexArray(vertex);
	waveGeometry->setColorArray(colors);
	waveGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	waveGeometry->addPrimitiveSet(new osg::DrawArrays(
		osg::PrimitiveSet::QUAD_STRIP,
		0,
		vertex->size()
	));

	// Or alternatively, disable lighting if that's the color issue
	waveGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	osg::ref_ptr<osg::StateSet> stateset = waveGeometry->getOrCreateStateSet();

	// 设置多边形模式为线框（LINE）
	//osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode;
	//polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
	//stateset->setAttributeAndModes(polygonMode, osg::StateAttribute::ON);

	this->addDrawable(waveGeometry);
}

void SoundWave::Update()
{
	for (int i =0; i <100; i++)
	{
		if (slave)
		{
			//这里要对比下之前的高度 如果比之前的高度高 那么就更新 如果低 那么之前高度直接降低
			if (*(ChangeSoundBoxHeight::spectum + i) > originPoints[i].y() + dropHeight - offsetHeight)
			{
				originPoints[i].y() = *(ChangeSoundBoxHeight::spectum + i);
			}
			else if (*(ChangeSoundBoxHeight::spectum + i) < originPoints[i].y() + dropHeight - offsetHeight)
			{
				originPoints[i].y() += dropHeight;
			}
		}
		else {
			originPoints[i].y() = *(ChangeSoundBoxHeight::spectum + i);
		}
	}

	// 创建B样条曲线
	BSpline  bspline(originPoints, 3); // 0.5是默认张力

	// 生成平滑后的曲线点
	std::vector<osg::Vec2> smoothedPoints = bspline.generateCurvePoints(1000);	//设置的彩色条
	GradientType slaveGradientType = slave ? GradientType::BLUEYELLOW2BLUE : GradientType::DEFAULT;

	for (int i = 0; i < smoothedPoints.size(); i++)
	{
		auto& point = smoothedPoints[i];
		vertex->at((smoothedPoints.size()-1-i)*2+1)=(osg::Vec3(point.x(), 0, point.y()));
		RGB_M oneColor = ColorGenerator::getInstance()->getColor(point.y() / (4 * 255.0), slaveGradientType);
		colors->at((smoothedPoints.size() - 1 - i) * 2 + 1)=(osg::Vec4(oneColor.r / 255.0, oneColor.g / 255.0, oneColor.b / 255.0, 1.0f));
	}
}

