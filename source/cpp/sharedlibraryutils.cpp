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

#include <windows.h>
#include "sharedlibraryutils.h"

namespace dynarithmic
{
    bool DynamicLibraryLoader::Load(const std::vector<std::string>& dllNames, const std::wstring& directory)
    {
        Unload();

        m_lastError = ERROR_SUCCESS;
        m_failedName.clear();

        for (const auto& name : dllNames)
        {
            boost::dll::shared_library library;

            fs::path path;

            if (directory.empty())
                path = fs::path(name);
            else
                path = fs::path(directory) / fs::path(name);

            auto ec = LoadSharedLibrary(library, path.wstring());
            if (ec)
            {
                m_lastError = ec;
                m_failedName = name;
                Unload();
                return false;
            }

            m_libraries.emplace_back(name, std::move(library));
        }

        m_loaded = !m_libraries.empty();
        return m_loaded;
    }
}