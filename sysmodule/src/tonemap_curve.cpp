#include "tonemap_curve.hpp"

#include <algorithm>
#include <cmath>

namespace hdr {

namespace {

float srgbDecode(float x)
{
    if (x <= 0.04045f)
        return x / 12.92f;
    return std::pow((x + 0.055f) / 1.055f, 2.4f);
}

float srgbEncode(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    if (x <= 0.0031308f)
        return x * 12.92f;
    return 1.055f * std::pow(x, 1.0f / 2.4f) - 0.055f;
}

float expandHighlights(float x, float strength)
{
    if (strength <= 0.0f)
        return x;
    const float k = std::min(strength, 0.95f);
    return x / std::max(1.0f - k * x, 1e-4f);
}

float hableCurve(float x)
{
    constexpr float A = 0.15f, B = 0.50f, C = 0.10f, D = 0.20f, E = 0.02f, F = 0.30f;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float applyContrast(float y, float contrast)
{
    if (contrast == 1.0f)
        return y;
    constexpr float pivot = 0.5f;
    return pivot + (y - pivot) * contrast;
}

float applyOperator(TonemapOperator op, float x, float whitePoint)
{
    switch (op)
    {
    case TonemapOperator::Reinhard:
        return x / (1.0f + x);

    case TonemapOperator::ReinhardExtended:
    {
        const float w2 = whitePoint * whitePoint;
        return (x * (1.0f + x / w2)) / (1.0f + x);
    }

    case TonemapOperator::Hable:
        return hableCurve(x) / hableCurve(whitePoint);

    case TonemapOperator::Aces:
    {
        constexpr float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
        return (x * (a * x + b)) / (x * (c * x + d) + e);
    }

    case TonemapOperator::Linear:
    default:
        return x;
    }
}

void sampleToneCurve(u16 *out, std::size_t count, float lo, float hi,
                      TonemapOperator op, float exposure, float whitePoint, float expandStrength, float contrast)
{
    const float step = (hi - lo) / static_cast<float>(count - 1);
    float x = lo;
    for (std::size_t i = 0; i < count; i++, x += step)
    {
        const float expanded = expandHighlights(x, expandStrength);
        float linear = applyOperator(op, expanded * exposure, whitePoint);
        linear = applyContrast(linear, contrast);
        out[i] = static_cast<u16>(std::lround(srgbEncode(linear) * 0xFF));
    }
}

} // namespace

Cmu buildTonemapCmu(TonemapOperator op, float exposure, float whitePoint, float expandStrength, float contrast)
{
    Cmu cmu{};

    cmu.enable = 1;

    cmu.krr = Q1_8(1.0f); cmu.kgr = 0; cmu.kbr = 0;
    cmu.krg = 0; cmu.kgg = Q1_8(1.0f); cmu.kbg = 0;
    cmu.krb = 0; cmu.kgb = 0; cmu.kbb = Q1_8(1.0f);

    {
        const std::size_t count = cmu.lut1.size();
        const float step = 1.0f / static_cast<float>(count - 1);
        float x = 0.0f;
        for (std::size_t i = 0; i < count; i++, x += step)
            cmu.lut1[i] = static_cast<u16>(std::lround(std::clamp(srgbDecode(x), 0.0f, 1.0f) * 0x0FFF));
    }

    sampleToneCurve(cmu.lut2.data(), 512, 0.0f, 0.125f, op, exposure, whitePoint, expandStrength, contrast);
    sampleToneCurve(cmu.lut2.data() + 512, cmu.lut2.size() - 512, 0.125f, 1.0f, op, exposure, whitePoint, expandStrength, contrast);

    return cmu;
}

} // namespace hdr
