#include "boot_wait.hpp"

#include <switch.h>

#include "log.hpp"

namespace {
    constexpr u64 Qlaunch = 0x0100000000001000ULL;
}

void WaitForQLaunch()
{
    LOG("boot: waiting for qlaunch");

    u64 pid = 0;
    Result rc;
    do
    {
        rc = pmdmntGetProcessId(&pid, Qlaunch);
        svcSleepThread(50'000'000ull); // 50ms
    } while (R_FAILED(rc));

    LOG("boot: finished waiting");
}
