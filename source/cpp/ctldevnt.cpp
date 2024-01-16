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
#include "ctltrp.h"
#include "ctltr010.h"
#include "ctldevnt.h"
#include "dtwain.h"
#include "twainfix32.h"
#include "arrayfactory.h"

using namespace dynarithmic;
CTL_DeviceEvent::CTL_DeviceEvent() : m_DeviceEvent() {}

TW_UINT32  CTL_DeviceEvent::GetEvent() const { return             m_DeviceEvent.Event;            }
CTL_StringType CTL_DeviceEvent::GetDeviceName() const { return    StringConversion::Convert_AnsiPtr_To_Native(m_DeviceEvent.DeviceName); }
TW_UINT32  CTL_DeviceEvent::GetBatteryMinutes() const { return    m_DeviceEvent.BatteryMinutes;   }
TW_INT16   CTL_DeviceEvent::GetBatteryPercentage() const { return m_DeviceEvent.BatteryPercentage;}
TW_INT32   CTL_DeviceEvent::GetPowerSupply() const { return       m_DeviceEvent.PowerSupply;      }

double CTL_DeviceEvent::GetXResolution() const
{
    return static_cast<double>(Fix32ToFloat(m_DeviceEvent.XResolution));
}

double CTL_DeviceEvent::GetYResolution() const
{
    return static_cast<double>(Fix32ToFloat(m_DeviceEvent.YResolution));
}

TW_UINT32  CTL_DeviceEvent::GetFlashUsed2() const { return             m_DeviceEvent.FlashUsed2;            }
TW_UINT32  CTL_DeviceEvent::GetAutomaticCapture() const { return       m_DeviceEvent.AutomaticCapture;      }
TW_UINT32  CTL_DeviceEvent::GetTimeBeforeFirstCapture() const { return m_DeviceEvent.TimeBeforeFirstCapture;}
TW_UINT32  CTL_DeviceEvent::GetTimeBetweenCaptures() const { return    m_DeviceEvent.TimeBetweenCaptures;   }


bool CTL_DeviceEvent::GetEventInfoEx(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY Array)  const
{
    auto& factory = pHandle->m_ArrayFactory;
    factory->clear(Array);
    switch (m_DeviceEvent.Event )
    {
        case TWDE_CHECKBATTERY:
        {
            auto val = m_DeviceEvent.BatteryMinutes;
            factory->add_to_back(Array, &val, 1);
            val = m_DeviceEvent.BatteryPercentage;
            factory->add_to_back(Array, &val, 1);
        }
        break;

        case TWDE_CHECKPOWERSUPPLY:
        {
            auto val = m_DeviceEvent.PowerSupply;
            factory->add_to_back(Array, &val, 1);
        }
        break;

        case TWDE_CHECKRESOLUTION:
        {
            auto val = GetXResolution();
            factory->add_to_back(Array, &val, 1);
            val = GetYResolution();
            factory->add_to_back(Array, &val, 1);
        }
        break;

        case TWDE_CHECKFLASH:
        {
            auto val = m_DeviceEvent.FlashUsed2;
            factory->add_to_back(Array, &val, 1);
        }
        break;

        case TWDE_CHECKAUTOMATICCAPTURE:
        {
            auto val = m_DeviceEvent.AutomaticCapture;
            factory->add_to_back(Array, &val, 1);
            val = m_DeviceEvent.TimeBeforeFirstCapture;
            factory->add_to_back(Array, &val, 1);
            val = m_DeviceEvent.TimeBetweenCaptures;
            factory->add_to_back(Array, &val, 1);
        }
        break;

        default:
            return false;
    }
    return true;
}













