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

    bool parent_directory_exists(LPCTSTR filename)
    {
        try
        {
            auto p = filesys::path(filename);
            const auto p2 = p.remove_filename();
            auto native_str = p2.native();
            CTL_StringType str(native_str.begin(), native_str.end());
            str = StringWrapper::AddBackslashToDirectory(str);
            if (filesys::exists(str))
                return true;
        }
        catch (filesys::filesystem_error&)
        {
        }
        return false;
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
            if (!tempStr.empty() && *tempStr.rbegin() != filesys::path::preferred_separator)
                tempStr += filesys::path::preferred_separator;
            return tempStr;
        }
        return {retVal.begin(), retVal.end()};
    }
}
