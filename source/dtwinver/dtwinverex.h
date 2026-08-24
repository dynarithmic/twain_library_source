#ifndef DTWINVEREX_H
#define DTWINVEREX_H

#include <string_view>
#include <string>
#include <windows.h>
#include "ctlstringutils.h"
#include "ctlstringdefs.h"

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
                                    std::basic_string_view<CTL_StringType::value_type> crlf =
                                    CharTraits<CTL_StringType::value_type>::NewLineString());
    bool GetDLLVersionNumbersSmall(HMODULE dllModule, VersionNumbersSmall& info);
}
#endif

