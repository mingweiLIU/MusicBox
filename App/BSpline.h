#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <osg/Vec2>


class BSpline {
private:
	std::vector<osg::Vec2> controlPoints;
	int degree; // ���ߵĴ��� (���� = ���� + 1)
	std::vector<double> knots; // �ڵ�����

	// ���������
	double basisFunction(int i, int k, double t);
public:
	BSpline(const std::vector<osg::Vec2>& points, int degree = 3);

	void createKnots();

	osg::Vec2 evaluate(double t);

	// ����ƽ�������ϵĵ�
	std::vector<osg::Vec2> generateCurvePoints(int numPoints);
};