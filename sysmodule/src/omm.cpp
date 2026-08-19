#include "omm.hpp"

namespace {
Service g_ommSrv;
}

Result ommInitialize()
{
    return smGetService(&g_ommSrv, "omm");
}

void ommExit()
{
    serviceClose(&g_ommSrv);
}

Result ommGetOperationMode(AppletOperationMode *mode)
{
    u8 tmp = 0;
    Result rc = serviceDispatchOut(&g_ommSrv, 0, tmp);
    if (R_SUCCEEDED(rc) && mode)
        *mode = static_cast<AppletOperationMode>(tmp);
    return rc;
}
