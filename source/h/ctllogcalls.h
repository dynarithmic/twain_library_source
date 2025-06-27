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
#ifndef CTLLOGCALLS_H
#define CTLLOGCALLS_H

namespace dynarithmic
{
    long& GetLogFilterFlags();
    std::string CTL_LogFunctionCallHelper(const char* pFuncName, int nWhich, const char* pOptionalString = nullptr);
    std::string CTL_LogFunctionCallA(int32_t logFlags, const char *pFuncName, int nWhich, const char* pOptionalString = nullptr);
}
#endif
