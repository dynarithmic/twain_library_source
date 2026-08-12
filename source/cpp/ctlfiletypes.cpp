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
#include "cppfunc.h"
#include "ctlstringutilsx.h"
#include "ctldtwainhandle.h"
#include "ctlstaticdata.h"
#include "ctltwaindllhandle.h"
#include "ctlarray.h"
using namespace dynarithmic;

namespace
{
    DTWAIN_ARRAY GetFileTypes(CTL_TwainDLLHandle* pHandle, int nType)
    {
        constexpr std::array<std::string_view, 3> sNames = { "","-Single","-Multi" };
        DTWAIN_ARRAY aFileTypes = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYLONG, 0).second;
        if (aFileTypes)
        {
            auto& availableFileTypes = CTL_StaticData::GetAvailableFileFormatsMap();
            auto& factory = pHandle->m_ArrayFactory;
            for (auto& pr : availableFileTypes)
            {
                auto val = pr.first;
                if (basicstringutils::EndsWith(std::string_view(pr.second.m_formatName),
                    sNames[nType]))
                    factory->add_to_back(aFileTypes, &val, 1);
            }
        }
        return aFileTypes;
    }

    std::string GetFileTypeExtensionsInternal(int nType)
    {
        const auto& availableFileTypes = CTL_StaticData::GetAvailableFileFormatsMap();
        const auto iter = availableFileTypes.find(nType);
        if (iter != availableFileTypes.end())
            return basicstringutils::Join<std::string>(iter->second.m_vExtensions, "|");
        return {};
    }

    std::string GetFileTypeNameInternal(int nType)
    {
        const auto& availableFileTypes = CTL_StaticData::GetAvailableFileFormatsMap();
        const auto iter = availableFileTypes.find(nType);
        if (iter != availableFileTypes.end())
            return iter->second.m_formatName;
        return{};
    }

    template <typename Fn>
    LONG GetFileTypeInfo(Fn infoFn, int nType, LPTSTR lpszName, LONG nMaxLen)
    {
        LONG realLen = -1;
        std::string str = infoFn(nType);
        if (!str.empty())
        {
            const CTL_StringType str2 = stringconversion::Convert_Ansi_To_Native(str, str.size());
            realLen = CopyInfoToCString(str2, lpszName, nMaxLen);
        }
        return realLen;
    }
}
extern "C"
{
    LONG DLLENTRY_DEF DTWAIN_GetFileTypeName(LONG nType, LPTSTR lpszName, LONG nMaxLen)
    {
        LOG_FUNC_ENTRY_PARAMS((nType, lpszName, nMaxLen))
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszName))
        LOG_FUNC_EXIT_NONAME_PARAMS(GetFileTypeInfo(&GetFileTypeNameInternal, nType, lpszName, nMaxLen))
        CATCH_BLOCK(-1)
    }

    LONG DLLENTRY_DEF DTWAIN_GetFileTypeExtensions(LONG nType, LPTSTR lpszName, LONG nMaxLen)
    {
        LOG_FUNC_ENTRY_PARAMS((nType, lpszName, nMaxLen))
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszName))
        LOG_FUNC_EXIT_NONAME_PARAMS(GetFileTypeInfo(&GetFileTypeExtensionsInternal, nType, lpszName, nMaxLen))
        CATCH_BLOCK(-1)
    }

    DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumSupportedFileTypes(VOID_PROTOTYPE)
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        LOG_FUNC_EXIT_NONAME_PARAMS(GetFileTypes(pHandle, 0))
        CATCH_BLOCK(nullptr)
    }

    DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumSupportedMultiPageFileTypes(VOID_PROTOTYPE)
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        LOG_FUNC_EXIT_NONAME_PARAMS(GetFileTypes(pHandle, 2))
        CATCH_BLOCK(nullptr)
    }

    DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumSupportedSinglePageFileTypes(VOID_PROTOTYPE)
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        LOG_FUNC_EXIT_NONAME_PARAMS(GetFileTypes(pHandle, 1))
        CATCH_BLOCK(nullptr)
    }
}