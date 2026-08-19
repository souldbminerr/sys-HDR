#pragma once

#include <switch.h>

#include "tonemap_curve.hpp"

struct ToneMapConfig
{
    bool enabled;
    hdr::TonemapOperator curve;
    float exposure;
    float white_point;
    float expand_strength;
    u32 poll_interval_ms;
};

void configLoad(ToneMapConfig *cfg);
