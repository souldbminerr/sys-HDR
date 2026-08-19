#pragma once

#include "config.hpp"
#include "nvdisp.hpp"

class ToneMap
{
public:
    bool init(const ToneMapConfig &cfg);

    [[noreturn]] void run();

private:
    void applyCurrent();
    void disable();

    ToneMapConfig m_cfg{};
    hdr::DisplayController m_disp;
    AppletOperationMode m_lastMode = AppletOperationMode_Handheld;
};
