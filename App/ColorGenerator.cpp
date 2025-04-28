#include "ColorGenerator.h"
#include <cmath>
#include<algorithm>

ColorGenerator* ColorGenerator::colorGenIntance = nullptr;

ColorGenerator::ColorGenerator(std::vector<ColorStop> stops):colorStops(stops){
    //������
    std::vector<ColorStop> blue2White = {
        {0.0,RGB_M(2,48,71)},{0.25,RGB_M(2,48,71)},{0.5,RGB_M(39,158,188)},{0.75,RGB_M(144,201,230)},{1.0,RGB_M(255,255,255)}
    };
    std::vector<ColorStop> blue2Yellow = {
        {0.0,RGB_M(68,4,90)},{1.0 / 6,RGB_M(65,62,133)}
        ,{1.0 / 3,RGB_M(48,104,141)},{0.5,RGB_M(31,146,139)}
        ,{2.0 / 3,RGB_M(53,183,119)},{5.0 / 6,RGB_M(145,213,66)}
        ,{1.0,RGB_M(248,230,32)}
    };

    std::vector<ColorStop> blueYellow2Blue = {
        {0.0,RGB_M(119, 232, 11)},{0.2,RGB_M(0, 215, 81)}
        ,{0.4,RGB_M(0, 194, 125)},{0.6,RGB_M(0, 170, 163)}
        ,{0.8,RGB_M(0, 145, 191)},{1.0,RGB_M(0, 120, 204)}
    };

    std::vector<ColorStop> blue2Orange = {
        {0.0,RGB_M(28, 176, 255)},{0.5,RGB_M(28, 246, 104)},{1.0,RGB_M(255, 206, 30)}
    };

    preSets.emplace(GradientType::BLUE2WHITE, blue2White);
    preSets.emplace(GradientType::BLUE2YELLOW, blue2Yellow);
    preSets.emplace(GradientType::BLUEYELLOW2BLUE, blueYellow2Blue);
    preSets.emplace(GradientType::BLUE2ORANGE, blue2Orange);
    
	if (stops.size() == 0) {
		defaultGradient = GradientType::BLUE2ORANGE;
        this->colorStops = preSets[defaultGradient];
    }
}

 ColorGenerator* ColorGenerator::getInstance() {
    if (ColorGenerator::colorGenIntance==nullptr)
    {
        ColorGenerator::colorGenIntance = new ColorGenerator(std::vector<ColorStop>());
    }
    return ColorGenerator::colorGenIntance;
}

void ColorGenerator::setPreSetGradient(GradientType gridientType) {
    this->colorStops = preSets[gridientType];
}

HSL ColorGenerator::rgbToHsl(const RGB_M& rgb) {
    HSL hsl;

    double r = rgb.r / 255.0;
    double g = rgb.g / 255.0;
    double b = rgb.b / 255.0;

    double max = std::max({ r, g, b });
    double min = std::min({ r, g, b });
    double delta = max - min;

    // ��������
    hsl.l = (max + min) / 2.0;

    if (delta < 0.00001) {
        // ��ɫ����ɫ��
        hsl.h = 0;
        hsl.s = 0;
    }
    else {
        // ���㱥�Ͷ�
        hsl.s = (hsl.l <= 0.5) ? (delta / (max + min)) : (delta / (2.0 - max - min));

        // ����ɫ��
        if (r >= max) {
            hsl.h = (g - b) / delta + (g < b ? 6.0 : 0.0);
        }
        else if (g >= max) {
            hsl.h = (b - r) / delta + 2.0;
        }
        else {
            hsl.h = (r - g) / delta + 4.0;
        }

        hsl.h *= 60.0; // ת��������
    }

    return hsl;
}

// HSLתRGBMINI
RGB_M  ColorGenerator::hslToRgb(const HSL& hsl) {
    if (hsl.s <= 0.00001) {
        // ��ɫ
        unsigned char value = static_cast<unsigned char>(hsl.l * 255);
        return RGB_M(value, value, value);
    }

    double q = (hsl.l < 0.5) ? (hsl.l * (1.0 + hsl.s)) : (hsl.l + hsl.s - hsl.l * hsl.s);
    double p = 2.0 * hsl.l - q;

    double h = hsl.h / 360.0; // ��һ���� [0, 1]

    auto hueToRgb = [&](double t) {
        if (t < 0.0) t += 1.0;
        if (t > 1.0) t -= 1.0;

        if (t < 1.0 / 6.0) return p + (q - p) * 6.0 * t;
        if (t < 1.0 / 2.0) return q;
        if (t < 2.0 / 3.0) return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
        return p;
    };

    double r = hueToRgb(h + 1.0 / 3.0);
    double g = hueToRgb(h);
    double b = hueToRgb(h - 1.0 / 3.0);

    return RGB_M(
        static_cast<unsigned char>(r * 255),
        static_cast<unsigned char>(g * 255),
        static_cast<unsigned char>(b * 255)
    );
}


double ColorGenerator::lerp(double a, double b, double t) {
    return a + t * (b - a);
}


double ColorGenerator::lerpHue(double h1, double h2, double t) {
    double diff = std::fmod(h2 - h1 + 360.0, 360.0);
    if (diff > 180.0) {
        diff = diff - 360.0;
    }
    return std::fmod(h1 + t * diff + 360.0, 360.0);
}

RGB_M ColorGenerator::toSRGB(const RGB_M& linear, double gamma) {
    double r = applyGamma(linear.r / 255.0, gamma);
    double g = applyGamma(linear.g / 255.0, gamma);
    double b = applyGamma(linear.b / 255.0, gamma);

    return RGB_M(
        static_cast<unsigned char>(r * 255),
        static_cast<unsigned char>(g * 255),
        static_cast<unsigned char>(b * 255)
    );
}

RGB_M ColorGenerator::toLinearRGB(const RGB_M& srgb, double gamma) {
    double r = removeGamma(srgb.r / 255.0, gamma);
    double g = removeGamma(srgb.g / 255.0, gamma);
    double b = removeGamma(srgb.b / 255.0, gamma);

    return RGB_M(
        static_cast<unsigned char>(r * 255),
        static_cast<unsigned char>(g * 255),
        static_cast<unsigned char>(b * 255)
    );
}

// Ӧ��GammaУ����������RGBת��ΪsRGB��
double ColorGenerator::applyGamma(double linearValue, double gamma) {
    return std::pow(linearValue, 1.0 / gamma);
}

// ����GammaУ������sRGBת��Ϊ����RGB��
double ColorGenerator::removeGamma(double srgbValue, double gamma) {
    return std::pow(srgbValue, gamma);
}

//����ɫ�ڵ㽥��
RGB_M ColorGenerator::getColor(double position, GradientType gradientType , bool useGamma, double gamma) {
	this->colorStops = preSets[gradientType == GradientType::DEFAULT ? defaultGradient : gradientType ];

    // ȷ��λ����[0,1]��Χ��
    position = std::max(0.0, std::min(1.0, position));

    // ����߽����
    if (position <= colorStops.front().position) {
        return colorStops.front().color;
    }
    if (position >= colorStops.back().position) {
        return colorStops.back().color;
    }

    // �ҵ�λ�����ڵ�����
    size_t i = 0;
    while (i < colorStops.size() - 1 && position > colorStops[i + 1].position) {
        i++;
    }

    // ��ȡ�����������ɫֹͣ��
    const ColorStop& start = colorStops[i];
    const ColorStop& end = colorStops[i + 1];

    // ��������������ڵ����λ��
    double localT = (position - start.position) / (end.position - start.position);

    if (useGamma) {
        // ����1����RGBת��Ϊ���Կռ䣬�����Կռ��ֵ��Ȼ��ת��sRGB
        RGB_M startLinear = toLinearRGB(start.color, gamma);
        RGB_M endLinear = toLinearRGB(end.color, gamma);

        // �����Կռ��н���RGB��ֵ
        RGB_M linearResult(
            static_cast<unsigned char>(lerp(startLinear.r, endLinear.r, localT)),
            static_cast<unsigned char>(lerp(startLinear.g, endLinear.g, localT)),
            static_cast<unsigned char>(lerp(startLinear.b, endLinear.b, localT))
        );

        // ת��sRGB�ռ�
        return toSRGB(linearResult, gamma);
    }
    else {
        // ����2��ʹ��HSL�ռ��ֵ����Ӧ��GammaУ����
        HSL startHsl = rgbToHsl(start.color);
        HSL endHsl = rgbToHsl(end.color);

        HSL resultHsl(
            lerpHue(startHsl.h, endHsl.h, localT),
            lerp(startHsl.s, endHsl.s, localT),
            lerp(startHsl.l, endHsl.l, localT)
        );

        // ת��RGB������
        return hslToRgb(resultHsl);
    }
}
