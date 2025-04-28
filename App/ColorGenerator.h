#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>

enum class GradientType
{
	DEFAULT,
	BLUE2YELLOW,
	BLUE2RED,
	BLUE2WHITE,
	BLUEYELLOW2BLUE,
	BLUE2ORANGE
};

// 定义HSL颜色结构体
struct HSL {
	double h; // 色相 [0, 360)
	double s; // 饱和度 [0, 1]
	double l; // 亮度 [0, 1]

	HSL(double hue = 0, double saturation = 0, double lightness = 0)
		: h(hue), s(saturation), l(lightness) {}
};


// 定义RGB颜色结构体
struct RGB_M {
	unsigned char r, g, b;

	RGB_M(unsigned char red = 0, unsigned char green = 0, unsigned char blue = 0)
		: r(red), g(green), b(blue) {}
};

// 定义颜色停止点结构
struct ColorStop {
	double position; // 位置 [0, 1]
	RGB_M color;

	ColorStop(double pos, const RGB_M& col) : position(pos), color(col) {}

	// 比较运算符，用于排序
	bool operator<(const ColorStop& other) const {
		return position < other.position;
	}
};

class ColorGenerator
{
private:
	ColorGenerator(std::vector<ColorStop> stops);
	~ColorGenerator() {};
public:
	RGB_M getColor(double pos,GradientType gradientType=GradientType::DEFAULT,bool useGamma = true, double gamma = 2.2);
	void setPreSetGradient(GradientType gridientType);
	static ColorGenerator* getInstance();

private:
	std::vector<ColorStop> colorStops;
	std::map<GradientType, const std::vector<ColorStop>> preSets;
	static ColorGenerator* colorGenIntance;
	GradientType defaultGradient;
private:
	// RGB转HSL
	HSL rgbToHsl(const RGB_M& rgb);
	// HSL转RGB
	RGB_M hslToRgb(const HSL& hsl);
	//线性插值函数
	double lerp(double a, double b, double t);
	// 色相插值特殊处理（按最短路径）
	double lerpHue(double h1, double h2, double t);
	// 线性RGB转换为sRGB（反向Gamma校正）
	RGB_M toSRGB(const RGB_M& linear, double gamma = 2.2);
	// RGB转换为线性RGB（应用Gamma校正）
	RGB_M toLinearRGB(const RGB_M& srgb, double gamma = 2.2);
	// 反向Gamma校正（将sRGB转换为线性RGB）
	double removeGamma(double srgbValue, double gamma);
	// 应用Gamma校正（将线性RGB转换为sRGB）
	double applyGamma(double linearValue, double gamma);
};
