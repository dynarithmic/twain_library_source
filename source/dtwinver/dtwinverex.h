#ifndef DTWINVEREX_H
#define DTWINVEREX_H

#include "ctlobstr.h"

namespace dynarithmic
{
    CTL_StringType GetWinVersion();
    CTL_StringType GetVersionInfo();
    CTL_StringType GetVersionInfo(HMODULE dllModule, int indent, CTL_StringType crlf = _T("\r\n"));
}
#endif

