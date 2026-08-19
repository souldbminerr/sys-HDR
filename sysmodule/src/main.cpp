#include <switch.h>

#include "config.hpp"
#include "log.hpp"
#include "omm.hpp"
#include "tonemap.hpp"

#define INNER_HEAP_SIZE 0x180000

#ifdef __cplusplus
extern "C" {
#endif

u32 __nx_applet_type = AppletType_None;

u32 __nx_fs_num_sessions = 1;

u32 __nx_nv_service_type = NvServiceType_Factory;

u32 __nx_nv_transfermem_size = 0x8000;

void __libnx_initheap(void)
{
    static u8 inner_heap[INNER_HEAP_SIZE];
    extern void* fake_heap_start;
    extern void* fake_heap_end;

    // Configure the newlib heap.
    fake_heap_start = inner_heap;
    fake_heap_end   = inner_heap + sizeof(inner_heap);
}

void __appInit(void)
{
    Result rc;

    rc = smInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

    rc = setsysInitialize();
    if (R_SUCCEEDED(rc)) {
        SetSysFirmwareVersion fw;
        rc = setsysGetFirmwareVersion(&fw);
        if (R_SUCCEEDED(rc))
            hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
        setsysExit();
    }

    rc = fsInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    fsdevMountSdmc();

    rc = nvInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(rc);

    rc = ommInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(rc);

    smExit();
}

void __appExit(void)
{
    ommExit();
    nvExit();
    fsdevUnmountAll();
    fsExit();
}

#ifdef __cplusplus
}
#endif

// Main program entrypoint
int main(int argc, char* argv[])
{
    logInit();
    LOG("main: starting");

    ToneMapConfig cfg;
    configLoad(&cfg);
    LOG("main: config loaded enabled=%d curve=%d exposure=%.3f white_point=%.3f expand=%.3f poll=%ums",
        cfg.enabled, static_cast<int>(cfg.curve), cfg.exposure, cfg.white_point, cfg.expand_strength, cfg.poll_interval_ms);

    static ToneMap toneMap;
    if (toneMap.init(cfg)) {
        LOG("main: toneMap.init ok, entering run loop");
        toneMap.run(); // never returns
    } else {
        LOG("main: toneMap.init FAILED");
    }

    logClose();
    return 0;
}
