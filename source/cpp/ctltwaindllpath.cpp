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
#include "ctlwindowsimpl.h"
#include "ctlfileutils.h"
#include "ctltwaindllpath.h"
#include "ctlstaticdata.h"
#include "ctlinternalconstants.h"
#include "ctlverinfoutils.h"

using namespace dynarithmic; 

namespace dynarithmic
{
    CTL_StringType GetDTWAINExecutionPath()
    {
        // Get the total path name.
        GetDTWAINDLLPath();

        // Return the parent directory of the executable
        return CTL_StaticData::GetDLLParentPath();
    }

    // Gets the entire path, including file name of the loaded DTWAIN DLL.
    CTL_StringType GetDTWAINDLLPath()
    {
        auto& dllPath = CTL_StaticData::GetDLLPath();
        if (!dllPath.empty())
            return dllPath;
        dllPath = WindowsAPIImplDef::GetModuleFileName(CTL_StaticData::GetDLLInstanceHandle());

        // Also remember the parent path.
        auto& dllParentPath = CTL_StaticData::GetDLLParentPath();
        dllParentPath = fileutils::get_parent_directory(dllPath.c_str(), false);
        return dllPath;
    }

    void DTWAIN_InternalThrowException() THIS_FUNCTION_THROWS
    {
        throw;
    }

    CTL_StringType GetDTWAININIPath()
    {
        CTL_StringType szName = DTWAIN_ININAME_NATIVE;
        return CreateResourcePathName() + szName;
    }

    std::string GetDTWAININIPathA()
    {
        return stringconversion::Convert_Native_To_Ansi(GetDTWAININIPath());
    }
}

