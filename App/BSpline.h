#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <osg/Vec2>


class BSpline {
private:
	std::vector<osg::Vec2> controlPoints;
	int degree; // 曲线的次数 (阶数 = 次数 + 1)
	std::vector<double> knots; // 节点向量

	// 计算基函数
	double basisFunction(int i, int k, double t);
public:
	BSpline(const std::vector<osg::Vec2>& points, int degree = 3);

	void createKnots();

	osg::Vec2 evaluate(double t);

	// 生成平滑曲线上的点
	std::vector<osg::Vec2> generateCurvePoints(int numPoints);
};