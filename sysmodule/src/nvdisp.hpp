#pragma once

#include <array>
#include <cstdint>

#include <switch.h>

namespace hdr {

struct Cmu
{
    u16 enable;

    s16 krr, kgr, kbr;
    s16 krg, kgg, kbg;
    s16 krb, kgb, kbb;

    std::array<u16, 256> lut1;
    std::array<u16, 960> lut2;

    u16 csc_modified;
    u16 lut1_modified;
    u16 lut2_modified;
};

static_assert(sizeof(Cmu) == 2458, "Cmu layout is not correct");

inline Result nvioctlNvDisp_SetCmu(u32 fd, Cmu *cmu)
{
    return nvIoctl(fd, _NV_IOWR(2, 14, Cmu), cmu);
}

constexpr s16 Q1_8(float v)
{
    return static_cast<s16>(v * 256.0f);
}

class DisplayController
{
public:
    bool init();
    void exit();

    bool apply(Cmu &cmu);

private:
    u32 m_disp0Fd = 0;
    u32 m_disp1Fd = 0;
    bool m_disp0Open = false;
    bool m_disp1Open = false;
};

} // namespace hdr
