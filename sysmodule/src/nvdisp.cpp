#include "nvdisp.hpp"

#include "log.hpp"
#include "omm.hpp"

namespace hdr {

bool DisplayController::init()
{
    LOG("nvdisp: opening /dev/nvdisp-disp0");
    Result rc0 = nvOpen(&m_disp0Fd, "/dev/nvdisp-disp0");
    LOG("nvdisp: disp0 open rc=0x%x fd=%u", rc0, m_disp0Fd);
    m_disp0Open = R_SUCCEEDED(rc0);

    LOG("nvdisp: opening /dev/nvdisp-disp1");
    Result rc1 = nvOpen(&m_disp1Fd, "/dev/nvdisp-disp1");
    LOG("nvdisp: disp1 open rc=0x%x fd=%u", rc1, m_disp1Fd);
    m_disp1Open = R_SUCCEEDED(rc1);

    return m_disp0Open;
}

void DisplayController::exit()
{
    if (m_disp0Open)
        nvClose(m_disp0Fd);
    if (m_disp1Open)
        nvClose(m_disp1Fd);
    m_disp0Open = m_disp1Open = false;
}

bool DisplayController::apply(Cmu &cmu)
{
    AppletOperationMode mode = AppletOperationMode_Handheld;
    Result modeRc = ommGetOperationMode(&mode);
    if (R_FAILED(modeRc)) {
        LOG("nvdisp: ommGetOperationMode FAILED rc=0x%x, defaulting to handheld/disp0", modeRc);
        mode = AppletOperationMode_Handheld;
    }

    const bool docked = mode == AppletOperationMode_Console;
    const u32 fd = docked ? m_disp1Fd : m_disp0Fd;
    const bool fdOpen = docked ? m_disp1Open : m_disp0Open;

    if (!fdOpen) {
        LOG("nvdisp: apply skipped, target display (docked=%d) not open", docked);
        return false;
    }

    LOG("nvdisp: SetCmu docked=%d begin", docked);
    Result rc = nvioctlNvDisp_SetCmu(fd, &cmu);
    LOG("nvdisp: SetCmu docked=%d done rc=0x%x", docked, rc);

    return R_SUCCEEDED(rc);
}

} // namespace hdr
