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

// ����HSL��ɫ�ṹ��
struct HSL {
	double h; // ɫ�� [0, 360)
	double s; // ���Ͷ� [0, 1]
	double l; // ���� [0, 1]

	HSL(double hue = 0, double saturation = 0, double lightness = 0)
		: h(hue), s(saturation), l(lightness) {}
};


// ����RGB��ɫ�ṹ��
struct RGB_M {
	unsigned char r, g, b;

	RGB_M(unsigned char red = 0, unsigned char green = 0, unsigned char blue = 0)
		: r(red), g(green), b(blue) {}
};

// ������ɫֹͣ��ṹ
struct ColorStop {
	double position; // λ�� [0, 1]
	RGB_M color;

	ColorStop(double pos, const RGB_M& col) : position(pos), color(col) {}

	// �Ƚ����������������
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
	// RGBתHSL
	HSL rgbToHsl(const RGB_M& rgb);
	// HSLתRGB
	RGB_M hslToRgb(const HSL& hsl);
	//���Բ�ֵ����
	double lerp(double a, double b, double t);
	// ɫ���ֵ���⴦�������·����
	double lerpHue(double h1, double h2, double t);
	// ����RGBת��ΪsRGB������GammaУ����
	RGB_M toSRGB(const RGB_M& linear, double gamma = 2.2);
	// RGBת��Ϊ����RGB��Ӧ��GammaУ����
	RGB_M toLinearRGB(const RGB_M& srgb, double gamma = 2.2);
	// ����GammaУ������sRGBת��Ϊ����RGB��
	double removeGamma(double srgbValue, double gamma);
	// Ӧ��GammaУ����������RGBת��ΪsRGB��
	double applyGamma(double linearValue, double gamma);
};
