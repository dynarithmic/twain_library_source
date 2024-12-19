#ifdef _WIN32
#include "ctlobstr.h"
#include "ctliface.h"
#include "versioninfo.h"
#include "dtwinverex.h"

namespace dynarithmic
{
    CTL_StringType GetVersionInfo()
    {
        return GetVersionInfo(CTL_StaticData::GetDLLInstanceHandle(), 0);
    }

    CTL_StringType GetVersionInfo(HMODULE dllModule, int indent, CTL_StringType crlf)
    {
        const VersionInfo vInfo(dllModule);
        CTL_StringStreamType strm;
        vInfo.printit(strm, indent, crlf.c_str());
        return strm.str();
    }
}
#endif