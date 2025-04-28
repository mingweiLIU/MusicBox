#include "ColorGenerator.h"
#include <cmath>
#include<algorithm>

ColorGenerator* ColorGenerator::colorGenIntance = nullptr;

ColorGenerator::ColorGenerator(std::vector<ColorStop> stops):colorStops(stops){
    //蓝到白
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

    // 计算亮度
    hsl.l = (max + min) / 2.0;

    if (delta < 0.00001) {
        // 灰色，无色相
        hsl.h = 0;
        hsl.s = 0;
    }
    else {
        // 计算饱和度
        hsl.s = (hsl.l <= 0.5) ? (delta / (max + min)) : (delta / (2.0 - max - min));

        // 计算色相
        if (r >= max) {
            hsl.h = (g - b) / delta + (g < b ? 6.0 : 0.0);
        }
        else if (g >= max) {
            hsl.h = (b - r) / delta + 2.0;
        }
        else {
            hsl.h = (r - g) / delta + 4.0;
        }

        hsl.h *= 60.0; // 转换到度数
    }

    return hsl;
}

// HSL转RGBMINI
RGB_M  ColorGenerator::hslToRgb(const HSL& hsl) {
    if (hsl.s <= 0.00001) {
        // 灰色
        unsigned char value = static_cast<unsigned char>(hsl.l * 255);
        return RGB_M(value, value, value);
    }

    double q = (hsl.l < 0.5) ? (hsl.l * (1.0 + hsl.s)) : (hsl.l + hsl.s - hsl.l * hsl.s);
    double p = 2.0 * hsl.l - q;

    double h = hsl.h / 360.0; // 归一化到 [0, 1]

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

// 应用Gamma校正（将线性RGB转换为sRGB）
double ColorGenerator::applyGamma(double linearValue, double gamma) {
    return std::pow(linearValue, 1.0 / gamma);
}

// 反向Gamma校正（将sRGB转换为线性RGB）
double ColorGenerator::removeGamma(double srgbValue, double gamma) {
    return std::pow(srgbValue, gamma);
}

//多颜色节点渐变
RGB_M ColorGenerator::getColor(double position, GradientType gradientType , bool useGamma, double gamma) {
	this->colorStops = preSets[gradientType == GradientType::DEFAULT ? defaultGradient : gradientType ];

    // 确保位置在[0,1]范围内
    position = std::max(0.0, std::min(1.0, position));

    // 处理边界情况
    if (position <= colorStops.front().position) {
        return colorStops.front().color;
    }
    if (position >= colorStops.back().position) {
        return colorStops.back().color;
    }

    // 找到位置所在的区间
    size_t i = 0;
    while (i < colorStops.size() - 1 && position > colorStops[i + 1].position) {
        i++;
    }

    // 获取区间的两个颜色停止点
    const ColorStop& start = colorStops[i];
    const ColorStop& end = colorStops[i + 1];

    // 计算在这个区间内的相对位置
    double localT = (position - start.position) / (end.position - start.position);

    if (useGamma) {
        // 方法1：将RGB转换为线性空间，在线性空间插值，然后转回sRGB
        RGB_M startLinear = toLinearRGB(start.color, gamma);
        RGB_M endLinear = toLinearRGB(end.color, gamma);

        // 在线性空间中进行RGB插值
        RGB_M linearResult(
            static_cast<unsigned char>(lerp(startLinear.r, endLinear.r, localT)),
            static_cast<unsigned char>(lerp(startLinear.g, endLinear.g, localT)),
            static_cast<unsigned char>(lerp(startLinear.b, endLinear.b, localT))
        );

        // 转回sRGB空间
        return toSRGB(linearResult, gamma);
    }
    else {
        // 方法2：使用HSL空间插值（不应用Gamma校正）
        HSL startHsl = rgbToHsl(start.color);
        HSL endHsl = rgbToHsl(end.color);

        HSL resultHsl(
            lerpHue(startHsl.h, endHsl.h, localT),
            lerp(startHsl.s, endHsl.s, localT),
            lerp(startHsl.l, endHsl.l, localT)
        );

        // 转回RGB并返回
        return hslToRgb(resultHsl);
    }
}
