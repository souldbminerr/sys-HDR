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
    const ToneMapProfile &activeProfile() const;
    bool shouldApply() const;

    ToneMapConfig m_cfg{};
    hdr::DisplayController m_disp;
    AppletOperationMode m_lastMode = AppletOperationMode_Handheld;
};
