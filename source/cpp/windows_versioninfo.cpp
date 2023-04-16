#ifdef _WIN32
#include "ctlobstr.h"
#include "ctliface.h"
#include "versioninfo.h"

namespace dynarithmic
{
    CTL_StringType GetVersionInfo()
    {
        const VersionInfo vInfo(CTL_StaticData::s_DLLInstance);
        CTL_StringStreamType strm;
        vInfo.printit(strm, _T("\r\n"));
        return strm.str();
    }
}
#endif