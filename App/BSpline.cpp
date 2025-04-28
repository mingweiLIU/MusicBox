#include "BSpline.h"

double BSpline::basisFunction(int i, int k, double t)
{
	if (k == 1) {
		if (knots[i] <= t && t < knots[i + 1]) {
			return 1.0;
		}
		return 0.0;
	}

	double d1 = knots[i + k - 1] - knots[i];
	double d2 = knots[i + k] - knots[i + 1];

	double c1 = (d1 > 1e-10) ? (t - knots[i]) / d1 * basisFunction(i, k - 1, t) : 0.0;
	double c2 = (d2 > 1e-10) ? (knots[i + k] - t) / d2 * basisFunction(i + 1, k - 1, t) : 0.0;

	return c1 + c2;
}

BSpline::BSpline(const std::vector<osg::Vec2>& points, int degree /*= 3*/)
	: controlPoints(points), degree(degree) {
	// 创建节点向量
	createKnots();
}

void BSpline::createKnots()
{
	int n = controlPoints.size() - 1; // 控制点数量减1
	int m = n + degree + 1; // 节点向量的长度

	knots.resize(m + 1);

	// 构造节点向量
	for (int i = 0; i <= m; i++) {
		if (i < degree + 1) {
			knots[i] = 0.0;
		}
		else if (i >= m - degree) {
			knots[i] = 1.0;
		}
		else {
			knots[i] = static_cast<double>(i - degree) / (m - 2 * degree);
		}
	}
}

osg::Vec2 BSpline::evaluate(double t)
{
	// 确保t在有效范围内
	t = std::max(knots[degree], std::min(t, knots[knots.size() - degree - 1] - 1e-10));

	osg::Vec2 result(0.0f, 0.0f);

	for (size_t i = 0; i < controlPoints.size(); i++) {
		double basis = basisFunction(i, degree + 1, t);
		result.x() += controlPoints[i].x() * basis;
		result.y() += controlPoints[i].y() * basis;
	}

	return result;
}

std::vector<osg::Vec2> BSpline::generateCurvePoints(int numPoints)
{
	std::vector<osg::Vec2> curvePoints;

	double start = knots[degree];
	double end = knots[knots.size() - degree - 1];
	double step = (end - start) / (numPoints - 1);

	for (int i = 0; i < numPoints; i++) {
		double t = start + i * step;
		curvePoints.push_back(evaluate(t));
	}

	return curvePoints;
}