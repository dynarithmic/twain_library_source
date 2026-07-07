#ifndef DTWINVEREX_H
#define DTWINVEREX_H

#include "ctlobstr.h"

namespace dynarithmic
{
    struct VersionNumbersSmall
    {
        std::wstring FileVersion;
        std::wstring ProductVersion;
    };

    CTL_StringType GetWinVersion();
    CTL_StringType GetVersionInfo();
    CTL_StringType GetVersionInfo(HMODULE dllModule, int indent, 
                                  StringWrapper::traits_type::stringview_type crlf = StringWrapper::traits_type::GetNewLineString());
    bool GetDLLVersionNumbersSmall(HMODULE dllModule, VersionNumbersSmall& info);
}
#endif

