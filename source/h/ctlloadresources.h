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
#ifndef CTLLOADRESOURCES_H
#define CTLLOADRESOURCES_H
#include <vector>
#include <string>
#include <string_view>
#include <array>
#include <boost/container/flat_map.hpp>

#define DTWAINLANGRESOURCEFILE _T("twainresourcestrings_")
#define DTWAINRESOURCEINFOFILE _T("twaininfo.txt")
#define DTWAINLANGRESOURCENAMESFILE  _T("twainlanguage.txt")
#define DTWAINCUSTOMRESOURCESFILE _T("twaincustomresources_")

namespace dynarithmic
{
    typedef boost::container::flat_map<std::string, bool> CTL_ResourceRegistryMap;

    bool LoadLanguageResourceA(const char* lpszName, const CTL_ResourceRegistryMap& registryMap, bool bClear);
    bool LoadLanguageResourceA(const char* lpszName, bool bClear);
    bool LoadLanguageResourceA(std::string_view lpszName, const CTL_ResourceRegistryMap& registryMap, bool bClear);
    bool LoadLanguageResourceA(std::string_view lpszName, bool bClear);
    bool LoadLanguageResourceFromRC();

    struct ResourceLoadingInfo
    {
        enum {DTWAIN_RESLOAD_INFOFILE_LOADED,
              DTWAIN_RESLOAD_INIFILE_LOADED,
              DTWAIN_RESLOAD_INFOFILE_VERSION_READ,
              DTWAIN_RESLOAD_CRC_CHECK,
              DTWAIN_RESLOAD_NODUPLICATE_ID,
              DTWAIN_RESLOAD_EXCEPTION_OK};
        std::array<bool, 6> errorValue;
        CTL_StringType errorMessage;
        CTL_StringType resourcePath;
        struct DuplicateInfo
        {
            int lineNumber = -1;
            int64_t duplicateID = -1;
            std::string line;
        };
        DuplicateInfo m_dupInfo;
        ResourceLoadingInfo() : errorValue{} { errorValue[2] = true; }
    };

    bool LoadTwainResources(ResourceLoadingInfo& ret);
    void UnloadStringResources();
    void UnloadErrorResources();
    std::vector<std::string> GetLangResourceNames();
    std::string GetResourceFileNameA(LPCSTR lpszName, LPCTSTR szPrefix);
    std::string GetCustomResourceFileNameA(LPCSTR lpszName);
    std::string& GetResourceStringFromMap(LONG resourceNum);
    CTL_StringType GetResourceStringFromMap_Native(LONG resourceNum);
}
#endif
