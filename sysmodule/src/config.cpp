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

} // namespace

void configLoad(ToneMapConfig *cfg)
{
    cfg->enabled         = ini_getbool("tonemap", "enabled", 1, CONFIG_PATH) != 0;
    cfg->curve           = curveFromInt(ini_getl("tonemap", "curve", 3, CONFIG_PATH));
    cfg->exposure        = static_cast<float>(ini_getf("tonemap", "exposure", 1.0, CONFIG_PATH));
    cfg->white_point     = static_cast<float>(ini_getf("tonemap", "white_point", 4.0, CONFIG_PATH));
    cfg->expand_strength = static_cast<float>(ini_getf("tonemap", "expand", 0.0, CONFIG_PATH));
    cfg->contrast         = static_cast<float>(ini_getf("tonemap", "contrast", 1.0, CONFIG_PATH));
    cfg->poll_interval_ms = static_cast<u32>(ini_getl("tonemap", "poll_interval_ms", 300, CONFIG_PATH));
}
