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
#include <functional>
#include <algorithm>
#include "cppfunc.h"
#include "errorcheck.h"
using namespace dynarithmic;

template <typename T>
struct StreamerImpl
{
    static void streamMe(OutputBaseStreamA* strm, size_t* pCur, T& val)
    {
        *strm << "Array[" << *pCur << "] = " << val << "\n";
    }
};

struct StreamerImplFrame
{
    CTL_OutputBaseStreamType* m_pStrm;
    size_t* m_pCurItem;
    StreamerImplFrame(CTL_OutputBaseStreamType* strm, size_t* curItem) : m_pStrm(strm), m_pCurItem(curItem) { *curItem = 0; }

    void operator()(const TwainFrameInternal& pPtr) const
    {
        *m_pStrm << _T("Array[") << *m_pCurItem << _T("]");
        *m_pStrm << _T("{left=" <<   pPtr.m_FrameComponent[0] << _T("}\n"));
        *m_pStrm << _T("{top=" <<    pPtr.m_FrameComponent[1] << _T("}\n"));
        *m_pStrm << _T("{right=" <<  pPtr.m_FrameComponent[2] << _T("}\n"));
        *m_pStrm << _T("{bottom=" << pPtr.m_FrameComponent[3] << _T("}\n"));
        ++*m_pCurItem;
    }
};

struct StreamerImplNativeString
{
    CTL_OutputBaseStreamType* m_pStrm;
    size_t* m_pCurItem;
    StreamerImplNativeString(CTL_OutputBaseStreamType* strm, size_t* curItem) : m_pStrm(strm), m_pCurItem(curItem) { *curItem = 0; }

    void operator()(CTL_StringType& pPtr) const
    {
        *m_pStrm << pPtr << _T("\n");
        ++*m_pCurItem;
    }
};

template <typename T, typename StreamFn = StreamerImpl<T> >
struct oStreamer
{

    OutputBaseStreamA* m_pStrm;
    size_t* m_pCurItem;
    oStreamer(OutputBaseStreamA* strm, size_t* curItem) : m_pStrm(strm), m_pCurItem(curItem) { *curItem = 0; }
    void operator()(T& n)
    {
        StreamFn::streamMe(m_pStrm, m_pCurItem, n);
        ++*m_pCurItem;
    }
};

template <typename T>
static void genericDumper(DTWAIN_ARRAY Array)
{
    // Get the array contents as a vector
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    auto& vCaps = pHandle->m_ArrayFactory->underlying_container_t<typename T::value_type>(Array);

    StringStreamA strm;
    size_t n;

    std::for_each(vCaps.begin(), vCaps.end(), oStreamer<typename T::value_type>(&strm, &n));
    LogWriterUtils::WriteMultiLineInfoIndentedA(strm.str(), "\n");
}

static void DumpArrayLONG(DTWAIN_ARRAY Array, LONG lCap)
{
    if ( lCap != CAP_SUPPORTEDCAPS )
        genericDumper<CTL_ArrayFactory::tagged_array_long>(Array);

    else
    {
        // Get the array contents as a vector
        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
        const auto& vCaps = pHandle->m_ArrayFactory->underlying_container_t<LONG>(Array);

        StringStreamA strm;
        size_t n;
        strm << "\n";

        // if the cap is for supported caps, then output the strings.
        // vector of names
        std::vector<std::string> CapNames;

        // get the vector of cap names given cap number
        std::transform(vCaps.begin(), vCaps.end(), std::back_inserter(CapNames),
                        [] (LONG n) {return CTL_TwainAppMgr::GetCapNameFromCap(n);});

        // stream the cap information from the cap names
        std::for_each(CapNames.begin(), CapNames.end(), oStreamer<std::string>(&strm, &n));
        LogWriterUtils::WriteMultiLineInfoIndentedA(strm.str(), "\n");
    }
}

static void DumpArrayFLOAT(DTWAIN_ARRAY Array)
{
    genericDumper<CTL_ArrayFactory::tagged_array_double>(Array);
}
static void DumpArrayWideString(DTWAIN_ARRAY Array);
static void DumpArrayAnsiString(DTWAIN_ARRAY Array);

static void DumpArrayNativeString(DTWAIN_ARRAY Array)
{
#ifdef _UNICODE
    DumpArrayWideString(Array);
#else
    DumpArrayAnsiString(Array);
#endif
}

template <typename StringWrapperType, typename WriterFn>
static void GenericDumpArrayString(DTWAIN_ARRAY Array, WriterFn fn)
{
    using string_type = StringWrapperType::traits_type::string_type;
    static constexpr auto newLine = StringWrapperType::traits_type::GetNewLineString();
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    const auto& vData = 
        pHandle->m_ArrayFactory->underlying_container_t<string_type>(Array);
    string_type allValues = StringWrapperType::Join(vData.begin(), vData.end(), newLine);
    fn(allValues, newLine);
}

static void DumpArrayWideString(DTWAIN_ARRAY Array)
{
    GenericDumpArrayString<StringWrapperW, decltype(LogWriterUtils::WriteMultiLineInfoIndentedW)>
        (Array, &LogWriterUtils::WriteMultiLineInfoIndentedW);
}

static void DumpArrayAnsiString(DTWAIN_ARRAY Array)
{
    GenericDumpArrayString<StringWrapperA, decltype(LogWriterUtils::WriteMultiLineInfoIndentedA)>
        (Array, &LogWriterUtils::WriteMultiLineInfoIndentedA);
}

static void DumpArrayFrame(DTWAIN_ARRAY Array)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    const auto& vData = pHandle->m_ArrayFactory->underlying_container_t<TwainFrameInternal>(Array);
    size_t n;
    CTL_StringStreamType strm;
    std::for_each(vData.begin(), vData.end(), StreamerImplFrame(&strm, &n));
    LogWriterUtils::WriteMultiLineInfoIndented(strm.str(), _T("\n"));
}

void dynarithmic::DumpArrayContents(DTWAIN_ARRAY Array, LONG lCap)
{
    if ( !(CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS ))
        return;

    std::string szBuf;
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    // This dumps contents of array to log file
    {
        // Turn off the error logging flags temporarily
        {
            DTWAINScopedLogController sLogger(0);
            if (!Array)
            {
                szBuf = "DTWAIN_ARRAY is NULL\n";
                // Turn on the error logging flags
                LogWriterUtils::WriteLogInfoIndentedA(szBuf);
                return;
            }
        }

        const LONG nCount = static_cast<LONG>(pHandle->m_ArrayFactory->size(Array));
        StringStreamA strm;
        if ( nCount < 0 )
        {
            strm << "Could not dump contents of DTWAIN_ARRAY " << Array << "\nNumber of elements: " << nCount;
            return;
        }
        strm << "Dumping contents of DTWAIN_ARRAY " << Array << "   : Number of elements: " << nCount;
        szBuf = strm.str();
    }

    LogWriterUtils::WriteLogInfoIndentedA(szBuf);

    // determine the type
    const LONG nType = CTL_ArrayFactory::tagtype_to_arraytype(pHandle->m_ArrayFactory->tag_type(Array));
    switch (nType)
    {
        case DTWAIN_ARRAYLONG:
            DumpArrayLONG(Array, lCap);
            break;

        case DTWAIN_ARRAYFLOAT:
            DumpArrayFLOAT(Array);
            break;

        case DTWAIN_ARRAYSTRING:
            DumpArrayNativeString(Array);
            break;

        case DTWAIN_ARRAYWIDESTRING:
            DumpArrayWideString(Array);
            break;

        case DTWAIN_ARRAYANSISTRING:
            DumpArrayAnsiString(Array);
            break;

        case DTWAIN_ARRAYFRAME:
            DumpArrayFrame(Array);
            break;
    }
}
