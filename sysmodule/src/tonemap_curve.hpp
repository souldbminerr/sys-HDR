#pragma once

#include "nvdisp.hpp"

namespace hdr {

enum class TonemapOperator
{
    Linear = 0,
    Reinhard,
    ReinhardExtended,
    Hable,
    Aces,
};

Cmu buildTonemapCmu(TonemapOperator op, float exposure, float whitePoint, float expandStrength = 0.0f, float contrast = 1.0f);

} // namespace hdr
