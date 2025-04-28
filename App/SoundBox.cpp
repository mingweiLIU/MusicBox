#include "SoundBox.h"
#include <osgUtil/SmoothingVisitor>
#include <osg/PrimitiveSet>
#include "ColorGenerator.h"

float SoundBox::OffSetBoxTop = 10;

SoundBox::SoundBox(float length, float width, float hight, osg::Vec3 pos)
	:boxLength(length), boxWidth(width), boxHeight(hight), boxPos(pos), times(4.0), maxValue(255.0)
{
	boxVertex = new osg::Vec3Array;
	boxVertex->push_back(boxPos + osg::Vec3(-width / 2.0f, -length / 2.0f, 0.0f));
	boxVertex->push_back(boxPos + osg::Vec3(width / 2.0f, -length / 2.0f, 0.0f));
	boxVertex->push_back(boxPos + osg::Vec3(width / 2.0f, length / 2.0f, 0.0f));
	boxVertex->push_back(boxPos + osg::Vec3(-width / 2.0f, length / 2.0f, 0.0f));
	boxVertex->push_back(boxPos + osg::Vec3(-width / 2.0f, -length / 2.0f, boxHeight));
	boxVertex->push_back(boxPos + osg::Vec3(width / 2.0f, -length / 2.0f, boxHeight));
	boxVertex->push_back(boxPos + osg::Vec3(width / 2.0f, length / 2.0f, boxHeight));
	boxVertex->push_back(boxPos + osg::Vec3(-width / 2.0f, length / 2.0f, boxHeight));

	boxVertexColor = new osg::Vec4Array();
	boxVertexColor->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
	boxVertexColor->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
	boxVertexColor->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
	boxVertexColor->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));

	float changedColor = boxHeight / maxValue;
	for (int i = 0; i < 4; i++)
	{
		boxVertexColor->push_back(osg::Vec4(changedColor, 1.0f - changedColor, 0.0f, 1.0f));
	}

	osg::ref_ptr<osg::DrawElementsUInt> drawIndex = new osg::DrawElementsUInt(GL_QUADS,0);
	drawIndex->push_back(0); drawIndex->push_back(3); drawIndex->push_back(2); drawIndex->push_back(1);
	drawIndex->push_back(0); drawIndex->push_back(1); drawIndex->push_back(5); drawIndex->push_back(4);
	drawIndex->push_back(1); drawIndex->push_back(2); drawIndex->push_back(6); drawIndex->push_back(5);
	drawIndex->push_back(5); drawIndex->push_back(6); drawIndex->push_back(7); drawIndex->push_back(4);
	drawIndex->push_back(2); drawIndex->push_back(3); drawIndex->push_back(7); drawIndex->push_back(6);
	drawIndex->push_back(3); drawIndex->push_back(0); drawIndex->push_back(4); drawIndex->push_back(7);

	box = new osg::Geometry;
	box->setDataVariance(osg::Object::DYNAMIC);
	box->setVertexArray(boxVertex);
	box->setColorArray(boxVertexColor);
	box->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	box->addPrimitiveSet(drawIndex);

	osgUtil::SmoothingVisitor::smooth(*box);

	this->addDrawable(box.get());
}
SoundBox::~SoundBox(void)
{
}

void SoundBox::UpdateBoxColor()
{
	float changedColor = boxHeight / (times*maxValue);
	RGB_M colorRGB = ColorGenerator::getInstance()->getColor(changedColor);
	
	for (int i = 0; i < 8; i++)
	{
		//boxVertexColor->at(i) = osg::Vec4(changedColor, 1.0f - changedColor, 0.0f, 1.0f);
		boxVertexColor->at(i) = osg::Vec4(colorRGB.r/255.0, colorRGB.g / 255.0, colorRGB.b / 255.0,1.0f);
	}
}
void SoundBox::UpdateHeight(int newHeight)
{
	boxHeight = newHeight;
	boxVertex->at(4) = boxPos + (osg::Vec3(-boxWidth / 2.0f, -boxLength / 2.0f, boxHeight));
	boxVertex->at(5) = boxPos + (osg::Vec3(boxWidth / 2.0f, -boxLength / 2.0f, boxHeight));
	boxVertex->at(6) = boxPos + (osg::Vec3(boxWidth / 2.0f, boxLength / 2.0f, boxHeight));
	boxVertex->at(7) = boxPos + (osg::Vec3(-boxWidth / 2.0f, boxLength / 2.0f, boxHeight));
	UpdateBoxColor();
}
void SoundBox::UpdatePosByHeight(int deltaHeight) {
	for (int i = 0; i < 8; ++i) {
		boxVertex->at(i) += osg::Vec3(0, 0, deltaHeight);
	}
	boxPos += osg::Vec3(0, 0, deltaHeight);
	UpdateBoxColor();
}

osg::ref_ptr<osg::Geometry> SoundBox::GetBox()
{
	return box;
}
int* ChangeSoundBoxHeight::spectum;