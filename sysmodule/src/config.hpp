#pragma once

#include <switch.h>

#include "tonemap_curve.hpp"

struct ToneMapProfile
{
    bool enabled;
    hdr::TonemapOperator curve;
    float exposure;
    float white_point;
    float expand_strength;
    float contrast;
};

struct ToneMapConfig
{
    bool enabled;
    u32 poll_interval_ms;
    ToneMapProfile handheld;
    ToneMapProfile docked;
};

void configLoad(ToneMapConfig *cfg);
