/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2024 Dynarithmic Software.

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
#ifndef CTLRES_H
#define CTLRES_H
#include <vector>
#include <string>
#include <array>

#define DTWAINLANGRESOURCEFILE _T("twainresourcestrings_")
#define DTWAINRESOURCEINFOFILE _T("twaininfo.txt")
#define DTWAINLANGRESOURCENAMESFILE  _T("twainlanguage.txt")
#define DTWAINCUSTOMRESOURCESFILE _T("twaincustomresources_")

namespace dynarithmic
{
    typedef std::unordered_map<std::string, bool> CTL_ResourceRegistryMap;

    bool LoadLanguageResourceA(LPCSTR lpszName, const CTL_ResourceRegistryMap& registryMap);
    bool LoadLanguageResourceA(LPCSTR lpszName);
    bool LoadLanguageResourceA(const std::string& lpszName, const CTL_ResourceRegistryMap& registryMap);
    bool LoadLanguageResourceA(const std::string& lpszName);

    struct ResourceLoadingInfo
    {
        std::array<bool, 3> errorValue;
        CTL_StringType errorMessage;
        ResourceLoadingInfo() : errorValue{} { errorValue[2] = true; }
    };

    bool LoadTwainResources(ResourceLoadingInfo& ret);
    void UnloadStringResources();
    void UnloadErrorResources();
    std::vector<std::string> GetLangResourceNames();
    std::string GetResourceFileNameA(LPCSTR lpszName, LPCTSTR szPrefix);
    std::string GetCustomResourceFileNameA(LPCSTR lpszName);
    std::string GetResourceStringFromMap(LONG resourceNum);
}
#endif
