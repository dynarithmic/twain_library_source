#ifdef _WIN32
#include "ctlobstr.h"
#include "ctliface.h"
#include "versioninfo.h"
#include "dtwinverex.h"

namespace dynarithmic
{
    CTL_StringType GetVersionInfo()
    {
        return GetVersionInfo(CTL_StaticData::GetDLLInstanceHandle(), 0, StringWrapper::traits_type::GetNewLineString());
    }

    CTL_StringType GetVersionInfo(HMODULE dllModule, int indent, StringWrapper::traits_type::stringview_type crlf)
    {
        const VersionInfo vInfo(dllModule);
        CTL_StringStreamType strm;
        vInfo.printit(strm, indent, crlf.data());
        return strm.str();
    }
}
#endif