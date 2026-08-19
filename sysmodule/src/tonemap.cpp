#include "tonemap.hpp"

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

    ommGetOperationMode(&m_lastMode);

    if (m_cfg.enabled)
        applyCurrent();

    LOG("tonemap: init done");
    return true;
}

void ToneMap::applyCurrent()
{
    LOG("tonemap: applyCurrent curve=%d exposure=%.3f white_point=%.3f expand=%.3f",
        static_cast<int>(m_cfg.curve), m_cfg.exposure, m_cfg.white_point, m_cfg.expand_strength);
    hdr::Cmu cmu = hdr::buildTonemapCmu(m_cfg.curve, m_cfg.exposure, m_cfg.white_point, m_cfg.expand_strength);
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
        LOG("tonemap: poll enabled=%d curve=%d exposure=%.3f white_point=%.3f",
            fresh.enabled, static_cast<int>(fresh.curve), fresh.exposure, fresh.white_point);

        AppletOperationMode mode = m_lastMode;
        ommGetOperationMode(&mode);
        const bool dockChanged = mode != m_lastMode;
        m_lastMode = mode;

        const bool curveChanged = fresh.curve != m_cfg.curve ||
            fresh.exposure != m_cfg.exposure || fresh.white_point != m_cfg.white_point ||
            fresh.expand_strength != m_cfg.expand_strength;

        if (fresh.enabled && (!m_cfg.enabled || curveChanged || dockChanged))
        {
            m_cfg = fresh;
            applyCurrent();
        }
        else if (!fresh.enabled && m_cfg.enabled)
        {
            m_cfg = fresh;
            disable();
        }
        else
        {
            m_cfg = fresh;
        }
    }
}
