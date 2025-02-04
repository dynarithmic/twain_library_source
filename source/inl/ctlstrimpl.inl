/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2025 Dynarithmic Software.

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
#ifndef CTLSTRIMPL_INL
#define CTLSTRIMPL_INL

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
    #include <string_view>
    namespace strview = std;
#else
    #include <boost/utility/string_view.hpp>
    namespace strview = boost;
#endif

#include <algorithm>

#ifdef _MSC_VER
#pragma warning (disable:4244)
#endif

using namespace dynarithmic;

template <typename StringTypeIn>
strview::basic_string_view<typename StringTypeIn::value_type> get_view(const StringTypeIn& str)
{
    if ( !str.empty() )
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
            *buffer = static_cast<std::remove_pointer<StringTypeOut>::type>(*iter);
            ++iter;
            ++buffer;
        }
        *buffer = 0;
    }
    return retVal;
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumSourceValuesW(DTWAIN_SOURCE Source, LPCWSTR capName, LPDTWAIN_ARRAY pArray, DTWAIN_BOOL expandIfRange)
{
#ifdef _UNICODE
    return DTWAIN_EnumSourceValues(Source, capName, pArray, expandIfRange);
#else
    return DTWAIN_EnumSourceValues(Source, StringConversion::Convert_WidePtr_To_Native(capName).c_str(), pArray, expandIfRange);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumSourceValuesA(DTWAIN_SOURCE Source, LPCSTR capName, LPDTWAIN_ARRAY pArray, DTWAIN_BOOL expandIfRange)
{
#ifdef _UNICODE
    return DTWAIN_EnumSourceValues(Source, StringConversion::Convert_AnsiPtr_To_Native(capName).c_str(), pArray, expandIfRange);
#else
    return DTWAIN_EnumSourceValues(Source, capName, pArray, expandIfRange);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AcquireFileA(DTWAIN_SOURCE Source, LPCSTR lpszFile, LONG lFileType, LONG lFileFlags, LONG PixelType, LONG lMaxPages, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
{
#ifdef _UNICODE
    return DTWAIN_AcquireFile(Source, StringConversion::Convert_AnsiPtr_To_Native(lpszFile).c_str(), lFileType, lFileFlags, PixelType, lMaxPages, bShowUI, bCloseSource, pStatus);
#else
    return DTWAIN_AcquireFile(Source, lpszFile, lFileType, lFileFlags, PixelType, lMaxPages, bShowUI, bCloseSource, pStatus);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AcquireFileW(DTWAIN_SOURCE Source, LPCWSTR lpszFile, LONG lFileType, LONG lFileFlags, LONG PixelType, LONG lMaxPages, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
{
#ifdef _UNICODE
    return DTWAIN_AcquireFile(Source, lpszFile, lFileType, lFileFlags, PixelType, lMaxPages, bShowUI, bCloseSource, pStatus);
#else
    return DTWAIN_AcquireFile(Source, StringConversion::Convert_WidePtr_To_Native(lpszFile).c_str(), lFileType, lFileFlags, PixelType, lMaxPages, bShowUI, bCloseSource, pStatus);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AcquireAudioFileA(DTWAIN_SOURCE Source, LPCSTR lpszFile, LONG lFileFlags, LONG lMaxPages, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
{
#ifdef _UNICODE
	return DTWAIN_AcquireAudioFile(Source, StringConversion::Convert_AnsiPtr_To_Native(lpszFile).c_str(), lFileFlags, lMaxPages, bShowUI, bCloseSource, pStatus);
#else
	return DTWAIN_AcquireAudioFile(Source, lpszFile, lFileFlags, lMaxPages, bShowUI, bCloseSource, pStatus);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AcquireAudioFileW(DTWAIN_SOURCE Source, LPCWSTR lpszFile, LONG lFileFlags, LONG lMaxPages, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
{
#ifdef _UNICODE
	return DTWAIN_AcquireAudioFile(Source, lpszFile, lFileFlags, lMaxPages, bShowUI, bCloseSource, pStatus);
#else
	return DTWAIN_AcquireAudioFile(Source, StringConversion::Convert_WidePtr_To_Native(lpszFile).c_str(), lFileFlags, lMaxPages, bShowUI, bCloseSource, pStatus);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddFileToAppendW(LPCWSTR szFile)
{
#ifdef _UNICODE
    return DTWAIN_AddFileToAppend(szFile);
#else
    return DTWAIN_AddFileToAppend(StringConversion::Convert_WidePtr_To_Native(szFile).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddFileToAppendA(LPCSTR szFile)
{
#ifdef _UNICODE
    return DTWAIN_AddFileToAppend(StringConversion::Convert_AnsiPtr_To_Native(szFile).c_str());
#else
    return DTWAIN_AddFileToAppend(szFile);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddPDFTextW(DTWAIN_SOURCE Source, LPCWSTR szText, LONG xPos, LONG yPos, LPCWSTR fontName, DTWAIN_FLOAT fontSize, LONG colorRGB, LONG renderMode, DTWAIN_FLOAT scaling, DTWAIN_FLOAT charSpacing, DTWAIN_FLOAT wordSpacing, LONG strokeWidth, LONG Flags)
{
#ifdef _UNICODE
    return DTWAIN_AddPDFText(Source, szText, xPos, yPos, fontName, fontSize, colorRGB, renderMode, scaling, charSpacing, wordSpacing, strokeWidth, Flags);
#else
    return DTWAIN_AddPDFText(Source, StringConversion::Convert_WidePtr_To_Native(szText).c_str(), xPos, yPos, StringConversion::Convert_WidePtr_To_Native(fontName).c_str(), fontSize, colorRGB, renderMode, scaling, charSpacing, wordSpacing, strokeWidth, Flags);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddPDFTextA(DTWAIN_SOURCE Source, LPCSTR szText, LONG xPos, LONG yPos, LPCSTR fontName, DTWAIN_FLOAT fontSize, LONG colorRGB, LONG renderMode, DTWAIN_FLOAT scaling, DTWAIN_FLOAT charSpacing, DTWAIN_FLOAT wordSpacing, LONG strokeWidth, LONG Flags)
{
#ifdef _UNICODE
    return DTWAIN_AddPDFText(Source, StringConversion::Convert_AnsiPtr_To_Native(szText).c_str(), xPos, yPos, StringConversion::Convert_AnsiPtr_To_Native(fontName).c_str(), fontSize, colorRGB, renderMode, scaling, charSpacing, wordSpacing, strokeWidth, Flags);
#else
    return DTWAIN_AddPDFText(Source, szText, xPos, yPos, fontName, fontSize, colorRGB, renderMode, scaling, charSpacing, wordSpacing, strokeWidth, Flags);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddStringA(DTWAIN_ARRAY pArray, LPCSTR Val)
{
#ifdef _UNICODE
    return DTWAIN_ArrayAddString(pArray, StringConversion::Convert_AnsiPtr_To_Native(Val).c_str());
#else
    return DTWAIN_ArrayAddString(pArray, Val);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddStringW(DTWAIN_ARRAY pArray, LPCWSTR Val)
{
#ifdef _UNICODE
    return DTWAIN_ArrayAddString(pArray, Val);
#else
    return DTWAIN_ArrayAddString(pArray, StringConversion::Convert_WidePtr_To_Native(Val).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddStringNA(DTWAIN_ARRAY pArray, LPCSTR Val, LONG num)
{
#ifdef _UNICODE
    return DTWAIN_ArrayAddStringN(pArray, StringConversion::Convert_AnsiPtr_To_Native(Val).c_str(), num);
#else
    return DTWAIN_ArrayAddStringN(pArray, Val, num);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddStringNW(DTWAIN_ARRAY pArray, LPCWSTR Val, LONG num)
{
#ifdef _UNICODE
    return DTWAIN_ArrayAddStringN(pArray, Val, num);
#else
    return DTWAIN_ArrayAddStringN(pArray, StringConversion::Convert_WidePtr_To_Native(Val).c_str(), num);
#endif
}

LONG DLLENTRY_DEF DTWAIN_ArrayFindStringA(DTWAIN_ARRAY pArray, LPCSTR pString)
{
#ifdef _UNICODE
    return DTWAIN_ArrayFindString(pArray, StringConversion::Convert_AnsiPtr_To_Native(pString).c_str());
#else
    return DTWAIN_ArrayFindString(pArray, pString);
#endif
}

LONG DLLENTRY_DEF DTWAIN_ArrayFindStringW(DTWAIN_ARRAY pArray, LPCWSTR pString)
{
#ifdef _UNICODE
    return DTWAIN_ArrayFindString(pArray, pString);
#else
    return DTWAIN_ArrayFindString(pArray, StringConversion::Convert_WidePtr_To_Native(pString).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAtStringA(DTWAIN_ARRAY pArray, LONG nWhere, LPSTR pStr)
{
#ifdef _UNICODE
    std::wstring arg(1024,0);
    const DTWAIN_BOOL retVal = DTWAIN_ArrayGetAtString(pArray, nWhere, &arg[0]);
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
    std::string arg(1024,0);
    DTWAIN_BOOL retVal = DTWAIN_ArrayGetAtString(pArray, nWhere, &arg[0]);
    return null_terminator_copier(get_view(arg), pStr, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtStringA(DTWAIN_ARRAY pArray, LONG nWhere, LPCSTR pVal)
{
#ifdef _UNICODE
    return DTWAIN_ArrayInsertAtString(pArray, nWhere, StringConversion::Convert_AnsiPtr_To_Native(pVal).c_str());
#else
    return DTWAIN_ArrayInsertAtString(pArray, nWhere, pVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtStringW(DTWAIN_ARRAY pArray, LONG nWhere, LPCWSTR pVal)
{
#ifdef _UNICODE
    return DTWAIN_ArrayInsertAtString(pArray, nWhere, pVal);
#else
    return DTWAIN_ArrayInsertAtString(pArray, nWhere, StringConversion::Convert_WidePtr_To_Native(pVal).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtStringNA(DTWAIN_ARRAY pArray, LONG nWhere, LPCSTR Val, LONG num)
{
#ifdef _UNICODE
    return DTWAIN_ArrayInsertAtStringN(pArray, nWhere, StringConversion::Convert_AnsiPtr_To_Native(Val).c_str(), num);
#else
    return DTWAIN_ArrayInsertAtStringN(pArray, nWhere, Val, num);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtStringNW(DTWAIN_ARRAY pArray, LONG nWhere, LPCWSTR Val, LONG num)
{
#ifdef _UNICODE
    return DTWAIN_ArrayInsertAtStringN(pArray, nWhere, Val, num);
#else
    return DTWAIN_ArrayInsertAtStringN(pArray, nWhere, StringConversion::Convert_WidePtr_To_Native(Val).c_str(), num);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtStringW(DTWAIN_ARRAY pArray, LONG nWhere, LPCWSTR pStr)
{
#ifdef _UNICODE
    return DTWAIN_ArraySetAtString(pArray, nWhere, pStr);
#else
    return DTWAIN_ArraySetAtString(pArray, nWhere, StringConversion::Convert_WidePtr_To_Native(pStr).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtStringA(DTWAIN_ARRAY pArray, LONG nWhere, LPCSTR pStr)
{
#ifdef _UNICODE
    return DTWAIN_ArraySetAtString(pArray, nWhere, StringConversion::Convert_AnsiPtr_To_Native(pStr).c_str());
#else
    return DTWAIN_ArraySetAtString(pArray, nWhere, pStr);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ExecuteOCRW(DTWAIN_OCRENGINE Engine, LPCWSTR szFileName, LONG nStartPage, LONG nEndPage)
{
#ifdef _UNICODE
    return DTWAIN_ExecuteOCR(Engine, szFileName, nStartPage, nEndPage);
#else
    return DTWAIN_ExecuteOCR(Engine, StringConversion::Convert_WidePtr_To_Native(szFileName).c_str(), nStartPage, nEndPage);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ExecuteOCRA(DTWAIN_OCRENGINE Engine, LPCSTR szFileName, LONG nStartPage, LONG nEndPage)
{
#ifdef _UNICODE
    return DTWAIN_ExecuteOCR(Engine, StringConversion::Convert_AnsiPtr_To_Native(szFileName).c_str(), nStartPage, nEndPage);
#else
    return DTWAIN_ExecuteOCR(Engine, szFileName, nStartPage, nEndPage);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAcquireArea2StringW(DTWAIN_SOURCE Source, LPWSTR left, LPWSTR top, LPWSTR right, LPWSTR bottom, LPLONG Unit)
{
#ifdef _UNICODE
    return DTWAIN_GetAcquireArea2String(Source, left, top, right, bottom, Unit);
#else
    std::array<LPWSTR, 4> outarg = {left, top, right, bottom};
    std::array<std::string, 4> args = {{std::string(1024, 0), std::string(1024, 0), std::string(1024, 0), std::string(1024, 0)}};
    DTWAIN_BOOL retVal = DTWAIN_GetAcquireArea2String(Source, &args[0][0], &args[1][0], &args[2][0], &args[3][0], Unit);
    for ( size_t i = 0; i < args.size(); ++i )
        null_terminator_copier(get_view(args[i]), outarg[i], retVal);
    return retVal;
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAcquireArea2StringA(DTWAIN_SOURCE Source, LPSTR left, LPSTR top, LPSTR right, LPSTR bottom, LPLONG Unit)
{
#ifdef _UNICODE
    const std::array<LPSTR, 4> outarg = {left, top, right, bottom};
    std::array<std::wstring, 4> args = {{std::wstring(1024, 0), std::wstring(1024, 0), std::wstring(1024, 0), std::wstring(1024, 0)}};
    const DTWAIN_BOOL retVal = DTWAIN_GetAcquireArea2String(Source, &args[0][0], &args[1][0], &args[2][0], &args[3][0], Unit);
    for ( size_t i = 0; i < 4; ++i )
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
    const DTWAIN_BOOL retVal = DTWAIN_GetAppInfo(&args[0][0], &args[1][0], &args[2][0], &args[3][0]);
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
    DTWAIN_BOOL retVal = DTWAIN_GetAppInfo(&args[0][0], &args[1][0], &args[2][0], &args[3][0]);
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
    std::string arg(1024,0);
    DTWAIN_BOOL retVal = DTWAIN_GetAuthor(Source, &arg[0]);
    return null_terminator_copier(get_view(arg), szAuthor, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAuthorA(DTWAIN_SOURCE Source, LPSTR szAuthor)
{
#ifdef _UNICODE
    std::wstring arg(1024,0);
    const DTWAIN_BOOL retVal = DTWAIN_GetAuthor(Source, &arg[0]);
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
    DTWAIN_BOOL retVal = DTWAIN_GetBrightnessString(Source, &arg[0]);
    return null_terminator_copier(get_view(arg), Contrast, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetBrightnessStringA(DTWAIN_SOURCE Source, LPSTR Contrast)
{
#ifdef _UNICODE
    std::wstring arg(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetBrightnessString(Source, &arg[0]);
    return null_terminator_copier(get_view(arg), Contrast, retVal);
#else
    return DTWAIN_GetBrightnessString(Source, Contrast);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetCapFromNameA(LPCSTR szName)
{
#ifdef _UNICODE
    return DTWAIN_GetCapFromName(StringConversion::Convert_AnsiPtr_To_Native(szName).c_str());
#else
    return DTWAIN_GetCapFromName(szName);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetCapFromNameW(LPCWSTR szName)
{
#ifdef _UNICODE
    return DTWAIN_GetCapFromName(szName);
#else
    return DTWAIN_GetCapFromName(StringConversion::Convert_WidePtr_To_Native(szName).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetCaptionW(DTWAIN_SOURCE Source, LPWSTR Caption)
{
#ifdef _UNICODE
    return DTWAIN_GetCaption(Source, Caption);
#else
    std::string arg(1024, 0);
    DTWAIN_BOOL retVal = DTWAIN_GetCaption(Source, &arg[0]);
    return null_terminator_copier(get_view(arg), Caption, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetCaptionA(DTWAIN_SOURCE Source, LPSTR Caption)
{
#ifdef _UNICODE
    std::wstring arg(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetCaption(Source, &arg[0]);
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
    DTWAIN_BOOL retVal = DTWAIN_GetContrastString(Source, &arg[0]);
    return null_terminator_copier(get_view(arg), Contrast, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetContrastStringA(DTWAIN_SOURCE Source, LPSTR Contrast)
{
#ifdef _UNICODE
    std::wstring arg(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetContrastString(Source, &arg[0]);
    return null_terminator_copier(get_view(arg), Contrast, retVal);
#else
    return DTWAIN_GetContrastString(Source, Contrast);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetCurrentFileNameA(DTWAIN_SOURCE Source, LPSTR szName, LONG MaxLen)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(MaxLen, 0L),0);
    const LONG retVal = DTWAIN_GetCurrentFileName(Source, (MaxLen > 0 && szName) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), szName, retVal);
#else
    return DTWAIN_GetCurrentFileName(Source, szName, MaxLen);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetCurrentFileNameW(DTWAIN_SOURCE Source, LPWSTR szName, LONG MaxLen)
{
#ifdef _UNICODE
    return DTWAIN_GetCurrentFileName(Source, szName, MaxLen);
#else
    std::string arg((std::max)(MaxLen,0L), 0);
    LONG retVal = DTWAIN_GetCurrentFileName(Source, (MaxLen>0 && szName)? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), szName, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetDSMFullNameW(LONG DSMType, LPWSTR szDLLName, LONG nMaxLen, LPLONG pWhichSearch)
{
#ifdef _UNICODE
    return DTWAIN_GetDSMFullName(DSMType, szDLLName, nMaxLen, pWhichSearch);
#else
    std::string arg((std::max)(nMaxLen,0L), 0);
    LONG retVal = DTWAIN_GetDSMFullName(DSMType, (nMaxLen > 0 && szDLLName) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()), pWhichSearch);
    return null_terminator_copier(get_view(arg), szDLLName, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetDSMFullNameA(LONG DSMType, LPSTR szDLLName, LONG nMaxLen, LPLONG pWhichSearch)
{
#ifdef _UNICODE
    std::wstring arg(nMaxLen, 0);
    const LONG retVal = DTWAIN_GetDSMFullName(DSMType, (nMaxLen>0 && szDLLName) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()), pWhichSearch);
    return null_terminator_copier(get_view(arg), szDLLName, retVal);
#else
    return DTWAIN_GetDSMFullName(DSMType, szDLLName, nMaxLen, pWhichSearch);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetDeviceTimeDateA(DTWAIN_SOURCE Source, LPSTR szTimeDate)
{
#ifdef _UNICODE
    std::wstring arg(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetDeviceTimeDate(Source, &arg[0]);
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
    DTWAIN_BOOL retVal = DTWAIN_GetDeviceTimeDate(Source, &arg[0]);
    return null_terminator_copier(get_view(arg), szTimeDate, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetErrorStringA(LONG lError, LPSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetErrorString(lError, (nLength > 0 && lpszBuffer) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), lpszBuffer, retVal);
#else
    return DTWAIN_GetErrorString(lError, lpszBuffer, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetErrorStringW(LONG lError, LPWSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetErrorString(lError, lpszBuffer, nLength);
#else
    std::string arg((std::max)(nLength,0L), 0);
    LONG retVal = DTWAIN_GetErrorString(lError, (nLength > 0 && lpszBuffer) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), lpszBuffer, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetResourceStringA(LONG ResourceID, LPSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetResourceString(ResourceID, (nLength > 0 && lpszBuffer) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), lpszBuffer, retVal);
#else
    return DTWAIN_GetResourceString(ResourceID, lpszBuffer, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetResourceStringW(LONG ResourceID, LPWSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetResourceString(ResourceID, lpszBuffer, nLength);
#else
    std::string arg((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetResourceString(ResourceID, (nLength > 0 && lpszBuffer) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), lpszBuffer, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetConditionCodeStringA(LONG lError, LPSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
	std::wstring arg((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetConditionCodeString(lError, (nLength > 0 && lpszBuffer) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
	return null_terminator_copier(get_view(arg), lpszBuffer, retVal);
#else
	return DTWAIN_GetConditionCodeString(lError, lpszBuffer, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetConditionCodeStringW(LONG lError, LPWSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
	return DTWAIN_GetConditionCodeString(lError, lpszBuffer, nLength);
#else
	std::string arg((std::max)(nLength,0L), 0);
	LONG retVal = DTWAIN_GetConditionCodeString(lError, (nLength>0 && lpszBuffer)? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
	return null_terminator_copier(get_view(arg), lpszBuffer, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetPaperSizeNameW(LONG paperNumber, LPWSTR outName, LONG nSize)
{
#ifdef _UNICODE
    return DTWAIN_GetPaperSizeName(paperNumber, outName, nSize);
#else
    std::string arg((std::max)(nSize,0L), 0);
    LONG retVal = DTWAIN_GetPaperSizeName(paperNumber, (nSize > 0 && outName) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), outName, retVal);
#endif
}


LONG DLLENTRY_DEF DTWAIN_GetPaperSizeNameA(LONG paperNumber, LPSTR outName, LONG nSize)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nSize,0L), 0);
    LONG retVal = DTWAIN_GetPaperSizeName(paperNumber, (nSize>0 && outName)? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), outName, retVal);
#else
    return DTWAIN_GetPaperSizeName(paperNumber, outName, nSize);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetTwainNameFromConstantW(LONG lConstantType, LONG lTwainConstant, LPWSTR lpszOut, LONG nSize)
{
#ifdef _UNICODE
    return DTWAIN_GetTwainNameFromConstant(lConstantType, lTwainConstant, lpszOut, nSize);
#else
    std::string arg((std::max)(nSize, 0L), 0);
    LONG retVal = DTWAIN_GetTwainNameFromConstant(lConstantType, lTwainConstant, (nSize > 0 && lpszOut) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), lpszOut, retVal);
#endif
}


LONG DLLENTRY_DEF DTWAIN_GetTwainNameFromConstantA(LONG lConstantType, LONG lTwainConstant, LPSTR lpszOut, LONG nSize)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nSize, 0L), 0);
    LONG retVal = DTWAIN_GetTwainNameFromConstant(lConstantType, lTwainConstant, (nSize > 0 && lpszOut)? &arg[0] : nullptr, 
                                                  static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), lpszOut, retVal);
#else
    return DTWAIN_GetTwainNameFromConstant(lConstantType, lTwainConstant, lpszOut, nSize);
#endif
}


LONG DLLENTRY_DEF DTWAIN_GetExtCapFromNameW(LPCWSTR szName)
{
#ifdef _UNICODE
    return DTWAIN_GetExtCapFromName(szName);
#else
    return DTWAIN_GetExtCapFromName(StringConversion::Convert_WidePtr_To_Native(szName).c_str());
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetExtCapFromNameA(LPCSTR szName)
{
#ifdef _UNICODE
    return DTWAIN_GetExtCapFromName(StringConversion::Convert_AnsiPtr_To_Native(szName).c_str());
#else
    return DTWAIN_GetExtCapFromName(szName);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetExtNameFromCapW(LONG nValue, LPWSTR szValue, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetExtNameFromCap(nValue, szValue, nLength);
#else
    std::string arg((std::max)(nLength,0L), 0);
    LONG retVal = DTWAIN_GetExtNameFromCap(nValue, (nLength > 0 && szValue) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), szValue, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetExtNameFromCapA(LONG nValue, LPSTR szValue, LONG nLength)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetExtNameFromCap(nValue, (nLength > 0 && szValue) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), szValue, retVal);
#else
    return DTWAIN_GetExtNameFromCap(nValue, szValue, nLength);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetHalftoneA(DTWAIN_SOURCE Source, LPSTR lpHalftone, LONG GetType)
{
#ifdef _UNICODE
    std::wstring arg(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetHalftone(Source, &arg[0], GetType);
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
    DTWAIN_BOOL retVal = DTWAIN_GetHalftone(Source, &arg[0], GetType);
    return null_terminator_copier(get_view(arg), lpHalftone, retVal);
#endif
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetHighlightStringW(DTWAIN_SOURCE Source, LPWSTR Highlight)
{
#ifdef _UNICODE
    return DTWAIN_GetHighlightString(Source, Highlight);
#else
    std::string arg(1024, 0);
    DTWAIN_BOOL retVal = DTWAIN_GetHighlightString(Source, &arg[0]);
    return null_terminator_copier(get_view(arg), Highlight, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetHighlightStringA(DTWAIN_SOURCE Source, LPSTR Highlight)
{
#ifdef _UNICODE
    std::wstring arg(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetHighlightString(Source, &arg[0]);
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
    std::array<LPWSTR, 2> outarg = { lpXResolution, lpYResolution};
    std::array<std::string, 2> args = { { std::string(1024, 0), std::string(1024, 0) } };
    DTWAIN_BOOL retVal = DTWAIN_GetImageInfoString(Source, &args[0][0], &args[1][0], lpWidth, lpLength, lpNumSamples, lpBitsPerSample, lpBitsPerPixel, lpPlanar, lpPixelType, lpCompression);
    for (size_t i = 0; i < 2; ++i)
        null_terminator_copier(get_view(args[i]), outarg[i], retVal);
    return retVal;
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetImageInfoStringA(DTWAIN_SOURCE Source, LPSTR lpXResolution, LPSTR lpYResolution, LPLONG lpWidth, LPLONG lpLength, LPLONG lpNumSamples, LPDTWAIN_ARRAY lpBitsPerSample, LPLONG lpBitsPerPixel, LPLONG lpPlanar, LPLONG lpPixelType, LPLONG lpCompression)
{
#ifdef _UNICODE
    const std::array<LPSTR, 2> outarg = { lpXResolution, lpYResolution};
    std::array<std::wstring, 2> args = { { std::wstring(1024, 0), std::wstring(1024, 0) } };
    const DTWAIN_BOOL retVal = DTWAIN_GetImageInfoString(Source, &args[0][0], &args[1][0], lpWidth, lpLength, lpNumSamples, lpBitsPerSample, lpBitsPerPixel, lpPlanar, lpPixelType, lpCompression);
    for (size_t i = 0; i < 2; ++i)
        null_terminator_copier(get_view(args[i]), outarg[i], retVal);
    return retVal;
#else
    return DTWAIN_GetImageInfoString(Source, lpXResolution, lpYResolution, lpWidth, lpLength, lpNumSamples, lpBitsPerSample, lpBitsPerPixel, lpPlanar, lpPixelType, lpCompression);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetNameFromCapA(LONG nCapValue, LPSTR szValue, LONG nLength)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nLength, 0L), 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetNameFromCap(nCapValue, (nLength > 0 && szValue)?&arg[0]:nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), szValue, retVal);
#else
    return DTWAIN_GetNameFromCap(nCapValue, szValue, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetNameFromCapW(LONG nCapValue, LPWSTR szValue, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetNameFromCap(nCapValue, szValue, nLength);
#else
    std::string arg((std::max)(nLength,0L), 0);
    DTWAIN_BOOL retVal = DTWAIN_GetNameFromCap(nCapValue, (nLength>0 && szValue)? &arg[0] : nullptr, nLength);
    return null_terminator_copier(get_view(arg), szValue, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetOCRErrorStringW(DTWAIN_OCRENGINE Engine, LONG lError, LPWSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetOCRErrorString(Engine, lError, lpszBuffer, nLength);
#else
    std::string arg((std::max)(nLength,0L), 0);
    LONG retVal = DTWAIN_GetOCRErrorString(Engine, lError, (nLength > 0 && lpszBuffer) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), lpszBuffer, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetOCRErrorStringA(DTWAIN_OCRENGINE Engine, LONG lError, LPSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetOCRErrorString(Engine, lError, (nLength>0 && lpszBuffer) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), lpszBuffer, retVal);
#else
    return DTWAIN_GetOCRErrorString(Engine, lError, lpszBuffer, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetOCRManufacturerW(DTWAIN_OCRENGINE Engine, LPWSTR szManufacturer, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetOCRManufacturer(Engine, szManufacturer, nLength);
#else
    std::string arg((std::max)(nLength,0L), 0);
    LONG retVal = DTWAIN_GetOCRManufacturer(Engine, (nLength > 0 && szManufacturer) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), szManufacturer, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetOCRManufacturerA(DTWAIN_OCRENGINE Engine, LPSTR szManufacturer, LONG nLength)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetOCRManufacturer(Engine, (nLength > 0 && szManufacturer) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), szManufacturer, retVal);
#else
    return DTWAIN_GetOCRManufacturer(Engine, szManufacturer, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetOCRProductFamilyA(DTWAIN_OCRENGINE Engine, LPSTR szProductFamily, LONG nLength)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetOCRProductFamily(Engine, (nLength > 0 && szProductFamily) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), szProductFamily, retVal);
#else
    return DTWAIN_GetOCRProductFamily(Engine, szProductFamily, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetOCRProductFamilyW(DTWAIN_OCRENGINE Engine, LPWSTR szProductFamily, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetOCRProductFamily(Engine, szProductFamily, nLength);
#else
    std::string arg((std::max)(nLength,0L), 0);
    LONG retVal = DTWAIN_GetOCRProductFamily(Engine, (nLength > 0 && szProductFamily) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), szProductFamily, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetOCRProductNameW(DTWAIN_OCRENGINE Engine, LPWSTR szProductName, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetOCRProductName(Engine, szProductName, nLength);
#else
    std::string arg((std::max)(nLength,0L), 0);
    LONG retVal = DTWAIN_GetOCRProductName(Engine, (nLength > 0 && szProductName) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), szProductName, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetOCRProductNameA(DTWAIN_OCRENGINE Engine, LPSTR szProductName, LONG nLength)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetOCRProductName(Engine, (nLength>0 && szProductName) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), szProductName, retVal);
#else
    return DTWAIN_GetOCRProductName(Engine, szProductName, nLength);
#endif
}

HANDLE DLLENTRY_DEF DTWAIN_GetOCRTextA(DTWAIN_OCRENGINE Engine, LONG nPageNo, LPSTR Data, LONG dSize, LPLONG pActualSize, LONG nFlags)
{
#ifdef _UNICODE
    std::wstring arg(dSize, 0);
    const HANDLE retVal = DTWAIN_GetOCRText(Engine, nPageNo, (dSize > 0 && Data) ? &arg[0] : nullptr, dSize, pActualSize, nFlags);
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
    HANDLE retVal = DTWAIN_GetOCRText(Engine, nPageNo, (dSize>0 && Data)?&arg[0] : nullptr, dSize, pActualSize, nFlags);
    return null_terminator_copier(get_view(arg), Data, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetOCRVersionInfoA(DTWAIN_OCRENGINE Engine, LPSTR buffer, LONG nLength)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetOCRVersionInfo(Engine, (nLength > 0 && buffer) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), buffer, retVal);
#else
    return DTWAIN_GetOCRVersionInfo(Engine, buffer, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetOCRVersionInfoW(DTWAIN_OCRENGINE Engine, LPWSTR buffer, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetOCRVersionInfo(Engine, buffer, nLength);
#else
    std::string arg((std::max)(nLength,0L), 0);
    LONG retVal = DTWAIN_GetOCRVersionInfo(Engine, (nLength>0 && buffer)? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), buffer, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetPDFTextElementStringA(DTWAIN_PDFTEXTELEMENT TextElement, LPSTR szData, LONG nMaxLen, LONG Flags)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nMaxLen, 0L),0);
    const DTWAIN_BOOL retVal = DTWAIN_GetPDFTextElementString(TextElement, (nMaxLen > 0 && szData) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()), Flags);
    return null_terminator_copier(get_view(arg), szData, retVal);
#else
    return DTWAIN_GetPDFTextElementString(TextElement, szData, nMaxLen, Flags);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetPDFTextElementStringW(DTWAIN_PDFTEXTELEMENT TextElement, LPWSTR szData, LONG maxLen, LONG Flags)
{
#ifdef _UNICODE
    return DTWAIN_GetPDFTextElementString(TextElement, szData, maxLen, Flags);
#else
    std::string arg((std::max)(maxLen,0L), 0);
    DTWAIN_BOOL retVal = DTWAIN_GetPDFTextElementString(TextElement, (maxLen>0 && szData)? &arg[0] : nullptr, static_cast<LONG>(arg.size()), Flags);
    return null_terminator_copier(get_view(arg), szData, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetPDFType1FontNameA(LONG FontVal, LPSTR szFont, LONG nChars)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nChars, 0L), 0);
    const LONG retVal = DTWAIN_GetPDFType1FontName(FontVal, (nChars > 0 && szFont) ? &arg[0] : nullptr, nChars);
    return null_terminator_copier(get_view(arg), szFont, retVal);
#else
    return DTWAIN_GetPDFType1FontName(FontVal, szFont, nChars);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetPDFType1FontNameW(LONG FontVal, LPWSTR szFont, LONG nChars)
{
#ifdef _UNICODE
    return DTWAIN_GetPDFType1FontName(FontVal, szFont, nChars);
#else
    std::string arg((std::max)(nChars, 0L), 0);
    LONG retVal = DTWAIN_GetPDFType1FontName(FontVal, (nChars>0 && szFont)? &arg[0] : nullptr, nChars);
    return null_terminator_copier(get_view(arg), szFont, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetPrinterSuffixStringA(DTWAIN_SOURCE Source, LPSTR Suffix, LONG nLength)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nLength, 0L), 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetPrinterSuffixString(Source, (nLength > 0 && Suffix) ? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
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
    std::string arg((std::max)(nLength,0L), 0);
    DTWAIN_BOOL retVal = DTWAIN_GetPrinterSuffixString(Source, (nLength>0 && Suffix)? &arg[0] : nullptr, static_cast<LONG>(arg.size()));
    return null_terminator_copier(get_view(arg), Suffix, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetResolutionStringA(DTWAIN_SOURCE Source, LPSTR Resolution)
{
#ifdef _UNICODE
    std::wstring arg(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetResolutionString(Source, &arg[0]);
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
    DTWAIN_BOOL retVal = DTWAIN_GetResolutionString(Source, &arg[0]);
    return null_terminator_copier(get_view(arg), Resolution, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetRotationStringW(DTWAIN_SOURCE Source, LPWSTR Rotation)
{
#ifdef _UNICODE
    return DTWAIN_GetRotationString(Source, Rotation);
#else
    std::string arg(1024, 0);
    DTWAIN_BOOL retVal = DTWAIN_GetRotationString(Source, &arg[0]);
    return null_terminator_copier(get_view(arg), Rotation, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetRotationStringA(DTWAIN_SOURCE Source, LPSTR Rotation)
{
#ifdef _UNICODE
    std::wstring arg(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetRotationString(Source, &arg[0]);
    return null_terminator_copier(get_view(arg), Rotation, retVal);
#else
    return DTWAIN_GetRotationString(Source, Rotation);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSaveFileNameW(DTWAIN_SOURCE Source, LPWSTR fName, LONG nMaxLen)
{
#ifdef _UNICODE
    return DTWAIN_GetSaveFileName(Source, fName, nMaxLen);
#else
    std::string args((std::max)(nMaxLen, 0L), 0);
    LONG retVal = DTWAIN_GetSaveFileName(Source, (nMaxLen > 0 && fName) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), fName, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSaveFileNameA(DTWAIN_SOURCE Source, LPSTR fName, LONG nMaxLen)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nMaxLen, 0L), 0);
    const LONG retVal = DTWAIN_GetSaveFileName(Source, (nMaxLen>0 && fName) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), fName, retVal);
#else
    return DTWAIN_GetSaveFileName(Source, fName, nMaxLen);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetShadowStringW(DTWAIN_SOURCE Source, LPWSTR Shadow)
{
#ifdef _UNICODE
    return DTWAIN_GetShadowString(Source, Shadow);
#else
    std::string args(1024, 0);
    DTWAIN_BOOL retVal = DTWAIN_GetShadowString(Source, &args[0]);
    return null_terminator_copier(get_view(args), Shadow, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetShadowStringA(DTWAIN_SOURCE Source, LPSTR Shadow)
{
#ifdef _UNICODE
    std::wstring args(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetShadowString(Source, &args[0]);
    return null_terminator_copier(get_view(args), Shadow, retVal);
#else
    return DTWAIN_GetShadowString(Source, Shadow);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSourceManufacturerA(DTWAIN_SOURCE Source, LPSTR szProduct, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetSourceManufacturer(Source, (nLength > 0 && szProduct) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szProduct, retVal);
#else
    return DTWAIN_GetSourceManufacturer(Source, szProduct, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSourceManufacturerW(DTWAIN_SOURCE Source, LPWSTR szProduct, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetSourceManufacturer(Source, szProduct, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetSourceManufacturer(Source, (nLength > 0 && szProduct) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szProduct, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSourceProductFamilyW(DTWAIN_SOURCE Source, LPWSTR szProduct, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetSourceProductFamily(Source, szProduct, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetSourceProductFamily(Source, (nLength > 0 && szProduct) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szProduct, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSourceProductFamilyA(DTWAIN_SOURCE Source, LPSTR szProduct, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetSourceProductFamily(Source, (nLength > 0 && szProduct) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szProduct, retVal);
#else
    return DTWAIN_GetSourceProductFamily(Source, szProduct, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSourceProductNameA(DTWAIN_SOURCE Source, LPSTR szProduct, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetSourceProductName(Source, (nLength>0 && szProduct) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szProduct, retVal);
#else
    return DTWAIN_GetSourceProductName(Source, szProduct, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSourceProductNameW(DTWAIN_SOURCE Source, LPWSTR szProduct, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetSourceProductName(Source, szProduct, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetSourceProductName(Source, (nLength>0 && szProduct)? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szProduct, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSourceVersionInfoA(DTWAIN_SOURCE Source, LPSTR szProduct, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetSourceVersionInfo(Source, (nLength>0 && szProduct) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szProduct, retVal);
#else
    return DTWAIN_GetSourceVersionInfo(Source, szProduct, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSourceVersionInfoW(DTWAIN_SOURCE Source, LPWSTR szProduct, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetSourceVersionInfo(Source, szProduct, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetSourceVersionInfo(Source, (nLength>0 && szProduct)? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szProduct, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetTempFileDirectoryW(LPWSTR szFilePath, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetTempFileDirectory(szFilePath, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetTempFileDirectory((nLength>0 && szFilePath)? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szFilePath, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetTempFileDirectoryA(LPSTR szFilePath, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetTempFileDirectory((nLength>0 && szFilePath) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szFilePath, retVal);
#else
    return DTWAIN_GetTempFileDirectory(szFilePath, nLength);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetThresholdStringA(DTWAIN_SOURCE Source, LPSTR Threshold)
{
#ifdef _UNICODE
    std::wstring args(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetThresholdString(Source, &args[0]);
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
    DTWAIN_BOOL retVal = DTWAIN_GetThresholdString(Source, &args[0]);
    return null_terminator_copier(get_view(args), Threshold, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetTimeDateW(DTWAIN_SOURCE Source, LPWSTR szTimeDate)
{
#ifdef _UNICODE
    return DTWAIN_GetTimeDate(Source, szTimeDate);
#else
    std::string args(1024, 0);
    DTWAIN_BOOL retVal = DTWAIN_GetTimeDate(Source, &args[0]);
    return null_terminator_copier(get_view(args), szTimeDate, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetTimeDateA(DTWAIN_SOURCE Source, LPSTR szTimeDate)
{
#ifdef _UNICODE
    std::wstring args(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetTimeDate(Source, &args[0]);
    return null_terminator_copier(get_view(args), szTimeDate, retVal);
#else
    return DTWAIN_GetTimeDate(Source, szTimeDate);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetTwainAvailabilityExA(LPSTR szDirectories, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetTwainAvailabilityEx((nLength > 0 && szDirectories) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szDirectories, retVal);
#else
    return DTWAIN_GetTwainAvailabilityEx(szDirectories, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetTwainAvailabilityExW(LPWSTR szDirectories, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetTwainAvailabilityEx(szDirectories, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetTwainAvailabilityEx((nLength > 0 && szDirectories) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szDirectories, retVal);
#endif
}


LONG DLLENTRY_DEF DTWAIN_IsTwainAvailableExA(LPSTR szDirectories, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_IsTwainAvailableEx((nLength > 0 && szDirectories) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szDirectories, retVal);
#else
    return DTWAIN_IsTwainAvailableEx(szDirectories, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_IsTwainAvailableExW(LPWSTR szDirectories, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_IsTwainAvailableEx(szDirectories, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_IsTwainAvailableEx((nLength > 0 && szDirectories) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), szDirectories, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetTwainCountryNameA(LONG nameID, LPSTR ret)
{
	#ifdef _UNICODE
		std::wstring arg(1024, 0);
        const DTWAIN_BOOL retVal = DTWAIN_GetTwainCountryName(nameID, &arg[0]);
		return null_terminator_copier(get_view(arg), ret, retVal);
	#else
		return DTWAIN_GetTwainCountryName(nameID, ret);
	#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetTwainCountryNameW(LONG nameID, LPWSTR ret)
{
#ifdef _UNICODE
	return DTWAIN_GetTwainCountryName(nameID, ret);
#else
	std::string arg(1024, 0);
	DTWAIN_BOOL retVal = DTWAIN_GetTwainCountryName(nameID, &arg[0]);
	return null_terminator_copier(get_view(arg), ret, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetTwainCountryValueA(LPCSTR country)
{
#ifdef _UNICODE
	return DTWAIN_GetTwainCountryValue(StringConversion::Convert_AnsiPtr_To_Native(country).c_str());
#else
	return DTWAIN_GetTwainCountryValue(country);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetTwainCountryValueW(LPCWSTR country)
{
#ifdef _UNICODE
	return DTWAIN_GetTwainCountryValue(country);
#else
	return DTWAIN_GetTwainCountryValue(StringConversion::Convert_WidePtr_To_Native(country).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetTwainLanguageNameA(LONG nameID, LPSTR ret)
{
#ifdef _UNICODE
	std::wstring arg(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetTwainLanguageName(nameID, &arg[0]);
	return null_terminator_copier(get_view(arg), ret, retVal);
#else
	return DTWAIN_GetTwainLanguageName(nameID, ret);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetTwainLanguageNameW(LONG nameID, LPWSTR ret)
{
#ifdef _UNICODE
	return DTWAIN_GetTwainLanguageName(nameID, ret);
#else
	std::string arg(1024, 0);
	DTWAIN_BOOL retVal = DTWAIN_GetTwainLanguageName(nameID, &arg[0]);
	return null_terminator_copier(get_view(arg), ret, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetTwainLanguageValueA(LPCSTR lang)
{
#ifdef _UNICODE
	return DTWAIN_GetTwainLanguageValue(StringConversion::Convert_AnsiPtr_To_Native(lang).c_str());
#else
	return DTWAIN_GetTwainLanguageValue(lang);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetTwainLanguageValueW(LPCWSTR lang)
{
#ifdef _UNICODE
	return DTWAIN_GetTwainLanguageValue(lang);
#else
	return DTWAIN_GetTwainLanguageValue(StringConversion::Convert_WidePtr_To_Native(lang).c_str());
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetVersionInfoW(LPWSTR lpszVer, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetVersionInfo(lpszVer, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetVersionInfo((nLength>0 && lpszVer)? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszVer, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetVersionInfoA(LPSTR lpszVer, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetVersionInfo((nLength>0 && lpszVer) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszVer, retVal);
#else
    return DTWAIN_GetVersionInfo(lpszVer, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetVersionStringW(LPWSTR lpszVer, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetVersionString(lpszVer, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetVersionString((nLength>0 && lpszVer) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszVer, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetLibraryPathA(LPSTR lpszVer, LONG nLength)
{
#ifdef _UNICODE
	std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetLibraryPath((nLength>0 && lpszVer) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
	return null_terminator_copier(get_view(args), lpszVer, retVal);
#else
	return DTWAIN_GetLibraryPath(lpszVer, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetLibraryPathW(LPWSTR lpszVer, LONG nLength)
{
#ifdef _UNICODE
	return DTWAIN_GetLibraryPath(lpszVer, nLength);
#else
	std::string args((std::max)(nLength, 0L), 0);
	LONG retVal = DTWAIN_GetLibraryPath((nLength>0 && lpszVer) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
	return null_terminator_copier(get_view(args), lpszVer, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetVersionStringA(LPSTR lpszVer, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetVersionString((nLength>0 && lpszVer) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszVer, retVal);
#else
    return DTWAIN_GetVersionString(lpszVer, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetShortVersionStringW(LPWSTR lpszVer, LONG nLength)
{
#ifdef _UNICODE
	return DTWAIN_GetShortVersionString(lpszVer, nLength);
#else
	std::string args((std::max)(nLength, 0L), 0);
	LONG retVal = DTWAIN_GetShortVersionString((nLength>0 && lpszVer)? &args[0] : nullptr, static_cast<LONG>(args.size()));
	return null_terminator_copier(get_view(args), lpszVer, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetShortVersionStringA(LPSTR lpszVer, LONG nLength)
{
#ifdef _UNICODE
	std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetShortVersionString((nLength>0 && lpszVer) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
	return null_terminator_copier(get_view(args), lpszVer, retVal);
#else
	return DTWAIN_GetShortVersionString(lpszVer, nLength);
#endif
}


LONG DLLENTRY_DEF DTWAIN_GetTwainStringNameW(LONG category, LONG TwainID, LPWSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetTwainStringName(category, TwainID, lpszBuffer, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetTwainStringName(category, TwainID, (nLength > 0 && lpszBuffer)? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszBuffer, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetTwainStringNameA(LONG category, LONG TwainID, LPSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetTwainStringName(category, TwainID, (nLength > 0 && lpszBuffer) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszBuffer, retVal);
#else
    return DTWAIN_GetTwainStringName(category, TwainID, lpszBuffer, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetTwainIDFromNameA(LPCSTR lpszBuffer)
{
#ifdef _UNICODE
    return DTWAIN_GetTwainIDFromName(StringConversion::Convert_AnsiPtr_To_Native(lpszBuffer).c_str());
#else
    return DTWAIN_GetTwainIDFromName(lpszBuffer);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetTwainIDFromNameW(LPCWSTR lpszBuffer)
{
#ifdef _UNICODE
    return DTWAIN_GetTwainIDFromName(lpszBuffer);
#else
    return DTWAIN_GetTwainIDFromName(StringConversion::Convert_WidePtr_To_Native(lpszBuffer).c_str());
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetWindowsVersionInfoW(LPWSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetWindowsVersionInfo(lpszBuffer, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetWindowsVersionInfo((nLength > 0 && lpszBuffer)? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszBuffer, retVal);
#endif
}


LONG DLLENTRY_DEF DTWAIN_GetWindowsVersionInfoA(LPSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetWindowsVersionInfo((nLength > 0 && lpszBuffer) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszBuffer, retVal);
#else
    return DTWAIN_GetWindowsVersionInfo(lpszBuffer, nLength);
#endif
}


LONG DLLENTRY_DEF DTWAIN_GetActiveDSMPathW(LPWSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetActiveDSMPath(lpszBuffer, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetActiveDSMPath((nLength > 0 && lpszBuffer)? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszBuffer, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetActiveDSMPathA(LPSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetActiveDSMPath((nLength > 0 && lpszBuffer) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszBuffer, retVal);
#else
    return DTWAIN_GetActiveDSMPath(lpszBuffer, nLength);
#endif
}


LONG DLLENTRY_DEF DTWAIN_GetActiveDSMVersionInfoW(LPWSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetActiveDSMVersionInfo(lpszBuffer, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetActiveDSMVersionInfo((nLength > 0 && lpszBuffer) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszBuffer, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetActiveDSMVersionInfoA(LPSTR lpszBuffer, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetActiveDSMVersionInfo((nLength > 0 && lpszBuffer) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszBuffer, retVal);
#else
    return DTWAIN_GetActiveDSMVersionInfo(lpszBuffer, nLength);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetXResolutionStringA(DTWAIN_SOURCE Source, LPSTR Resolution)
{
#ifdef _UNICODE
    std::wstring args(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetXResolutionString(Source, &args[0]);
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
    DTWAIN_BOOL retVal = DTWAIN_GetXResolutionString(Source, &args[0]);
    return null_terminator_copier(get_view(args), Resolution, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetYResolutionStringA(DTWAIN_SOURCE Source, LPSTR Resolution)
{
#ifdef _UNICODE
    std::wstring args(1024, 0);
    const DTWAIN_BOOL retVal = DTWAIN_GetYResolutionString(Source, &args[0]);
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
    DTWAIN_BOOL retVal = DTWAIN_GetYResolutionString(Source, &args[0]);
    return null_terminator_copier(get_view(args), Resolution, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_InitImageFileAppendW(LPCWSTR szFile, LONG fType)
{
#ifdef _UNICODE
    return DTWAIN_InitImageFileAppend(szFile, fType);
#else
    return DTWAIN_InitImageFileAppend(StringConversion::Convert_WidePtr_To_Native(szFile).c_str(), fType);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_InitImageFileAppendA(LPCSTR szFile, LONG fType)
{
#ifdef _UNICODE
    return DTWAIN_InitImageFileAppend(StringConversion::Convert_AnsiPtr_To_Native(szFile).c_str(), fType);
#else
    return DTWAIN_InitImageFileAppend(szFile, fType);
#endif
}

LONG DLLENTRY_DEF DTWAIN_IsDIBBlankStringA(HANDLE hDib, LPCSTR threshold)
{
#ifdef _UNICODE
    return DTWAIN_IsDIBBlankString(hDib, StringConversion::Convert_AnsiPtr_To_Native(threshold).c_str());
#else
    return DTWAIN_IsDIBBlankString(hDib, threshold);
#endif
}

LONG DLLENTRY_DEF DTWAIN_IsDIBBlankStringW(HANDLE hDib, LPCWSTR threshold)
{
#ifdef _UNICODE
    return DTWAIN_IsDIBBlankString(hDib, threshold);
#else
    return DTWAIN_IsDIBBlankString(hDib, StringConversion::Convert_WidePtr_To_Native(threshold).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResourcesW(LPCWSTR sLangDLL)
{
#ifdef _UNICODE
    return DTWAIN_LoadCustomStringResources(sLangDLL);
#else
    return DTWAIN_LoadCustomStringResources(StringConversion::Convert_WidePtr_To_Native(sLangDLL).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResourcesA(LPCSTR sLangDLL)
{
#ifdef _UNICODE
    return DTWAIN_LoadCustomStringResources(StringConversion::Convert_AnsiPtr_To_Native(sLangDLL).c_str());
#else
    return DTWAIN_LoadCustomStringResources(sLangDLL);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResourcesExW(LPCWSTR sLangDLL, DTWAIN_BOOL bClear)
{
#ifdef _UNICODE
    return DTWAIN_LoadCustomStringResourcesEx(sLangDLL, bClear);
#else
    return DTWAIN_LoadCustomStringResourcesEx(StringConversion::Convert_WidePtr_To_Native(sLangDLL).c_str(), bClear);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResourcesExA(LPCSTR sLangDLL, DTWAIN_BOOL bClear)
{
#ifdef _UNICODE
    return DTWAIN_LoadCustomStringResourcesEx(StringConversion::Convert_AnsiPtr_To_Native(sLangDLL).c_str(), bClear);
#else
    return DTWAIN_LoadCustomStringResourcesEx(sLangDLL, bClear);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LogMessageW(LPCWSTR message)
{
#ifdef _UNICODE
    return DTWAIN_LogMessage(message);
#else
    return DTWAIN_LogMessage(StringConversion::Convert_WidePtr_To_Native(message).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LogMessageA(LPCSTR message)
{
#ifdef _UNICODE
    return DTWAIN_LogMessage(StringConversion::Convert_AnsiPtr_To_Native(message).c_str());
#else
    return DTWAIN_LogMessage(message);
#endif
}

DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngineByNameW(LPCWSTR lpszName)
{
#ifdef _UNICODE
    return DTWAIN_SelectOCREngineByName(lpszName);
#else
    return DTWAIN_SelectOCREngineByName(StringConversion::Convert_WidePtr_To_Native(lpszName).c_str());
#endif
}

DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngineByNameA(LPCSTR lpszName)
{
#ifdef _UNICODE
    return DTWAIN_SelectOCREngineByName(StringConversion::Convert_AnsiPtr_To_Native(lpszName).c_str());
#else
    return DTWAIN_SelectOCREngineByName(lpszName);
#endif
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSource2A(HWND hWndParent, LPCSTR szTitle, LONG xPos, LONG yPos, LONG nOptions)
{
#ifdef _UNICODE
    return DTWAIN_SelectSource2(hWndParent, szTitle?StringConversion::Convert_AnsiPtr_To_Native(szTitle).c_str(): nullptr, xPos, yPos, nOptions);
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
                                szTitle?StringConversion::Convert_WidePtr_To_Native(szTitle).c_str():NULL, xPos, yPos, nOptions);
#endif
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSource2ExA(HWND hWndParent, LPCSTR szTitle, LONG xPos, LONG yPos, 
                                                   LPCSTR szIncludeNames, LPCSTR szExcludeNames, LPCSTR szNameMapping, LONG nOptions)
{
#ifdef _UNICODE
    return DTWAIN_SelectSource2Ex(hWndParent, 
                                  szTitle?StringConversion::Convert_AnsiPtr_To_Native(szTitle).c_str(): nullptr, xPos, yPos, 
                                  szIncludeNames?StringConversion::Convert_AnsiPtr_To_Native(szIncludeNames).c_str(): nullptr, 
                                  szExcludeNames?StringConversion::Convert_AnsiPtr_To_Native(szExcludeNames).c_str(): nullptr,
                                  szNameMapping?StringConversion::Convert_AnsiPtr_To_Native(szNameMapping).c_str(): nullptr,
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
                                  szTitle ? StringConversion::Convert_WidePtr_To_Native(szTitle).c_str() : NULL,
                                  xPos, yPos,
                                  szIncludeNames ? StringConversion::Convert_WidePtr_To_Native(szIncludeNames).c_str() : NULL,
                                  szExcludeNames ? StringConversion::Convert_WidePtr_To_Native(szExcludeNames).c_str() : NULL,
                                  szNameMapping ? StringConversion::Convert_WidePtr_To_Native(szNameMapping).c_str() : NULL,
                                  nOptions);
#endif
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceByNameW(LPCWSTR lpszName)
{
#ifdef _UNICODE
    return DTWAIN_SelectSourceByName(lpszName);
#else
    return DTWAIN_SelectSourceByName(StringConversion::Convert_WidePtr_To_Native(lpszName).c_str());
#endif
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceByNameA(LPCSTR lpszName)
{
#ifdef _UNICODE
    return DTWAIN_SelectSourceByName(StringConversion::Convert_AnsiPtr_To_Native(lpszName).c_str());
#else
    return DTWAIN_SelectSourceByName(lpszName);
#endif
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceByNameWithOpenW(LPCWSTR lpszName, DTWAIN_BOOL bOpen)
{
#ifdef _UNICODE
    return DTWAIN_SelectSourceByNameWithOpen(lpszName, bOpen);
#else
    return DTWAIN_SelectSourceByNameWithOpen(StringConversion::Convert_WidePtr_To_Native(lpszName).c_str(), bOpen);
#endif
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceByNameWithOpenA(LPCSTR lpszName, DTWAIN_BOOL bOpen)
{
#ifdef _UNICODE
    return DTWAIN_SelectSourceByNameWithOpen(StringConversion::Convert_AnsiPtr_To_Native(lpszName).c_str(), bOpen);
#else
    return DTWAIN_SelectSourceByNameWithOpen(lpszName, bOpen);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireArea2StringW(DTWAIN_SOURCE Source, LPCWSTR left, LPCWSTR top, LPCWSTR right, LPCWSTR bottom, LONG lUnit, LONG Flags)
{
#ifdef _UNICODE
    return DTWAIN_SetAcquireArea2String(Source, left, top, right, bottom, lUnit, Flags);
#else
    return DTWAIN_SetAcquireArea2String(Source, StringConversion::Convert_WidePtr_To_Native(left).c_str(), StringConversion::Convert_WidePtr_To_Native(top).c_str(), StringConversion::Convert_WidePtr_To_Native(right).c_str(), StringConversion::Convert_WidePtr_To_Native(bottom).c_str(), lUnit, Flags);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireArea2StringA(DTWAIN_SOURCE Source, LPCSTR left, LPCSTR top, LPCSTR right, LPCSTR bottom, LONG lUnit, LONG Flags)
{
#ifdef _UNICODE
    return DTWAIN_SetAcquireArea2String(Source, StringConversion::Convert_AnsiPtr_To_Native(left).c_str(), StringConversion::Convert_AnsiPtr_To_Native(top).c_str(), StringConversion::Convert_AnsiPtr_To_Native(right).c_str(), StringConversion::Convert_AnsiPtr_To_Native(bottom).c_str(), lUnit, Flags);
#else
    return DTWAIN_SetAcquireArea2String(Source, left, top, right, bottom, lUnit, Flags);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireImageScaleStringA(DTWAIN_SOURCE Source, LPCSTR xscale, LPCSTR yscale)
{
#ifdef _UNICODE
    return DTWAIN_SetAcquireImageScaleString(Source, StringConversion::Convert_AnsiPtr_To_Native(xscale).c_str(), StringConversion::Convert_AnsiPtr_To_Native(yscale).c_str());
#else
    return DTWAIN_SetAcquireImageScaleString(Source, xscale, yscale);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireImageScaleStringW(DTWAIN_SOURCE Source, LPCWSTR xscale, LPCWSTR yscale)
{
#ifdef _UNICODE
    return DTWAIN_SetAcquireImageScaleString(Source, xscale, yscale);
#else
    return DTWAIN_SetAcquireImageScaleString(Source, StringConversion::Convert_WidePtr_To_Native(xscale).c_str(), StringConversion::Convert_WidePtr_To_Native(yscale).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAppInfoW(LPCWSTR szVerStr, LPCWSTR szManu, LPCWSTR szProdFam, LPCWSTR szProdName)
{
#ifdef _UNICODE
    return DTWAIN_SetAppInfo(szVerStr, szManu, szProdFam, szProdName);
#else
    return DTWAIN_SetAppInfo(StringConversion::Convert_WidePtr_To_Native(szVerStr).c_str(), StringConversion::Convert_WidePtr_To_Native(szManu).c_str(), StringConversion::Convert_WidePtr_To_Native(szProdFam).c_str(), StringConversion::Convert_WidePtr_To_Native(szProdName).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAppInfoA(LPCSTR szVerStr, LPCSTR szManu, LPCSTR szProdFam, LPCSTR szProdName)
{
#ifdef _UNICODE
    return DTWAIN_SetAppInfo(StringConversion::Convert_AnsiPtr_To_Native(szVerStr).c_str(), StringConversion::Convert_AnsiPtr_To_Native(szManu).c_str(), StringConversion::Convert_AnsiPtr_To_Native(szProdFam).c_str(), StringConversion::Convert_AnsiPtr_To_Native(szProdName).c_str());
#else
    return DTWAIN_SetAppInfo(szVerStr, szManu, szProdFam, szProdName);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAuthorA(DTWAIN_SOURCE Source, LPCSTR szAuthor)
{
#ifdef _UNICODE
    return DTWAIN_SetAuthor(Source, StringConversion::Convert_AnsiPtr_To_Native(szAuthor).c_str());
#else
    return DTWAIN_SetAuthor(Source, szAuthor);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAuthorW(DTWAIN_SOURCE Source, LPCWSTR szAuthor)
{
#ifdef _UNICODE
    return DTWAIN_SetAuthor(Source, szAuthor);
#else
    return DTWAIN_SetAuthor(Source, StringConversion::Convert_WidePtr_To_Native(szAuthor).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBlankPageDetectionStringA(DTWAIN_SOURCE Source, LPCSTR threshold, LONG autodetect_option, DTWAIN_BOOL bSet)
{
#ifdef _UNICODE
    return DTWAIN_SetBlankPageDetectionString(Source, StringConversion::Convert_AnsiPtr_To_Native(threshold).c_str(), autodetect_option, bSet);
#else
    return DTWAIN_SetBlankPageDetectionString(Source, threshold, autodetect_option, bSet);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBlankPageDetectionStringW(DTWAIN_SOURCE Source, LPCWSTR threshold, LONG autodetect_option, DTWAIN_BOOL bSet)
{
#ifdef _UNICODE
    return DTWAIN_SetBlankPageDetectionString(Source, threshold, autodetect_option, bSet);
#else
    return DTWAIN_SetBlankPageDetectionString(Source, StringConversion::Convert_WidePtr_To_Native(threshold).c_str(), autodetect_option, bSet);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBlankPageDetectionExStringA(DTWAIN_SOURCE Source, LPCSTR threshold, LONG autodetect_option, LONG detectOpts, DTWAIN_BOOL bSet)
{
#ifdef _UNICODE
    return DTWAIN_SetBlankPageDetectionExString(Source, StringConversion::Convert_AnsiPtr_To_Native(threshold).c_str(), autodetect_option, detectOpts, bSet);
#else
    return DTWAIN_SetBlankPageDetectionExString(Source, threshold, autodetect_option, detectOpts, bSet);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBlankPageDetectionExStringW(DTWAIN_SOURCE Source, LPCWSTR threshold, LONG autodetect_option, LONG detectOpts, DTWAIN_BOOL bSet)
{
#ifdef _UNICODE
    return DTWAIN_SetBlankPageDetectionExString(Source, threshold, autodetect_option, detectOpts, bSet);
#else
    return DTWAIN_SetBlankPageDetectionExString(Source, StringConversion::Convert_WidePtr_To_Native(threshold).c_str(), autodetect_option, detectOpts, bSet);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBrightnessStringA(DTWAIN_SOURCE Source, LPCSTR Contrast)
{
#ifdef _UNICODE
    return DTWAIN_SetBrightnessString(Source, StringConversion::Convert_AnsiPtr_To_Native(Contrast).c_str());
#else
    return DTWAIN_SetBrightnessString(Source, Contrast);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBrightnessStringW(DTWAIN_SOURCE Source, LPCWSTR Contrast)
{
#ifdef _UNICODE
    return DTWAIN_SetBrightnessString(Source, Contrast);
#else
    return DTWAIN_SetBrightnessString(Source, StringConversion::Convert_WidePtr_To_Native(Contrast).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCameraW(DTWAIN_SOURCE Source, LPCWSTR szCamera)
{
#ifdef _UNICODE
    return DTWAIN_SetCamera(Source, szCamera);
#else
    return DTWAIN_SetCamera(Source, StringConversion::Convert_WidePtr_To_Native(szCamera).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCameraA(DTWAIN_SOURCE Source, LPCSTR szCamera)
{
#ifdef _UNICODE
    return DTWAIN_SetCamera(Source, StringConversion::Convert_AnsiPtr_To_Native(szCamera).c_str());
#else
    return DTWAIN_SetCamera(Source, szCamera);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCaptionA(DTWAIN_SOURCE Source, LPCSTR Caption)
{
#ifdef _UNICODE
    return DTWAIN_SetCaption(Source, StringConversion::Convert_AnsiPtr_To_Native(Caption).c_str());
#else
    return DTWAIN_SetCaption(Source, Caption);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCaptionW(DTWAIN_SOURCE Source, LPCWSTR Caption)
{
#ifdef _UNICODE
    return DTWAIN_SetCaption(Source, Caption);
#else
    return DTWAIN_SetCaption(Source, StringConversion::Convert_WidePtr_To_Native(Caption).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetContrastStringA(DTWAIN_SOURCE Source, LPCSTR Contrast)
{
#ifdef _UNICODE
    return DTWAIN_SetContrastString(Source, StringConversion::Convert_AnsiPtr_To_Native(Contrast).c_str());
#else
    return DTWAIN_SetContrastString(Source, Contrast);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetContrastStringW(DTWAIN_SOURCE Source, LPCWSTR Contrast)
{
#ifdef _UNICODE
    return DTWAIN_SetContrastString(Source, Contrast);
#else
    return DTWAIN_SetContrastString(Source, StringConversion::Convert_WidePtr_To_Native(Contrast).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDeviceTimeDateA(DTWAIN_SOURCE Source, LPCSTR szTimeDate)
{
#ifdef _UNICODE
    return DTWAIN_SetDeviceTimeDate(Source, StringConversion::Convert_AnsiPtr_To_Native(szTimeDate).c_str());
#else
    return DTWAIN_SetDeviceTimeDate(Source, szTimeDate);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDeviceTimeDateW(DTWAIN_SOURCE Source, LPCWSTR szTimeDate)
{
#ifdef _UNICODE
    return DTWAIN_SetDeviceTimeDate(Source, szTimeDate);
#else
    return DTWAIN_SetDeviceTimeDate(Source, StringConversion::Convert_WidePtr_To_Native(szTimeDate).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetFileSavePosW(HWND hWndParent, LPCWSTR szTitle, LONG xPos, LONG yPos, LONG nFlags)
{
#ifdef _UNICODE
    return DTWAIN_SetFileSavePos(hWndParent, szTitle, xPos, yPos, nFlags);
#else
    return DTWAIN_SetFileSavePos(hWndParent, StringConversion::Convert_WidePtr_To_Native(szTitle).c_str(), xPos, yPos, nFlags);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetFileSavePosA(HWND hWndParent, LPCSTR szTitle, LONG xPos, LONG yPos, LONG nFlags)
{
#ifdef _UNICODE
    return DTWAIN_SetFileSavePos(hWndParent, StringConversion::Convert_AnsiPtr_To_Native(szTitle).c_str(), xPos, yPos, nFlags);
#else
    return DTWAIN_SetFileSavePos(hWndParent, szTitle, xPos, yPos, nFlags);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetHalftoneW(DTWAIN_SOURCE Source, LPCWSTR lpHalftone)
{
#ifdef _UNICODE
    return DTWAIN_SetHalftone(Source, lpHalftone);
#else
    return DTWAIN_SetHalftone(Source, StringConversion::Convert_WidePtr_To_Native(lpHalftone).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetHalftoneA(DTWAIN_SOURCE Source, LPCSTR lpHalftone)
{
#ifdef _UNICODE
    return DTWAIN_SetHalftone(Source, StringConversion::Convert_AnsiPtr_To_Native(lpHalftone).c_str());
#else
    return DTWAIN_SetHalftone(Source, lpHalftone);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetHighlightStringA(DTWAIN_SOURCE Source, LPCSTR Highlight)
{
#ifdef _UNICODE
    return DTWAIN_SetHighlightString(Source, StringConversion::Convert_AnsiPtr_To_Native(Highlight).c_str());
#else
    return DTWAIN_SetHighlightString(Source, Highlight);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetHighlightStringW(DTWAIN_SOURCE Source, LPCWSTR Highlight)
{
#ifdef _UNICODE
    return DTWAIN_SetHighlightString(Source, Highlight);
#else
    return DTWAIN_SetHighlightString(Source, StringConversion::Convert_WidePtr_To_Native(Highlight).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFAuthorW(DTWAIN_SOURCE Source, LPCWSTR lpAuthor)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFAuthor(Source, lpAuthor);
#else
    return DTWAIN_SetPDFAuthor(Source, StringConversion::Convert_WidePtr_To_Native(lpAuthor).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFAuthorA(DTWAIN_SOURCE Source, LPCSTR lpAuthor)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFAuthor(Source, StringConversion::Convert_AnsiPtr_To_Native(lpAuthor).c_str());
#else
    return DTWAIN_SetPDFAuthor(Source, lpAuthor);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFCreatorW(DTWAIN_SOURCE Source, LPCWSTR lpCreator)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFCreator(Source, lpCreator);
#else
    return DTWAIN_SetPDFCreator(Source, StringConversion::Convert_WidePtr_To_Native(lpCreator).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFCreatorA(DTWAIN_SOURCE Source, LPCSTR lpCreator)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFCreator(Source, StringConversion::Convert_AnsiPtr_To_Native(lpCreator).c_str());
#else
    return DTWAIN_SetPDFCreator(Source, lpCreator);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFEncryptionW(DTWAIN_SOURCE Source, DTWAIN_BOOL bUseEncryption, LPCWSTR lpszUser, LPCWSTR lpszOwner, LONG Permissions, DTWAIN_BOOL UseStrongEncryption)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFEncryption(Source, bUseEncryption, lpszUser, lpszOwner, Permissions, UseStrongEncryption);
#else
    return DTWAIN_SetPDFEncryption(Source, bUseEncryption, StringConversion::Convert_WidePtr_To_Native(lpszUser).c_str(), StringConversion::Convert_WidePtr_To_Native(lpszOwner).c_str(), Permissions, UseStrongEncryption);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFEncryptionA(DTWAIN_SOURCE Source, DTWAIN_BOOL bUseEncryption, LPCSTR lpszUser, LPCSTR lpszOwner, LONG Permissions, DTWAIN_BOOL UseStrongEncryption)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFEncryption(Source, bUseEncryption, StringConversion::Convert_AnsiPtr_To_Native(lpszUser).c_str(), StringConversion::Convert_AnsiPtr_To_Native(lpszOwner).c_str(), Permissions, UseStrongEncryption);
#else
    return DTWAIN_SetPDFEncryption(Source, bUseEncryption, lpszUser, lpszOwner, Permissions, UseStrongEncryption);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFKeywordsA(DTWAIN_SOURCE Source, LPCSTR lpKeyWords)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFKeywords(Source, StringConversion::Convert_AnsiPtr_To_Native(lpKeyWords).c_str());
#else
    return DTWAIN_SetPDFKeywords(Source, lpKeyWords);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFKeywordsW(DTWAIN_SOURCE Source, LPCWSTR lpKeyWords)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFKeywords(Source, lpKeyWords);
#else
    return DTWAIN_SetPDFKeywords(Source, StringConversion::Convert_WidePtr_To_Native(lpKeyWords).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageScaleStringA(DTWAIN_SOURCE Source, LONG nOptions, LPCSTR xScale, LPCSTR yScale)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFPageScaleString(Source, nOptions, StringConversion::Convert_AnsiPtr_To_Native(xScale).c_str(), StringConversion::Convert_AnsiPtr_To_Native(yScale).c_str());
#else
    return DTWAIN_SetPDFPageScaleString(Source, nOptions, xScale, yScale);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageScaleStringW(DTWAIN_SOURCE Source, LONG nOptions, LPCWSTR xScale, LPCWSTR yScale)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFPageScaleString(Source, nOptions, xScale, yScale);
#else
    return DTWAIN_SetPDFPageScaleString(Source, nOptions, StringConversion::Convert_WidePtr_To_Native(xScale).c_str(), StringConversion::Convert_WidePtr_To_Native(yScale).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageSizeStringA(DTWAIN_SOURCE Source, LONG PageSize, LPCSTR CustomWidth, LPCSTR CustomHeight)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFPageSizeString(Source, PageSize, StringConversion::Convert_AnsiPtr_To_Native(CustomWidth).c_str(), StringConversion::Convert_AnsiPtr_To_Native(CustomHeight).c_str());
#else
    return DTWAIN_SetPDFPageSizeString(Source, PageSize, CustomWidth, CustomHeight);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageSizeStringW(DTWAIN_SOURCE Source, LONG PageSize, LPCWSTR CustomWidth, LPCWSTR CustomHeight)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFPageSizeString(Source, PageSize, CustomWidth, CustomHeight);
#else
    return DTWAIN_SetPDFPageSizeString(Source, PageSize, StringConversion::Convert_WidePtr_To_Native(CustomWidth).c_str(), StringConversion::Convert_WidePtr_To_Native(CustomHeight).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFProducerW(DTWAIN_SOURCE Source, LPCWSTR lpProducer)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFProducer(Source, lpProducer);
#else
    return DTWAIN_SetPDFProducer(Source, StringConversion::Convert_WidePtr_To_Native(lpProducer).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFProducerA(DTWAIN_SOURCE Source, LPCSTR lpProducer)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFProducer(Source, StringConversion::Convert_AnsiPtr_To_Native(lpProducer).c_str());
#else
    return DTWAIN_SetPDFProducer(Source, lpProducer);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFSubjectA(DTWAIN_SOURCE Source, LPCSTR lpSubject)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFSubject(Source, StringConversion::Convert_AnsiPtr_To_Native(lpSubject).c_str());
#else
    return DTWAIN_SetPDFSubject(Source, lpSubject);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFSubjectW(DTWAIN_SOURCE Source, LPCWSTR lpSubject)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFSubject(Source, lpSubject);
#else
    return DTWAIN_SetPDFSubject(Source, StringConversion::Convert_WidePtr_To_Native(lpSubject).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTitleW(DTWAIN_SOURCE Source, LPCWSTR lpTitle)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFTitle(Source, lpTitle);
#else
    return DTWAIN_SetPDFTitle(Source, StringConversion::Convert_WidePtr_To_Native(lpTitle).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTitleA(DTWAIN_SOURCE Source, LPCSTR lpTitle)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFTitle(Source, StringConversion::Convert_AnsiPtr_To_Native(lpTitle).c_str());
#else
    return DTWAIN_SetPDFTitle(Source, lpTitle);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPostScriptTitleW(DTWAIN_SOURCE Source, LPCWSTR szTitle)
{
#ifdef _UNICODE
    return DTWAIN_SetPostScriptTitle(Source, szTitle);
#else
    return DTWAIN_SetPostScriptTitle(Source, StringConversion::Convert_WidePtr_To_Native(szTitle).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPostScriptTitleA(DTWAIN_SOURCE Source, LPCSTR szTitle)
{
#ifdef _UNICODE
    return DTWAIN_SetPostScriptTitle(Source, StringConversion::Convert_AnsiPtr_To_Native(szTitle).c_str());
#else
    return DTWAIN_SetPostScriptTitle(Source, szTitle);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPrinterSuffixStringW(DTWAIN_SOURCE Source, LPCWSTR Suffix)
{
#ifdef _UNICODE
    return DTWAIN_SetPrinterSuffixString(Source, Suffix);
#else
    return DTWAIN_SetPrinterSuffixString(Source, StringConversion::Convert_WidePtr_To_Native(Suffix).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPrinterSuffixStringA(DTWAIN_SOURCE Source, LPCSTR Suffix)
{
#ifdef _UNICODE
    return DTWAIN_SetPrinterSuffixString(Source, StringConversion::Convert_AnsiPtr_To_Native(Suffix).c_str());
#else
    return DTWAIN_SetPrinterSuffixString(Source, Suffix);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetResolutionStringW(DTWAIN_SOURCE Source, LPCWSTR Resolution)
{
#ifdef _UNICODE
    return DTWAIN_SetResolutionString(Source, Resolution);
#else
    return DTWAIN_SetResolutionString(Source, StringConversion::Convert_WidePtr_To_Native(Resolution).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetResolutionStringA(DTWAIN_SOURCE Source, LPCSTR Resolution)
{
#ifdef _UNICODE
    return DTWAIN_SetResolutionString(Source, StringConversion::Convert_AnsiPtr_To_Native(Resolution).c_str());
#else
    return DTWAIN_SetResolutionString(Source, Resolution);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetRotationStringA(DTWAIN_SOURCE Source, LPCSTR Rotation)
{
#ifdef _UNICODE
    return DTWAIN_SetRotationString(Source, StringConversion::Convert_AnsiPtr_To_Native(Rotation).c_str());
#else
    return DTWAIN_SetRotationString(Source, Rotation);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetRotationStringW(DTWAIN_SOURCE Source, LPCWSTR Rotation)
{
#ifdef _UNICODE
    return DTWAIN_SetRotationString(Source, Rotation);
#else
    return DTWAIN_SetRotationString(Source, StringConversion::Convert_WidePtr_To_Native(Rotation).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetSaveFileNameW(DTWAIN_SOURCE Source, LPCWSTR fName)
{
#ifdef _UNICODE
    return DTWAIN_SetSaveFileName(Source, fName);
#else
    return DTWAIN_SetSaveFileName(Source, StringConversion::Convert_WidePtr_To_Native(fName).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetSaveFileNameA(DTWAIN_SOURCE Source, LPCSTR fName)
{
#ifdef _UNICODE
    return DTWAIN_SetSaveFileName(Source, StringConversion::Convert_AnsiPtr_To_Native(fName).c_str());
#else
    return DTWAIN_SetSaveFileName(Source, fName);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetShadowStringA(DTWAIN_SOURCE Source, LPCSTR Shadow)
{
#ifdef _UNICODE
    return DTWAIN_SetShadowString(Source, StringConversion::Convert_AnsiPtr_To_Native(Shadow).c_str());
#else
    return DTWAIN_SetShadowString(Source, Shadow);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetShadowStringW(DTWAIN_SOURCE Source, LPCWSTR Shadow)
{
#ifdef _UNICODE
    return DTWAIN_SetShadowString(Source, Shadow);
#else
    return DTWAIN_SetShadowString(Source, StringConversion::Convert_WidePtr_To_Native(Shadow).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTempFileDirectoryW(LPCWSTR szFilePath)
{
#ifdef _UNICODE
    return DTWAIN_SetTempFileDirectory(szFilePath);
#else
    return DTWAIN_SetTempFileDirectory(StringConversion::Convert_WidePtr_To_Native(szFilePath).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTempFileDirectoryA(LPCSTR szFilePath)
{
#ifdef _UNICODE
    return DTWAIN_SetTempFileDirectory(StringConversion::Convert_AnsiPtr_To_Native(szFilePath).c_str());
#else
    return DTWAIN_SetTempFileDirectory(szFilePath);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDSMSearchOrderExW(LPCWSTR szFilePath, LPCWSTR szUserPath)
{
#ifdef _UNICODE
    return DTWAIN_SetDSMSearchOrderEx(szFilePath, szUserPath);
#else
    return DTWAIN_SetDSMSearchOrderEx(StringConversion::Convert_WidePtr_To_Native(szFilePath).c_str(),
                                      szUserPath?StringConversion::Convert_WidePtr_To_Native(szUserPath).c_str():nullptr);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDSMSearchOrderExA(LPCSTR szFilePath, LPCSTR szUserPath)
{
#ifdef _UNICODE
    return DTWAIN_SetDSMSearchOrderEx(StringConversion::Convert_AnsiPtr_To_Native(szFilePath).c_str(),
                                      szUserPath ? StringConversion::Convert_AnsiPtr_To_Native(szUserPath).c_str() : nullptr);
#else
    return DTWAIN_SetDSMSearchOrderEx(szFilePath, szUserPath);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetResourcePathW(LPCWSTR szFilePath)
{
#ifdef _UNICODE
    return DTWAIN_SetResourcePath(szFilePath);
#else
    return DTWAIN_SetResourcePath(StringConversion::Convert_WidePtr_To_Native(szFilePath).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetResourcePathA(LPCSTR szFilePath)
{
#ifdef _UNICODE
    return DTWAIN_SetResourcePath(StringConversion::Convert_AnsiPtr_To_Native(szFilePath).c_str());
#else
    return DTWAIN_SetResourcePath(szFilePath);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetThresholdStringA(DTWAIN_SOURCE Source, LPCSTR Threshold, DTWAIN_BOOL bSetBitDepthReduction)
{
#ifdef _UNICODE
    return DTWAIN_SetThresholdString(Source, StringConversion::Convert_AnsiPtr_To_Native(Threshold).c_str(), bSetBitDepthReduction);
#else
    return DTWAIN_SetThresholdString(Source, Threshold, bSetBitDepthReduction);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetThresholdStringW(DTWAIN_SOURCE Source, LPCWSTR Threshold, DTWAIN_BOOL bSetBitDepthReduction)
{
#ifdef _UNICODE
    return DTWAIN_SetThresholdString(Source, Threshold, bSetBitDepthReduction);
#else
    return DTWAIN_SetThresholdString(Source, StringConversion::Convert_WidePtr_To_Native(Threshold).c_str(), bSetBitDepthReduction);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainLogA(LONG LogFlags, LPCSTR lpszLogFile)
{
#ifdef _UNICODE
    return DTWAIN_SetTwainLog(LogFlags, StringConversion::Convert_AnsiPtr_To_Native(lpszLogFile).c_str());
#else
    return DTWAIN_SetTwainLog(LogFlags, lpszLogFile);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainLogW(LONG LogFlags, LPCWSTR lpszLogFile)
{
#ifdef _UNICODE
    return DTWAIN_SetTwainLog(LogFlags, lpszLogFile);
#else
    return DTWAIN_SetTwainLog(LogFlags, StringConversion::Convert_WidePtr_To_Native(lpszLogFile).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetXResolutionStringW(DTWAIN_SOURCE Source, LPCWSTR Resolution)
{
#ifdef _UNICODE
    return DTWAIN_SetXResolutionString(Source, Resolution);
#else
    return DTWAIN_SetXResolutionString(Source, StringConversion::Convert_WidePtr_To_Native(Resolution).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetXResolutionStringA(DTWAIN_SOURCE Source, LPCSTR Resolution)
{
#ifdef _UNICODE
    return DTWAIN_SetXResolutionString(Source, StringConversion::Convert_AnsiPtr_To_Native(Resolution).c_str());
#else
    return DTWAIN_SetXResolutionString(Source, Resolution);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetYResolutionStringW(DTWAIN_SOURCE Source, LPCWSTR Resolution)
{
#ifdef _UNICODE
    return DTWAIN_SetYResolutionString(Source, Resolution);
#else
    return DTWAIN_SetYResolutionString(Source, StringConversion::Convert_WidePtr_To_Native(Resolution).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetYResolutionStringA(DTWAIN_SOURCE Source, LPCSTR Resolution)
{
#ifdef _UNICODE
    return DTWAIN_SetYResolutionString(Source, StringConversion::Convert_AnsiPtr_To_Native(Resolution).c_str());
#else
    return DTWAIN_SetYResolutionString(Source, Resolution);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_StartTwainSessionW(HWND hWndMsg, LPCWSTR lpszDLLName)
{
#ifdef _UNICODE
    return DTWAIN_StartTwainSession(hWndMsg, lpszDLLName);
#else
    return DTWAIN_StartTwainSession(hWndMsg, StringConversion::Convert_WidePtr_To_Native(lpszDLLName).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_StartTwainSessionA(HWND hWndMsg, LPCSTR lpszDLLName)
{
#ifdef _UNICODE
    return DTWAIN_StartTwainSession(hWndMsg, StringConversion::Convert_AnsiPtr_To_Native(lpszDLLName).c_str());
#else
    return DTWAIN_StartTwainSession(hWndMsg, lpszDLLName);
#endif
}

DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeExA(LPCSTR szINIPath)
{
#ifdef _UNICODE
    return DTWAIN_SysInitializeEx(StringConversion::Convert_AnsiPtr_To_Native(szINIPath).c_str());
#else
    return DTWAIN_SysInitializeEx(szINIPath);
#endif
}

DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeExW(LPCWSTR szINIPath)
{
#ifdef _UNICODE
    return DTWAIN_SysInitializeEx(szINIPath);
#else
    return DTWAIN_SysInitializeEx(StringConversion::Convert_WidePtr_To_Native(szINIPath).c_str());
#endif
}

DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeEx2W(LPCWSTR szINIPath, LPCWSTR szImageDLLPath, LPCWSTR szLangResourcePath)
{
#ifdef _UNICODE
    return DTWAIN_SysInitializeEx2(szINIPath, szImageDLLPath, szLangResourcePath);
#else
    return DTWAIN_SysInitializeEx2(StringConversion::Convert_WidePtr_To_Native(szINIPath).c_str(), StringConversion::Convert_WidePtr_To_Native(szImageDLLPath).c_str(), StringConversion::Convert_WidePtr_To_Native(szLangResourcePath).c_str());
#endif
}

DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeEx2A(LPCSTR szINIPath, LPCSTR szImageDLLPath, LPCSTR szLangResourcePath)
{
#ifdef _UNICODE
    return DTWAIN_SysInitializeEx2(StringConversion::Convert_AnsiPtr_To_Native(szINIPath).c_str(), StringConversion::Convert_AnsiPtr_To_Native(szImageDLLPath).c_str(), StringConversion::Convert_AnsiPtr_To_Native(szLangResourcePath).c_str());
#else
    return DTWAIN_SysInitializeEx2(szINIPath, szImageDLLPath, szLangResourcePath);
#endif
}

DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeLibExW(HINSTANCE hInstance, LPCWSTR szINIPath)
{
#ifdef _UNICODE
    return DTWAIN_SysInitializeLibEx(hInstance, szINIPath);
#else
    return DTWAIN_SysInitializeLibEx(hInstance, StringConversion::Convert_WidePtr_To_Native(szINIPath).c_str());
#endif
}

DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeLibExA(HINSTANCE hInstance, LPCSTR szINIPath)
{
#ifdef _UNICODE
    return DTWAIN_SysInitializeLibEx(hInstance, StringConversion::Convert_AnsiPtr_To_Native(szINIPath).c_str());
#else
    return DTWAIN_SysInitializeLibEx(hInstance, szINIPath);
#endif
}

DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeLibEx2W(HINSTANCE hInstance, LPCWSTR szINIPath, LPCWSTR szImageDLLPath, LPCWSTR szLangResourcePath)
{
#ifdef _UNICODE
    return DTWAIN_SysInitializeLibEx2(hInstance, szINIPath, szImageDLLPath, szLangResourcePath);
#else
    return DTWAIN_SysInitializeLibEx2(hInstance, StringConversion::Convert_WidePtr_To_Native(szINIPath).c_str(), StringConversion::Convert_WidePtr_To_Native(szImageDLLPath).c_str(), StringConversion::Convert_WidePtr_To_Native(szLangResourcePath).c_str());
#endif
}

DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeLibEx2A(HINSTANCE hInstance, LPCSTR szINIPath, LPCSTR szImageDLLPath, LPCSTR szLangResourcePath)
{
#ifdef _UNICODE
    return DTWAIN_SysInitializeLibEx2(hInstance, StringConversion::Convert_AnsiPtr_To_Native(szINIPath).c_str(), StringConversion::Convert_AnsiPtr_To_Native(szImageDLLPath).c_str(), StringConversion::Convert_AnsiPtr_To_Native(szLangResourcePath).c_str());
#else
    return DTWAIN_SysInitializeLibEx2(hInstance, szINIPath, szImageDLLPath, szLangResourcePath);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDoubleFeedDetectLengthStringA(DTWAIN_SOURCE Source, LPCSTR szLength)
{
#ifdef _UNICODE
    return DTWAIN_SetDoubleFeedDetectLengthString(Source, StringConversion::Convert_AnsiPtr_To_Native(szLength).c_str());
#else
    return DTWAIN_SetDoubleFeedDetectLengthString(Source, szLength);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDoubleFeedDetectLengthStringW(DTWAIN_SOURCE Source, LPCWSTR szLength)
{
#ifdef _UNICODE
    return DTWAIN_SetDoubleFeedDetectLengthString(Source, szLength);
#else
    return DTWAIN_SetDoubleFeedDetectLengthString(Source, StringConversion::Convert_WidePtr_To_Native(szLength).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTextElementStringW(DTWAIN_PDFTEXTELEMENT TextElement, LPCWSTR szString, LONG Flags)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFTextElementString(TextElement, szString, Flags);
#else
    return DTWAIN_SetPDFTextElementString(TextElement, StringConversion::Convert_WidePtr_To_Native(szString).c_str(), Flags);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTextElementStringA(DTWAIN_PDFTEXTELEMENT TextElement, LPCSTR szString, LONG Flags)
{
#ifdef _UNICODE
    return DTWAIN_SetPDFTextElementString(TextElement, StringConversion::Convert_AnsiPtr_To_Native(szString).c_str(), Flags);
#else
    return DTWAIN_SetPDFTextElementString(TextElement, szString, Flags);
#endif
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
    DTWAIN_BOOL retVal = DTWAIN_RangeGetAllFloatString(pArray, &args[0][0], &args[1][0], &args[2][0], &args[3][0], &args[4][0]);
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
    const DTWAIN_BOOL retVal = DTWAIN_RangeGetAllFloatString(pArray, &args[0][0], &args[1][0], &args[2][0], &args[3][0], &args[4][0]);
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
                                         StringConversion::Convert_WidePtr_To_Native(dLow).c_str(),
                                         StringConversion::Convert_WidePtr_To_Native(dUp).c_str(),
                                         StringConversion::Convert_WidePtr_To_Native(dStep).c_str(),
                                         StringConversion::Convert_WidePtr_To_Native(dDefault).c_str(),
                                         StringConversion::Convert_WidePtr_To_Native(dCurrent).c_str());
#endif
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeSetAllFloatStringA(DTWAIN_RANGE pArray, LPCSTR dLow,
                                                        LPCSTR dUp, LPCSTR dStep,
                                                        LPCSTR dDefault,
                                                        LPCSTR dCurrent)
{
#ifdef _UNICODE
    return DTWAIN_RangeSetAllFloatString(pArray,
                                         StringConversion::Convert_AnsiPtr_To_Native(dLow).c_str(),
                                         StringConversion::Convert_AnsiPtr_To_Native(dUp).c_str(),
                                         StringConversion::Convert_AnsiPtr_To_Native(dStep).c_str(),
                                         StringConversion::Convert_AnsiPtr_To_Native(dDefault).c_str(),
                                         StringConversion::Convert_AnsiPtr_To_Native(dCurrent).c_str());
#else
    return DTWAIN_RangeSetAllFloatString(pArray, dLow, dUp, dStep, dDefault, dCurrent);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeSetValueFloatStringW(DTWAIN_RANGE pArray, LONG nWhich, LPCWSTR dValue)
{
#ifdef _UNICODE
    return DTWAIN_RangeSetValueFloatString(pArray, nWhich, dValue);
#else
    return DTWAIN_RangeSetValueFloatString(pArray, nWhich, StringConversion::Convert_WidePtr_To_Native(dValue).c_str());
#endif
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeSetValueFloatStringA(DTWAIN_RANGE pArray, LONG nWhich, LPCSTR dValue)
{
#ifdef _UNICODE
    return DTWAIN_RangeSetValueFloatString(pArray, nWhich, StringConversion::Convert_AnsiPtr_To_Native(dValue).c_str());
#else
    return DTWAIN_RangeSetValueFloatString(pArray, nWhich, dValue);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeNearestValueFloatStringW(DTWAIN_RANGE pArray, LPCWSTR dIn, LPWSTR dOut, LONG RoundType)
{
#ifdef _UNICODE
    return DTWAIN_RangeNearestValueFloatString(pArray, dIn, dOut, RoundType);
#else
    std::string arg(128, 0);
    DTWAIN_BOOL retVal = DTWAIN_RangeNearestValueFloatString(pArray,
                                               StringConversion::Convert_WidePtr_To_Native(dIn).c_str(),
                                               &arg[0], RoundType);
    return null_terminator_copier(get_view(arg), dOut, retVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeNearestValueFloatStringA(DTWAIN_RANGE pArray, LPCSTR dIn, LPSTR dOut, LONG RoundType)
{
#ifdef _UNICODE
    std::wstring arg(128, 0);
    const DTWAIN_BOOL retVal = DTWAIN_RangeNearestValueFloatString(pArray,
                                                                   StringConversion::Convert_AnsiPtr_To_Native(dIn).c_str(),
                                                                   &arg[0], RoundType);
    return null_terminator_copier(get_view(arg), dOut, retVal);
#else
    return DTWAIN_RangeNearestValueFloatString(pArray, dIn, dOut, RoundType);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetValueFloatStringW(DTWAIN_RANGE pArray, LONG nWhich, LPWSTR dValue)
{
#ifdef _UNICODE
    return DTWAIN_RangeGetValueFloatString(pArray, nWhich, dValue);
#else
    std::string arg(128, 0);
    DTWAIN_BOOL retval = DTWAIN_RangeGetValueFloatString(pArray, nWhich, &arg[0]);
    return null_terminator_copier(get_view(arg), dValue, retval);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetValueFloatStringA(DTWAIN_RANGE pArray, LONG nWhich, LPSTR dValue)
{
#ifdef _UNICODE
    std::wstring arg(128, 0);
    const DTWAIN_BOOL retval = DTWAIN_RangeGetValueFloatString(pArray, nWhich, &arg[0]);
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
                                         StringConversion::Convert_WidePtr_To_Native(Val).c_str(),
                                         pPos);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetPosFloatStringA(DTWAIN_RANGE pArray, LPCSTR Val, LPLONG pPos)
{
#ifdef _UNICODE
    return DTWAIN_RangeGetPosFloatString(pArray, 
                                         StringConversion::Convert_AnsiPtr_To_Native(Val).c_str(),
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
    DTWAIN_BOOL retval = DTWAIN_RangeGetExpValueFloatString(pArray, lPos, &arg[0]);
    return null_terminator_copier(get_view(arg), pVal, retval);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetExpValueFloatStringA(DTWAIN_RANGE pArray, LONG lPos, LPSTR pVal)
{
#ifdef _UNICODE
    std::wstring arg(128, 0);
    const DTWAIN_BOOL retval = DTWAIN_RangeGetExpValueFloatString(pArray, lPos, &arg[0]);
    return null_terminator_copier(get_view(arg), pVal, retval);
#else
    return DTWAIN_RangeGetExpValueFloatString(pArray, lPos, pVal);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameSetValueStringW(DTWAIN_FRAME Frame, LONG nWhich, LPCWSTR Value)
{
#ifdef _UNICODE
    return DTWAIN_FrameSetValueString(Frame, nWhich, Value);
#else
    return DTWAIN_FrameSetValueString(Frame, nWhich, StringConversion::Convert_WidePtr_To_Native(Value).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameSetValueStringA(DTWAIN_FRAME Frame, LONG nWhich, LPCSTR Value)
{
#ifdef _UNICODE
    return DTWAIN_FrameSetValueString(Frame, nWhich, StringConversion::Convert_AnsiPtr_To_Native(Value).c_str());
#else
    return DTWAIN_FrameSetValueString(Frame, nWhich, Value);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameSetAllStringW(DTWAIN_FRAME Frame, LPCWSTR Left, LPCWSTR Top, LPCWSTR Right, LPCWSTR Bottom)
{
#ifdef _UNICODE
    return DTWAIN_FrameSetAllString(Frame, Left, Top, Right, Bottom);
#else
    return DTWAIN_FrameSetAllString(Frame,
                                    StringConversion::Convert_WidePtr_To_Native(Left).c_str(),
                                    StringConversion::Convert_WidePtr_To_Native(Top).c_str(),
                                    StringConversion::Convert_WidePtr_To_Native(Right).c_str(),
                                    StringConversion::Convert_WidePtr_To_Native(Bottom).c_str());
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameSetAllStringA(DTWAIN_FRAME Frame, LPCSTR Left, LPCSTR Top, LPCSTR Right, LPCSTR Bottom)
{
#ifdef _UNICODE
    return DTWAIN_FrameSetAllString(Frame,
                                    StringConversion::Convert_AnsiPtr_To_Native(Left).c_str(),
                                    StringConversion::Convert_AnsiPtr_To_Native(Top).c_str(),
                                    StringConversion::Convert_AnsiPtr_To_Native(Right).c_str(),
                                    StringConversion::Convert_AnsiPtr_To_Native(Bottom).c_str());
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
    DTWAIN_BOOL retval = DTWAIN_FrameGetValueString(Frame, nWhich, &arg[0]);
    return null_terminator_copier(get_view(arg), Value, retval);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameGetValueStringA(DTWAIN_FRAME Frame, LONG nWhich, LPSTR Value)
{
#ifdef _UNICODE
    std::wstring arg(128, 0);
    const DTWAIN_BOOL retval = DTWAIN_FrameGetValueString(Frame, nWhich, &arg[0]);
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
    DTWAIN_BOOL retVal = DTWAIN_FrameGetAllString(Frame, &args[0][0], &args[1][0], &args[2][0], &args[3][0]);
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
    const DTWAIN_BOOL retVal = DTWAIN_FrameGetAllString(Frame, &args[0][0], &args[1][0], &args[2][0], &args[3][0]);
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
    return DTWAIN_FrameCreateString(StringConversion::Convert_WidePtr_To_Native(Left).c_str(),
                                    StringConversion::Convert_WidePtr_To_Native(Top).c_str(),
                                    StringConversion::Convert_WidePtr_To_Native(Right).c_str(),
                                    StringConversion::Convert_WidePtr_To_Native(Bottom).c_str());
#endif
}

DTWAIN_FRAME DLLENTRY_DEF DTWAIN_FrameCreateStringA(LPCSTR Left, LPCSTR Top, LPCSTR Right, LPCSTR Bottom)
{
#ifdef _UNICODE
    return DTWAIN_FrameCreateString(StringConversion::Convert_AnsiPtr_To_Native(Left).c_str(),
                                    StringConversion::Convert_AnsiPtr_To_Native(Top).c_str(),
                                    StringConversion::Convert_AnsiPtr_To_Native(Right).c_str(),
                                    StringConversion::Convert_AnsiPtr_To_Native(Bottom).c_str());
#else
    return DTWAIN_FrameCreateString(Left, Top, Right, Bottom);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetFileTypeExtensionsA(LONG nType, LPSTR lpszName, LONG nMaxLen)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nMaxLen, 0L), 0);
    const LONG retVal = DTWAIN_GetFileTypeExtensions(nType, (nMaxLen > 0 && lpszName) ? &arg[0] : nullptr, nMaxLen);
    return null_terminator_copier(get_view(arg), lpszName, retVal);
#else
    return DTWAIN_GetFileTypeExtensions(nType, lpszName, nMaxLen);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetFileTypeExtensionsW(LONG nType, LPWSTR lpszName, LONG nMaxLen)
{
#ifdef _UNICODE
    return DTWAIN_GetFileTypeExtensions(nType, lpszName, nMaxLen);
#else
    std::string arg((std::max)(nMaxLen, 0L), 0);
    const LONG retVal = DTWAIN_GetFileTypeExtensions(nType, (nMaxLen > 0 && lpszName)? &arg[0] : nullptr, nMaxLen);
    return null_terminator_copier(get_view(arg), lpszName, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetFileTypeNameA(LONG nType, LPSTR lpszName, LONG nMaxLen)
{
#ifdef _UNICODE
    std::wstring arg((std::max)(nMaxLen, 0L), 0);
    const LONG retVal = DTWAIN_GetFileTypeName(nType, (nMaxLen > 0 && lpszName) ? &arg[0] : nullptr, nMaxLen);
    return null_terminator_copier(get_view(arg), lpszName, retVal);
#else
    return DTWAIN_GetFileTypeName(nType, lpszName, nMaxLen);
#endif
}


LONG DLLENTRY_DEF DTWAIN_GetFileTypeNameW(LONG nType, LPWSTR lpszName, LONG nMaxLen)
{
#ifdef _UNICODE
    return DTWAIN_GetFileTypeName(nType, lpszName, nMaxLen);
#else
    std::string arg((std::max)(nMaxLen, 0L), 0);
    const LONG retVal = DTWAIN_GetFileTypeName(nType, (nMaxLen > 0 && lpszName) ? &arg[0] : nullptr, nMaxLen);
    return null_terminator_copier(get_view(arg), lpszName, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSessionDetailsA(LPSTR lpszBuf, LONG nLength, LONG indentFactor, BOOL bRefresh)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetSessionDetails((nLength > 0 && lpszBuf) ? &args[0] : nullptr, static_cast<LONG>(args.size()), indentFactor, bRefresh);
    return null_terminator_copier(get_view(args), lpszBuf, retVal);
#else
    return DTWAIN_GetSessionDetails(lpszBuf, nLength, indentFactor, bRefresh);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSessionDetailsW(LPWSTR lpszBuf, LONG nLength, LONG indentFactor, BOOL bRefresh)
{
#ifdef _UNICODE
    return DTWAIN_GetSessionDetails(lpszBuf, nLength, indentFactor, bRefresh);
#else
    std::string args((std::max)(nLength, 0L), 0);
    LONG retVal = DTWAIN_GetSessionDetails((nLength > 0 && lpszBuf) ? &args[0] : nullptr, static_cast<LONG>(args.size()), indentFactor, bRefresh);
    return null_terminator_copier(get_view(args), lpszBuf, retVal);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSourceDetailsA(LPCSTR lpszSources, LPSTR lpszBuf, LONG nLength, LONG indentFactor, BOOL bRefresh)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetSourceDetails(StringConversion::Convert_AnsiPtr_To_Native(lpszSources).c_str(),
                                                (nLength > 0 && lpszBuf) ? &args[0] : nullptr, static_cast<LONG>(args.size()), indentFactor, bRefresh);
    return null_terminator_copier(get_view(args), lpszBuf, retVal);
#else
    return DTWAIN_GetSourceDetails(lpszSources, lpszBuf, nLength, indentFactor, bRefresh);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetSourceDetailsW(LPCWSTR lpszSources, LPWSTR lpszBuf, LONG nLength, LONG indentFactor, BOOL bRefresh)
{
#ifdef _UNICODE
    return DTWAIN_GetSourceDetails(lpszSources, lpszBuf, nLength, indentFactor, bRefresh);
#else
    std::string args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetSourceDetails(StringConversion::Convert_WidePtr_To_Ansi(lpszSources).c_str(),
                                                (nLength > 0 && lpszBuf) ? &args[0] : nullptr, static_cast<LONG>(args.size()), indentFactor, bRefresh);
    return null_terminator_copier(get_view(args), lpszBuf, retVal);
#endif
}

LONG  DLLENTRY_DEF DTWAIN_GetVersionCopyrightA(LPSTR lpszApp, LONG nLength)
{
#ifdef _UNICODE
    std::wstring args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetVersionCopyright((nLength > 0 && lpszApp) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszApp , retVal);
#else
    return DTWAIN_GetVersionCopyright(lpszApp, nLength);
#endif
}

LONG DLLENTRY_DEF DTWAIN_GetVersionCopyrightW(LPWSTR lpszApp, LONG nLength)
{
#ifdef _UNICODE
    return DTWAIN_GetVersionCopyright(lpszApp, nLength);
#else
    std::string args((std::max)(nLength, 0L), 0);
    const LONG retVal = DTWAIN_GetVersionCopyright((nLength > 0 && lpszApp) ? &args[0] : nullptr, static_cast<LONG>(args.size()));
    return null_terminator_copier(get_view(args), lpszApp, retVal);
#endif
}

DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngine2A(HWND hWndParent, LPCSTR szTitle, LONG xPos, LONG yPos, LONG nOptions)
{
#ifdef _UNICODE
    return DTWAIN_SelectOCREngine2(hWndParent, szTitle ? StringConversion::Convert_AnsiPtr_To_Native(szTitle).c_str() : nullptr, xPos, yPos, nOptions);
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
                                szTitle?StringConversion::Convert_WidePtr_To_Native(szTitle).c_str():NULL, xPos, yPos, nOptions);
#endif
}

DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngine2ExA(HWND hWndParent, LPCSTR szTitle, LONG xPos, LONG yPos,
    LPCSTR szIncludeNames, LPCSTR szExcludeNames, LPCSTR szNameMapping, LONG nOptions)
{
#ifdef _UNICODE
    return DTWAIN_SelectOCREngine2Ex(hWndParent,
        szTitle ? StringConversion::Convert_AnsiPtr_To_Native(szTitle).c_str() : nullptr, xPos, yPos,
        szIncludeNames ? StringConversion::Convert_AnsiPtr_To_Native(szIncludeNames).c_str() : nullptr,
        szExcludeNames ? StringConversion::Convert_AnsiPtr_To_Native(szExcludeNames).c_str() : nullptr,
        szNameMapping ? StringConversion::Convert_AnsiPtr_To_Native(szNameMapping).c_str() : nullptr,
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
        szTitle ? StringConversion::Convert_WidePtr_To_Native(szTitle).c_str() : NULL,
        xPos, yPos,
        szIncludeNames ? StringConversion::Convert_WidePtr_To_Native(szIncludeNames).c_str() : NULL,
        szExcludeNames ? StringConversion::Convert_WidePtr_To_Native(szExcludeNames).c_str() : NULL,
        szNameMapping ? StringConversion::Convert_WidePtr_To_Native(szNameMapping).c_str() : NULL,
        nOptions);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTempFileDirectoryExW(LPCWSTR sLangDLL, DTWAIN_BOOL bClear)
{
#ifdef _UNICODE
    return DTWAIN_SetTempFileDirectoryEx(sLangDLL, bClear);
#else
    return DTWAIN_SetTempFileDirectoryEx(StringConversion::Convert_WidePtr_To_Native(sLangDLL).c_str(), bClear);
#endif
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTempFileDirectoryExA(LPCSTR sLangDLL, DTWAIN_BOOL bClear)
{
#ifdef _UNICODE
    return DTWAIN_SetTempFileDirectoryEx(StringConversion::Convert_AnsiPtr_To_Native(sLangDLL).c_str(), bClear);
#else
    return DTWAIN_SetTempFileDirectoryEx(sLangDLL, bClear);
#endif
}


//LONG DLLENTRY_DEF DTWAIN_ConvertToAPIStringExA(LPCSTR lpOrigString, LPSTR lpOutString, LONG nSize);
//LONG DLLENTRY_DEF DTWAIN_ConvertToAPIStringExW(LPCWSTR lpOrigString, LPWSTR lpOutString, LONG nSize);

LONG DLLENTRY_DEF DTWAIN_ConvertToAPIStringExA(LPCSTR lpOrigString, LPSTR lpOutString, LONG nSize)
{
#ifdef _UNICODE
    std::wstring arg(nSize + 1, 0);
    const DTWAIN_BOOL retVal = DTWAIN_ConvertToAPIStringEx(StringConversion::Convert_AnsiPtr_To_Native(lpOrigString).c_str(),
                                                          lpOutString?&arg[0]:nullptr, nSize);
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
    DTWAIN_BOOL retVal = DTWAIN_ConvertToAPIStringEx(StringConversion::Convert_WidePtr_To_Native(lpOrigString).c_str(),
                                                    lpOutString?&arg[0]:nullptr, nSize);
    return null_terminator_copier(get_view(arg), lpOutString, retVal);
#endif
}

#endif // CTLSTRIMPL_INL
