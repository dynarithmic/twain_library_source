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
#include "cppfunc.h"
#include "ctlarraydumper.h"
#include "twainframe.h"
#include "ctltwainsource.h"
#include "ctltwainmanager.h"
#include "ctltwainlogging.h"

namespace stringutils = dynarithmic::basicstringutils;
using namespace dynarithmic;

namespace
{
    template <typename T>
    struct StreamerImpl
    {
        static void streamMe(OutputBaseStreamA* strm, size_t* pCur, T& val, bool makeUnsigned)
        {
            using D = std::decay_t<T>;

            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
            {
                if (makeUnsigned)
                {
                    using U = std::make_unsigned_t<D>;
                    *strm << "Array[" << *pCur << "] = " << static_cast<U>(val) << "\n";
                }
                else
                {
                    *strm << "Array[" << *pCur << "] = " << val << "\n";
                }
            }
            else
            {
                *strm << "Array[" << *pCur << "] = " << val << "\n";
            }
        }
    };

    struct StreamerImplFrame
    {
        CTL_OutputBaseStreamType* m_pStrm;
        size_t* m_pCurItem;
        StreamerImplFrame(CTL_OutputBaseStreamType* strm, size_t* curItem) : m_pStrm(strm), m_pCurItem(curItem) { *curItem = 0; }

        void operator()(const TwainFrameInternal& pPtr) const
        {
            auto frameComponent = pPtr.GetFrameComponent();
            *m_pStrm << _T("Array[") << *m_pCurItem << _T("]");
            *m_pStrm << _T("{left=" << frameComponent[TwainFrameInternal::FRAMELEFT] << _T("}\n"));
            *m_pStrm << _T("{top=" <<  frameComponent[TwainFrameInternal::FRAMETOP] << _T("}\n"));
            *m_pStrm << _T("{right=" << frameComponent[TwainFrameInternal::FRAMERIGHT] << _T("}\n"));
            *m_pStrm << _T("{bottom=" << frameComponent[TwainFrameInternal::FRAMEBOTTOM] << _T("}\n"));
            ++* m_pCurItem;
        }
    };

    struct StreamerImplTwainSource
    {
        CTL_OutputBaseStreamType* m_pStrm;
        size_t* m_pCurItem;
        StreamerImplTwainSource(CTL_OutputBaseStreamType* strm, size_t* curItem) : m_pStrm(strm), m_pCurItem(curItem) { *curItem = 0; }

        void operator()(CTL_ITwainSource* pPtr) const
        {
            if (pPtr)
                *m_pStrm << _T("Source ") << *m_pCurItem + 1 << _T(": ") <<
                stringconversion::Convert_Ansi_To_Native(pPtr->GetTwainIdentity().get_product_name()) << "\n";
            ++* m_pCurItem;
        }
    };

    template <typename T, typename StreamFn = StreamerImpl<T> >
    struct oStreamer
    {

        OutputBaseStreamA* m_pStrm;
        size_t* m_pCurItem;
        bool m_bMakeUnsigned;
        oStreamer(OutputBaseStreamA* strm, size_t* curItem, bool makeUnsigned = false) :
            m_pStrm(strm), m_pCurItem(curItem), m_bMakeUnsigned(makeUnsigned) {
            *curItem = 0;
        }
        void operator()(T& n)
        {
            StreamFn::streamMe(m_pStrm, m_pCurItem, n, m_bMakeUnsigned);
            ++* m_pCurItem;
        }
    };

    template <typename T>
    void genericDumper(DTWAIN_ARRAY Array, bool makeUnsigned = false)
    {
        // Get the array contents as a vector
        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
        auto& vCaps = pHandle->m_ArrayFactory->underlying_container_t<typename T::value_type>(Array);

        StringStreamA strm;
        size_t n;

        std::for_each(vCaps.begin(), vCaps.end(), oStreamer<typename T::value_type>(&strm, &n, makeUnsigned));
        LogWriterUtils::WriteMultiLineInfoIndentedA(strm.str(), "\n");
    }

    template <typename IntType>
    void CapDumper(DTWAIN_ARRAY Array)
    {
        // Get the array contents as a vector
        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
        const auto& vCaps = pHandle->m_ArrayFactory->underlying_container_t<IntType>(Array);

        StringStreamA strm;
        size_t n;
        strm << "\n";

        // if the cap is for supported caps, then output the strings.
        // vector of names
        std::vector<std::string> CapNames;

        // get the vector of cap names given cap number
        std::transform(vCaps.begin(), vCaps.end(), std::back_inserter(CapNames),
            [](IntType n) {return CTL_TwainAppMgr::GetCapNameFromCap(n); });

        // stream the cap information from the cap names
        std::for_each(CapNames.begin(), CapNames.end(), oStreamer<std::string>(&strm, &n));
        LogWriterUtils::WriteMultiLineInfoIndentedA(strm.str(), "\n");
    }
    
    void DumpArrayULONG(DTWAIN_ARRAY Array)
    {
        genericDumper<CTL_ArrayFactory::tagged_array_long>(Array, true);
    }

    void DumpArrayLONG(DTWAIN_ARRAY Array, LONG lCap, bool bAsUnsigned)
    {
        if (lCap != CAP_SUPPORTEDCAPS)
            genericDumper<CTL_ArrayFactory::tagged_array_long>(Array, bAsUnsigned);
        else
            CapDumper<LONG>(Array);
    }

    void DumpArrayFLOAT(DTWAIN_ARRAY Array)
    {
        genericDumper<CTL_ArrayFactory::tagged_array_double>(Array);
    }

    void DumpArrayAcquisitions(DTWAIN_ARRAY Array)
    {
        genericDumper<CTL_ArrayFactory::tagged_array_tagged_array_voidptr>(Array);
    }

    void DumpArrayHandles(DTWAIN_ARRAY Array)
    {
        genericDumper<CTL_ArrayFactory::tagged_array_voidptr>(Array);
    }

    void DumpArrayLONG64(DTWAIN_ARRAY Array)
    {
        genericDumper<CTL_ArrayFactory::tagged_array_long64>(Array);
    }

    void DumpArrayWideString(DTWAIN_ARRAY Array);
    void DumpArrayAnsiString(DTWAIN_ARRAY Array);

    void DumpArrayNativeString(DTWAIN_ARRAY Array)
    {
#ifdef _UNICODE
        DumpArrayWideString(Array);
#else
        DumpArrayAnsiString(Array);
#endif
    }

    template <typename StringType, typename WriterFn, typename StringViewType>
    void GenericDumpArrayString(DTWAIN_ARRAY Array, WriterFn fn)
    {
        static constexpr auto newLine = CharTraits<typename StringType::value_type>::NewLineString();
        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
        const auto& vData = pHandle->m_ArrayFactory->underlying_container_t<StringType>(Array);
        StringType allValues = stringutils::Join<StringType>(vData, newLine);
        fn(allValues, newLine);
    }

    void DumpArrayWideString(DTWAIN_ARRAY Array)
    {
        GenericDumpArrayString<CTL_StringTypeW, decltype(LogWriterUtils::WriteMultiLineInfoIndentedW), std::wstring_view>
            (Array, &LogWriterUtils::WriteMultiLineInfoIndentedW);
    }

    void DumpArrayAnsiString(DTWAIN_ARRAY Array)
    {
        GenericDumpArrayString<CTL_StringTypeA, decltype(LogWriterUtils::WriteMultiLineInfoIndentedA), std::string_view>
            (Array, &LogWriterUtils::WriteMultiLineInfoIndentedA);
    }

    void DumpArrayFrame(DTWAIN_ARRAY Array)
    {
        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
        const auto& vData = pHandle->m_ArrayFactory->underlying_container_t<TwainFrameInternal>(Array);
        size_t n;
        CTL_StringStreamType strm;
        std::for_each(vData.begin(), vData.end(), StreamerImplFrame(&strm, &n));
        LogWriterUtils::WriteMultiLineInfoIndented(strm.str(), _T("\n"));
    }

    void DumpSourceNames(DTWAIN_ARRAY Array)
    {
        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
        const auto& vData = pHandle->m_ArrayFactory->underlying_container_t<CTL_ITwainSource*>(Array);
        size_t n;
        CTL_StringStreamType strm;
        std::for_each(vData.begin(), vData.end(), StreamerImplTwainSource(&strm, &n));
        LogWriterUtils::WriteMultiLineInfoIndented(strm.str(), _T("\n"));
    }
}

namespace dynarithmic
{
    void DumpArrayContents(DTWAIN_ARRAY Array, LONG lCap, bool anyLogFlags, bool bAsUnsigned)
    {
        auto logFlags = CTL_StaticData::GetLogFilterFlags();
        bool doArrayDump = ((logFlags && anyLogFlags) || (logFlags & DTWAIN_LOG_MISCELLANEOUS));
        if (!doArrayDump)
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
            if (nCount < 0)
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
                DumpArrayLONG(Array, lCap, bAsUnsigned);
                break;

            case DTWAIN_ARRAYUINT32:
                DumpArrayULONG(Array);
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

            case DTWAIN_ARRAYLONG64:
                DumpArrayLONG64(Array);
                break;

            case DTWAIN_ARRAYSOURCE:
                DumpSourceNames(Array);
                break;

            case DTWAIN_ARRAYOFHANDLEARRAYS:
                DumpArrayAcquisitions(Array);
                break;

            case DTWAIN_ARRAYHANDLE:
                DumpArrayHandles(Array);
                break;
        }
    }
}