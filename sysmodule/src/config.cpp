#include "config.hpp"

#include <minIni.h>

#define CONFIG_PATH "sdmc:/config/" CONFIG_DIR "/config.ini"

namespace {

hdr::TonemapOperator curveFromInt(long v)
{
    switch (v)
    {
    case 1: return hdr::TonemapOperator::Reinhard;
    case 2: return hdr::TonemapOperator::ReinhardExtended;
    case 3: return hdr::TonemapOperator::Hable;
    case 4: return hdr::TonemapOperator::Aces;
    case 0:
    default:
        return hdr::TonemapOperator::Linear;
    }
}

void profileLoad(ToneMapProfile *profile, const char *section)
{
    profile->enabled         = ini_getbool(section, "enabled", 1, CONFIG_PATH) != 0;
    profile->curve           = curveFromInt(ini_getl(section, "curve", 3, CONFIG_PATH));
    profile->exposure        = static_cast<float>(ini_getf(section, "exposure", 1.0, CONFIG_PATH));
    profile->white_point     = static_cast<float>(ini_getf(section, "white_point", 4.0, CONFIG_PATH));
    profile->expand_strength = static_cast<float>(ini_getf(section, "expand", 0.0, CONFIG_PATH));
    profile->contrast        = static_cast<float>(ini_getf(section, "contrast", 1.0, CONFIG_PATH));
}

} // namespace

void configLoad(ToneMapConfig *cfg)
{
    cfg->enabled         = ini_getbool("tonemap", "enabled", 1, CONFIG_PATH) != 0;
    cfg->poll_interval_ms = static_cast<u32>(ini_getl("tonemap", "poll_interval_ms", 300, CONFIG_PATH));

    profileLoad(&cfg->handheld, "tonemap_handheld");
    profileLoad(&cfg->docked, "tonemap_docked");
}
