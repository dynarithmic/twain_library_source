#ifdef _WIN32
#include "ctlobstr.h"
#include "ctliface.h"
#include "versioninfo.h"
#include "dtwinverex.h"

namespace dynarithmic
{
    CTL_StringType GetVersionInfo()
    {
        return GetVersionInfo(CTL_StaticData::s_DLLInstance, 0);
    }

    CTL_StringType GetVersionInfo(HMODULE dllModule, int indent)
    {
        const VersionInfo vInfo(dllModule);
        CTL_StringStreamType strm;
        vInfo.printit(strm, indent, _T("\r\n"));
        return strm.str();
    }
}
#endif