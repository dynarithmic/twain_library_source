/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2026 Dynarithmic Software.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    FOR ANY PART OF THE COVERED WORK IN WHICH THE COPYRIGHT IS OWNED BY
    DYNARITHMIC SOFTWARE. DYNARITHMIC SOFTWARE DISCLAIMS THE WARRANTY OF NON INFRINGEMENT
    OF THIRD PARTY RIGHTS.
 */
#include <algorithm>
#include <string_view>
#include "dtwainc.h"
#include "dtwainx.h"
#include "ctlstringconversion.h"
#include "ctlstringutils.h"
#include "arrayfactory.h"

namespace strview = std;

#ifdef _UNICODE
    #pragma message ("Creating UNICODE version of DTWAIN functions")
#else
    #pragma message ("Creating MBCS version of DTWAIN functions")
#endif

#ifdef _MSC_VER
    #pragma warning (disable:4244)
#endif

using namespace dynarithmic;

#include <type_traits>
#include <string>
#include <vector>

// Internal adapters. These are not exported and do not change the DTWAIN ABI.
namespace dtwain_aw_detail
{
    template <class CharT>
    class native_input
    {
        std::basic_string<TCHAR> storage_;
        const TCHAR* pointer_ = nullptr;
    public:
        explicit native_input(const CharT* value)
        {
            if constexpr (std::is_same_v<CharT, TCHAR>)
                pointer_ = value; // zero-copy native fast path
            else if (value)
            {
                if constexpr (std::is_same_v<CharT, char>)
                    storage_ = stringconversion::Convert_AnsiPtr_To_Native(value);
                else
                    storage_ = stringconversion::Convert_WidePtr_To_Native(value);
                pointer_ = storage_.c_str();
            }
        }
        native_input(const native_input&) = delete;
        native_input& operator=(const native_input&) = delete;
        native_input(native_input&&) = delete;
        native_input& operator=(native_input&&) = delete;
        const TCHAR* c_str() const noexcept { return pointer_; }
    };
}

namespace
{
    // Windows A means the active ANSI code page (CP_ACP), not UTF-8.
    template <class OutChar, class NativeChar>
    std::basic_string<OutChar> convert_output(const NativeChar* native)
    {
        if (!native)
            return {};

        if constexpr (std::is_same_v<OutChar, NativeChar>)
        {
            return native;
        }
        else if constexpr (std::is_same_v<NativeChar, wchar_t> && std::is_same_v<OutChar, char>)
        {
            // Unicode -> ANSI
            const int count = WideCharToMultiByte(CP_ACP, 0, native, -1,
                nullptr, 0, nullptr, nullptr);

            if (count <= 0)
                return {};

            std::string result(static_cast<size_t>(count), '\0');

            if (!WideCharToMultiByte(CP_ACP, 0, native, -1,
                result.data(), count, nullptr, nullptr))
                return {};

            result.pop_back(); // Remove null terminator
            return result;
        }
        else if constexpr (std::is_same_v<NativeChar, char> && std::is_same_v<OutChar, wchar_t>)
        {
            // ANSI -> Unicode
            const int count = MultiByteToWideChar(CP_ACP, 0, native, -1,
                nullptr, 0);

            if (count <= 0)
                return {};

            std::wstring result(static_cast<size_t>(count), L'\0');

            if (!MultiByteToWideChar(CP_ACP, 0, native, -1,
                result.data(), count))
                return {};

            result.pop_back(); // Remove null terminator
            return result;
        }
        else
        {
            static_assert(std::is_same_v<OutChar, char> ||
                std::is_same_v<OutChar, wchar_t>,
                "Unsupported output character type");
        }
    }

    // MaxLen is measured in destination elements, including the terminator.
    // The caller is responsible for supplying a buffer of the stated capacity.
    template <class CharT>
    void copy_bounded(const std::basic_string<CharT>& value, CharT* output, LONG maxLen)
    {
        if (!output || maxLen <= 0)
            return;
        const auto count = (std::min)(value.size(), static_cast<size_t>(maxLen - 1));
        std::char_traits<CharT>::copy(output, value.data(), count);
        output[count] = CharT{};
    }

    template <class CharT, class Func>
    LONG output_with_length(CharT* output, LONG maxLen, Func&& func)
    {
        if constexpr (std::is_same_v<CharT, TCHAR>)
        {
            // Native character type: forward directly.
            return func(output, maxLen);
        }
        else
        {
            // Preserve the caller's length-query operation.
            if (!output)
                return func(static_cast<TCHAR*>(nullptr), maxLen);

            // Allocate the intermediate native buffer.
            const LONG nativeLen = (std::max)(maxLen, 0L);

            std::basic_string<TCHAR> native(static_cast<size_t>(nativeLen), TCHAR{});

            // Exactly one call to the underlying DTWAIN function.
            const LONG result = func(nativeLen > 0 ? native.data() : nullptr, nativeLen);

            if (result <= 0)
                return result;

            if (nativeLen == 0)
                return result;

            // Find the terminator without reading beyond the buffer.
            const auto end = std::char_traits<TCHAR>::find(native.data(), native.size(), TCHAR{});

            if (!end)
                return 0;

            native.resize(static_cast<size_t>(end - native.data()));

            const auto converted = convert_output<CharT>(native.c_str());

            copy_bounded(converted, output, maxLen);

            return static_cast<LONG>(converted.size() + 1);
        }
    }
}

namespace
{
    template <typename StringTypeIn>
    strview::basic_string_view<typename StringTypeIn::value_type> get_view(const StringTypeIn& str)
    {
        if (!str.empty())
            return { &str[0] };
        return {};
    }

    template <typename StringTypeIn, typename StringTypeOut, typename RetvalType>
    RetvalType null_terminator_copier(StringTypeIn arg, StringTypeOut buffer, RetvalType retVal)
    {
        if (buffer)
        {
            auto iter = arg.begin();
            while (iter != arg.end() && buffer)
            {
                *buffer = static_cast<std::remove_pointer_t<StringTypeOut>>(*iter);
                ++iter;
                ++buffer;
            }
            *buffer = 0;
        }
        return retVal;
    }
}

extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumSourceValuesW(DTWAIN_SOURCE Source, LPCWSTR capName, LPDTWAIN_ARRAY pArray, DTWAIN_BOOL expandIfRange)
    {
        return DTWAIN_EnumSourceValues(Source, dtwain_aw_detail::native_input(capName).c_str(), pArray, expandIfRange);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumSourceValuesA(DTWAIN_SOURCE Source, LPCSTR capName, LPDTWAIN_ARRAY pArray, DTWAIN_BOOL expandIfRange)
    {
        return DTWAIN_EnumSourceValues(Source, dtwain_aw_detail::native_input(capName).c_str(), pArray, expandIfRange);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AcquireFileA(DTWAIN_SOURCE Source, LPCSTR lpszFile, LONG lFileType, LONG lFileFlags, LONG PixelType, LONG lMaxPages, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
    {
        return DTWAIN_AcquireFile(Source, dtwain_aw_detail::native_input(lpszFile).c_str(), lFileType, lFileFlags, PixelType, lMaxPages, bShowUI, bCloseSource, pStatus);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AcquireFileW(DTWAIN_SOURCE Source, LPCWSTR lpszFile, LONG lFileType, LONG lFileFlags, LONG PixelType, LONG lMaxPages, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
    {
        return DTWAIN_AcquireFile(Source, dtwain_aw_detail::native_input(lpszFile).c_str(), lFileType, lFileFlags, PixelType, lMaxPages, bShowUI, bCloseSource, pStatus);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AcquireAudioFileA(DTWAIN_SOURCE Source, LPCSTR lpszFile, LONG lFileFlags, LONG lMaxPages, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
    {
        return DTWAIN_AcquireAudioFile(Source, dtwain_aw_detail::native_input(lpszFile).c_str(), lFileFlags, lMaxPages, bShowUI, bCloseSource, pStatus);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AcquireAudioFileW(DTWAIN_SOURCE Source, LPCWSTR lpszFile, LONG lFileFlags, LONG lMaxPages, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
    {
        return DTWAIN_AcquireAudioFile(Source, dtwain_aw_detail::native_input(lpszFile).c_str(), lFileFlags, lMaxPages, bShowUI, bCloseSource, pStatus);
    }


    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddPDFTextW(DTWAIN_SOURCE Source, LPCWSTR szText, LONG xPos, LONG yPos, LPCWSTR fontName, DTWAIN_FLOAT fontSize, LONG colorRGB, LONG renderMode, DTWAIN_FLOAT scaling, DTWAIN_FLOAT charSpacing, DTWAIN_FLOAT wordSpacing, DTWAIN_FLOAT strokeWidth, DWORD Flags)
    {
        return DTWAIN_AddPDFText(Source, dtwain_aw_detail::native_input(szText).c_str(), xPos, yPos, dtwain_aw_detail::native_input(fontName).c_str(), fontSize, colorRGB, renderMode, scaling, charSpacing, wordSpacing, strokeWidth, Flags);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddPDFTextA(DTWAIN_SOURCE Source, LPCSTR szText, LONG xPos, LONG yPos, LPCSTR fontName, DTWAIN_FLOAT fontSize, LONG colorRGB, LONG renderMode, DTWAIN_FLOAT scaling, DTWAIN_FLOAT charSpacing, DTWAIN_FLOAT wordSpacing, DTWAIN_FLOAT strokeWidth, DWORD Flags)
    {
        return DTWAIN_AddPDFText(Source, dtwain_aw_detail::native_input(szText).c_str(), xPos, yPos, dtwain_aw_detail::native_input(fontName).c_str(), fontSize, colorRGB, renderMode, scaling, charSpacing, wordSpacing, strokeWidth, Flags);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddStringA(DTWAIN_ARRAY pArray, LPCSTR Val)
    {
        return DTWAIN_ArrayAddString(pArray, dtwain_aw_detail::native_input(Val).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddStringW(DTWAIN_ARRAY pArray, LPCWSTR Val)
    {
        return DTWAIN_ArrayAddString(pArray, dtwain_aw_detail::native_input(Val).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddStringNA(DTWAIN_ARRAY pArray, LPCSTR Val, LONG num)
    {
        return DTWAIN_ArrayAddStringN(pArray, dtwain_aw_detail::native_input(Val).c_str(), num);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddStringNW(DTWAIN_ARRAY pArray, LPCWSTR Val, LONG num)
    {
        return DTWAIN_ArrayAddStringN(pArray, dtwain_aw_detail::native_input(Val).c_str(), num);
    }

    LONG DLLENTRY_DEF DTWAIN_ArrayFindStringA(DTWAIN_ARRAY pArray, LPCSTR pString)
    {
        return DTWAIN_ArrayFindString(pArray, dtwain_aw_detail::native_input(pString).c_str());
    }

    LONG DLLENTRY_DEF DTWAIN_ArrayFindStringW(DTWAIN_ARRAY pArray, LPCWSTR pString)
    {
        return DTWAIN_ArrayFindString(pArray, dtwain_aw_detail::native_input(pString).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAtStringA(DTWAIN_ARRAY pArray, LONG nWhere, LPSTR pStr)
    {
#ifdef _UNICODE
        std::wstring arg(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_ArrayGetAtString(pArray, nWhere, arg.data());
        return null_terminator_copier(get_view(arg), pStr, retVal);
#else
        return DTWAIN_ArrayGetAtString(pArray, nWhere, pStr);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAtStringW(DTWAIN_ARRAY pArray, LONG nWhere, LPWSTR pStr)
    {
#ifdef _UNICODE
        return DTWAIN_ArrayGetAtString(pArray, nWhere, pStr);
#else
        std::string arg(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_ArrayGetAtString(pArray, nWhere, arg.data());
        return null_terminator_copier(get_view(arg), pStr, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtStringA(DTWAIN_ARRAY pArray, LONG nWhere, LPCSTR pVal)
    {
        return DTWAIN_ArrayInsertAtString(pArray, nWhere, dtwain_aw_detail::native_input(pVal).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtStringW(DTWAIN_ARRAY pArray, LONG nWhere, LPCWSTR pVal)
    {
        return DTWAIN_ArrayInsertAtString(pArray, nWhere, dtwain_aw_detail::native_input(pVal).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtStringNA(DTWAIN_ARRAY pArray, LONG nWhere, LPCSTR Val, LONG num)
    {
        return DTWAIN_ArrayInsertAtStringN(pArray, nWhere, dtwain_aw_detail::native_input(Val).c_str(), num);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtStringNW(DTWAIN_ARRAY pArray, LONG nWhere, LPCWSTR Val, LONG num)
    {
        return DTWAIN_ArrayInsertAtStringN(pArray, nWhere, dtwain_aw_detail::native_input(Val).c_str(), num);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtStringW(DTWAIN_ARRAY pArray, LONG nWhere, LPCWSTR pStr)
    {
        return DTWAIN_ArraySetAtString(pArray, nWhere, dtwain_aw_detail::native_input(pStr).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtStringA(DTWAIN_ARRAY pArray, LONG nWhere, LPCSTR pStr)
    {
        return DTWAIN_ArraySetAtString(pArray, nWhere, dtwain_aw_detail::native_input(pStr).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ExecuteOCRW(DTWAIN_OCRENGINE Engine, LPCWSTR szFileName, LONG nStartPage, LONG nEndPage)
    {
        return DTWAIN_ExecuteOCR(Engine, dtwain_aw_detail::native_input(szFileName).c_str(), nStartPage, nEndPage);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ExecuteOCRA(DTWAIN_OCRENGINE Engine, LPCSTR szFileName, LONG nStartPage, LONG nEndPage)
    {
        return DTWAIN_ExecuteOCR(Engine, dtwain_aw_detail::native_input(szFileName).c_str(), nStartPage, nEndPage);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAcquireArea2StringW(DTWAIN_SOURCE Source, LPWSTR left, LPWSTR top, LPWSTR right, LPWSTR bottom, LPLONG Unit)
    {
#ifdef _UNICODE
        return DTWAIN_GetAcquireArea2String(Source, left, top, right, bottom, Unit);
#else
        std::array<LPWSTR, 4> outarg = { left, top, right, bottom };
        std::array<std::string, 4> args = { {std::string(1024, 0), std::string(1024, 0), std::string(1024, 0), std::string(1024, 0)} };
        DTWAIN_BOOL retVal = DTWAIN_GetAcquireArea2String(Source, args[0].data(), args[1].data(), args[2].data(),
                                                          args[3].data(), Unit);
        for (size_t i = 0; i < args.size(); ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAcquireArea2StringA(DTWAIN_SOURCE Source, LPSTR left, LPSTR top, LPSTR right, LPSTR bottom, LPLONG Unit)
    {
#ifdef _UNICODE
        const std::array<LPSTR, 4> outarg = { left, top, right, bottom };
        std::array<std::wstring, 4> args = { {std::wstring(1024, 0), std::wstring(1024, 0), std::wstring(1024, 0), std::wstring(1024, 0)} };
        const DTWAIN_BOOL retVal = DTWAIN_GetAcquireArea2String(Source, args[0].data(), args[1].data(), args[2].data(),
                                                                args[3].data(), Unit);
        for (size_t i = 0; i < 4; ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#else
        return DTWAIN_GetAcquireArea2String(Source, left, top, right, bottom, Unit);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAppInfoA(LPSTR szVerStr, LPSTR szManu, LPSTR szProdFam, LPSTR szProdName)
    {
#ifdef _UNICODE
        const std::array<LPSTR, 4> outarg = { szVerStr, szManu, szProdFam, szProdName };
        std::array<std::wstring, 4> args = { { std::wstring(1024, 0), std::wstring(1024, 0), std::wstring(1024, 0), std::wstring(1024, 0) } };
        const DTWAIN_BOOL retVal = DTWAIN_GetAppInfo(args[0].data(), args[1].data(), args[2].data(), args[3].data());
        for (size_t i = 0; i < 4; ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#else
        return DTWAIN_GetAppInfo(szVerStr, szManu, szProdFam, szProdName);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAppInfoW(LPWSTR szVerStr, LPWSTR szManu, LPWSTR szProdFam, LPWSTR szProdName)
    {
#ifdef _UNICODE
        return DTWAIN_GetAppInfo(szVerStr, szManu, szProdFam, szProdName);
#else
        std::array<LPWSTR, 4> outarg = { szVerStr, szManu, szProdFam, szProdName };
        std::array<std::string, 4> args = { { std::string(1024, 0), std::string(1024, 0), std::string(1024, 0), std::string(1024, 0) } };
        DTWAIN_BOOL retVal = DTWAIN_GetAppInfo(args[0].data(), args[1].data(), args[2].data(), args[3].data());
        for (size_t i = 0; i < args.size(); ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAuthorW(DTWAIN_SOURCE Source, LPWSTR szAuthor)
    {
#ifdef _UNICODE
        return DTWAIN_GetAuthor(Source, szAuthor);
#else
        std::string arg(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetAuthor(Source, arg.data());
        return null_terminator_copier(get_view(arg), szAuthor, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAuthorA(DTWAIN_SOURCE Source, LPSTR szAuthor)
    {
#ifdef _UNICODE
        std::wstring arg(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetAuthor(Source, arg.data());
        return null_terminator_copier(get_view(arg), szAuthor, retVal);
#else
        return DTWAIN_GetAuthor(Source, szAuthor);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetBrightnessStringW(DTWAIN_SOURCE Source, LPWSTR Contrast)
    {
#ifdef _UNICODE
        return DTWAIN_GetBrightnessString(Source, Contrast);
#else
        std::string arg(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetBrightnessString(Source, arg.data());
        return null_terminator_copier(get_view(arg), Contrast, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetBrightnessStringA(DTWAIN_SOURCE Source, LPSTR Contrast)
    {
#ifdef _UNICODE
        std::wstring arg(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetBrightnessString(Source, arg.data());
        return null_terminator_copier(get_view(arg), Contrast, retVal);
#else
        return DTWAIN_GetBrightnessString(Source, Contrast);
#endif
    }

    LONG DLLENTRY_DEF DTWAIN_GetCapFromNameA(LPCSTR szName)
    {
        return DTWAIN_GetCapFromName(dtwain_aw_detail::native_input(szName).c_str());
    }

    LONG DLLENTRY_DEF DTWAIN_GetCapFromNameW(LPCWSTR szName)
    {
        return DTWAIN_GetCapFromName(dtwain_aw_detail::native_input(szName).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetCaptionW(DTWAIN_SOURCE Source, LPWSTR Caption)
    {
#ifdef _UNICODE
        return DTWAIN_GetCaption(Source, Caption);
#else
        std::string arg(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetCaption(Source, arg.data());
        return null_terminator_copier(get_view(arg), Caption, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetCaptionA(DTWAIN_SOURCE Source, LPSTR Caption)
    {
#ifdef _UNICODE
        std::wstring arg(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetCaption(Source, arg.data());
        return null_terminator_copier(get_view(arg), Caption, retVal);
#else
        return DTWAIN_GetCaption(Source, Caption);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetContrastStringW(DTWAIN_SOURCE Source, LPWSTR Contrast)
    {
#ifdef _UNICODE
        return DTWAIN_GetContrastString(Source, Contrast);
#else
        std::string arg(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetContrastString(Source, arg.data());
        return null_terminator_copier(get_view(arg), Contrast, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetContrastStringA(DTWAIN_SOURCE Source, LPSTR Contrast)
    {
#ifdef _UNICODE
        std::wstring arg(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetContrastString(Source, arg.data());
        return null_terminator_copier(get_view(arg), Contrast, retVal);
#else
        return DTWAIN_GetContrastString(Source, Contrast);
#endif
    }

    LONG DLLENTRY_DEF DTWAIN_GetCurrentFileNameA(DTWAIN_SOURCE Source, LPSTR szName, LONG MaxLen)
    {
        return output_with_length(szName, MaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetCurrentFileName(Source, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetCurrentFileNameW(DTWAIN_SOURCE Source, LPWSTR szName, LONG MaxLen)
    {
        return output_with_length(szName, MaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetCurrentFileName(Source, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetDSMFullNameW(LONG DSMType, LPWSTR szDLLName, LONG nMaxLen, LPLONG pWhichSearch)
    {
        return output_with_length(szDLLName, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetDSMFullName(DSMType, nativeOut, nativeCap, pWhichSearch); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetDSMFullNameA(LONG DSMType, LPSTR szDLLName, LONG nMaxLen, LPLONG pWhichSearch)
    {
        return output_with_length(szDLLName, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetDSMFullName(DSMType, nativeOut, nativeCap, pWhichSearch); });
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetDeviceTimeDateA(DTWAIN_SOURCE Source, LPSTR szTimeDate)
    {
#ifdef _UNICODE
        std::wstring arg(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetDeviceTimeDate(Source, arg.data());
        return null_terminator_copier(get_view(arg), szTimeDate, retVal);
#else
        return DTWAIN_GetDeviceTimeDate(Source, szTimeDate);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetDeviceTimeDateW(DTWAIN_SOURCE Source, LPWSTR szTimeDate)
    {
#ifdef _UNICODE
        return DTWAIN_GetDeviceTimeDate(Source, szTimeDate);
#else
        std::string arg(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetDeviceTimeDate(Source, arg.data());
        return null_terminator_copier(get_view(arg), szTimeDate, retVal);
#endif
    }

    LONG DLLENTRY_DEF DTWAIN_GetErrorStringA(LONG lError, LPSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetErrorString(lError, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetErrorStringW(LONG lError, LPWSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetErrorString(lError, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetResourceStringA(LONG ResourceID, LPSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetResourceString(ResourceID, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetResourceStringW(LONG ResourceID, LPWSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetResourceString(ResourceID, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetConditionCodeStringA(LONG lError, LPSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetConditionCodeString(lError, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetConditionCodeStringW(LONG lError, LPWSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetConditionCodeString(lError, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetPaperSizeNameW(LONG paperNumber, LPWSTR outName, LONG nSize)
    {
        return output_with_length(outName, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetPaperSizeName(paperNumber, nativeOut, nativeCap); });
    }


    LONG DLLENTRY_DEF DTWAIN_GetPaperSizeNameA(LONG paperNumber, LPSTR outName, LONG nSize)
    {
        return output_with_length(outName, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetPaperSizeName(paperNumber, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetTwainNameFromConstantW(LONG lConstantType, LONG lTwainConstant, LPWSTR lpszOut, LONG nSize)
    {
        return output_with_length(lpszOut, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetTwainNameFromConstant(lConstantType, lTwainConstant, nativeOut, nativeCap); });
    }


    LONG DLLENTRY_DEF DTWAIN_GetTwainNameFromConstantA(LONG lConstantType, LONG lTwainConstant, LPSTR lpszOut, LONG nSize)
    {
        return output_with_length(lpszOut, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetTwainNameFromConstant(lConstantType, lTwainConstant, nativeOut, nativeCap); });
    }


    LONG DLLENTRY_DEF DTWAIN_GetExtCapFromNameW(LPCWSTR szName)
    {
        return DTWAIN_GetExtCapFromName(dtwain_aw_detail::native_input(szName).c_str());
    }

    LONG DLLENTRY_DEF DTWAIN_GetExtCapFromNameA(LPCSTR szName)
    {
        return DTWAIN_GetExtCapFromName(dtwain_aw_detail::native_input(szName).c_str());
    }

    LONG DLLENTRY_DEF DTWAIN_GetExtNameFromCapW(LONG nValue, LPWSTR szValue, LONG nLength)
    {
        return output_with_length(szValue, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetExtNameFromCap(nValue, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetExtNameFromCapA(LONG nValue, LPSTR szValue, LONG nLength)
    {
        return output_with_length(szValue, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetExtNameFromCap(nValue, nativeOut, nativeCap); });
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetHalftoneA(DTWAIN_SOURCE Source, LPSTR lpHalftone, LONG GetType)
    {
#ifdef _UNICODE
        std::wstring arg(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetHalftone(Source, arg.data(), GetType);
        return null_terminator_copier(get_view(arg), lpHalftone, retVal);
#else
        return DTWAIN_GetHalftone(Source, lpHalftone, GetType);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetHalftoneW(DTWAIN_SOURCE Source, LPWSTR lpHalftone, LONG GetType)
    {
#ifdef _UNICODE
        return DTWAIN_GetHalftone(Source, lpHalftone, GetType);
#else
        std::string arg(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetHalftone(Source, arg.data(), GetType);
        return null_terminator_copier(get_view(arg), lpHalftone, retVal);
#endif
    }


    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetHighlightStringW(DTWAIN_SOURCE Source, LPWSTR Highlight)
    {
#ifdef _UNICODE
        return DTWAIN_GetHighlightString(Source, Highlight);
#else
        std::string arg(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetHighlightString(Source, arg.data());
        return null_terminator_copier(get_view(arg), Highlight, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetHighlightStringA(DTWAIN_SOURCE Source, LPSTR Highlight)
    {
#ifdef _UNICODE
        std::wstring arg(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetHighlightString(Source, arg.data());
        return null_terminator_copier(get_view(arg), Highlight, retVal);
#else
        return DTWAIN_GetHighlightString(Source, Highlight);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetImageInfoStringW(DTWAIN_SOURCE Source, LPWSTR lpXResolution, LPWSTR lpYResolution, LPLONG lpWidth, LPLONG lpLength, LPLONG lpNumSamples, LPDTWAIN_ARRAY lpBitsPerSample, LPLONG lpBitsPerPixel, LPLONG lpPlanar, LPLONG lpPixelType, LPLONG lpCompression)
    {
#ifdef _UNICODE
        return DTWAIN_GetImageInfoString(Source, lpXResolution, lpYResolution, lpWidth, lpLength, lpNumSamples, lpBitsPerSample, lpBitsPerPixel, lpPlanar, lpPixelType, lpCompression);
#else
        std::array<LPWSTR, 2> outarg = { lpXResolution, lpYResolution };
        std::array<std::string, 2> args = { { std::string(1024, 0), std::string(1024, 0) } };
        DTWAIN_BOOL retVal = DTWAIN_GetImageInfoString(Source, args[0].data(), args[1].data(), lpWidth, lpLength, lpNumSamples, lpBitsPerSample, lpBitsPerPixel, lpPlanar, lpPixelType, lpCompression);
        for (size_t i = 0; i < 2; ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetImageInfoStringA(DTWAIN_SOURCE Source, LPSTR lpXResolution, LPSTR lpYResolution, LPLONG lpWidth, LPLONG lpLength, LPLONG lpNumSamples, LPDTWAIN_ARRAY lpBitsPerSample, LPLONG lpBitsPerPixel, LPLONG lpPlanar, LPLONG lpPixelType, LPLONG lpCompression)
    {
#ifdef _UNICODE
        const std::array<LPSTR, 2> outarg = { lpXResolution, lpYResolution };
        std::array<std::wstring, 2> args = { { std::wstring(1024, 0), std::wstring(1024, 0) } };
        const DTWAIN_BOOL retVal = DTWAIN_GetImageInfoString(Source, args[0].data(), args[1].data(), lpWidth, lpLength, lpNumSamples, lpBitsPerSample, lpBitsPerPixel, lpPlanar, lpPixelType, lpCompression);
        for (size_t i = 0; i < 2; ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#else
        return DTWAIN_GetImageInfoString(Source, lpXResolution, lpYResolution, lpWidth, lpLength, lpNumSamples, lpBitsPerSample, lpBitsPerPixel, lpPlanar, lpPixelType, lpCompression);
#endif
    }

    LONG DLLENTRY_DEF DTWAIN_GetNameFromCapA(LONG nCapValue, LPSTR szValue, LONG nLength)
    {
        return output_with_length(szValue, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetNameFromCap(nCapValue, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetNameFromCapW(LONG nCapValue, LPWSTR szValue, LONG nLength)
    {
        return output_with_length(szValue, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetNameFromCap(nCapValue, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetOCRErrorStringW(DTWAIN_OCRENGINE Engine, LONG lError, LPWSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetOCRErrorString(Engine, lError, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetOCRErrorStringA(DTWAIN_OCRENGINE Engine, LONG lError, LPSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetOCRErrorString(Engine, lError, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetOCRManufacturerW(DTWAIN_OCRENGINE Engine, LPWSTR szManufacturer, LONG nLength)
    {
        return output_with_length(szManufacturer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetOCRManufacturer(Engine, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetOCRManufacturerA(DTWAIN_OCRENGINE Engine, LPSTR szManufacturer, LONG nLength)
    {
        return output_with_length(szManufacturer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetOCRManufacturer(Engine, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetOCRProductFamilyA(DTWAIN_OCRENGINE Engine, LPSTR szProductFamily, LONG nLength)
    {
        return output_with_length(szProductFamily, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetOCRProductFamily(Engine, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetOCRProductFamilyW(DTWAIN_OCRENGINE Engine, LPWSTR szProductFamily, LONG nLength)
    {
        return output_with_length(szProductFamily, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetOCRProductFamily(Engine, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetOCRProductNameW(DTWAIN_OCRENGINE Engine, LPWSTR szProductName, LONG nLength)
    {
        return output_with_length(szProductName, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetOCRProductName(Engine, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetOCRProductNameA(DTWAIN_OCRENGINE Engine, LPSTR szProductName, LONG nLength)
    {
        return output_with_length(szProductName, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetOCRProductName(Engine, nativeOut, nativeCap); });
    }

    HANDLE DLLENTRY_DEF DTWAIN_GetOCRTextA(DTWAIN_OCRENGINE Engine, LONG nPageNo, LPSTR Data, LONG dSize, LPLONG pActualSize, LONG nFlags)
    {
#ifdef _UNICODE
        std::wstring arg(dSize, 0);
        const HANDLE retVal = DTWAIN_GetOCRText(Engine, nPageNo, Data ? arg.data() : nullptr, dSize, pActualSize, nFlags);
        return null_terminator_copier(get_view(arg), Data, retVal);
#else
        return DTWAIN_GetOCRText(Engine, nPageNo, Data, dSize, pActualSize, nFlags);
#endif
    }

    HANDLE DLLENTRY_DEF DTWAIN_GetOCRTextW(DTWAIN_OCRENGINE Engine, LONG nPageNo, LPWSTR Data, LONG dSize, LPLONG pActualSize, LONG nFlags)
    {
#ifdef _UNICODE
        return DTWAIN_GetOCRText(Engine, nPageNo, Data, dSize, pActualSize, nFlags);
#else
        std::string arg(dSize, 0);
        HANDLE retVal = DTWAIN_GetOCRText(Engine, nPageNo, Data ? arg.data() : nullptr, dSize, pActualSize, nFlags);
        return null_terminator_copier(get_view(arg), Data, retVal);
#endif
    }

    LONG DLLENTRY_DEF DTWAIN_GetOCRVersionInfoA(DTWAIN_OCRENGINE Engine, LPSTR buffer, LONG nLength)
    {
        return output_with_length(buffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetOCRVersionInfo(Engine, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetOCRVersionInfoW(DTWAIN_OCRENGINE Engine, LPWSTR buffer, LONG nLength)
    {
        return output_with_length(buffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetOCRVersionInfo(Engine, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetPDFTextElementStringA(DTWAIN_PDFTEXTELEMENT TextElement, LPSTR szData, LONG nMaxLen, LONG Flags)
    {
        return output_with_length(szData, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetPDFTextElementString(TextElement, nativeOut, nativeCap, Flags); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetPDFTextElementStringW(DTWAIN_PDFTEXTELEMENT TextElement, LPWSTR szData, LONG maxLen, LONG Flags)
    {
        return output_with_length(szData, maxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetPDFTextElementString(TextElement, nativeOut, nativeCap, Flags); });
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetPrinterSuffixStringA(DTWAIN_SOURCE Source, LPSTR Suffix, LONG nLength)
    {
#ifdef _UNICODE
        std::wstring arg((std::max)(nLength, 0L), 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetPrinterSuffixString(Source, Suffix ? arg.data() : nullptr, static_cast<LONG>(arg.size()));
        return null_terminator_copier(get_view(arg), Suffix, retVal);
#else
        return DTWAIN_GetPrinterSuffixString(Source, Suffix, nLength);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetPrinterSuffixStringW(DTWAIN_SOURCE Source, LPWSTR Suffix, LONG nLength)
    {
#ifdef _UNICODE
        return DTWAIN_GetPrinterSuffixString(Source, Suffix, nLength);
#else
        std::string arg((std::max)(nLength, 0L), 0);
        DTWAIN_BOOL retVal = DTWAIN_GetPrinterSuffixString(Source, Suffix ? arg.data() : nullptr, static_cast<LONG>(arg.size()));
        return null_terminator_copier(get_view(arg), Suffix, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetResolutionStringA(DTWAIN_SOURCE Source, LPSTR Resolution)
    {
#ifdef _UNICODE
        std::wstring arg(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetResolutionString(Source, arg.data());
        return null_terminator_copier(get_view(arg), Resolution, retVal);
#else
        return DTWAIN_GetResolutionString(Source, Resolution);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetResolutionStringW(DTWAIN_SOURCE Source, LPWSTR Resolution)
    {
#ifdef _UNICODE
        return DTWAIN_GetResolutionString(Source, Resolution);
#else
        std::string arg(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetResolutionString(Source, arg.data());
        return null_terminator_copier(get_view(arg), Resolution, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetRotationStringW(DTWAIN_SOURCE Source, LPWSTR Rotation)
    {
#ifdef _UNICODE
        return DTWAIN_GetRotationString(Source, Rotation);
#else
        std::string arg(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetRotationString(Source, arg.data());
        return null_terminator_copier(get_view(arg), Rotation, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetRotationStringA(DTWAIN_SOURCE Source, LPSTR Rotation)
    {
#ifdef _UNICODE
        std::wstring arg(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetRotationString(Source, arg.data());
        return null_terminator_copier(get_view(arg), Rotation, retVal);
#else
        return DTWAIN_GetRotationString(Source, Rotation);
#endif
    }

    LONG DLLENTRY_DEF DTWAIN_GetSaveFileNameW(DTWAIN_SOURCE Source, LPWSTR fName, LONG nMaxLen)
    {
        return output_with_length(fName, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSaveFileName(Source, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetSaveFileNameA(DTWAIN_SOURCE Source, LPSTR fName, LONG nMaxLen)
    {
        return output_with_length(fName, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSaveFileName(Source, nativeOut, nativeCap); });
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetShadowStringW(DTWAIN_SOURCE Source, LPWSTR Shadow)
    {
#ifdef _UNICODE
        return DTWAIN_GetShadowString(Source, Shadow);
#else
        std::string args(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetShadowString(Source, args.data());
        return null_terminator_copier(get_view(args), Shadow, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetShadowStringA(DTWAIN_SOURCE Source, LPSTR Shadow)
    {
#ifdef _UNICODE
        std::wstring args(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetShadowString(Source, args.data());
        return null_terminator_copier(get_view(args), Shadow, retVal);
#else
        return DTWAIN_GetShadowString(Source, Shadow);
#endif
    }

    LONG DLLENTRY_DEF DTWAIN_GetSourceManufacturerA(DTWAIN_SOURCE Source, LPSTR szProduct, LONG nLength)
    {
        return output_with_length(szProduct, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSourceManufacturer(Source, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetSourceManufacturerW(DTWAIN_SOURCE Source, LPWSTR szProduct, LONG nLength)
    {
        return output_with_length(szProduct, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSourceManufacturer(Source, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetSourceProductFamilyW(DTWAIN_SOURCE Source, LPWSTR szProduct, LONG nLength)
    {
        return output_with_length(szProduct, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSourceProductFamily(Source, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetSourceProductFamilyA(DTWAIN_SOURCE Source, LPSTR szProduct, LONG nLength)
    {
        return output_with_length(szProduct, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSourceProductFamily(Source, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetSourceProductNameA(DTWAIN_SOURCE Source, LPSTR szProduct, LONG nLength)
    {
        return output_with_length(szProduct, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSourceProductName(Source, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetSourceProductNameW(DTWAIN_SOURCE Source, LPWSTR szProduct, LONG nLength)
    {
        return output_with_length(szProduct, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSourceProductName(Source, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetSourceVersionInfoA(DTWAIN_SOURCE Source, LPSTR szProduct, LONG nLength)
    {
        return output_with_length(szProduct, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSourceVersionInfo(Source, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetSourceVersionInfoW(DTWAIN_SOURCE Source, LPWSTR szProduct, LONG nLength)
    {
        return output_with_length(szProduct, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSourceVersionInfo(Source, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetTempFileDirectoryW(LPWSTR szFilePath, LONG nLength)
    {
        return output_with_length(szFilePath, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetTempFileDirectory(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetTempFileDirectoryA(LPSTR szFilePath, LONG nLength)
    {
        return output_with_length(szFilePath, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetTempFileDirectory(nativeOut, nativeCap); });
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetThresholdStringA(DTWAIN_SOURCE Source, LPSTR Threshold)
    {
#ifdef _UNICODE
        std::wstring args(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetThresholdString(Source, args.data());
        return null_terminator_copier(get_view(args), Threshold, retVal);
#else
        return DTWAIN_GetThresholdString(Source, Threshold);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetThresholdStringW(DTWAIN_SOURCE Source, LPWSTR Threshold)
    {
#ifdef _UNICODE
        return DTWAIN_GetThresholdString(Source, Threshold);
#else
        std::string args(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetThresholdString(Source, args.data());
        return null_terminator_copier(get_view(args), Threshold, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetTimeDateW(DTWAIN_SOURCE Source, LPWSTR szTimeDate)
    {
#ifdef _UNICODE
        return DTWAIN_GetTimeDate(Source, szTimeDate);
#else
        std::string args(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetTimeDate(Source, args.data());
        return null_terminator_copier(get_view(args), szTimeDate, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetTimeDateA(DTWAIN_SOURCE Source, LPSTR szTimeDate)
    {
#ifdef _UNICODE
        std::wstring args(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetTimeDate(Source, args.data());
        return null_terminator_copier(get_view(args), szTimeDate, retVal);
#else
        return DTWAIN_GetTimeDate(Source, szTimeDate);
#endif
    }

    LONG DLLENTRY_DEF DTWAIN_GetTwainAvailabilityExA(LPSTR szDirectories, LONG nLength)
    {
        return output_with_length(szDirectories, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetTwainAvailabilityEx(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetTwainAvailabilityExW(LPWSTR szDirectories, LONG nLength)
    {
        return output_with_length(szDirectories, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetTwainAvailabilityEx(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_IsTwainAvailableExA(LPSTR szDirectories, LONG nLength)
    {
        return output_with_length(szDirectories, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_IsTwainAvailableEx(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_IsTwainAvailableExW(LPWSTR szDirectories, LONG nLength)
    {
        return output_with_length(szDirectories, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_IsTwainAvailableEx(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetVersionInfoW(LPWSTR lpszVer, LONG nLength)
    {
        return output_with_length(lpszVer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetVersionInfo(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetVersionInfoA(LPSTR lpszVer, LONG nLength)
    {
        return output_with_length(lpszVer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetVersionInfo(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetVersionStringW(LPWSTR lpszVer, LONG nLength)
    {
        return output_with_length(lpszVer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetVersionString(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetLibraryPathA(LPSTR lpszVer, LONG nLength)
    {
        return output_with_length(lpszVer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetLibraryPath(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetLibraryPathW(LPWSTR lpszVer, LONG nLength)
    {
        return output_with_length(lpszVer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetLibraryPath(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetVersionStringA(LPSTR lpszVer, LONG nLength)
    {
        return output_with_length(lpszVer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetVersionString(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetShortVersionStringW(LPWSTR lpszVer, LONG nLength)
    {
        return output_with_length(lpszVer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetShortVersionString(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetShortVersionStringA(LPSTR lpszVer, LONG nLength)
    {
        return output_with_length(lpszVer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetShortVersionString(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetConstantFromTwainNameA(LPCSTR lpszBuffer)
    {
        return DTWAIN_GetConstantFromTwainName(dtwain_aw_detail::native_input(lpszBuffer).c_str());
    }

    LONG DLLENTRY_DEF DTWAIN_GetConstantFromTwainNameW(LPCWSTR lpszBuffer)
    {
        return DTWAIN_GetConstantFromTwainName(dtwain_aw_detail::native_input(lpszBuffer).c_str());
    }

    LONG DLLENTRY_DEF DTWAIN_GetWindowsVersionInfoW(LPWSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetWindowsVersionInfo(nativeOut, nativeCap); });
    }


    LONG DLLENTRY_DEF DTWAIN_GetWindowsVersionInfoA(LPSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetWindowsVersionInfo(nativeOut, nativeCap); });
    }


    LONG DLLENTRY_DEF DTWAIN_GetActiveDSMPathW(LPWSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetActiveDSMPath(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetActiveDSMPathA(LPSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetActiveDSMPath(nativeOut, nativeCap); });
    }


    LONG DLLENTRY_DEF DTWAIN_GetActiveDSMVersionInfoW(LPWSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetActiveDSMVersionInfo(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetActiveDSMVersionInfoA(LPSTR lpszBuffer, LONG nLength)
    {
        return output_with_length(lpszBuffer, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetActiveDSMVersionInfo(nativeOut, nativeCap); });
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetXResolutionStringA(DTWAIN_SOURCE Source, LPSTR Resolution)
    {
#ifdef _UNICODE
        std::wstring args(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetXResolutionString(Source, args.data());
        return null_terminator_copier(get_view(args), Resolution, retVal);
#else
        return DTWAIN_GetXResolutionString(Source, Resolution);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetXResolutionStringW(DTWAIN_SOURCE Source, LPWSTR Resolution)
    {
#ifdef _UNICODE
        return DTWAIN_GetXResolutionString(Source, Resolution);
#else
        std::string args(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetXResolutionString(Source, args.data());
        return null_terminator_copier(get_view(args), Resolution, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetYResolutionStringA(DTWAIN_SOURCE Source, LPSTR Resolution)
    {
#ifdef _UNICODE
        std::wstring args(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetYResolutionString(Source, args.data());
        return null_terminator_copier(get_view(args), Resolution, retVal);
#else
        return DTWAIN_GetYResolutionString(Source, Resolution);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetYResolutionStringW(DTWAIN_SOURCE Source, LPWSTR Resolution)
    {
#ifdef _UNICODE
        return DTWAIN_GetYResolutionString(Source, Resolution);
#else
        std::string args(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_GetYResolutionString(Source, args.data());
        return null_terminator_copier(get_view(args), Resolution, retVal);
#endif
    }

    LONG DLLENTRY_DEF DTWAIN_IsDIBBlankStringA(HANDLE hDib, LPCSTR threshold)
    {
        return DTWAIN_IsDIBBlankString(hDib, dtwain_aw_detail::native_input(threshold).c_str());
    }

    LONG DLLENTRY_DEF DTWAIN_IsDIBBlankStringW(HANDLE hDib, LPCWSTR threshold)
    {
        return DTWAIN_IsDIBBlankString(hDib, dtwain_aw_detail::native_input(threshold).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResourcesW(LPCWSTR sLangDLL)
    {
        return DTWAIN_LoadCustomStringResources(dtwain_aw_detail::native_input(sLangDLL).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResourcesA(LPCSTR sLangDLL)
    {
        return DTWAIN_LoadCustomStringResources(dtwain_aw_detail::native_input(sLangDLL).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResourcesExW(LPCWSTR sLangDLL, DTWAIN_BOOL bClear)
    {
        return DTWAIN_LoadCustomStringResourcesEx(dtwain_aw_detail::native_input(sLangDLL).c_str(), bClear);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResourcesExA(LPCSTR sLangDLL, DTWAIN_BOOL bClear)
    {
        return DTWAIN_LoadCustomStringResourcesEx(dtwain_aw_detail::native_input(sLangDLL).c_str(), bClear);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LogMessageW(LPCWSTR message)
    {
        return DTWAIN_LogMessage(dtwain_aw_detail::native_input(message).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LogMessageA(LPCSTR message)
    {
        return DTWAIN_LogMessage(dtwain_aw_detail::native_input(message).c_str());
    }

    DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngineByNameW(LPCWSTR lpszName)
    {
        return DTWAIN_SelectOCREngineByName(dtwain_aw_detail::native_input(lpszName).c_str());
    }

    DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngineByNameA(LPCSTR lpszName)
    {
        return DTWAIN_SelectOCREngineByName(dtwain_aw_detail::native_input(lpszName).c_str());
    }

    DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSource2A(HWND hWndParent, LPCSTR szTitle, LONG xPos, LONG yPos, LONG nOptions)
    {
#ifdef _UNICODE
        return DTWAIN_SelectSource2(hWndParent, szTitle ? stringconversion::Convert_AnsiPtr_To_Native(szTitle).c_str() : nullptr, xPos, yPos, nOptions);
#else
        return DTWAIN_SelectSource2(hWndParent, szTitle, xPos, yPos, nOptions);
#endif
    }

    DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSource2W(HWND hWndParent, LPCWSTR szTitle, LONG xPos, LONG yPos, LONG nOptions)
    {
#ifdef _UNICODE
        return DTWAIN_SelectSource2(hWndParent, szTitle, xPos, yPos, nOptions);
#else
        return DTWAIN_SelectSource2(hWndParent,
            szTitle ? stringconversion::Convert_WidePtr_To_Native(szTitle).c_str() : nullptr, xPos, yPos, nOptions);
#endif
    }

    DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSource2ExA(HWND hWndParent, LPCSTR szTitle, LONG xPos, LONG yPos,
        LPCSTR szIncludeNames, LPCSTR szExcludeNames, LPCSTR szNameMapping, LONG nOptions)
    {
#ifdef _UNICODE
        return DTWAIN_SelectSource2Ex(hWndParent,
            szTitle ? stringconversion::Convert_AnsiPtr_To_Native(szTitle).c_str() : nullptr, xPos, yPos,
            szIncludeNames ? stringconversion::Convert_AnsiPtr_To_Native(szIncludeNames).c_str() : nullptr,
            szExcludeNames ? stringconversion::Convert_AnsiPtr_To_Native(szExcludeNames).c_str() : nullptr,
            szNameMapping ? stringconversion::Convert_AnsiPtr_To_Native(szNameMapping).c_str() : nullptr,
            nOptions);
#else
        return DTWAIN_SelectSource2Ex(hWndParent, szTitle, xPos, yPos, szIncludeNames, szExcludeNames, szNameMapping, nOptions);
#endif
    }

    DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSource2ExW(HWND hWndParent, LPCWSTR szTitle, LONG xPos, LONG yPos, LPCWSTR szIncludeNames, LPCWSTR szExcludeNames, LPCWSTR szNameMapping, LONG nOptions)
    {
#ifdef _UNICODE
        return DTWAIN_SelectSource2Ex(hWndParent, szTitle, xPos, yPos, szIncludeNames, szExcludeNames, szNameMapping, nOptions);
#else
        return DTWAIN_SelectSource2Ex(hWndParent,
            szTitle ? stringconversion::Convert_WidePtr_To_Native(szTitle).c_str() : nullptr,
            xPos, yPos,
            szIncludeNames ? stringconversion::Convert_WidePtr_To_Native(szIncludeNames).c_str() : nullptr,
            szExcludeNames ? stringconversion::Convert_WidePtr_To_Native(szExcludeNames).c_str() : nullptr,
            szNameMapping ? stringconversion::Convert_WidePtr_To_Native(szNameMapping).c_str() : nullptr,
            nOptions);
#endif
    }

    DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceByNameW(LPCWSTR lpszName)
    {
        return DTWAIN_SelectSourceByName(dtwain_aw_detail::native_input(lpszName).c_str());
    }

    DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceByNameA(LPCSTR lpszName)
    {
        return DTWAIN_SelectSourceByName(dtwain_aw_detail::native_input(lpszName).c_str());
    }

    DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceByNameWithOpenW(LPCWSTR lpszName, DTWAIN_BOOL bOpen)
    {
        return DTWAIN_SelectSourceByNameWithOpen(dtwain_aw_detail::native_input(lpszName).c_str(), bOpen);
    }

    DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceByNameWithOpenA(LPCSTR lpszName, DTWAIN_BOOL bOpen)
    {
        return DTWAIN_SelectSourceByNameWithOpen(dtwain_aw_detail::native_input(lpszName).c_str(), bOpen);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireArea2StringW(DTWAIN_SOURCE Source, LPCWSTR left, LPCWSTR top, LPCWSTR right, LPCWSTR bottom, LONG lUnit, LONG Flags)
    {
        return DTWAIN_SetAcquireArea2String(Source, dtwain_aw_detail::native_input(left).c_str(), dtwain_aw_detail::native_input(top).c_str(), dtwain_aw_detail::native_input(right).c_str(), dtwain_aw_detail::native_input(bottom).c_str(), lUnit, Flags);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireArea2StringA(DTWAIN_SOURCE Source, LPCSTR left, LPCSTR top, LPCSTR right, LPCSTR bottom, LONG lUnit, LONG Flags)
    {
        return DTWAIN_SetAcquireArea2String(Source, dtwain_aw_detail::native_input(left).c_str(), dtwain_aw_detail::native_input(top).c_str(), dtwain_aw_detail::native_input(right).c_str(), dtwain_aw_detail::native_input(bottom).c_str(), lUnit, Flags);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireImageScaleStringA(DTWAIN_SOURCE Source, LPCSTR xscale, LPCSTR yscale)
    {
        return DTWAIN_SetAcquireImageScaleString(Source, dtwain_aw_detail::native_input(xscale).c_str(), dtwain_aw_detail::native_input(yscale).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireImageScaleStringW(DTWAIN_SOURCE Source, LPCWSTR xscale, LPCWSTR yscale)
    {
        return DTWAIN_SetAcquireImageScaleString(Source, dtwain_aw_detail::native_input(xscale).c_str(), dtwain_aw_detail::native_input(yscale).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAppInfoW(LPCWSTR szVerStr, LPCWSTR szManu, LPCWSTR szProdFam, LPCWSTR szProdName)
    {
        return DTWAIN_SetAppInfo(dtwain_aw_detail::native_input(szVerStr).c_str(), dtwain_aw_detail::native_input(szManu).c_str(), dtwain_aw_detail::native_input(szProdFam).c_str(), dtwain_aw_detail::native_input(szProdName).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAppInfoA(LPCSTR szVerStr, LPCSTR szManu, LPCSTR szProdFam, LPCSTR szProdName)
    {
        return DTWAIN_SetAppInfo(dtwain_aw_detail::native_input(szVerStr).c_str(), dtwain_aw_detail::native_input(szManu).c_str(), dtwain_aw_detail::native_input(szProdFam).c_str(), dtwain_aw_detail::native_input(szProdName).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAuthorA(DTWAIN_SOURCE Source, LPCSTR szAuthor)
    {
        return DTWAIN_SetAuthor(Source, dtwain_aw_detail::native_input(szAuthor).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAuthorW(DTWAIN_SOURCE Source, LPCWSTR szAuthor)
    {
        return DTWAIN_SetAuthor(Source, dtwain_aw_detail::native_input(szAuthor).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBlankPageDetectionStringA(DTWAIN_SOURCE Source, LPCSTR threshold, LONG autodetect_option, DTWAIN_BOOL bSet)
    {
        return DTWAIN_SetBlankPageDetectionString(Source, dtwain_aw_detail::native_input(threshold).c_str(), autodetect_option, bSet);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBlankPageDetectionStringW(DTWAIN_SOURCE Source, LPCWSTR threshold, LONG autodetect_option, DTWAIN_BOOL bSet)
    {
        return DTWAIN_SetBlankPageDetectionString(Source, dtwain_aw_detail::native_input(threshold).c_str(), autodetect_option, bSet);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBlankPageDetectionExStringA(DTWAIN_SOURCE Source, LPCSTR threshold, LONG autodetect_option, LONG detectOpts, DTWAIN_BOOL bSet)
    {
        return DTWAIN_SetBlankPageDetectionExString(Source, dtwain_aw_detail::native_input(threshold).c_str(), autodetect_option, detectOpts, bSet);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBlankPageDetectionExStringW(DTWAIN_SOURCE Source, LPCWSTR threshold, LONG autodetect_option, LONG detectOpts, DTWAIN_BOOL bSet)
    {
        return DTWAIN_SetBlankPageDetectionExString(Source, dtwain_aw_detail::native_input(threshold).c_str(), autodetect_option, detectOpts, bSet);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBrightnessStringA(DTWAIN_SOURCE Source, LPCSTR Contrast)
    {
        return DTWAIN_SetBrightnessString(Source, dtwain_aw_detail::native_input(Contrast).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBrightnessStringW(DTWAIN_SOURCE Source, LPCWSTR Contrast)
    {
        return DTWAIN_SetBrightnessString(Source, dtwain_aw_detail::native_input(Contrast).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCameraW(DTWAIN_SOURCE Source, LPCWSTR szCamera)
    {
        return DTWAIN_SetCamera(Source, dtwain_aw_detail::native_input(szCamera).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCameraA(DTWAIN_SOURCE Source, LPCSTR szCamera)
    {
        return DTWAIN_SetCamera(Source, dtwain_aw_detail::native_input(szCamera).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCaptionA(DTWAIN_SOURCE Source, LPCSTR Caption)
    {
        return DTWAIN_SetCaption(Source, dtwain_aw_detail::native_input(Caption).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCaptionW(DTWAIN_SOURCE Source, LPCWSTR Caption)
    {
        return DTWAIN_SetCaption(Source, dtwain_aw_detail::native_input(Caption).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetContrastStringA(DTWAIN_SOURCE Source, LPCSTR Contrast)
    {
        return DTWAIN_SetContrastString(Source, dtwain_aw_detail::native_input(Contrast).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetContrastStringW(DTWAIN_SOURCE Source, LPCWSTR Contrast)
    {
        return DTWAIN_SetContrastString(Source, dtwain_aw_detail::native_input(Contrast).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDeviceTimeDateA(DTWAIN_SOURCE Source, LPCSTR szTimeDate)
    {
        return DTWAIN_SetDeviceTimeDate(Source, dtwain_aw_detail::native_input(szTimeDate).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDeviceTimeDateW(DTWAIN_SOURCE Source, LPCWSTR szTimeDate)
    {
        return DTWAIN_SetDeviceTimeDate(Source, dtwain_aw_detail::native_input(szTimeDate).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetFileSavePosW(HWND hWndParent, LPCWSTR szTitle, LONG xPos, LONG yPos, LONG nFlags)
    {
        return DTWAIN_SetFileSavePos(hWndParent, dtwain_aw_detail::native_input(szTitle).c_str(), xPos, yPos, nFlags);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetFileSavePosA(HWND hWndParent, LPCSTR szTitle, LONG xPos, LONG yPos, LONG nFlags)
    {
        return DTWAIN_SetFileSavePos(hWndParent, dtwain_aw_detail::native_input(szTitle).c_str(), xPos, yPos, nFlags);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetHalftoneW(DTWAIN_SOURCE Source, LPCWSTR lpHalftone)
    {
        return DTWAIN_SetHalftone(Source, dtwain_aw_detail::native_input(lpHalftone).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetHalftoneA(DTWAIN_SOURCE Source, LPCSTR lpHalftone)
    {
        return DTWAIN_SetHalftone(Source, dtwain_aw_detail::native_input(lpHalftone).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetHighlightStringA(DTWAIN_SOURCE Source, LPCSTR Highlight)
    {
        return DTWAIN_SetHighlightString(Source, dtwain_aw_detail::native_input(Highlight).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetHighlightStringW(DTWAIN_SOURCE Source, LPCWSTR Highlight)
    {
        return DTWAIN_SetHighlightString(Source, dtwain_aw_detail::native_input(Highlight).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFAuthorW(DTWAIN_SOURCE Source, LPCWSTR lpAuthor)
    {
        return DTWAIN_SetPDFAuthor(Source, dtwain_aw_detail::native_input(lpAuthor).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFAuthorA(DTWAIN_SOURCE Source, LPCSTR lpAuthor)
    {
        return DTWAIN_SetPDFAuthor(Source, dtwain_aw_detail::native_input(lpAuthor).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFCreatorW(DTWAIN_SOURCE Source, LPCWSTR lpCreator)
    {
        return DTWAIN_SetPDFCreator(Source, dtwain_aw_detail::native_input(lpCreator).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFCreatorA(DTWAIN_SOURCE Source, LPCSTR lpCreator)
    {
        return DTWAIN_SetPDFCreator(Source, dtwain_aw_detail::native_input(lpCreator).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFEncryptionW(DTWAIN_SOURCE Source, DTWAIN_BOOL bUseEncryption, LPCWSTR lpszUser, LPCWSTR lpszOwner, DWORD Permissions, DTWAIN_BOOL UseStrongEncryption)
    {
        return DTWAIN_SetPDFEncryption(Source, bUseEncryption, dtwain_aw_detail::native_input(lpszUser).c_str(), dtwain_aw_detail::native_input(lpszOwner).c_str(), Permissions, UseStrongEncryption);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFEncryptionA(DTWAIN_SOURCE Source, DTWAIN_BOOL bUseEncryption, LPCSTR lpszUser, LPCSTR lpszOwner, DWORD Permissions, DTWAIN_BOOL UseStrongEncryption)
    {
        return DTWAIN_SetPDFEncryption(Source, bUseEncryption, dtwain_aw_detail::native_input(lpszUser).c_str(), dtwain_aw_detail::native_input(lpszOwner).c_str(), Permissions, UseStrongEncryption);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFKeywordsA(DTWAIN_SOURCE Source, LPCSTR lpKeyWords)
    {
        return DTWAIN_SetPDFKeywords(Source, dtwain_aw_detail::native_input(lpKeyWords).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFKeywordsW(DTWAIN_SOURCE Source, LPCWSTR lpKeyWords)
    {
        return DTWAIN_SetPDFKeywords(Source, dtwain_aw_detail::native_input(lpKeyWords).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageScaleStringA(DTWAIN_SOURCE Source, LONG nOptions, LPCSTR xScale, LPCSTR yScale)
    {
        return DTWAIN_SetPDFPageScaleString(Source, nOptions, dtwain_aw_detail::native_input(xScale).c_str(), dtwain_aw_detail::native_input(yScale).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageScaleStringW(DTWAIN_SOURCE Source, LONG nOptions, LPCWSTR xScale, LPCWSTR yScale)
    {
        return DTWAIN_SetPDFPageScaleString(Source, nOptions, dtwain_aw_detail::native_input(xScale).c_str(), dtwain_aw_detail::native_input(yScale).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageSizeStringA(DTWAIN_SOURCE Source, LONG PageSize, LPCSTR CustomWidth, LPCSTR CustomHeight)
    {
        return DTWAIN_SetPDFPageSizeString(Source, PageSize, dtwain_aw_detail::native_input(CustomWidth).c_str(), dtwain_aw_detail::native_input(CustomHeight).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageSizeStringW(DTWAIN_SOURCE Source, LONG PageSize, LPCWSTR CustomWidth, LPCWSTR CustomHeight)
    {
        return DTWAIN_SetPDFPageSizeString(Source, PageSize, dtwain_aw_detail::native_input(CustomWidth).c_str(), dtwain_aw_detail::native_input(CustomHeight).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFProducerW(DTWAIN_SOURCE Source, LPCWSTR lpProducer)
    {
        return DTWAIN_SetPDFProducer(Source, dtwain_aw_detail::native_input(lpProducer).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFProducerA(DTWAIN_SOURCE Source, LPCSTR lpProducer)
    {
        return DTWAIN_SetPDFProducer(Source, dtwain_aw_detail::native_input(lpProducer).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFSubjectA(DTWAIN_SOURCE Source, LPCSTR lpSubject)
    {
        return DTWAIN_SetPDFSubject(Source, dtwain_aw_detail::native_input(lpSubject).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFSubjectW(DTWAIN_SOURCE Source, LPCWSTR lpSubject)
    {
        return DTWAIN_SetPDFSubject(Source, dtwain_aw_detail::native_input(lpSubject).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTitleW(DTWAIN_SOURCE Source, LPCWSTR lpTitle)
    {
        return DTWAIN_SetPDFTitle(Source, dtwain_aw_detail::native_input(lpTitle).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTitleA(DTWAIN_SOURCE Source, LPCSTR lpTitle)
    {
        return DTWAIN_SetPDFTitle(Source, dtwain_aw_detail::native_input(lpTitle).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPostScriptTitleW(DTWAIN_SOURCE Source, LPCWSTR szTitle)
    {
        return DTWAIN_SetPostScriptTitle(Source, dtwain_aw_detail::native_input(szTitle).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPostScriptTitleA(DTWAIN_SOURCE Source, LPCSTR szTitle)
    {
        return DTWAIN_SetPostScriptTitle(Source, dtwain_aw_detail::native_input(szTitle).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPrinterSuffixStringW(DTWAIN_SOURCE Source, LPCWSTR Suffix)
    {
        return DTWAIN_SetPrinterSuffixString(Source, dtwain_aw_detail::native_input(Suffix).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPrinterSuffixStringA(DTWAIN_SOURCE Source, LPCSTR Suffix)
    {
        return DTWAIN_SetPrinterSuffixString(Source, dtwain_aw_detail::native_input(Suffix).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetResolutionStringW(DTWAIN_SOURCE Source, LPCWSTR Resolution)
    {
        return DTWAIN_SetResolutionString(Source, dtwain_aw_detail::native_input(Resolution).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetResolutionStringA(DTWAIN_SOURCE Source, LPCSTR Resolution)
    {
        return DTWAIN_SetResolutionString(Source, dtwain_aw_detail::native_input(Resolution).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetRotationStringA(DTWAIN_SOURCE Source, LPCSTR Rotation)
    {
        return DTWAIN_SetRotationString(Source, dtwain_aw_detail::native_input(Rotation).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetRotationStringW(DTWAIN_SOURCE Source, LPCWSTR Rotation)
    {
        return DTWAIN_SetRotationString(Source, dtwain_aw_detail::native_input(Rotation).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetSaveFileNameW(DTWAIN_SOURCE Source, LPCWSTR fName)
    {
        return DTWAIN_SetSaveFileName(Source, dtwain_aw_detail::native_input(fName).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetSaveFileNameA(DTWAIN_SOURCE Source, LPCSTR fName)
    {
        return DTWAIN_SetSaveFileName(Source, dtwain_aw_detail::native_input(fName).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetShadowStringA(DTWAIN_SOURCE Source, LPCSTR Shadow)
    {
        return DTWAIN_SetShadowString(Source, dtwain_aw_detail::native_input(Shadow).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetShadowStringW(DTWAIN_SOURCE Source, LPCWSTR Shadow)
    {
        return DTWAIN_SetShadowString(Source, dtwain_aw_detail::native_input(Shadow).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTempFileDirectoryW(LPCWSTR szFilePath)
    {
        return DTWAIN_SetTempFileDirectory(dtwain_aw_detail::native_input(szFilePath).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTempFileDirectoryA(LPCSTR szFilePath)
    {
        return DTWAIN_SetTempFileDirectory(dtwain_aw_detail::native_input(szFilePath).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDSMSearchOrderExW(LPCWSTR szFilePath, LPCWSTR szUserPath)
    {
#ifdef _UNICODE
        return DTWAIN_SetDSMSearchOrderEx(szFilePath, szUserPath);
#else
        return DTWAIN_SetDSMSearchOrderEx(stringconversion::Convert_WidePtr_To_Native(szFilePath).c_str(),
            szUserPath ? stringconversion::Convert_WidePtr_To_Native(szUserPath).c_str() : nullptr);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDSMSearchOrderExA(LPCSTR szFilePath, LPCSTR szUserPath)
    {
#ifdef _UNICODE
        return DTWAIN_SetDSMSearchOrderEx(stringconversion::Convert_AnsiPtr_To_Native(szFilePath).c_str(),
            szUserPath ? stringconversion::Convert_AnsiPtr_To_Native(szUserPath).c_str() : nullptr);
#else
        return DTWAIN_SetDSMSearchOrderEx(szFilePath, szUserPath);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetResourcePathW(LPCWSTR szFilePath)
    {
        return DTWAIN_SetResourcePath(dtwain_aw_detail::native_input(szFilePath).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetResourcePathA(LPCSTR szFilePath)
    {
        return DTWAIN_SetResourcePath(dtwain_aw_detail::native_input(szFilePath).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetThresholdStringA(DTWAIN_SOURCE Source, LPCSTR Threshold, DTWAIN_BOOL bSetBitDepthReduction)
    {
        return DTWAIN_SetThresholdString(Source, dtwain_aw_detail::native_input(Threshold).c_str(), bSetBitDepthReduction);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetThresholdStringW(DTWAIN_SOURCE Source, LPCWSTR Threshold, DTWAIN_BOOL bSetBitDepthReduction)
    {
        return DTWAIN_SetThresholdString(Source, dtwain_aw_detail::native_input(Threshold).c_str(), bSetBitDepthReduction);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainLogA(DWORD LogFlags, LPCSTR lpszLogFile)
    {
        return DTWAIN_SetTwainLog(LogFlags, dtwain_aw_detail::native_input(lpszLogFile).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainLogW(DWORD LogFlags, LPCWSTR lpszLogFile)
    {
        return DTWAIN_SetTwainLog(LogFlags, dtwain_aw_detail::native_input(lpszLogFile).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetXResolutionStringW(DTWAIN_SOURCE Source, LPCWSTR Resolution)
    {
        return DTWAIN_SetXResolutionString(Source, dtwain_aw_detail::native_input(Resolution).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetXResolutionStringA(DTWAIN_SOURCE Source, LPCSTR Resolution)
    {
        return DTWAIN_SetXResolutionString(Source, dtwain_aw_detail::native_input(Resolution).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetYResolutionStringW(DTWAIN_SOURCE Source, LPCWSTR Resolution)
    {
        return DTWAIN_SetYResolutionString(Source, dtwain_aw_detail::native_input(Resolution).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetYResolutionStringA(DTWAIN_SOURCE Source, LPCSTR Resolution)
    {
        return DTWAIN_SetYResolutionString(Source, dtwain_aw_detail::native_input(Resolution).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_StartTwainSessionW(HWND hWndMsg, LPCWSTR lpszDLLName)
    {
        return DTWAIN_StartTwainSession(hWndMsg, dtwain_aw_detail::native_input(lpszDLLName).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_StartTwainSessionA(HWND hWndMsg, LPCSTR lpszDLLName)
    {
        return DTWAIN_StartTwainSession(hWndMsg, dtwain_aw_detail::native_input(lpszDLLName).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDoubleFeedDetectLengthStringA(DTWAIN_SOURCE Source, LPCSTR szLength)
    {
        return DTWAIN_SetDoubleFeedDetectLengthString(Source, dtwain_aw_detail::native_input(szLength).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDoubleFeedDetectLengthStringW(DTWAIN_SOURCE Source, LPCWSTR szLength)
    {
        return DTWAIN_SetDoubleFeedDetectLengthString(Source, dtwain_aw_detail::native_input(szLength).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTextElementStringW(DTWAIN_PDFTEXTELEMENT TextElement, LPCWSTR szString, LONG Flags)
    {
        return DTWAIN_SetPDFTextElementString(TextElement, dtwain_aw_detail::native_input(szString).c_str(), Flags);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTextElementStringA(DTWAIN_PDFTEXTELEMENT TextElement, LPCSTR szString, LONG Flags)
    {
        return DTWAIN_SetPDFTextElementString(TextElement, dtwain_aw_detail::native_input(szString).c_str(), Flags);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetAllFloatStringW(DTWAIN_RANGE pArray, LPWSTR dLow,
        LPWSTR dUp, LPWSTR dStep,
        LPWSTR dDefault,
        LPWSTR dCurrent)
    {
#ifdef _UNICODE
        return DTWAIN_RangeGetAllFloatString(pArray, dLow, dUp, dStep, dDefault, dCurrent);
#else
        std::array<LPWSTR, 5> outarg = { dLow, dUp, dStep, dDefault, dCurrent };
        std::array<std::string, 5> args = { { std::string(128, 0), std::string(128, 0), std::string(128, 0), std::string(128, 0), std::string(128, 0) } };
        DTWAIN_BOOL retVal = DTWAIN_RangeGetAllFloatString(pArray, args[0].data(), args[1].data(), args[2].data(),
                                                           args[3].data(), args[4].data());
        for (size_t i = 0; i < args.size(); ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#endif
    }


    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetAllFloatStringA(DTWAIN_RANGE pArray, LPSTR dLow,
        LPSTR dUp, LPSTR dStep,
        LPSTR dDefault,
        LPSTR dCurrent)
    {
#ifdef _UNICODE
        const std::array<LPSTR, 5> outarg = { dLow, dUp, dStep, dDefault, dCurrent };
        std::array<std::wstring, 5> args = { { std::wstring(128, 0), std::wstring(128, 0), std::wstring(128, 0), std::wstring(128, 0), std::wstring(128, 0) } };
        const DTWAIN_BOOL retVal = DTWAIN_RangeGetAllFloatString(pArray, args[0].data(), args[1].data(), args[2].data(),
                                                                 args[3].data(), args[4].data());
        for (size_t i = 0; i < outarg.size(); ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#else
        return DTWAIN_RangeGetAllFloatString(pArray, dLow, dUp, dStep, dDefault, dCurrent);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeSetAllFloatStringW(DTWAIN_RANGE pArray, LPCWSTR dLow,
        LPCWSTR dUp, LPCWSTR dStep,
        LPCWSTR dDefault,
        LPCWSTR dCurrent)
    {
#ifdef _UNICODE
        return DTWAIN_RangeSetAllFloatString(pArray, dLow, dUp, dStep, dDefault, dCurrent);
#else
        return DTWAIN_RangeSetAllFloatString(pArray,
            stringconversion::Convert_WidePtr_To_Native(dLow).c_str(),
            stringconversion::Convert_WidePtr_To_Native(dUp).c_str(),
            stringconversion::Convert_WidePtr_To_Native(dStep).c_str(),
            stringconversion::Convert_WidePtr_To_Native(dDefault).c_str(),
            stringconversion::Convert_WidePtr_To_Native(dCurrent).c_str());
#endif
    }


    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeSetAllFloatStringA(DTWAIN_RANGE pArray, LPCSTR dLow,
        LPCSTR dUp, LPCSTR dStep,
        LPCSTR dDefault,
        LPCSTR dCurrent)
    {
#ifdef _UNICODE
        return DTWAIN_RangeSetAllFloatString(pArray,
            stringconversion::Convert_AnsiPtr_To_Native(dLow).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(dUp).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(dStep).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(dDefault).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(dCurrent).c_str());
#else
        return DTWAIN_RangeSetAllFloatString(pArray, dLow, dUp, dStep, dDefault, dCurrent);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeSetValueFloatStringW(DTWAIN_RANGE pArray, LONG nWhich, LPCWSTR dValue)
    {
        return DTWAIN_RangeSetValueFloatString(pArray, nWhich, dtwain_aw_detail::native_input(dValue).c_str());
    }


    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeSetValueFloatStringA(DTWAIN_RANGE pArray, LONG nWhich, LPCSTR dValue)
    {
        return DTWAIN_RangeSetValueFloatString(pArray, nWhich, dtwain_aw_detail::native_input(dValue).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetNearestValueFloatStringW(DTWAIN_RANGE pArray, LPCWSTR dIn, LPWSTR dOut, LONG RoundType)
    {
#ifdef _UNICODE
        return DTWAIN_RangeGetNearestValueFloatString(pArray, dIn, dOut, RoundType);
#else
        std::string arg(128, 0);
        DTWAIN_BOOL retVal = DTWAIN_RangeGetNearestValueFloatString(pArray,
            stringconversion::Convert_WidePtr_To_Native(dIn).c_str(),
            arg.data(), RoundType);
        return null_terminator_copier(get_view(arg), dOut, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetNearestValueFloatStringA(DTWAIN_RANGE pArray, LPCSTR dIn, LPSTR dOut, LONG RoundType)
    {
#ifdef _UNICODE
        std::wstring arg(128, 0);
        const DTWAIN_BOOL retVal = DTWAIN_RangeGetNearestValueFloatString(pArray,
            stringconversion::Convert_AnsiPtr_To_Native(dIn).c_str(),
            arg.data(), RoundType);
        return null_terminator_copier(get_view(arg), dOut, retVal);
#else
        return DTWAIN_RangeGetNearestValueFloatString(pArray, dIn, dOut, RoundType);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetValueFloatStringW(DTWAIN_RANGE pArray, LONG nWhich, LPWSTR dValue)
    {
#ifdef _UNICODE
        return DTWAIN_RangeGetValueFloatString(pArray, nWhich, dValue);
#else
        std::string arg(128, 0);
        DTWAIN_BOOL retval = DTWAIN_RangeGetValueFloatString(pArray, nWhich, arg.data());
        return null_terminator_copier(get_view(arg), dValue, retval);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetValueFloatStringA(DTWAIN_RANGE pArray, LONG nWhich, LPSTR dValue)
    {
#ifdef _UNICODE
        std::wstring arg(128, 0);
        const DTWAIN_BOOL retval = DTWAIN_RangeGetValueFloatString(pArray, nWhich, arg.data());
        return null_terminator_copier(get_view(arg), dValue, retval);
#else
        return DTWAIN_RangeGetValueFloatString(pArray, nWhich, dValue);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetPosFloatStringW(DTWAIN_RANGE pArray, LPCWSTR Val, LPLONG pPos)
    {
#ifdef _UNICODE
        return DTWAIN_RangeGetPosFloatString(pArray, Val, pPos);
#else
        return DTWAIN_RangeGetPosFloatString(pArray,
            stringconversion::Convert_WidePtr_To_Native(Val).c_str(),
            pPos);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetPosFloatStringA(DTWAIN_RANGE pArray, LPCSTR Val, LPLONG pPos)
    {
#ifdef _UNICODE
        return DTWAIN_RangeGetPosFloatString(pArray,
            stringconversion::Convert_AnsiPtr_To_Native(Val).c_str(),
            pPos);
#else
        return DTWAIN_RangeGetPosFloatString(pArray, Val, pPos);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetExpValueFloatStringW(DTWAIN_RANGE pArray, LONG lPos, LPWSTR pVal)
    {
#ifdef _UNICODE
        return DTWAIN_RangeGetExpValueFloatString(pArray, lPos, pVal);
#else
        std::string arg(128, 0);
        DTWAIN_BOOL retval = DTWAIN_RangeGetExpValueFloatString(pArray, lPos, arg.data());
        return null_terminator_copier(get_view(arg), pVal, retval);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetExpValueFloatStringA(DTWAIN_RANGE pArray, LONG lPos, LPSTR pVal)
    {
#ifdef _UNICODE
        std::wstring arg(128, 0);
        const DTWAIN_BOOL retval = DTWAIN_RangeGetExpValueFloatString(pArray, lPos, arg.data());
        return null_terminator_copier(get_view(arg), pVal, retval);
#else
        return DTWAIN_RangeGetExpValueFloatString(pArray, lPos, pVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameSetValueStringW(DTWAIN_FRAME Frame, LONG nWhich, LPCWSTR Value)
    {
        return DTWAIN_FrameSetValueString(Frame, nWhich, dtwain_aw_detail::native_input(Value).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameSetValueStringA(DTWAIN_FRAME Frame, LONG nWhich, LPCSTR Value)
    {
        return DTWAIN_FrameSetValueString(Frame, nWhich, dtwain_aw_detail::native_input(Value).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameSetAllStringW(DTWAIN_FRAME Frame, LPCWSTR Left, LPCWSTR Top, LPCWSTR Right, LPCWSTR Bottom)
    {
#ifdef _UNICODE
        return DTWAIN_FrameSetAllString(Frame, Left, Top, Right, Bottom);
#else
        return DTWAIN_FrameSetAllString(Frame,
            stringconversion::Convert_WidePtr_To_Native(Left).c_str(),
            stringconversion::Convert_WidePtr_To_Native(Top).c_str(),
            stringconversion::Convert_WidePtr_To_Native(Right).c_str(),
            stringconversion::Convert_WidePtr_To_Native(Bottom).c_str());
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameSetAllStringA(DTWAIN_FRAME Frame, LPCSTR Left, LPCSTR Top, LPCSTR Right, LPCSTR Bottom)
    {
#ifdef _UNICODE
        return DTWAIN_FrameSetAllString(Frame,
            stringconversion::Convert_AnsiPtr_To_Native(Left).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(Top).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(Right).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(Bottom).c_str());
#else
        return DTWAIN_FrameSetAllString(Frame, Left, Top, Right, Bottom);
#endif
    }


    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameGetValueStringW(DTWAIN_FRAME Frame, LONG nWhich, LPWSTR Value)
    {
#ifdef _UNICODE
        return DTWAIN_FrameGetValueString(Frame, nWhich, Value);
#else
        std::string arg(128, 0);
        DTWAIN_BOOL retval = DTWAIN_FrameGetValueString(Frame, nWhich, arg.data());
        return null_terminator_copier(get_view(arg), Value, retval);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameGetValueStringA(DTWAIN_FRAME Frame, LONG nWhich, LPSTR Value)
    {
#ifdef _UNICODE
        std::wstring arg(128, 0);
        const DTWAIN_BOOL retval = DTWAIN_FrameGetValueString(Frame, nWhich, arg.data());
        return null_terminator_copier(get_view(arg), Value, retval);
#else
        return DTWAIN_FrameGetValueString(Frame, nWhich, Value);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameGetAllStringW(DTWAIN_FRAME Frame, LPWSTR Left, LPWSTR Top, LPWSTR Right, LPWSTR Bottom)
    {
#ifdef _UNICODE
        return DTWAIN_FrameGetAllString(Frame, Left, Top, Right, Bottom);
#else
        std::array<LPWSTR, 4> outarg = { Left, Top, Right, Bottom };
        std::array<std::string, 4> args = { { std::string(128, 0), std::string(128, 0), std::string(128, 0), std::string(128, 0) } };
        DTWAIN_BOOL retVal = DTWAIN_FrameGetAllString(Frame, args[0].data(), args[1].data(), args[2].data(),
                                                      args[3].data());
        for (size_t i = 0; i < outarg.size(); ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameGetAllStringA(DTWAIN_FRAME Frame, LPSTR Left, LPSTR Top, LPSTR Right, LPSTR Bottom)
    {
#ifdef _UNICODE
        const std::array<LPSTR, 4> outarg = { Left, Top, Right, Bottom };
        std::array<std::wstring, 4> args = { { std::wstring(128, 0), std::wstring(128, 0), std::wstring(128, 0), std::wstring(128, 0) } };
        const DTWAIN_BOOL retVal = DTWAIN_FrameGetAllString(Frame, args[0].data(), args[1].data(), args[2].data(),
                                                            args[3].data());
        for (size_t i = 0; i < outarg.size(); ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#else
        return DTWAIN_FrameGetAllString(Frame, Left, Top, Right, Bottom);
#endif
    }

    DTWAIN_FRAME DLLENTRY_DEF DTWAIN_FrameCreateStringW(LPCWSTR Left, LPCWSTR Top, LPCWSTR Right, LPCWSTR Bottom)
    {
#ifdef _UNICODE
        return DTWAIN_FrameCreateString(Left, Top, Right, Bottom);
#else
        return DTWAIN_FrameCreateString(stringconversion::Convert_WidePtr_To_Native(Left).c_str(),
            stringconversion::Convert_WidePtr_To_Native(Top).c_str(),
            stringconversion::Convert_WidePtr_To_Native(Right).c_str(),
            stringconversion::Convert_WidePtr_To_Native(Bottom).c_str());
#endif
    }

    DTWAIN_FRAME DLLENTRY_DEF DTWAIN_FrameCreateStringA(LPCSTR Left, LPCSTR Top, LPCSTR Right, LPCSTR Bottom)
    {
#ifdef _UNICODE
        return DTWAIN_FrameCreateString(stringconversion::Convert_AnsiPtr_To_Native(Left).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(Top).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(Right).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(Bottom).c_str());
#else
        return DTWAIN_FrameCreateString(Left, Top, Right, Bottom);
#endif
    }

    LONG DLLENTRY_DEF DTWAIN_GetFileTypeExtensionsA(LONG nType, LPSTR lpszName, LONG nMaxLen)
    {
        return output_with_length(lpszName, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetFileTypeExtensions(nType, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetFileTypeExtensionsW(LONG nType, LPWSTR lpszName, LONG nMaxLen)
    {
        return output_with_length(lpszName, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetFileTypeExtensions(nType, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetFileTypeNameA(LONG nType, LPSTR lpszName, LONG nMaxLen)
    {
        return output_with_length(lpszName, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetFileTypeName(nType, nativeOut, nativeCap); });
    }


    LONG DLLENTRY_DEF DTWAIN_GetFileTypeNameW(LONG nType, LPWSTR lpszName, LONG nMaxLen)
    {
        return output_with_length(lpszName, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetFileTypeName(nType, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetSessionDetailsA(LPSTR lpszBuf, LONG nLength, LONG indentFactor, BOOL bRefresh)
    {
        return output_with_length(lpszBuf, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSessionDetails(nativeOut, nativeCap, indentFactor, bRefresh); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetSessionDetailsW(LPWSTR lpszBuf, LONG nLength, LONG indentFactor, BOOL bRefresh)
    {
        return output_with_length(lpszBuf, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSessionDetails(nativeOut, nativeCap, indentFactor, bRefresh); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetSourceDetailsA(LPCSTR lpszSources, LPSTR lpszBuf, LONG nLength, LONG indentFactor, BOOL bRefresh)
    {
        return output_with_length(lpszBuf, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSourceDetails(dtwain_aw_detail::native_input(lpszSources).c_str(), nativeOut, nativeCap, indentFactor, bRefresh); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetSourceDetailsW(LPCWSTR lpszSources, LPWSTR lpszBuf, LONG nLength, LONG indentFactor, BOOL bRefresh)
    {
        return output_with_length(lpszBuf, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetSourceDetails(dtwain_aw_detail::native_input(lpszSources).c_str(), nativeOut, nativeCap, indentFactor, bRefresh); });
    }

    LONG  DLLENTRY_DEF DTWAIN_GetVersionCopyrightA(LPSTR lpszApp, LONG nLength)
    {
        return output_with_length(lpszApp, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetVersionCopyright(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetVersionCopyrightW(LPWSTR lpszApp, LONG nLength)
    {
        return output_with_length(lpszApp, nLength,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetVersionCopyright(nativeOut, nativeCap); });
    }

    DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngine2A(HWND hWndParent, LPCSTR szTitle, LONG xPos, LONG yPos, LONG nOptions)
    {
#ifdef _UNICODE
        return DTWAIN_SelectOCREngine2(hWndParent, szTitle ? stringconversion::Convert_AnsiPtr_To_Native(szTitle).c_str() : nullptr, xPos, yPos, nOptions);
#else
        return DTWAIN_SelectOCREngine2(hWndParent, szTitle, xPos, yPos, nOptions);
#endif
    }

    DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngine2W(HWND hWndParent, LPCWSTR szTitle, LONG xPos, LONG yPos, LONG nOptions)
    {
#ifdef _UNICODE
        return DTWAIN_SelectOCREngine2(hWndParent, szTitle, xPos, yPos, nOptions);
#else
        return DTWAIN_SelectOCREngine2(hWndParent,
            szTitle ? stringconversion::Convert_WidePtr_To_Native(szTitle).c_str() : nullptr, xPos, yPos, nOptions);
#endif
    }

    DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngine2ExA(HWND hWndParent, LPCSTR szTitle, LONG xPos, LONG yPos,
        LPCSTR szIncludeNames, LPCSTR szExcludeNames, LPCSTR szNameMapping, LONG nOptions)
    {
#ifdef _UNICODE
        return DTWAIN_SelectOCREngine2Ex(hWndParent,
            szTitle ? stringconversion::Convert_AnsiPtr_To_Native(szTitle).c_str() : nullptr, xPos, yPos,
            szIncludeNames ? stringconversion::Convert_AnsiPtr_To_Native(szIncludeNames).c_str() : nullptr,
            szExcludeNames ? stringconversion::Convert_AnsiPtr_To_Native(szExcludeNames).c_str() : nullptr,
            szNameMapping ? stringconversion::Convert_AnsiPtr_To_Native(szNameMapping).c_str() : nullptr,
            nOptions);
#else
        return DTWAIN_SelectOCREngine2Ex(hWndParent, szTitle, xPos, yPos, szIncludeNames, szExcludeNames, szNameMapping, nOptions);
#endif
    }

    DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectOCREngine2ExW(HWND hWndParent, LPCWSTR szTitle, LONG xPos, LONG yPos, LPCWSTR szIncludeNames, LPCWSTR szExcludeNames, LPCWSTR szNameMapping, LONG nOptions)
    {
#ifdef _UNICODE
        return DTWAIN_SelectOCREngine2Ex(hWndParent, szTitle, xPos, yPos, szIncludeNames, szExcludeNames, szNameMapping, nOptions);
#else
        return DTWAIN_SelectOCREngine2Ex(hWndParent,
            szTitle ? stringconversion::Convert_WidePtr_To_Native(szTitle).c_str() : nullptr,
            xPos, yPos,
            szIncludeNames ? stringconversion::Convert_WidePtr_To_Native(szIncludeNames).c_str() : nullptr,
            szExcludeNames ? stringconversion::Convert_WidePtr_To_Native(szExcludeNames).c_str() : nullptr,
            szNameMapping ? stringconversion::Convert_WidePtr_To_Native(szNameMapping).c_str() : nullptr,
            nOptions);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTempFileDirectoryExW(LPCWSTR sLangDLL, DTWAIN_BOOL bClear)
    {
        return DTWAIN_SetTempFileDirectoryEx(dtwain_aw_detail::native_input(sLangDLL).c_str(), bClear);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTempFileDirectoryExA(LPCSTR sLangDLL, DTWAIN_BOOL bClear)
    {
        return DTWAIN_SetTempFileDirectoryEx(dtwain_aw_detail::native_input(sLangDLL).c_str(), bClear);
    }

    LONG DLLENTRY_DEF DTWAIN_ConvertToAPIStringExA(LPCSTR lpOrigString, LPSTR lpOutString, LONG nSize)
    {
#ifdef _UNICODE
        std::wstring arg(nSize + 1, 0);
        const DTWAIN_BOOL retVal = DTWAIN_ConvertToAPIStringEx(stringconversion::Convert_AnsiPtr_To_Native(lpOrigString).c_str(),
            lpOutString ? arg.data() : nullptr, nSize);
        return null_terminator_copier(get_view(arg), lpOutString, retVal);
#else
        return DTWAIN_ConvertToAPIStringEx(lpOrigString, lpOutString, nSize);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ConvertToAPIStringExW(LPCWSTR lpOrigString, LPWSTR lpOutString, LONG nSize)
    {
#ifdef _UNICODE
        return DTWAIN_ConvertToAPIStringEx(lpOrigString, lpOutString, nSize);
#else
        std::string arg(nSize + 1, 0);
        DTWAIN_BOOL retVal = DTWAIN_ConvertToAPIStringEx(stringconversion::Convert_WidePtr_To_Native(lpOrigString).c_str(),
            lpOutString ? arg.data() : nullptr, nSize);
        return null_terminator_copier(get_view(arg), lpOutString, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAtFrameStringA(DTWAIN_ARRAY FrameArray, LONG nWhere,
        LPSTR left, LPSTR top, LPSTR right, LPSTR bottom)
    {
#ifdef _UNICODE
        const std::array<LPSTR, 4> outarg = { left, top, right, bottom };
        std::array<std::wstring, 4> args = { {std::wstring(1024, 0), std::wstring(1024, 0), std::wstring(1024, 0), std::wstring(1024, 0)} };
        const DTWAIN_BOOL retVal = DTWAIN_ArrayGetAtFrameString(FrameArray, nWhere, args[0].data(), args[1].data(),
                                                                args[2].data(), args[3].data());
        for (size_t i = 0; i < 4; ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#else
        return DTWAIN_ArrayGetAtFrameString(FrameArray, nWhere, left, top, right, bottom);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAtFrameStringW(DTWAIN_ARRAY FrameArray, LONG nWhere,
        LPWSTR left, LPWSTR top, LPWSTR right, LPWSTR bottom)
    {
#ifdef _UNICODE
        return DTWAIN_ArrayGetAtFrameString(FrameArray, nWhere, left, top, right, bottom);
#else
        std::array<LPWSTR, 4> outarg = { left, top, right, bottom };
        std::array<std::string, 4> args = { {std::string(1024, 0), std::string(1024, 0), std::string(1024, 0), std::string(1024, 0)} };
        DTWAIN_BOOL retVal = DTWAIN_ArrayGetAtFrameString(FrameArray, nWhere, args[0].data(), args[1].data(),
                                                          args[2].data(), args[3].data());
        for (size_t i = 0; i < args.size(); ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtFrameStringA(DTWAIN_ARRAY FrameArray, LONG nWhere,
        LPCSTR left, LPCSTR top, LPCSTR right, LPCSTR bottom)
    {
#ifdef _UNICODE
        return DTWAIN_ArraySetAtFrameString(FrameArray, nWhere,
            left ? stringconversion::Convert_AnsiPtr_To_Native(left).c_str() : nullptr,
            top ? stringconversion::Convert_AnsiPtr_To_Native(top).c_str() : nullptr,
            right ? stringconversion::Convert_AnsiPtr_To_Native(right).c_str() : nullptr,
            bottom ? stringconversion::Convert_AnsiPtr_To_Native(bottom).c_str() : nullptr);

#else
        return DTWAIN_ArraySetAtFrameString(FrameArray, nWhere, left, top, right, bottom);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtFrameStringW(DTWAIN_ARRAY FrameArray, LONG nWhere,
        LPCWSTR left, LPCWSTR top, LPCWSTR right, LPCWSTR bottom)
    {
#ifdef _UNICODE
        return DTWAIN_ArraySetAtFrameString(FrameArray, nWhere, left, top, right, bottom);
#else
        return DTWAIN_ArraySetAtFrameString(FrameArray, nWhere,
            left ? stringconversion::Convert_WidePtr_To_Native(left).c_str() : nullptr,
            top ? stringconversion::Convert_WidePtr_To_Native(top).c_str() : nullptr,
            right ? stringconversion::Convert_WidePtr_To_Native(right).c_str() : nullptr,
            bottom ? stringconversion::Convert_WidePtr_To_Native(bottom).c_str() : nullptr);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtFloatStringW(DTWAIN_RANGE pArray, LONG nWhich, LPCWSTR dValue)
    {
        return DTWAIN_ArraySetAtFloatString(pArray, nWhich, dtwain_aw_detail::native_input(dValue).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtFloatStringA(DTWAIN_RANGE pArray, LONG nWhich, LPCSTR dValue)
    {
        return DTWAIN_ArraySetAtFloatString(pArray, nWhich, dtwain_aw_detail::native_input(dValue).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAtFloatStringA(DTWAIN_ARRAY pArray, LONG nWhere, LPSTR dValue)
    {
#ifdef _UNICODE
        std::wstring arg(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_ArrayGetAtFloatString(pArray, nWhere, arg.data());
        return null_terminator_copier(get_view(arg), dValue, retVal);
#else
        return DTWAIN_ArrayGetAtFloatString(pArray, nWhere, dValue);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAtFloatStringW(DTWAIN_ARRAY pArray, LONG nWhere, LPWSTR dValue)
    {
#ifdef _UNICODE
        return DTWAIN_ArrayGetAtFloatString(pArray, nWhere, dValue);
#else
        std::string arg(1024, 0);
        DTWAIN_BOOL retVal = DTWAIN_ArrayGetAtFloatString(pArray, nWhere, arg.data());
        return null_terminator_copier(get_view(arg), dValue, retVal);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtFloatStringW(DTWAIN_RANGE pArray, LONG nWhich, LPCWSTR dValue)
    {
        return DTWAIN_ArrayInsertAtFloatString(pArray, nWhich, dtwain_aw_detail::native_input(dValue).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtFloatStringA(DTWAIN_RANGE pArray, LONG nWhich, LPCSTR dValue)
    {
        return DTWAIN_ArrayInsertAtFloatString(pArray, nWhich, dtwain_aw_detail::native_input(dValue).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtFloatStringNW(DTWAIN_ARRAY pArray, LONG nWhere, LPCWSTR Val, LONG num)
    {
        return DTWAIN_ArrayInsertAtFloatStringN(pArray, nWhere, dtwain_aw_detail::native_input(Val).c_str(), num);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtFloatStringNA(DTWAIN_ARRAY pArray, LONG nWhere, LPCSTR Val, LONG num)
    {
        return DTWAIN_ArrayInsertAtFloatStringN(pArray, nWhere, dtwain_aw_detail::native_input(Val).c_str(), num);
    }

    LONG DLLENTRY_DEF DTWAIN_ArrayFindFloatStringW(DTWAIN_ARRAY pArray, LPCWSTR Val, LPCWSTR Tolerance)
    {
#ifdef _UNICODE
        return DTWAIN_ArrayFindFloatString(pArray, Val, Tolerance);
#else
        return DTWAIN_ArrayFindFloatString(pArray, stringconversion::Convert_WidePtr_To_Native(Val).c_str(),
            stringconversion::Convert_WidePtr_To_Native(Tolerance).c_str());
#endif
    }

    LONG DLLENTRY_DEF DTWAIN_ArrayFindFloatStringA(DTWAIN_ARRAY pArray, LPCSTR Val, LPCSTR Tolerance)
    {
#ifdef _UNICODE
        return DTWAIN_ArrayFindFloatString(pArray, stringconversion::Convert_AnsiPtr_To_Native(Val).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(Tolerance).c_str());
#else
        return DTWAIN_ArrayFindFloatString(pArray, Val, Tolerance);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddFloatStringW(DTWAIN_ARRAY pArray, LPCWSTR Val)
    {
        return DTWAIN_ArrayAddFloatString(pArray, dtwain_aw_detail::native_input(Val).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddFloatStringA(DTWAIN_ARRAY pArray, LPCSTR Val)
    {
        return DTWAIN_ArrayAddFloatString(pArray, dtwain_aw_detail::native_input(Val).c_str());
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddFloatStringNW(DTWAIN_ARRAY pArray, LPCWSTR Val, LONG num)
    {
        return DTWAIN_ArrayAddFloatStringN(pArray, dtwain_aw_detail::native_input(Val).c_str(), num);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddFloatStringNA(DTWAIN_ARRAY pArray, LPCSTR Val, LONG num)
    {
        return DTWAIN_ArrayAddFloatStringN(pArray, dtwain_aw_detail::native_input(Val).c_str(), num);
    }


    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTextElementFloatStringA(DTWAIN_PDFTEXTELEMENT TextElement, LPCSTR val1, LPCSTR val2, LONG Flags)
    {
#ifdef _UNICODE
        return DTWAIN_SetPDFTextElementFloatString(TextElement,
            stringconversion::Convert_AnsiPtr_To_Native(val1).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(val2).c_str(),
            Flags);
#else
        return DTWAIN_SetPDFTextElementFloatString(TextElement, val1, val2, Flags);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTextElementFloatStringW(DTWAIN_PDFTEXTELEMENT TextElement, LPCWSTR val1, LPCWSTR val2, LONG Flags)
    {
#ifdef _UNICODE
        return DTWAIN_SetPDFTextElementFloatString(TextElement, val1, val2, Flags);
#else
        return DTWAIN_SetPDFTextElementFloatString(TextElement,
            stringconversion::Convert_WidePtr_To_Native(val1).c_str(),
            stringconversion::Convert_WidePtr_To_Native(val2).c_str(),
            Flags);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddPDFTextStringA(DTWAIN_SOURCE Source, LPCSTR szText, LONG xPos, LONG yPos, LPCSTR fontName, LPCSTR fontSize,
        LONG colorRGB, LONG renderMode, LPCSTR scaling, LPCSTR charSpacing, LPCSTR wordSpacing,
        LPCSTR strokeWidth, DWORD Flags)
    {
#ifdef _UNICODE
        return DTWAIN_AddPDFTextString(Source,
            stringconversion::Convert_AnsiPtr_To_Native(szText).c_str(), xPos, yPos,
            stringconversion::Convert_AnsiPtr_To_Native(fontName).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(fontSize).c_str(), colorRGB, renderMode,
            stringconversion::Convert_AnsiPtr_To_Native(scaling).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(charSpacing).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(wordSpacing).c_str(),
            stringconversion::Convert_AnsiPtr_To_Native(strokeWidth).c_str(), Flags);
#else
        return DTWAIN_AddPDFTextString(Source, szText, xPos, yPos, fontName, fontSize, colorRGB,
            renderMode, scaling, charSpacing, wordSpacing, strokeWidth, Flags);
#endif
    }


    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddPDFTextStringW(DTWAIN_SOURCE Source, LPCWSTR szText, LONG xPos, LONG yPos, LPCWSTR fontName, LPCWSTR fontSize,
        LONG colorRGB, LONG renderMode, LPCWSTR scaling, LPCWSTR charSpacing, LPCWSTR wordSpacing,
        LPCWSTR strokeWidth, DWORD Flags)
    {
#ifdef _UNICODE
        return DTWAIN_AddPDFTextString(Source, szText, xPos, yPos, fontName, fontSize, colorRGB,
            renderMode, scaling, charSpacing, wordSpacing, strokeWidth, Flags);
#else
        return DTWAIN_AddPDFTextString(Source,
            stringconversion::Convert_WidePtr_To_Native(szText).c_str(), xPos, yPos,
            stringconversion::Convert_WidePtr_To_Native(fontName).c_str(),
            stringconversion::Convert_WidePtr_To_Native(fontSize).c_str(), colorRGB, renderMode,
            stringconversion::Convert_WidePtr_To_Native(scaling).c_str(),
            stringconversion::Convert_WidePtr_To_Native(charSpacing).c_str(),
            stringconversion::Convert_WidePtr_To_Native(wordSpacing).c_str(),
            stringconversion::Convert_WidePtr_To_Native(strokeWidth).c_str(),
            Flags);
#endif
    }

    LONG DLLENTRY_DEF DTWAIN_GetTwainNameFromConstantExW(LONG lConstantType, LONG lTwainConstant, LPWSTR lpszOut, LONG nSize)
    {
        return output_with_length(lpszOut, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetTwainNameFromConstantEx(lConstantType, lTwainConstant, nativeOut, nativeCap); });
    }


    LONG DLLENTRY_DEF DTWAIN_GetTwainNameFromConstantExA(LONG lConstantType, LONG lTwainConstant, LPSTR lpszOut, LONG nSize)
    {
        return output_with_length(lpszOut, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetTwainNameFromConstantEx(lConstantType, lTwainConstant, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetDSMSearchOrderExA(LPSTR SearchDirectory, LPSTR UserDirectory)
    {
#ifdef _UNICODE
        std::array<LPSTR, 2> outarg = { SearchDirectory, UserDirectory };
        std::array<std::wstring, 2> args = { {std::wstring(10, 0), std::wstring(32767, 0)} };
        const LONG retVal = DTWAIN_GetDSMSearchOrderEx(args[0].data(), args[1].data());
        for (size_t i = 0; i < args.size(); ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#else
        return DTWAIN_GetDSMSearchOrderEx(SearchDirectory, UserDirectory);
#endif
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetDSMSearchOrderExW(LPWSTR SearchDirectory, LPWSTR UserDirectory)
    {
#ifdef _UNICODE
        return DTWAIN_GetDSMSearchOrderEx(SearchDirectory, UserDirectory);
#else
        std::array<LPWSTR, 2> outarg = { SearchDirectory, UserDirectory };
        std::array<std::string, 2> args = { {std::string(10, 0), std::string(32767, 0)} };
        const LONG retVal = DTWAIN_GetDSMSearchOrderEx(args[0].data(), args[1].data());
        for (size_t i = 0; i < args.size(); ++i)
            null_terminator_copier(get_view(args[i]), outarg[i], retVal);
        return retVal;
#endif
    }

    HANDLE DLLENTRY_DEF DTWAIN_RotateImageStringA(HANDLE hDib, LPCSTR angle)
    {
        return DTWAIN_RotateImageString(hDib, dtwain_aw_detail::native_input(angle).c_str());
    }

    HANDLE DLLENTRY_DEF DTWAIN_RotateImageStringW(HANDLE hDib, LPCWSTR angle)
    {
        return DTWAIN_RotateImageString(hDib, dtwain_aw_detail::native_input(angle).c_str());
    }

    LONG DLLENTRY_DEF DTWAIN_GetCapLabelW(LONG lCapability, LPWSTR lpszOut, LONG nSize)
    {
        return output_with_length(lpszOut, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetCapLabel(lCapability, nativeOut, nativeCap); });
    }


    LONG DLLENTRY_DEF DTWAIN_GetCapLabelA(LONG lCapability, LPSTR lpszOut, LONG nSize)
    {
        return output_with_length(lpszOut, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetCapLabel(lCapability, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetCapHelpW(LONG lCapability, LPWSTR lpszOut, LONG nSize)
    {
        return output_with_length(lpszOut, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetCapHelp(lCapability, nativeOut, nativeCap); });
    }


    LONG DLLENTRY_DEF DTWAIN_GetCapHelpA(LONG lCapability, LPSTR lpszOut, LONG nSize)
    {
        return output_with_length(lpszOut, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetCapHelp(lCapability, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetAllSourceInfoA(DTWAIN_SOURCE Source, LPSTR lpszOut, LONG indentFactor, LONG nSize)
    {
        return output_with_length(lpszOut, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetAllSourceInfo(Source, nativeOut, indentFactor, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetAllSourceInfoW(DTWAIN_SOURCE Source, LPWSTR lpszOut, LONG indentFactor, LONG nSize)
    {
        return output_with_length(lpszOut, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetAllSourceInfo(Source, nativeOut, indentFactor, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetAllSessionInfoA(LPSTR lpszOut, LONG indentFactor, LONG nSize)
    {
        return output_with_length(lpszOut, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetAllSessionInfo(nativeOut, indentFactor, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetAllSessionInfoW(LPWSTR lpszOut, LONG indentFactor, LONG nSize)
    {
        return output_with_length(lpszOut, nSize,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetAllSessionInfo(nativeOut, indentFactor, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetCurrentCustomResourceNameA(LPSTR lpszOut, LONG nMaxLen)
    {
        return output_with_length(lpszOut, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetCurrentCustomResourceName(nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetCurrentCustomResourceNameW(LPWSTR lpszOut, LONG nMaxLen)
    {
        return output_with_length(lpszOut, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetCurrentCustomResourceName(nativeOut, nativeCap); });
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetComponentPathA(LONG nComponent, LPCSTR szFilePath)
    {
        return DTWAIN_SetComponentPath(nComponent, szFilePath ? dtwain_aw_detail::native_input(szFilePath).c_str() : nullptr);
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetComponentPathW(LONG nComponent, LPCWSTR szFilePath)
    {
        return DTWAIN_SetComponentPath(nComponent, szFilePath ? dtwain_aw_detail::native_input(szFilePath).c_str() : nullptr);
    }

    LONG DLLENTRY_DEF DTWAIN_GetComponentPathA(LONG nComponent, LPSTR lpszOut, LONG nMaxLen)
    {
        return output_with_length(lpszOut, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetComponentPath(nComponent, nativeOut, nativeCap); });
    }

    LONG DLLENTRY_DEF DTWAIN_GetComponentPathW(LONG nComponent, LPWSTR lpszOut, LONG nMaxLen)
    {
        return output_with_length(lpszOut, nMaxLen,
            [&](TCHAR* nativeOut, LONG nativeCap) { return DTWAIN_GetComponentPath(nComponent, nativeOut, nativeCap); });
    }
}
