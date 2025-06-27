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
#include "cppfunc.h"
#include "ctltwainmanager.h"
#include "errorcheck.h"
#include "arrayfactory.h"
#include "ctlsetgetcaps.h"

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

////////////////////////////////////////////////////////////////////////////////////////////
// Device notifications
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDeviceNotifications(DTWAIN_SOURCE Source, LONG DeviceEvents)
{
    LOG_FUNC_ENTRY_PARAMS((Source, DeviceEvents))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    auto pTheSource = pSource;

    // See if Source supports the DEVICEEVENTS capability
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !CTL_TwainAppMgr::IsCapabilitySupported(pTheSource, CAP_DEVICEEVENT); },
                                      DTWAIN_ERR_DEVICEEVENT_NOT_SUPPORTED, false, FUNC_MACRO);

    // Set the notifications
    DTWAIN_ARRAY Array = nullptr;
    DTWAINArrayPtr_RAII a(pHandle, &Array);

    LONG SetType = DTWAIN_CAPSET;
    if (!DeviceEvents)
        SetType = DTWAIN_CAPRESET;
    else
    {
        LONG nBits = 0;
        LONG i;
        for (i = 0; i < 32; i++)
        {
            if (DeviceEvents & 1L << i)
                nBits++;
        }
        if (nBits == 0)
            LOG_FUNC_EXIT_NONAME_PARAMS(false)

        Array = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYLONG, nBits);

        if (!Array)
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
        auto& factory = pHandle->m_ArrayFactory;
        auto& vValues = factory->underlying_container_t<LONG>(Array);
        LONG nIndex = 0;

        for (i = 0; i < 32; i++)
        {
            if (DeviceEvents & 1L << i)
            {
                vValues[nIndex] = i;
                ++nIndex;
            }
        }
    }
    const bool bRet = SetCapValuesEx2_Internal(pSource, CAP_DEVICEEVENT, SetType, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, Array)?true:false;
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetDeviceNotifications(DTWAIN_SOURCE Source, LPLONG lpDeviceEvents)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpDeviceEvents))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    auto pTheSource = pSource;
    DTWAIN_ARRAY Array = nullptr;
    DTWAINArrayPtr_RAII raii(pHandle, &Array);

    // See if Source supports the DEVICEEVENTS capability
    // See if Source supports the DEVICEEVENTS capability
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return !CTL_TwainAppMgr::IsCapabilitySupported(pTheSource, CAP_DEVICEEVENT); },
        DTWAIN_ERR_DEVICEEVENT_NOT_SUPPORTED, false, FUNC_MACRO);

    const bool bRet = GetCapValuesEx2_Internal(pSource, CAP_DEVICEEVENT, DTWAIN_CAPGETCURRENT, 
                                DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &Array) ? true : false;
    if (!bRet)
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    auto& factory = pHandle->m_ArrayFactory;

    *lpDeviceEvents = 0L;
    auto& vValues = factory->underlying_container_t<LONG>(Array);
    std::for_each(vValues.begin(), vValues.end(), [&](LONG Value)
    {
        if (Value < 32 && Value > 0L)
            *lpDeviceEvents |= 1L << (Value - 1L);
    });
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpDeviceEvents))
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetDeviceEvent(DTWAIN_SOURCE Source, LPLONG lpEvent)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpEvent))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    const CTL_DeviceEvent DeviceEvent = pSource->GetDeviceEvent();
    *lpEvent = DeviceEvent.GetEvent() + 1;
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpEvent))
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetDeviceEventEx(DTWAIN_SOURCE Source, LPLONG lpEvent, LPDTWAIN_ARRAY pArray)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpEvent, pArray))
    if (!DTWAIN_GetDeviceEvent(Source, lpEvent))
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    if (!pArray)
        LOG_FUNC_EXIT_NONAME_PARAMS(true)

    CTL_ITwainSource* pSource = static_cast<CTL_ITwainSource*>(Source);
    const CTL_DeviceEvent DeviceEvent = pSource->GetDeviceEvent();
    const auto pHandle = pSource->GetDTWAINHandle();
    const DTWAIN_BOOL bRet = DeviceEvent.GetEventInfoEx(pHandle, pArray);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpEvent))
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetDeviceEventInfo(DTWAIN_SOURCE Source, LONG nWhichInfo, LPVOID pValue)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nWhichInfo, pValue))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);

    const CTL_DeviceEvent DeviceEvent = pSource->GetDeviceEvent();
    switch (nWhichInfo)
    {
        case DTWAIN_GETDE_EVENT:
        {
            const auto p = static_cast<LPLONG>(pValue);
            *p = DeviceEvent.GetEvent() + 1;
        }
        break;

        case DTWAIN_GETDE_DEVNAME:
        {
            char *s = static_cast<char*>(pValue);
            auto devName = DeviceEvent.GetDeviceNameA();
            strcpy(s, devName.c_str());
        }
        break;

        case DTWAIN_GETDE_BATTERYMINUTES:
        {
            const LPLONG p = static_cast<LPLONG>(pValue);
            *p = DeviceEvent.GetBatteryMinutes();
        }
        break;

        case DTWAIN_GETDE_BATTERYPCT:
        {
            const LPLONG p = static_cast<LPLONG>(pValue);
            *p = DeviceEvent.GetBatteryPercentage();
        }
        break;

        case DTWAIN_GETDE_XRESOLUTION:
        {
            double *p = static_cast<double*>(pValue);
            *p = DeviceEvent.GetXResolution();
        }
        break;

        case DTWAIN_GETDE_YRESOLUTION:
        {
            double *p = static_cast<double*>(pValue);
            *p = DeviceEvent.GetYResolution();
        }
        break;

        case DTWAIN_GETDE_FLASHUSED:
        {
            const LPLONG p = static_cast<LPLONG>(pValue);
            *p = DeviceEvent.GetFlashUsed2();
        }
        break;

        case DTWAIN_GETDE_AUTOCAPTURE:
        {
            const LPLONG p = static_cast<LPLONG>(pValue);
            *p = DeviceEvent.GetAutomaticCapture();
        }
        break;

        case DTWAIN_GETDE_TIMEBEFORECAPTURE:
        {
            const LPLONG p = static_cast<LPLONG>(pValue);
            *p = DeviceEvent.GetTimeBeforeFirstCapture();
        }
        break;

        case DTWAIN_GETDE_TIMEBETWEENCAPTURES:
        {
            const LPLONG p = static_cast<LPLONG>(pValue);
            *p = DeviceEvent.GetTimeBetweenCaptures();
        }
        break;

        case DTWAIN_GETDE_POWERSUPPLY:
        {
            const LPLONG p = static_cast<LPLONG>(pValue);
            *p = DeviceEvent.GetPowerSupply();
        }
        break;

    default:
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{return true; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}
