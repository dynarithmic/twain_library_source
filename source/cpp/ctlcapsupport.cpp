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
#include "ctltwainmanager.h"
#include "errorcheck.h"
#include "ctltmpl5.h"
#include "ctlsetgetcaps.h"

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test a capability to see what data type and what container types produce a successful MSG_GET operation
DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_TestGetCap(DTWAIN_SOURCE Source, LONG lCapability)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCapability))

    static constexpr LONG DataTypeArray[] = {
                                            TWTY_INT16,
                                            TWTY_UINT16,
                                            TWTY_INT32,
                                            TWTY_UINT32,
                                            TWTY_BOOL,
                                            TWTY_FIX32,
                                            TWTY_INT8,
                                            TWTY_UINT8,
                                            TWTY_STR32,
                                            TWTY_STR64,
                                            TWTY_STR255,
                                            TWTY_STR128,
                                            TWTY_STR1024,
                                            TWTY_UNI512,
                                            TWTY_FRAME
                                        };

    static constexpr LONG ContainerTypeArray[] = {
                                            DTWAIN_CONTENUMERATION,
                                            DTWAIN_CONTARRAY,
                                            DTWAIN_CONTRANGE,
                                            DTWAIN_CONTONEVALUE
                                        };

    static constexpr size_t DataTypeArraySize = std::size(DataTypeArray);
    static constexpr size_t ContainerArraySize = std::size(ContainerTypeArray);
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);

    DTWAIN_ARRAY outputArray = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYLONG, 0);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return outputArray == nullptr; }, DTWAIN_ERR_OUT_OF_MEMORY, nullptr, FUNC_MACRO);

    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(outputArray);

    // Loop and test MSG_GET for the capability.
    for (size_t i = 0; i < DataTypeArraySize; ++i)
    {
        for (size_t j = 0; j < ContainerArraySize; ++j)
        {
            DTWAIN_ARRAY testArray = {};
            DTWAINArrayPtr_RAII raii(pHandle, &testArray);
            bool ok = GetCapValuesEx2_Internal(pSource, lCapability, DTWAIN_CAPGET, ContainerTypeArray[j], DataTypeArray[i], &testArray);
            if (ok)
            {
                LONG statusValue = (LONG)DataTypeArray[i] << 16 | ContainerTypeArray[j];
                vValues.push_back(statusValue);
                DumpArrayContents(testArray, lCapability);
            }
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(outputArray)
    CATCH_BLOCK_LOG_PARAMS(nullptr)
}

////////////////////////////////////////////////////////////////////////////////
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsCapSupported(DTWAIN_SOURCE Source, LONG lCapability)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCapability))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);

    // Test if the capability is supported
    bool bInList = pSource->IsCapInSupportedList(static_cast<TW_UINT16>(lCapability));
    DTWAIN_Check_Error_Condition_2_Ex(pHandle, [&] {return !bInList; }, DTWAIN_ERR_CAP_NO_SUPPORT, false, FUNC_MACRO, false);
    LOG_FUNC_EXIT_NONAME_PARAMS(bInList)
    CATCH_BLOCK_LOG_PARAMS(false)
}
