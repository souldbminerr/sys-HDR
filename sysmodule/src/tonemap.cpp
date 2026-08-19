#include "tonemap.hpp"

#include "boot_wait.hpp"
#include "log.hpp"
#include "omm.hpp"
#include "tonemap_curve.hpp"

bool ToneMap::init(const ToneMapConfig &cfg)
{
    m_cfg = cfg;

    LOG("tonemap: init begin");
    if (!m_disp.init()) {
        LOG("tonemap: display controller init FAILED");
        return false;
    }
    LOG("tonemap: display controller init ok");

    if (m_cfg.enabled)
    {
        // Avoid race condition
        WaitForQLaunch();
    }

    ommGetOperationMode(&m_lastMode);

    if (m_cfg.enabled)
        applyCurrent();

    LOG("tonemap: init done");
    return true;
}

void ToneMap::applyCurrent()
{
    LOG("tonemap: applyCurrent curve=%d exposure=%.3f white_point=%.3f expand=%.3f contrast=%.3f",
        static_cast<int>(m_cfg.curve), m_cfg.exposure, m_cfg.white_point, m_cfg.expand_strength, m_cfg.contrast);
    hdr::Cmu cmu = hdr::buildTonemapCmu(m_cfg.curve, m_cfg.exposure, m_cfg.white_point, m_cfg.expand_strength, m_cfg.contrast);
    LOG("tonemap: cmu built, applying");
    [[maybe_unused]] bool ok = m_disp.apply(cmu);
    LOG("tonemap: applyCurrent done ok=%d", ok);
}

void ToneMap::disable()
{
    LOG("tonemap: disable begin");
    hdr::Cmu cmu = hdr::buildTonemapCmu(hdr::TonemapOperator::Linear, 1.0f, 1.0f);
    cmu.enable = 0;
    [[maybe_unused]] bool ok = m_disp.apply(cmu);
    LOG("tonemap: disable done ok=%d", ok);
}

void ToneMap::run()
{
    LOG("tonemap: run loop entered");
    while (true)
    {
        u64 pollIntervalNs = static_cast<u64>(m_cfg.poll_interval_ms) * 1'000'000ull;
        if (pollIntervalNs == 0)
            pollIntervalNs = 300'000'000ull;

        svcSleepThread(pollIntervalNs);

        ToneMapConfig fresh;
        configLoad(&fresh);
        m_cfg = fresh;

        ommGetOperationMode(&m_lastMode);

        // Reapply every tick to prevent sleep mode issue
        if (m_cfg.enabled)
            applyCurrent();
        else
            disable();
    }
}
