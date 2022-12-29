/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2022 Dynarithmic Software.

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

#include "ctlfileutils.h"
#include <dtwain_filesystem.h>

namespace dynarithmic
{
    bool delete_file(LPCTSTR filename)
    {
        try
        {
            filesys::remove(filename);
        }
        catch (filesys::filesystem_error &)
        {
            return false;
        }
        return true;
    }

    CTL_StringType get_parent_directory(LPCTSTR filename)
    {
        auto p = filesys::path(filename);
        const auto p2 = p.remove_filename();
#ifdef UNICODE
        auto str = p2.wstring();
#else
        auto str = p2.string();
#endif
        str = StringWrapper::AddBackslashToDirectory(str);
        return str;
    }

    std::pair<bool, std::string> parent_directory_exists(LPCTSTR filename)
    {
        try
        {
            const auto str = get_parent_directory(filename);
            if (str.empty())
                return { true, "" };
            if (filesys::exists(str))
                return {true, ""};
            return {false,""};
        }
        catch (filesys::filesystem_error& e)
        {
            return {false, e.what()};
        }
        return {true,""};
    }

    bool file_exists(LPCTSTR filename)
    {
        return filesys::exists(filename);
    }

    CTL_StringType temp_directory_path(bool bWithSeparator)
    {
        auto retVal = filesys::temp_directory_path().string();
        if ( bWithSeparator && !retVal.empty() )
        {
            CTL_StringType tempStr(retVal.begin(), retVal.end());
            tempStr = StringWrapper::AddBackslashToDirectory(tempStr);
            return tempStr;
        }
        return {retVal.begin(), retVal.end()};
    }

    std::pair<bool, std::string> create_directory(LPCTSTR directory)
    {
        bool directory_created = false;
        try
        {
            directory_created = filesys::create_directories(directory);
        }
        catch (std::exception& e)
        {
            return {false, e.what()};
        }
        return{ directory_created, "" };
    }

    bool directory_writeable(LPCTSTR filename)
    {
        auto parentDir = get_parent_directory(filename);
        auto guidName = parentDir + StringWrapper::GetGUID();
        std::ofstream testStream(StringConversion::Convert_Native_To_Ansi(guidName));
        if ( testStream.is_open())
        { 
            testStream.close();
            delete_file(guidName.c_str());
            return true;
        }
        return false;
    }
}
