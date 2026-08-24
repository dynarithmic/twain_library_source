#ifdef _WIN32
#include "ctliface.h"
#include "ctlstringdefs.h"
#include "versioninfo.h"
#include "dtwinverex.h"
#include "ctlstaticdata.h"

namespace dynarithmic
{
    CTL_StringType GetVersionInfo()
    {
        return GetVersionInfo(CTL_StaticData::GetDLLInstanceHandle(), 0, _T("\n"));
    }

    CTL_StringType GetVersionInfo(HMODULE dllModule, int indent, 
                                  std::basic_string_view<CTL_StringType::value_type> crlf)
    {
        const VersionInfo vInfo(dllModule);
        CTL_StringStreamType strm;
        vInfo.printit(strm, indent, crlf.data());
        return strm.str();
    }

    std::wstring MakeVersionString(DWORD ms, DWORD ls)
    {
        return std::to_wstring(HIWORD(ms)) + L"." +
            std::to_wstring(LOWORD(ms)) + L"." +
            std::to_wstring(HIWORD(ls)) + L"." +
            std::to_wstring(LOWORD(ls));
    }

    bool GetDLLVersionNumbersSmall(HMODULE hModule, VersionNumbersSmall& out)
    {
        if (!hModule)
            return false;

        std::wstring modulePath(32768, L'\0');

        DWORD len = GetModuleFileNameW(hModule,modulePath.data(),modulePath.size());

        if (len == 0 || len >= modulePath.size())
            return false;

        modulePath.resize(len);

        DWORD dummy = 0;
        DWORD size = GetFileVersionInfoSizeW(modulePath.c_str(), &dummy);

        if (size == 0)
            return false;

        std::wstring versionData;
        versionData.resize((size + sizeof(wchar_t) - 1) / sizeof(wchar_t));

        if (!GetFileVersionInfoW(modulePath.c_str(),0,size,versionData.data()))
        {
            return false;
        }

        VS_FIXEDFILEINFO* fixedInfo = nullptr;
        UINT fixedInfoSize = 0;

        if (!VerQueryValueW(versionData.data(),L"\\",reinterpret_cast<LPVOID*>(&fixedInfo),&fixedInfoSize))
        {
            return false;
        }

        if (!fixedInfo || fixedInfoSize < sizeof(VS_FIXEDFILEINFO))
            return false;

        if (fixedInfo->dwSignature != 0xFEEF04BD)
            return false;

        out.FileVersion = MakeVersionString(fixedInfo->dwFileVersionMS,fixedInfo->dwFileVersionLS);

        out.ProductVersion = MakeVersionString(fixedInfo->dwProductVersionMS,fixedInfo->dwProductVersionLS);

        return true;
    }
}
#endif
