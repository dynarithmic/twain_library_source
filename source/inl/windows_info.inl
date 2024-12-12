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

#define _CRT_NON_CONFORMING_SWPRINTFS
#if defined(_WIN32) || defined(_WIN64)
#include <tchar.h>
#endif
#include <cstring>
#include <sstream>
#include <iomanip>
#include "dtwinverex.h"
#include "dtwinver.h"

#if !defined(_WIN32) && !defined(_WIN64)
#define _stprintf sprintf
#define _tcscat strcat
#define LPTSTR LPSTR
#endif

#ifndef DEVICEFAMILYINFOENUM_WINDOWS_8X
#define DEVICEFAMILYINFOENUM_WINDOWS_8X 0x00000001
#endif //#ifndef DEVICEFAMILYINFOENUM_WINDOWS_8X

#ifndef DEVICEFAMILYINFOENUM_WINDOWS_PHONE_8X
#define DEVICEFAMILYINFOENUM_WINDOWS_PHONE_8X 0x00000002
#endif //#ifndef DEVICEFAMILYINFOENUM_WINDOWS_PHONE_8X

#ifndef DEVICEFAMILYINFOENUM_DESKTOP
#define DEVICEFAMILYINFOENUM_DESKTOP 0x00000003
#endif //#ifndef DEVICEFAMILYINFOENUM_DESKTOP

#ifndef DEVICEFAMILYINFOENUM_MOBILE
#define DEVICEFAMILYINFOENUM_MOBILE 0x00000004
#endif //#ifndef DEVICEFAMILYINFOENUM_MOBILE

#ifndef DEVICEFAMILYINFOENUM_XBOX
#define DEVICEFAMILYINFOENUM_XBOX 0x00000005
#endif //#ifndef DEVICEFAMILYINFOENUM_XBOX

#ifndef DEVICEFAMILYINFOENUM_TEAM
#define DEVICEFAMILYINFOENUM_TEAM 0x00000006
#endif //#ifndef DEVICEFAMILYINFOENUM_TEAM

#ifndef DEVICEFAMILYINFOENUM_IOT
#define DEVICEFAMILYINFOENUM_IOT 0x00000007
#endif //#ifndef DEVICEFAMILYINFOENUM_IOT

#ifndef DEVICEFAMILYINFOENUM_IOT_HEADLESS
#define DEVICEFAMILYINFOENUM_IOT_HEADLESS 0x00000008
#endif //#ifndef DEVICEFAMILYINFOENUM_IOT_HEADLESS

#ifndef DEVICEFAMILYINFOENUM_SERVER
#define DEVICEFAMILYINFOENUM_SERVER 0x00000009
#endif //#ifndef DEVICEFAMILYINFOENUM_SERVER

#ifndef DEVICEFAMILYINFOENUM_HOLOGRAPHIC
#define DEVICEFAMILYINFOENUM_HOLOGRAPHIC 0x0000000A
#endif //#ifndef DEVICEFAMILYINFOENUM_HOLOGRAPHIC

#ifndef DEVICEFAMILYINFOENUM_XBOXSRA
#define DEVICEFAMILYINFOENUM_XBOXSRA 0x0000000B
#endif //#ifndef DEVICEFAMILYINFOENUM_XBOXSRA

#ifndef DEVICEFAMILYINFOENUM_XBOXERA
#define DEVICEFAMILYINFOENUM_XBOXERA 0x0000000C
#endif //#ifndef DEVICEFAMILYINFOENUM_XBOXERA

#ifndef DEVICEFAMILYINFOENUM_SERVER_NANO
#define DEVICEFAMILYINFOENUM_SERVER_NANO 0x0000000D
#endif //#ifndef DEVICEFAMILYINFOENUM_SERVER_NANO

#ifndef DEVICEFAMILYINFOENUM_8828080
#define DEVICEFAMILYINFOENUM_8828080 0x0000000E
#endif //#ifndef DEVICEFAMILYINFOENUM_8828080

#ifndef DEVICEFAMILYINFOENUM_7067329
#define DEVICEFAMILYINFOENUM_7067329 0x0000000F
#endif //#ifndef DEVICEFAMILYINFOENUM_7067329

#ifndef DEVICEFAMILYINFOENUM_WINDOWS_CORE
#define DEVICEFAMILYINFOENUM_WINDOWS_CORE 0x00000010
#endif //#ifndef DEVICEFAMILYINFOENUM_WINDOWS_CORE

#ifndef DEVICEFAMILYINFOENUM_WINDOWS_CORE_HEADLESS
#define DEVICEFAMILYINFOENUM_WINDOWS_CORE_HEADLESS 0x00000011
#endif //#ifndef DEVICEFAMILYINFOENUM_WINDOWS_CORE_HEADLESS

#ifndef DEVICEFAMILYDEVICEFORM_PHONE
#define DEVICEFAMILYDEVICEFORM_PHONE 0x00000001
#endif //#ifndef DEVICEFAMILYDEVICEFORM_PHONE

#ifndef DEVICEFAMILYDEVICEFORM_TABLET
#define DEVICEFAMILYDEVICEFORM_TABLET 0x00000002
#endif //#ifndef DEVICEFAMILYDEVICEFORM_TABLET

#ifndef DEVICEFAMILYDEVICEFORM_DESKTOP
#define DEVICEFAMILYDEVICEFORM_DESKTOP 0x00000003
#endif //#ifndef DEVICEFAMILYDEVICEFORM_DESKTOP

#ifndef DEVICEFAMILYDEVICEFORM_NOTEBOOK
#define DEVICEFAMILYDEVICEFORM_NOTEBOOK 0x00000004
#endif //#ifndef DEVICEFAMILYDEVICEFORM_NOTEBOOK

#ifndef DEVICEFAMILYDEVICEFORM_CONVERTIBLE
#define DEVICEFAMILYDEVICEFORM_CONVERTIBLE 0x00000005
#endif //#ifndef DEVICEFAMILYDEVICEFORM_CONVERTIBLE

#ifndef DEVICEFAMILYDEVICEFORM_DETACHABLE
#define DEVICEFAMILYDEVICEFORM_DETACHABLE 0x00000006
#endif //#ifndef DEVICEFAMILYDEVICEFORM_DETACHABLE

#ifndef DEVICEFAMILYDEVICEFORM_ALLINONE
#define DEVICEFAMILYDEVICEFORM_ALLINONE 0x00000007
#endif //#ifndef DEVICEFAMILYDEVICEFORM_ALLINONE

#ifndef DEVICEFAMILYDEVICEFORM_STICKPC
#define DEVICEFAMILYDEVICEFORM_STICKPC 0x00000008
#endif //#ifndef DEVICEFAMILYDEVICEFORM_STICKPC

#ifndef DEVICEFAMILYDEVICEFORM_PUCK
#define DEVICEFAMILYDEVICEFORM_PUCK 0x00000009
#endif //#ifndef DEVICEFAMILYDEVICEFORM_PUCK

#ifndef DEVICEFAMILYDEVICEFORM_LARGESCREEN
#define DEVICEFAMILYDEVICEFORM_LARGESCREEN 0x0000000A
#endif //#ifndef DEVICEFAMILYDEVICEFORM_LARGESCREEN

#ifndef DEVICEFAMILYDEVICEFORM_HMD
#define DEVICEFAMILYDEVICEFORM_HMD 0x0000000B
#endif //#ifndef DEVICEFAMILYDEVICEFORM_HMD

#ifndef DEVICEFAMILYDEVICEFORM_INDUSTRY_HANDHELD
#define DEVICEFAMILYDEVICEFORM_INDUSTRY_HANDHELD 0x0000000C
#endif //#ifndef DEVICEFAMILYDEVICEFORM_INDUSTRY_HANDHELD

#ifndef DEVICEFAMILYDEVICEFORM_INDUSTRY_TABLET
#define DEVICEFAMILYDEVICEFORM_INDUSTRY_TABLET 0x0000000D
#endif //#ifndef DEVICEFAMILYDEVICEFORM_INDUSTRY_TABLET

#ifndef DEVICEFAMILYDEVICEFORM_BANKING
#define DEVICEFAMILYDEVICEFORM_BANKING 0x0000000E
#endif //#ifndef DEVICEFAMILYDEVICEFORM_BANKING

#ifndef DEVICEFAMILYDEVICEFORM_BUILDING_AUTOMATION
#define DEVICEFAMILYDEVICEFORM_BUILDING_AUTOMATION 0x0000000F
#endif //#ifndef DEVICEFAMILYDEVICEFORM_BUILDING_AUTOMATION

#ifndef DEVICEFAMILYDEVICEFORM_DIGITAL_SIGNAGE
#define DEVICEFAMILYDEVICEFORM_DIGITAL_SIGNAGE 0x00000010
#endif //#ifndef DEVICEFAMILYDEVICEFORM_DIGITAL_SIGNAGE

#ifndef DEVICEFAMILYDEVICEFORM_GAMING
#define DEVICEFAMILYDEVICEFORM_GAMING 0x00000011
#endif //#ifndef DEVICEFAMILYDEVICEFORM_GAMING

#ifndef DEVICEFAMILYDEVICEFORM_HOME_AUTOMATION
#define DEVICEFAMILYDEVICEFORM_HOME_AUTOMATION 0x00000012
#endif //#ifndef DEVICEFAMILYDEVICEFORM_HOME_AUTOMATION

#ifndef DEVICEFAMILYDEVICEFORM_INDUSTRIAL_AUTOMATION
#define DEVICEFAMILYDEVICEFORM_INDUSTRIAL_AUTOMATION 0x00000013
#endif //#ifndef DEVICEFAMILYDEVICEFORM_INDUSTRIAL_AUTOMATION

#ifndef DEVICEFAMILYDEVICEFORM_KIOSK
#define DEVICEFAMILYDEVICEFORM_KIOSK 0x00000014
#endif //#ifndef DEVICEFAMILYDEVICEFORM_KIOSK

#ifndef DEVICEFAMILYDEVICEFORM_MAKER_BOARD
#define DEVICEFAMILYDEVICEFORM_MAKER_BOARD 0x00000015
#endif //#ifndef DEVICEFAMILYDEVICEFORM_MAKER_BOARD

#ifndef DEVICEFAMILYDEVICEFORM_MEDICAL
#define DEVICEFAMILYDEVICEFORM_MEDICAL 0x00000016
#endif //#ifndef DEVICEFAMILYDEVICEFORM_MEDICAL

#ifndef DEVICEFAMILYDEVICEFORM_NETWORKING
#define DEVICEFAMILYDEVICEFORM_NETWORKING 0x00000017
#endif //#ifndef DEVICEFAMILYDEVICEFORM_NETWORKING

#ifndef DEVICEFAMILYDEVICEFORM_POINT_OF_SERVICE
#define DEVICEFAMILYDEVICEFORM_POINT_OF_SERVICE 0x00000018
#endif //#ifndef DEVICEFAMILYDEVICEFORM_POINT_OF_SERVICE

#ifndef DEVICEFAMILYDEVICEFORM_PRINTING
#define DEVICEFAMILYDEVICEFORM_PRINTING 0x00000019
#endif //#ifndef DEVICEFAMILYDEVICEFORM_PRINTING

#ifndef DEVICEFAMILYDEVICEFORM_THIN_CLIENT
#define DEVICEFAMILYDEVICEFORM_THIN_CLIENT 0x0000001A
#endif //#ifndef DEVICEFAMILYDEVICEFORM_THIN_CLIENT

#ifndef DEVICEFAMILYDEVICEFORM_TOY
#define DEVICEFAMILYDEVICEFORM_TOY 0x0000001B
#endif //#ifndef DEVICEFAMILYDEVICEFORM_TOY

#ifndef DEVICEFAMILYDEVICEFORM_VENDING
#define DEVICEFAMILYDEVICEFORM_VENDING 0x0000001C
#endif //#ifndef DEVICEFAMILYDEVICEFORM_VENDING

#ifndef DEVICEFAMILYDEVICEFORM_INDUSTRY_OTHER
#define DEVICEFAMILYDEVICEFORM_INDUSTRY_OTHER 0x0000001D
#endif //#ifndef DEVICEFAMILYDEVICEFORM_INDUSTRY_OTHER

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_ONE
#define DEVICEFAMILYDEVICEFORM_XBOX_ONE 0x0000001E
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_ONE

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_ONE_S
#define DEVICEFAMILYDEVICEFORM_XBOX_ONE_S 0x0000001F
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_ONE_S

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_ONE_X
#define DEVICEFAMILYDEVICEFORM_XBOX_ONE_X 0x00000020
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_ONE_X

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_ONE_X_DEVKIT
#define DEVICEFAMILYDEVICEFORM_XBOX_ONE_X_DEVKIT 0x00000021
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_ONE_X_DEVKIT

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_SERIES_X
#define DEVICEFAMILYDEVICEFORM_XBOX_SERIES_X 0x00000022
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_SERIES_X

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_SERIES_X_DEVKIT
#define DEVICEFAMILYDEVICEFORM_XBOX_SERIES_X_DEVKIT 0x00000023
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_SERIES_X_DEVKIT

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_SERIES_S
#define DEVICEFAMILYDEVICEFORM_XBOX_SERIES_S 0x00000024
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_SERIES_S

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_01
#define DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_01 0x00000025
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_01

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_02
#define DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_02 0x00000026
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_02

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_03
#define DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_03 0x00000027
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_03

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_04
#define DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_04 0x00000028
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_04

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_05
#define DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_05 0x00000029
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_05

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_06
#define DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_06 0x0000002A
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_06

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_07
#define DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_07 0x0000002B
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_07

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_08
#define DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_08 0x0000002C
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_08

#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_09
#define DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_09 0x0000002D
#endif //#ifndef DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_09


#pragma warning(disable: 26485)
#pragma warning(disable: 26493)
#pragma warning(disable: 26477)

using namespace dynarithmic;

CTL_StringType dynarithmic::GetWinVersion()
{
    COSVersion::OS_VERSION_INFO osvi {};
#ifdef _UNICODE
    std::wostringstream sText, sBuf;
#else
    std::ostringstream sText, sBuf;
#endif

    COSVersion os;
    if (os.GetVersion(&osvi))
    {
#ifndef UNDER_CE
        sText << _T("Emulated OS: ");

        switch (osvi.EmulatedPlatform)
        {
            case COSVersion::Dos:
            {
                sText << _T("DOS"); 
                break;
            }
            case COSVersion::Windows3x:
            {
                sText << _T("Windows");
                break;
            }
            case COSVersion::WindowsCE:
            {
                //This code will never really be executed, but for the same of completeness include it here
                if (os.IsWindowsEmbeddedCompact(&osvi, FALSE))
                    sText << _T("Windows Embedded Compact");
                else if (os.IsWindowsCENET(&osvi, FALSE))
                    sText << _T("Windows CE .NET");
                else
                    sText << _T("Windows CE");
                break;
            }

            case COSVersion::Windows9x:
            {
                if (os.IsWindows95(&osvi, FALSE))
                    sBuf << _T("Windows 95");
                else if (os.IsWindows95SP1(&osvi, FALSE))
                    sBuf << _T("Windows 95 SP1");
                else if (os.IsWindows95B(&osvi, FALSE))
                    sBuf << _T("Windows 95 B [aka OSR2]");
                else if (os.IsWindows95C(&osvi, FALSE))
                    sBuf << _T("Windows 95 C [aka OSR2.5]");
                else if (os.IsWindows98(&osvi, FALSE))
                    sBuf << _T("Windows 98");
                else if (os.IsWindows98SP1(&osvi, FALSE))
                    sBuf << _T("Windows 98 SP1");
                else if (os.IsWindows98SE(&osvi, FALSE))
                    sBuf << _T("Windows 98 Second Edition");
                else if (os.IsWindowsME(&osvi, FALSE))
                    sBuf << _T("Windows Millenium Edition");
                else
                    sBuf << _T("Windows \?\?");
                sText << sBuf.str();
                break;
            }

            case COSVersion::WindowsNT:
            {
                if (os.IsNTPreWin2k(&osvi, FALSE))
                {
                    sText << _T("Windows NT"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                    if (os.IsNTWorkstation(&osvi, FALSE))
                        sText << _T(" (Workstation)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsNTStandAloneServer(&osvi, FALSE))
                        sText << _T(" (Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsNTPDC(&osvi, FALSE))
                        sText << _T(" (Primary Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsNTBDC(&osvi, FALSE))
                        sText << _T(" (Backup Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                    if (os.IsNTDatacenterServer(&osvi, FALSE))
                        sText << _T(", (Datacenter)");
                    else if (os.IsNTEnterpriseServer(&osvi, FALSE))
                        sText << _T(", (Enterprise)");
                }
                else if (os.IsWindows2000(&osvi, FALSE))
                {
                    if (os.IsProfessional(&osvi))
                        sText <<  _T("Windows 2000 (Professional)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows2000Server(&osvi, FALSE))
                        sText <<  _T("Windows 2000 Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows2000DomainController(&osvi, FALSE))
                        sText <<  _T("Windows 2000 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows 2000"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                    if (os.IsWindows2000DatacenterServer(&osvi, FALSE))
                        sText <<  _T(", (Datacenter)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows2000AdvancedServer(&osvi, FALSE))
                        sText <<  _T(", (Advanced Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else if (os.IsWindowsXPOrWindowsServer2003(&osvi, FALSE))
                {
                    if (os.IsStarterEdition(&osvi))
                        sText <<  _T("Windows XP (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsPersonal(&osvi))
                        sText <<  _T("Windows XP (Personal)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsProfessional(&osvi))
                        sText <<  _T("Windows XP (Professional)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindowsServer2003(&osvi, FALSE))
                        sText <<  _T("Windows Server 2003"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsDomainControllerWindowsServer2003(&osvi, FALSE))
                        sText <<  _T("Windows Server 2003 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindowsServer2003R2(&osvi, FALSE))
                        sText <<  _T("Windows Server 2003 R2"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsDomainControllerWindowsServer2003R2(&osvi, FALSE))
                        sText <<  _T("Windows Server 2003 R2 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows XP"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                    if (os.IsDatacenterWindowsServer2003(&osvi, FALSE) || os.IsDatacenterWindowsServer2003R2(&osvi, FALSE))
                        sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterpriseWindowsServer2003(&osvi, FALSE) || os.IsEnterpriseWindowsServer2003R2(&osvi, FALSE))
                        sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWebWindowsServer2003(&osvi, FALSE) || os.IsWebWindowsServer2003R2(&osvi, FALSE))
                        sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsStandardWindowsServer2003(&osvi, FALSE) || os.IsStandardWindowsServer2003R2(&osvi, FALSE))
                        sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else if (os.IsWindowsVistaOrWindowsServer2008(&osvi, FALSE))
                {
                    if (os.IsWindowsVista(&osvi, FALSE))
                    {
                        if (os.IsStarterEdition(&osvi))
                            sText <<  _T("Windows Vista (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsHomeBasic(&osvi))
                            sText <<  _T("Windows Vista (Home Basic)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsHomeBasicPremium(&osvi))
                            sText <<  _T("Windows Vista (Home Premium)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsBusiness(&osvi))
                            sText <<  _T("Windows Vista (Business)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsEnterprise(&osvi))
                            sText <<  _T("Windows Vista (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsUltimate(&osvi))
                            sText <<  _T("Windows Vista (Ultimate)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else
                            sText <<  _T("Windows Vista"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    }
                    else
                    {
                        if (os.IsWindowsServer2008(&osvi, FALSE))
                            sText <<  _T("Windows Server 2008"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsDomainControllerWindowsServer2008(&osvi, FALSE))
                            sText <<  _T("Windows Server 2008 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else
                            sText <<  _T("Windows Server 2008"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    }

                    if (os.IsDatacenterWindowsServer2008(&osvi, FALSE))
                        sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterpriseWindowsServer2008(&osvi, FALSE))
                        sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWebWindowsServer2008(&osvi, FALSE))
                        sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsStandardWindowsServer2008(&osvi, FALSE))
                        sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else if (os.IsWindows7OrWindowsServer2008R2(&osvi, FALSE))
                {
                    if (os.IsWindows7(&osvi, FALSE))
                    {
                        if (os.IsThinPC(&osvi))
                            sText <<  _T("Windows 7 Thin PC"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsStarterEdition(&osvi))
                            sText <<  _T("Windows 7 (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsHomeBasic(&osvi))
                            sText <<  _T("Windows 7 (Home Basic)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsHomeBasicPremium(&osvi))
                            sText <<  _T("Windows 7 (Home Premium)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsProfessional(&osvi))
                            sText <<  _T("Windows 7 (Professional)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsEnterprise(&osvi))
                            sText <<  _T("Windows 7 (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsUltimate(&osvi))
                            sText <<  _T("Windows 7 (Ultimate)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else
                            sText <<  _T("Windows 7"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    }
                    else
                    {
                        if (os.IsWindowsServer2008R2(&osvi, FALSE))
                            sText <<  _T("Windows Server 2008 R2"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsDomainControllerWindowsServer2008R2(&osvi, FALSE))
                            sText <<  _T("Windows Server 2008 R2 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else
                            sText <<  _T("Windows Server 2008"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    }

                    if (os.IsDatacenterWindowsServer2008R2(&osvi, FALSE))
                        sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterpriseWindowsServer2008R2(&osvi, FALSE))
                        sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWebWindowsServer2008R2(&osvi, FALSE))
                        sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsStandardWindowsServer2008R2(&osvi, FALSE))
                        sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else if (os.IsWindows8OrWindowsServer2012(&osvi, FALSE))
                {
                    if (os.IsWindows8(&osvi, FALSE))
                    {
                        if (os.IsThinPC(&osvi))
                            sText <<  _T("Windows 8 Thin PC"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsWindowsRT(&osvi, FALSE))
                            sText <<  _T("Windows 8 RT"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsStarterEdition(&osvi))
                            sText <<  _T("Windows 8 (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsProfessional(&osvi))
                            sText <<  _T("Windows 8 (Pro)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsEnterprise(&osvi))
                            sText <<  _T("Windows 8 (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else
                            sText <<  _T("Windows 8"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    }
                    else
                    {
                        if (os.IsWindowsServer2012(&osvi, FALSE))
                            sText <<  _T("Windows Server 2012"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsDomainControllerWindowsServer2012(&osvi, FALSE))
                            sText <<  _T("Windows Server 2012 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else
                            sText <<  _T("Windows Server 2012"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    }

                    if (os.IsDatacenterWindowsServer2012(&osvi, FALSE))
                        sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterpriseWindowsServer2012(&osvi, FALSE))
                        sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWebWindowsServer2012(&osvi, FALSE))
                        sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsStandardWindowsServer2012(&osvi, FALSE))
                        sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else if (os.IsWindows8Point1OrWindowsServer2012R2(&osvi, FALSE))
                {
                    if (os.IsWindows8Point1(&osvi, FALSE))
                    {
                        if (os.IsThinPC(&osvi))
                            sText <<  _T("Windows 8.1 Thin PC"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsWindowsRT(&osvi, FALSE))
                            sText <<  _T("Windows 8.1 RT"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsStarterEdition(&osvi))
                            sText <<  _T("Windows 8.1 (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsProfessional(&osvi))
                            sText <<  _T("Windows 8.1 (Pro)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsEnterprise(&osvi))
                            sText <<  _T("Windows 8.1 (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else
                            sText <<  _T("Windows 8.1"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    }
                    else
                    {
                        if (os.IsWindowsServer2012R2(&osvi, FALSE))
                            sText <<  _T("Windows Server 2012 R2"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsDomainControllerWindowsServer2012R2(&osvi, FALSE))
                            sText <<  _T("Windows Server 2012 R2 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else
                            sText <<  _T("Windows Server 2012 R2"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    }

                    if (os.IsCoreConnected(&osvi))
                        sText <<  _T(", (with Bing / CoreConnected)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    if (os.IsWindows8Point1Or2012R2Update(&osvi))
                        sText <<  _T(", (Update)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                    if (os.IsDatacenterWindowsServer2012R2(&osvi, FALSE))
                        sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterpriseWindowsServer2012R2(&osvi, FALSE))
                        sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWebWindowsServer2012R2(&osvi, FALSE))
                        sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsStandardWindowsServer2012R2(&osvi, FALSE))
                        sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else if (os.IsWindows10OrWindowsServer2016(&osvi, FALSE))
                {
                    if (os.IsWindows10(&osvi, FALSE))
                    {
                        if (os.IsThinPC(&osvi))
                            sText <<  _T("Windows 10 Thin PC"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsWindowsRT(&osvi, FALSE))
                            sText <<  _T("Windows 10 RT"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsStarterEdition(&osvi))
                            sText <<  _T("Windows 10 (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsCore(&osvi))
                            sText <<  _T("Windows 10 (Home)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsProfessional(&osvi))
                            sText <<  _T("Windows 10 (Pro)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsProWorkstations(&osvi))
                            sText <<  _T("Windows 10 (Pro for Workstations)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsEnterprise(&osvi))
                            sText <<  _T("Windows 10 (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsSEdition(&osvi))
                            sText <<  _T("Windows 10 S"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsWindows10X(&osvi))
                            sText <<  _T("Windows 10X"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else
                            sText <<  _T("Windows 10"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    }
                    else
                    {
                        if (os.IsNanoServer(&osvi))
                            sText <<  _T("Windows Server 2016 Nano Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsARM64Server(&osvi))
                            sText <<  _T("Windows Server 2016 ARM64 Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsDomainControllerWindowsServer2016(&osvi, FALSE))
                            sText <<  _T("Windows Server 2016 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else
                            sText <<  _T("Windows Server 2016"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        if (os.IsWindowsServerVersion1709(&osvi, FALSE))
                            sText <<  _T(", (version 1709)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                        else if (os.IsWindowsServerVersion1803(&osvi, FALSE))
                            sText <<  _T(", (version 1803)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    }

                    if (os.IsWindows10Version1507(&osvi, FALSE))
                        sText <<  _T(", (version 1507)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version1511(&osvi, FALSE))
                        sText <<  _T(", (version 1511)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version1607(&osvi, FALSE))
                        sText <<  _T(", (version 1607)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version1703(&osvi, FALSE))
                        sText <<  _T(", (version 1703)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version1709(&osvi, FALSE))
                        sText <<  _T(", (version 1709)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version1803(&osvi, FALSE))
                        sText <<  _T(", (version 1803)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version1809(&osvi, FALSE))
                        sText <<  _T(", (version 1809)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version1903(&osvi, FALSE))
                        sText <<  _T(", (version 1903)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version1909(&osvi, FALSE))
                        sText <<  _T(", (version 1909)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version2004(&osvi, FALSE))
                        sText <<  _T(", (version 2004)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version20H2(&osvi, FALSE))
                        sText <<  _T(", (version 20H2)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version21H1(&osvi, FALSE))
                        sText <<  _T(", (version 21H1)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version21H2(&osvi, FALSE))
                        sText <<  _T(", (version 21H2)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10Version22H2(&osvi, FALSE))
                        sText <<  _T(", (version 22H2)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10ActiveDevelopmentBranch(&osvi, FALSE))
                        sText <<  _T(", (Active Development Branch)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                    if (os.IsCoreConnected(&osvi))
                        sText <<  _T(", (with Bing / CoreConnected)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                    if (os.IsDatacenterWindowsServer2016(&osvi, FALSE))
                        sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterpriseWindowsServer2016(&osvi, FALSE))
                        sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWebWindowsServer2016(&osvi, FALSE))
                        sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsStandardWindowsServer2016(&osvi, FALSE))
                        sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else if (os.IsWindows11(&osvi, FALSE))
                {
                    if (os.IsStarterEdition(&osvi))
                        sText <<  _T("Windows 11 (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsCore(&osvi))
                        sText <<  _T("Windows 11 (Home)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsProfessional(&osvi))
                        sText <<  _T("Windows 11 (Pro)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsProWorkstations(&osvi))
                        sText <<  _T("Windows 11 (Pro for Workstations)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterprise(&osvi))
                        sText <<  _T("Windows 11 (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsSEdition(&osvi))
                        sText <<  _T("Windows 11 S"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows 11"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                    if (os.IsWindows11Version21H2(&osvi, FALSE))
                        sText <<  _T(", (version 21H2)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows11Version22H2(&osvi, FALSE))
                        sText <<  _T(", (version 22H2)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows11ActiveDevelopmentBranch(&osvi, FALSE))
                        sText <<  _T(", (Active Development Branch)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else if (os.IsWindowsServer2019(&osvi, FALSE))
                {
                    if (os.IsNanoServer(&osvi))
                        sText <<  _T("Windows Server 2019 Nano Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsARM64Server(&osvi))
                        sText <<  _T("Windows Server 2019 ARM64 Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsDomainControllerWindowsServer2019(&osvi, FALSE))
                        sText <<  _T("Windows Server 2019 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows Server 2019"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    if (os.IsWindowsServerVersion1809(&osvi, FALSE))
                        sText <<  _T(", (version 1809)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindowsServerVersion1903(&osvi, FALSE))
                        sText <<  _T(", (version 1903)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindowsServerVersion1909(&osvi, FALSE))
                        sText <<  _T(", (version 1909)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindowsServerVersion2004(&osvi, FALSE))
                        sText <<  _T(", (version 2004)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindowsServerVersion20H2(&osvi, FALSE))
                        sText <<  _T(", (version 20H2)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindowsServer2019ActiveDevelopmentBranch(&osvi, FALSE))
                        sText <<  _T(", (vNext aka Active Development Branch)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    if (os.IsDatacenterWindowsServer2019(&osvi, FALSE))
                        sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterpriseWindowsServer2019(&osvi, FALSE))
                        sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWebWindowsServer2019(&osvi, FALSE))
                        sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsStandardWindowsServer2019(&osvi, FALSE))
                        sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else if (os.IsWindowsServer2022(&osvi, FALSE))
                {
                    if (os.IsNanoServer(&osvi))
                        sText <<  _T("Windows Server 2022 Nano Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsARM64Server(&osvi))
                        sText <<  _T("Windows Server 2022 ARM64 Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsDomainControllerWindowsServer2022(&osvi, FALSE))
                        sText <<  _T("Windows Server 2022 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows Server 2022"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    if (os.IsWindowsServer2022ActiveDevelopmentBranch(&osvi, FALSE))
                        sText <<  _T(", (vNext aka Active Development Branch)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    if (os.IsDatacenterWindowsServer2022(&osvi, FALSE))
                        sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterpriseWindowsServer2022(&osvi, FALSE))
                        sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWebWindowsServer2022(&osvi, FALSE))
                        sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsStandardWindowsServer2022(&osvi, FALSE))
                        sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                break;
            }
            default:
            {
                sText <<  _T("Unknown OS"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
        }

#ifndef UNDER_CE
        switch (osvi.EmulatedProcessorType)
        {
            case COSVersion::X86_PROCESSOR:
            {
                sText <<  _T(", (x86 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::MIPS_PROCESSOR:
            {
                sText <<  _T(", (MIPS Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::ALPHA_PROCESSOR:
            {
                sText <<  _T(", (Alpha Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::PPC_PROCESSOR:
            {
                sText <<  _T(", (PPC Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::IA64_PROCESSOR:
            {
                sText <<  _T(", (IA64 Itanium[2] Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::AMD64_PROCESSOR:
            {
                sText <<  _T(", (x64 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::ALPHA64_PROCESSOR:
            {
                sText <<  _T(", (Alpha64 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::MSIL_PROCESSOR:
            {
                sText <<  _T(", (MSIL Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::ARM_PROCESSOR:
            {
                sText <<  _T(", (ARM Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::SHX_PROCESSOR:
            {
                sText <<  _T(", (SHX Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::IA32_ON_WIN64_PROCESSOR:
            {
                sText <<  _T(", (IA32 on Win64 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::NEUTRAL_PROCESSOR:
            {
                sText <<  _T(", (Neutral Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::ARM64_PROCESSOR:
            {
                sText <<  _T(", (ARM64 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::ARM32_ON_WIN64_PROCESSOR:
            {
                sText <<  _T(", (ARM32 on Win64 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::IA32_ON_ARM64_PROCESSOR:
            {
                sText <<  _T(", (IA32 on ARM64 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::UNKNOWN_PROCESSOR: //deliberate fallthrough
            default:
            {
                sText <<  _T(", (Unknown Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
        }
#endif //#ifndef UNDER_CE
        sBuf.str(_T(""));
        sBuf << _T(" v") << (int)(osvi.dwEmulatedMajorVersion) << _T(".");
        sText << sBuf.str();
        sBuf.str(_T(""));
        if (osvi.dwEmulatedMinorVersion % 10)
        {
            if (osvi.dwEmulatedMinorVersion > 9)
                sBuf << std::setfill(_T('0')) << std::setw(2) << static_cast<int>(osvi.dwEmulatedMinorVersion);
            else
                sBuf << std::setfill(_T('0')) << std::setw(1) << static_cast<int>(osvi.dwEmulatedMinorVersion);
        }
        else
            sBuf << std::setfill(_T('0')) << std::setw(1) << osvi.dwEmulatedMinorVersion / 10;
        sText <<  sBuf.str(); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        sBuf.str(_T(""));
        if (osvi.dwEmulatedBuildNumber)
        {
            sBuf << _T(" Build:") << (int)(osvi.dwEmulatedBuildNumber);
            sText << sBuf.str();
        }
        sBuf.str(_T(""));
        if (osvi.wEmulatedServicePackMajor)
        {
            if (osvi.wEmulatedServicePackMinor)
            {
                //Handle the special case of NT 4 SP 6a which Dtwinver treats as SP 6.1
                if (os.IsNTPreWin2k(&osvi, FALSE) && (osvi.wEmulatedServicePackMajor == 6) && (osvi.wEmulatedServicePackMinor == 1))
                    sBuf << _T(" Service Pack: 6a");
                //Handle the special case of XP SP 1a which Dtwinver treats as SP 1.1
                else if (os.IsWindowsXP(&osvi, FALSE) && (osvi.wEmulatedServicePackMajor == 1) && (osvi.wEmulatedServicePackMinor == 1))
                    sBuf << _T(" Service Pack: 1a");
                else
                    sBuf << _T(" Service Pack: ") << (int)(osvi.wEmulatedServicePackMajor) << _T(".") << (int)(osvi.wEmulatedServicePackMinor);
            }
            else
                sBuf << _T(" Service Pack: ") << (int)(osvi.wEmulatedServicePackMajor);
            sText << sBuf.str(); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            sBuf.str(_T(""));
        }
        else
        {
            if (osvi.wEmulatedServicePackMinor)
                sBuf << _T(" Service Pack:0.%d") << (int)(osvi.wEmulatedServicePackMinor);
        }

        sText <<  _T("\n"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
#endif //#ifndef UNDER_CE

  //CE does not really have a concept of an emulated OS so
  //lets not bother displaying any info on this on CE
        if (osvi.UnderlyingPlatform == COSVersion::WindowsCE)
            sText << _T("OS: ");
        else
            sText <<  _T("Underlying OS: "); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

        switch (osvi.UnderlyingPlatform)
        {
        case COSVersion::Dos:
        {
            sText <<  _T("DOS"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            break;
        }
        case COSVersion::Windows3x:
        {
            sText <<  _T("Windows"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            break;
        }
        case COSVersion::WindowsCE:
        {
            if (os.IsWindowsEmbeddedCompact(&osvi, TRUE))
                sText <<  _T("Windows Embedded Compact"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            else if (os.IsWindowsCENET(&osvi, TRUE))
                sText <<  _T("Windows CE .NET"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            else
                sText <<  _T("Windows CE"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            break;
        }
        case COSVersion::Windows9x:
        {
            sBuf.str(_T(""));
            if (os.IsWindows95(&osvi, TRUE))
                sBuf << _T("Windows 95");
            else if (os.IsWindows95SP1(&osvi, TRUE))
                sBuf << _T("Windows 95 SP1");
            else if (os.IsWindows95B(&osvi, TRUE))
                sBuf << _T("Windows 95 B [aka OSR2]");
            else if (os.IsWindows95C(&osvi, TRUE))
                sBuf << _T("Windows 95 C [aka OSR2.5]");
            else if (os.IsWindows98(&osvi, TRUE))
                sBuf << _T("Windows 98");
            else if (os.IsWindows98SP1(&osvi, TRUE))
                sBuf << _T("Windows 98 SP1");
            else if (os.IsWindows98SE(&osvi, TRUE))
                sBuf << _T("Windows 98 Second Edition");
            else if (os.IsWindowsME(&osvi, TRUE))
                sBuf << _T("Windows Millenium Edition");
            else
                sBuf << _T("Windows \?\?");
            sText << sBuf.str();
            break;
        }
        case COSVersion::WindowsNT:
        {
            if (os.IsNTPreWin2k(&osvi, TRUE))
            {
                sText <<  _T("Windows NT"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                if (os.IsNTWorkstation(&osvi, TRUE))
                    sText <<  _T(" (Workstation)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsNTStandAloneServer(&osvi, TRUE))
                    sText <<  _T(" (Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsNTPDC(&osvi, TRUE))
                    sText <<  _T(" (Primary Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsNTBDC(&osvi, TRUE))
                    sText <<  _T(" (Backup Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                if (os.IsNTDatacenterServer(&osvi, TRUE))
                    sText <<  _T(", (Datacenter)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsNTEnterpriseServer(&osvi, TRUE))
                    sText <<  _T(", (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            }
            else if (os.IsWindows2000(&osvi, TRUE))
            {
                if (os.IsProfessional(&osvi))
                    sText <<  _T("Windows 2000 (Professional)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows2000Server(&osvi, TRUE))
                    sText <<  _T("Windows 2000 Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows2000DomainController(&osvi, TRUE))
                    sText <<  _T("Windows 2000 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else
                    sText <<  _T("Windows 2000"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                if (os.IsWindows2000DatacenterServer(&osvi, TRUE))
                    sText <<  _T(", (Datacenter)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows2000AdvancedServer(&osvi, TRUE))
                    sText <<  _T(", (Advanced Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            }
            else if (os.IsWindowsXPOrWindowsServer2003(&osvi, TRUE))
            {
                if (os.IsStarterEdition(&osvi))
                    sText <<  _T("Windows XP (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsPersonal(&osvi))
                    sText <<  _T("Windows XP (Personal)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsProfessional(&osvi))
                    sText <<  _T("Windows XP (Professional)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindowsServer2003(&osvi, TRUE))
                    sText <<  _T("Windows Server 2003"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsDomainControllerWindowsServer2003(&osvi, TRUE))
                    sText <<  _T("Windows Server 2003 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindowsServer2003R2(&osvi, TRUE))
                    sText <<  _T("Windows Server 2003 R2"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsDomainControllerWindowsServer2003R2(&osvi, TRUE))
                    sText <<  _T("Windows Server 2003 R2 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else
                    sText <<  _T("Windows XP"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                if (os.IsDatacenterWindowsServer2003(&osvi, TRUE) || os.IsDatacenterWindowsServer2003R2(&osvi, TRUE))
                    sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsEnterpriseWindowsServer2003(&osvi, TRUE) || os.IsEnterpriseWindowsServer2003(&osvi, TRUE))
                    sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWebWindowsServer2003(&osvi, TRUE) || os.IsWebWindowsServer2003R2(&osvi, TRUE))
                    sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsStandardWindowsServer2003(&osvi, TRUE) || os.IsStandardWindowsServer2003R2(&osvi, TRUE))
                    sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            }
            else if (os.IsWindowsVistaOrWindowsServer2008(&osvi, TRUE))
            {
                if (os.IsWindowsVista(&osvi, TRUE))
                {
                    if (os.IsStarterEdition(&osvi))
                        sText <<  _T("Windows Vista (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsHomeBasic(&osvi))
                        sText <<  _T("Windows Vista (Home Basic)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsHomeBasicPremium(&osvi))
                        sText <<  _T("Windows Vista (Home Premium)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsBusiness(&osvi))
                        sText <<  _T("Windows Vista (Business)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterprise(&osvi))
                        sText <<  _T("Windows Vista (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsUltimate(&osvi))
                        sText <<  _T("Windows Vista (Ultimate)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows Vista"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else
                {
                    if (os.IsWindowsServer2008(&osvi, TRUE))
                        sText <<  _T("Windows Server 2008"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsDomainControllerWindowsServer2008(&osvi, TRUE))
                        sText <<  _T("Windows Server 2008 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows Server 2008"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }

                if (os.IsDatacenterWindowsServer2008(&osvi, TRUE))
                    sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsEnterpriseWindowsServer2008(&osvi, TRUE))
                    sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWebWindowsServer2008(&osvi, TRUE))
                    sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsStandardWindowsServer2008(&osvi, TRUE))
                    sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            }
            else if (os.IsWindows7OrWindowsServer2008R2(&osvi, TRUE))
            {
                if (os.IsWindows7(&osvi, TRUE))
                {
                    if (os.IsThinPC(&osvi))
                        sText <<  _T("Windows 7 Thin PC"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsStarterEdition(&osvi))
                        sText <<  _T("Windows 7 (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsHomeBasic(&osvi))
                        sText <<  _T("Windows 7 (Home Basic)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsHomeBasicPremium(&osvi))
                        sText <<  _T("Windows 7 (Home Premium)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsProfessional(&osvi))
                        sText <<  _T("Windows 7 (Professional)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterprise(&osvi))
                        sText <<  _T("Windows 7 (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsUltimate(&osvi))
                        sText <<  _T("Windows 7 (Ultimate)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows 7"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else
                {
                    if (os.IsWindowsServer2008R2(&osvi, TRUE))
                        sText <<  _T("Windows Server 2008 R2"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsDomainControllerWindowsServer2008R2(&osvi, TRUE))
                        sText <<  _T("Windows Server 2008 R2 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows Server 2008 R2"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }

                if (os.IsDatacenterWindowsServer2008R2(&osvi, TRUE))
                    sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsEnterpriseWindowsServer2008R2(&osvi, TRUE))
                    sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWebWindowsServer2008R2(&osvi, TRUE))
                    sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsStandardWindowsServer2008R2(&osvi, TRUE))
                    sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            }
            else if (os.IsWindows8OrWindowsServer2012(&osvi, TRUE))
            {
                if (os.IsWindows8(&osvi, TRUE))
                {
                    if (os.IsThinPC(&osvi))
                        sText <<  _T("Windows 8 Thin PC"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindowsRT(&osvi, TRUE))
                        sText <<  _T("Windows 8 RT"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsStarterEdition(&osvi))
                        sText <<  _T("Windows 8 (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsProfessional(&osvi))
                        sText <<  _T("Windows 8 (Pro)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterprise(&osvi))
                        sText <<  _T("Windows 8 (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows 8"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else
                {
                    if (os.IsWindowsServer2012(&osvi, TRUE))
                        sText <<  _T("Windows Server 2012"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsDomainControllerWindowsServer2012(&osvi, TRUE))
                        sText <<  _T("Windows Server 2012 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows Server 2012"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }

                if (os.IsDatacenterWindowsServer2012(&osvi, TRUE))
                    sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsEnterpriseWindowsServer2012(&osvi, TRUE))
                    sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWebWindowsServer2012(&osvi, TRUE))
                    sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsStandardWindowsServer2012(&osvi, TRUE))
                    sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            }
            else if (os.IsWindows8Point1OrWindowsServer2012R2(&osvi, TRUE))
            {
                if (os.IsWindows8Point1(&osvi, TRUE))
                {
                    if (os.IsThinPC(&osvi))
                        sText <<  _T("Windows 8.1 Thin PC"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindowsRT(&osvi, TRUE))
                        sText <<  _T("Windows 8.1 RT"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsStarterEdition(&osvi))
                        sText <<  _T("Windows 8.1 (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsProfessional(&osvi))
                        sText <<  _T("Windows 8.1 (Pro)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterprise(&osvi))
                        sText <<  _T("Windows 8.1 (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows 8.1"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else
                {
                    if (os.IsWindowsServer2012R2(&osvi, TRUE))
                        sText <<  _T("Windows Server 2012 R2"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsDomainControllerWindowsServer2012R2(&osvi, TRUE))
                        sText <<  _T("Windows Server 2012 R2 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows Server 2012 R2"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }

                if (os.IsCoreConnected(&osvi))
                    sText <<  _T(", (with Bing / CoreConnected)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                if (os.IsWindows8Point1Or2012R2Update(&osvi))
                    sText <<  _T(", (Update)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                if (os.IsDatacenterWindowsServer2012R2(&osvi, TRUE))
                    sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsEnterpriseWindowsServer2012R2(&osvi, TRUE))
                    sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWebWindowsServer2012R2(&osvi, TRUE))
                    sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsStandardWindowsServer2012R2(&osvi, TRUE))
                    sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            }
            else if (os.IsWindows10OrWindowsServer2016(&osvi, TRUE))
            {
                if (os.IsWindows10(&osvi, TRUE))
                {
                    if (os.IsThinPC(&osvi))
                        sText <<  _T("Windows 10 Thin PC"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindowsRT(&osvi, TRUE))
                        sText <<  _T("Windows 10 RT"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsStarterEdition(&osvi))
                        sText <<  _T("Windows 10 (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsCore(&osvi))
                        sText <<  _T("Windows 10 (Home)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsProfessional(&osvi))
                        sText <<  _T("Windows 10 (Pro)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsProWorkstations(&osvi))
                        sText <<  _T("Windows 10 (Pro for Workstations)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsEnterprise(&osvi))
                        sText <<  _T("Windows 10 (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsSEdition(&osvi))
                        sText <<  _T("Windows 10 S"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindows10X(&osvi))
                        sText <<  _T("Windows 10X"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows 10"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }
                else
                {
                    if (os.IsNanoServer(&osvi))
                        sText <<  _T("Windows Server 2016 Nano Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsARM64Server(&osvi))
                        sText <<  _T("Windows Server 2016 ARM64 Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsDomainControllerWindowsServer2016(&osvi, TRUE))
                        sText <<  _T("Windows Server 2016 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else
                        sText <<  _T("Windows Server 2016"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    if (os.IsWindowsServerVersion1709(&osvi, TRUE))
                        sText <<  _T(", (version 1709)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    else if (os.IsWindowsServerVersion1803(&osvi, TRUE))
                        sText <<  _T(", (version 1803)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                }

                if (os.IsWindows10Version1507(&osvi, TRUE))
                    sText <<  _T(", (version 1507)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version1511(&osvi, TRUE))
                    sText <<  _T(", (version 1511)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version1607(&osvi, TRUE))
                    sText <<  _T(", (version 1607)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version1703(&osvi, TRUE))
                    sText <<  _T(", (version 1703)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version1709(&osvi, TRUE))
                    sText <<  _T(", (version 1709)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version1803(&osvi, TRUE))
                    sText <<  _T(", (version 1803)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version1809(&osvi, TRUE))
                    sText <<  _T(", (version 1809)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version1903(&osvi, TRUE))
                    sText <<  _T(", (version 1903)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version1909(&osvi, TRUE))
                    sText <<  _T(", (version 1909)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version2004(&osvi, TRUE))
                    sText <<  _T(", (version 2004)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version20H2(&osvi, TRUE))
                    sText <<  _T(", (version 20H2)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version21H1(&osvi, TRUE))
                    sText <<  _T(", (version 21H1)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version21H2(&osvi, TRUE))
                    sText <<  _T(", (version 21H2)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10Version22H2(&osvi, TRUE))
                    sText <<  _T(", (version 22H2)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows10ActiveDevelopmentBranch(&osvi, TRUE))
                    sText <<  _T(", (Active Development Branch)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                if (os.IsCoreConnected(&osvi))
                    sText <<  _T(", (with Bing / CoreConnected)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                if (os.IsDatacenterWindowsServer2016(&osvi, TRUE))
                    sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsEnterpriseWindowsServer2016(&osvi, TRUE))
                    sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWebWindowsServer2016(&osvi, TRUE))
                    sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsStandardWindowsServer2016(&osvi, TRUE))
                    sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
    }
            else if (os.IsWindows11(&osvi, TRUE))
            {
                if (os.IsStarterEdition(&osvi))
                    sText <<  _T("Windows 11 (Starter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsCore(&osvi))
                    sText <<  _T("Windows 11 (Home)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsProfessional(&osvi))
                    sText <<  _T("Windows 11 (Pro)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsProWorkstations(&osvi))
                    sText <<  _T("Windows 11 (Pro for Workstations)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsEnterprise(&osvi))
                    sText <<  _T("Windows 11 (Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsSEdition(&osvi))
                    sText <<  _T("Windows 11 S"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else
                    sText <<  _T("Windows 11"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

                if (os.IsWindows11Version21H2(&osvi, TRUE))
                    sText <<  _T(", (version 21H2)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows11Version22H2(&osvi, TRUE))
                    sText <<  _T(", (version 22H2)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindows11ActiveDevelopmentBranch(&osvi, TRUE))
                    sText <<  _T(", (Active Development Branch)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            }
            else if (os.IsWindowsServer2019(&osvi, TRUE))
            {
                if (os.IsNanoServer(&osvi))
                    sText <<  _T("Windows Server 2019 Nano Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsARM64Server(&osvi))
                    sText <<  _T("Windows Server 2019 ARM64 Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsDomainControllerWindowsServer2019(&osvi, TRUE))
                    sText <<  _T("Windows Server 2019 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else
                    sText <<  _T("Windows Server 2019"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                if (os.IsWindowsServerVersion1809(&osvi, TRUE))
                    sText <<  _T(", (version 1809)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindowsServerVersion1903(&osvi, TRUE))
                    sText <<  _T(", (version 1903)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindowsServerVersion1909(&osvi, TRUE))
                    sText <<  _T(", (version 1909)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindowsServerVersion2004(&osvi, TRUE))
                    sText <<  _T(", (version 2004)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindowsServerVersion20H2(&osvi, TRUE))
                    sText <<  _T(", (version 20H2)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWindowsServer2019ActiveDevelopmentBranch(&osvi, TRUE))
                    sText <<  _T(", (vNext aka Active Development Branch)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                if (os.IsDatacenterWindowsServer2019(&osvi, TRUE))
                    sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsEnterpriseWindowsServer2019(&osvi, TRUE))
                    sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWebWindowsServer2019(&osvi, TRUE))
                    sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsStandardWindowsServer2019(&osvi, TRUE))
                    sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            }
            else if (os.IsWindowsServer2022(&osvi, TRUE))
            {
                if (os.IsNanoServer(&osvi))
                    sText <<  _T("Windows Server 2022 Nano Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsARM64Server(&osvi))
                    sText <<  _T("Windows Server 2022 ARM64 Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsDomainControllerWindowsServer2022(&osvi, TRUE))
                    sText <<  _T("Windows Server 2022 (Domain Controller)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else
                    sText <<  _T("Windows Server 2022"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                if (os.IsWindowsServer2022ActiveDevelopmentBranch(&osvi, TRUE))
                    sText <<  _T(", (vNext aka Active Development Branch)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                if (os.IsDatacenterWindowsServer2022(&osvi, TRUE))
                    sText <<  _T(", (Datacenter Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsEnterpriseWindowsServer2022(&osvi, TRUE))
                    sText <<  _T(", (Enterprise Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsWebWindowsServer2022(&osvi, TRUE))
                    sText <<  _T(", (Web Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                else if (os.IsStandardWindowsServer2022(&osvi, TRUE))
                    sText <<  _T(", (Standard Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            }
            break;
}
        default:
        {
            sBuf.str(_T(""));
            sBuf << _T("Unknown OS");
            sText << sBuf.str(); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            break;
        }
    }

#ifndef UNDER_CE
        switch (osvi.UnderlyingProcessorType)
        {
            case COSVersion::X86_PROCESSOR:
            {
                sText <<  _T(", (x86 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::MIPS_PROCESSOR:
            {
                sText <<  _T(", (MIPS Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::ALPHA_PROCESSOR:
            {
                sText <<  _T(", (Alpha Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::PPC_PROCESSOR:
            {
                sText <<  _T(", (PPC Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::IA64_PROCESSOR:
            {
                sText <<  _T(", (IA64 Itanium[2] Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::AMD64_PROCESSOR:
            {
                sText <<  _T(", (x64 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::ALPHA64_PROCESSOR:
            {
                sText <<  _T(", (Alpha64 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::MSIL_PROCESSOR:
            {
                sText <<  _T(", (MSIL Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::ARM_PROCESSOR:
            {
                sText <<  _T(", (ARM Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::SHX_PROCESSOR:
            {
                sText <<  _T(", (SHX Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::IA32_ON_WIN64_PROCESSOR:
            {
                sText <<  _T(", (IA32 on Win64 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::NEUTRAL_PROCESSOR:
            {
                sText <<  _T(", (Neutral Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::ARM64_PROCESSOR:
            {
                sText <<  _T(", (ARM64 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::ARM32_ON_WIN64_PROCESSOR:
            {
                sText <<  _T(", (ARM32 on Win64 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::IA32_ON_ARM64_PROCESSOR:
            {
                sText <<  _T(", (IA32 on ARM64 Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
            case COSVersion::UNKNOWN_PROCESSOR: //deliberate fallthrough
            default:
            {
                sText <<  _T(", (Unknown Processor)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                break;
            }
        }
#endif //#ifndef UNDER_CE
        sBuf.str(_T(""));
        sBuf << _T(" v") << (int)(osvi.dwUnderlyingMajorVersion) << _T(".");
        sText << sBuf.str();
        sBuf.str(_T(""));
        if (osvi.dwUnderlyingMinorVersion % 10)
        {
            if (osvi.dwUnderlyingMinorVersion > 9)
                sBuf << std::setfill(_T('0')) << std::setw(2) << static_cast<int>(osvi.dwUnderlyingMinorVersion);
            else
                sBuf << std::setfill(_T('0')) << std::setw(1) << static_cast<int>(osvi.dwUnderlyingMinorVersion);
        }
        else
            sBuf << std::setfill(_T('0')) << std::setw(1) << (int)(osvi.dwUnderlyingMinorVersion / 10);
        sText << sBuf.str(); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        sBuf.str(_T(""));
        if (osvi.dwUnderlyingBuildNumber)
        {
            //Report the UBR on Windows 10 / Server 2016 and later
            if (os.IsWindows10OrWindowsServer2016(&osvi, TRUE) || os.IsWindowsServer2019(&osvi, TRUE))
                sBuf << _T(" Build:") << (int)(osvi.dwUnderlyingBuildNumber) << _T(".") << (int)(osvi.dwUBR);
            else
                sBuf << _T(" Build:") << (int)(osvi.dwUnderlyingBuildNumber);
            sText << sBuf.str(); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        }
        sBuf.str(_T(""));
        if (osvi.wUnderlyingServicePackMajor)
        {
            if (osvi.wUnderlyingServicePackMinor)
            {
                //Handle the special case of NT 4 SP 6a which Dtwinver treats as SP 6.1
                if (os.IsNTPreWin2k(&osvi, TRUE) && (osvi.wUnderlyingServicePackMajor == 6) && (osvi.wUnderlyingServicePackMinor == 1))
                    sBuf << _T(" Service Pack: 6a");
                //Handle the special case of XP SP 1a which Dtwinver treats as SP 1.1
                else if (os.IsWindowsXP(&osvi, TRUE) && (osvi.wUnderlyingServicePackMajor == 1) && (osvi.wUnderlyingServicePackMinor == 1))
                    sBuf << _T(" Service Pack: 1a");
                else
                    sBuf << _T(" Service Pack:") << (int)(osvi.wUnderlyingServicePackMajor) << _T(".") << (int)(osvi.wUnderlyingServicePackMinor);
            }
            else
                sBuf << _T(" Service Pack:%d"), (int)(osvi.wUnderlyingServicePackMajor);
            sText << sBuf.str(); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        }
        else
        {
            sBuf.str(_T(""));
            if (osvi.wUnderlyingServicePackMinor)
                sBuf << _T(" Service Pack:0.") << (int)(osvi.wUnderlyingServicePackMinor);
        }
#if defined(COSVERSION_WIN32) || defined(COSVERSION_WIN64)
        if (osvi.ullUAPInfo)
        {
            sBuf.str(_T(""));
            sBuf << _T(", RtlGetDeviceFamilyInfo UAPInfo:") << (DWORD)((osvi.ullUAPInfo & 0xFFFF000000000000ui64) >> 48) << _T(".") <<
                (DWORD)((osvi.ullUAPInfo & 0x0000FFFF00000000ui64) >> 32) << _T(".") <<
                (DWORD)((osvi.ullUAPInfo & 0x00000000FFFF0000ui64) >> 16) << _T(".") <<
                (DWORD)(osvi.ullUAPInfo & 0x000000000000FFFFui64); //NOLINT(clang-diagnostic-format)
            sText << sBuf.str();
        }
#endif //#if defined(COSVERSION_WIN32) || defined(COSVERSION_WIN64)
        if (osvi.ulDeviceFamily)
        {
            sBuf.str(_T(""));
            sBuf << _T(", Device Family:") << std::hex << osvi.ulDeviceFamily;
            sText <<  sBuf.str(); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            switch (osvi.ulDeviceFamily)
            {
                case DEVICEFAMILYINFOENUM_WINDOWS_8X:
                {
                    sText <<  _T(", Windows 8x"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_WINDOWS_PHONE_8X:
                {
                    sText <<  _T(", Windows Phone 8x"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_DESKTOP:
                {
                    sText <<  _T(", Desktop"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_MOBILE:
                {
                    sText <<  _T(", Mobile"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_XBOX:
                {
                    sText <<  _T(", Xbox"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_TEAM:
                {
                    sText <<  _T(", Team"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_IOT:
                {
                    sText <<  _T(", IoT"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_IOT_HEADLESS:
                {
                    sText <<  _T(", IoT Headless"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_SERVER:
                {
                    sText <<  _T(", Server"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_HOLOGRAPHIC:
                {
                    sText <<  _T(", Holographic"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_XBOXSRA:
                {
                    sText <<  _T(", Xbox SRA"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_XBOXERA:
                {
                    sText <<  _T(", Xbox ERA"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_SERVER_NANO:
                {
                    sText <<  _T(", Server Nano"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_8828080:
                {
                    sText <<  _T(", \"8828080\""); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_7067329:
                {
                    sText <<  _T(", \"7067329\""); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_WINDOWS_CORE:
                {
                    sText <<  _T(", Windows Core"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYINFOENUM_WINDOWS_CORE_HEADLESS:
                {
                    sText <<  _T(", Windows Core Headless"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        if (osvi.ulDeviceForm)
        {
            sBuf.str(_T(""));
            sBuf << _T(", Device Form:") << std::hex << osvi.ulDeviceForm;
            sText << sBuf.str();
            switch (osvi.ulDeviceForm)
            {
                case DEVICEFAMILYDEVICEFORM_PHONE:
                {
                    sText <<  _T(", Phone"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_TABLET:
                {
                    sText <<  _T(", Tablet"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_DESKTOP:
                {
                    sText <<  _T(", Desktop"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_NOTEBOOK:
                {
                    sText <<  _T(", Notebook"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_CONVERTIBLE:
                {
                    sText <<  _T(", Convertible"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_DETACHABLE:
                {
                    sText <<  _T(", Detachable"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_ALLINONE:
                {
                    sText <<  _T(", All In One"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_STICKPC:
                {
                    sText <<  _T(", Stick PC"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_PUCK:
                {
                    sText <<  _T(", Puck"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_LARGESCREEN:
                {
                    sText <<  _T(", Large Screen"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_HMD:
                {
                    sText <<  _T(", Head Mounted Display"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_INDUSTRY_HANDHELD:
                {
                    sText <<  _T(", Handheld"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_INDUSTRY_TABLET:
                {
                    sText <<  _T(", Industry Tablet"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_BANKING:
                {
                    sText <<  _T(", Banking"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_BUILDING_AUTOMATION:
                {
                    sText <<  _T(", Automation"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_DIGITAL_SIGNAGE:
                {
                    sText <<  _T(", Digital Signage"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_GAMING:
                {
                    sText <<  _T(", Gaming"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_HOME_AUTOMATION:
                {
                    sText <<  _T(", Home Automation"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_INDUSTRIAL_AUTOMATION:
                {
                    sText <<  _T(", Industrial Automation"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_KIOSK:
                {
                    sText <<  _T(", Kiosk"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_MAKER_BOARD:
                {
                    sText <<  _T(", Maker Board"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_MEDICAL:
                {
                    sText <<  _T(", Medical"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_NETWORKING:
                {
                    sText <<  _T(", Networking"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_POINT_OF_SERVICE:
                {
                    sText <<  _T(", Point Of Service"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_PRINTING:
                {
                    sText <<  _T(", Printing"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_THIN_CLIENT:
                {
                    sText <<  _T(", Thin Client"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_TOY:
                {
                    sText <<  _T(", Toy"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_VENDING:
                {
                    sText <<  _T(", Vending"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_INDUSTRY_OTHER:
                {
                    sText <<  _T(", Other Industry"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_ONE:
                {
                    sText <<  _T(", XBox One"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_ONE_S:
                {
                    sText <<  _T(", XBox One S"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_ONE_X:
                {
                    sText <<  _T(", XBox One X"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_ONE_X_DEVKIT:
                {
                    sText <<  _T(", XBox One X Devkit"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_SERIES_X:
                {
                    sText <<  _T(", XBox Series X"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_SERIES_X_DEVKIT:
                {
                    sText <<  _T(", XBox Series X Devkit"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_SERIES_S:
                {
                    sText <<  _T(", XBox Series S"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_01:
                {
                    sText <<  _T(", XBox Reserved 01"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_02:
                {
                    sText <<  _T(", XBox Reserved 02"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_03:
                {
                    sText <<  _T(", XBox Reserved 03"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_04:
                {
                    sText <<  _T(", XBox Reserved 04"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_05:
                {
                    sText <<  _T(", XBox Reserved 05"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_06:
                {
                    sText <<  _T(", XBox Reserved 06"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_07:
                {
                    sText <<  _T(", XBox Reserved 07"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_08:
                {
                    sText <<  _T(", XBox Reserved 08"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                case DEVICEFAMILYDEVICEFORM_XBOX_RESERVED_09:
                {
                    sText <<  _T(", XBox Reserved 09"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        sBuf.str(_T(""));
        sBuf << _T(", ProductType:") << std::hex << osvi.dwProductType; //NOLINT(clang-diagnostic-format)
        sText << sBuf.str(); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsEnterpriseStorageServer(&osvi))
            sText <<  _T(", (Storage Server Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        else if (os.IsExpressStorageServer(&osvi))
            sText <<  _T(", (Storage Server Express)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        else if (os.IsStandardStorageServer(&osvi))
            sText <<  _T(", (Storage Server Standard)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        else if (os.IsWorkgroupStorageServer(&osvi))
            sText <<  _T(", (Storage Server Workgroup)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        else if (os.IsEssentialsStorageServer(&osvi))
            sText <<  _T(", (Storage Server Essentials)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        else if (os.IsStorageServer(&osvi))
            sText <<  _T(", (Storage Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

        if (os.IsHomeServerPremiumEdition(&osvi))
            sText <<  _T(", (Home Server Premium Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        else if (os.IsHomeServerEdition(&osvi))
            sText <<  _T(", (Home Server Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

        if (os.IsTerminalServices(&osvi))
            sText <<  _T(", (Terminal Services)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsEmbedded(&osvi))
            sText <<  _T(", (Embedded)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsTerminalServicesInRemoteAdminMode(&osvi))
            sText <<  _T(", (Terminal Services in Remote Admin Mode)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.Is64Bit(&osvi, TRUE))
            sText <<  _T(", (64 Bit Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsMediaCenter(&osvi))
            sText <<  _T(", (Media Center Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsTabletPC(&osvi))
            sText <<  _T(", (Tablet PC Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsComputeClusterServerEdition(&osvi))
            sText <<  _T(", (Compute Cluster Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsServerFoundation(&osvi))
            sText <<  _T(", (Foundation Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsMultipointServerPremiumEdition(&osvi))
            sText <<  _T(", (MultiPoint Premium Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        else if (os.IsMultiPointServer(&osvi))
            sText <<  _T(", (MultiPoint Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsSecurityAppliance(&osvi))
            sText <<  _T(", (Security Appliance)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsBackOffice(&osvi))
            sText <<  _T(", (BackOffice)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsNEdition(&osvi))
            sText <<  _T(", (N Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsEEdition(&osvi))
            sText <<  _T(", (E Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsHyperVTools(&osvi))
            sText <<  _T(", (Hyper-V Tools)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsHyperVServer(&osvi))
            sText <<  _T(", (Hyper-V Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsServerCore(&osvi))
            sText <<  _T(", (Server Core)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsUniprocessorFree(&osvi))
            sText <<  _T(", (Uniprocessor Free)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsUniprocessorChecked(&osvi))
            sText <<  _T(", (Uniprocessor Checked)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsMultiprocessorFree(&osvi))
            sText <<  _T(", (Multiprocessor Free)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsMultiprocessorChecked(&osvi))
            sText <<  _T(", (Multiprocessor Checked)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsEssentialBusinessServerManagement(&osvi))
            sText <<  _T(", (Windows Essential Business Server Manangement Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsEssentialBusinessServerMessaging(&osvi))
            sText <<  _T(", (Windows Essential Business Server Messaging Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsEssentialBusinessServerSecurity(&osvi))
            sText <<  _T(", (Windows Essential Business Server Security Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsClusterServer(&osvi))
            sText <<  _T(", (Cluster Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsSmallBusiness(&osvi))
            sText <<  _T(", (Small Business)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsRestricted(&osvi))
            sText <<  _T(", (Restricted)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsSmallBusinessServerPremium(&osvi))
            sText <<  _T(", (Small Business Server Premium)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsPreRelease(&osvi))
            sText <<  _T(", (Prerelease)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsEvaluation(&osvi))
            sText <<  _T(", (Evaluation)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsAutomotive(&osvi))
            sText <<  _T(", (Automotive)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsChina(&osvi))
            sText <<  _T(", (China)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsSingleLanguage(&osvi))
            sText <<  _T(", (Single Language)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsWin32sInstalled(&osvi))
            sText <<  _T(", (Win32s)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsEducation(&osvi))
            sText <<  _T(", (Education)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsIndustry(&osvi))
            sText <<  _T(", (Industry)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsStudent(&osvi))
            sText <<  _T(", (Student)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsMobile(&osvi))
            sText <<  _T(", (Mobile)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsCloudHostInfrastructureServer(&osvi))
            sText <<  _T(", (Cloud Host Infrastructure Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsLTSB(&osvi))
            sText <<  _T(", (LTSB)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsCloudStorageServer(&osvi))
            sText <<  _T(", (Cloud Storage Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsPPIPro(&osvi))
            sText <<  _T(", (PPI Pro)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsConnectedCar(&osvi))
            sText <<  _T(", (Connected Car)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsHandheld(&osvi))
            sText <<  _T(", (Handheld)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsHolographic(&osvi))
            sText <<  _T(", (Holographic)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsBusiness(&osvi))
            sText <<  _T(", (Business)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsSubscription(&osvi))
            sText <<  _T(", (Subscription)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsUtilityVM(&osvi))
            sText <<  _T(", (Utility VM)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsAzure(&osvi))
            sText <<  _T(", (Azure)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsEnterpriseG(&osvi))
            sText <<  _T(", (Enterprise G)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsServerRDSH(&osvi))
            sText <<  _T(", (ServerRRDSH)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsHubOS(&osvi))
            sText <<  _T(", (HubOS)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsCommunicationsServer(&osvi))
            sText <<  _T(", (Communications Server)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsOneCoreUpdateOS(&osvi))
            sText <<  _T(", (One Core Update OS)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsAndromeda(&osvi))
            sText <<  _T(", (Andromeda)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsIoTCommercial(&osvi))
            sText <<  _T(", (IoT Commercial)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsIoTCore(&osvi))
            sText <<  _T(", (IoT Core)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsIoTOS(&osvi))
            sText <<  _T(", (IoT OS)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsIoTEdgeOS(&osvi))
            sText <<  _T(", (IoT EdgeOS)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsIoTEnterprise(&osvi))
            sText <<  _T(", (IoT Enterprise)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsIoTEnterpriseSK(&osvi))
            sText <<  _T(", (IoT EnterpriseSK)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsXBoxSystemOS(&osvi))
            sText <<  _T(", (XBox SystemOS)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsXBoxNativeOS(&osvi))
            sText <<  _T(", (XBox NativeOS)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsXBoxGamesOS(&osvi))
            sText <<  _T(", (XBox GameOS)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsXBoxEraOS(&osvi))
            sText <<  _T(", (XBox EraOS)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsXBoxDurangoHostOS(&osvi))
            sText <<  _T(", (XBox DurangoHostOS)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsXBoxScarlettHostOS(&osvi))
            sText <<  _T(", (XBox ScarlettHostOS)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsXBoxKeystone(&osvi))
            sText <<  _T(", (XBox Keystone)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsAzureStackHCIServerCore(&osvi))
            sText <<  _T(", (Azure Stack HCI Server Core)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsDatacenterServerAzureEdition(&osvi))
            sText <<  _T(", (Datacenter Server Azure Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsDatacenterServerCoreAzureEdition(&osvi))
            sText <<  _T(", (Datacenter Server Core Azure Edition)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsAzureServerCloudhost(&osvi))
            sText <<  _T(", (Azure Server Cloud Host)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsAzureServerCloudMOS(&osvi))
            sText <<  _T(", (Azure Server Cloud MOS)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        if (os.IsWindows365(&osvi))
            sText <<  _T(", (Windows 365)"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

        sText <<  _T("\n"); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)

        //Some extra info for CE
#ifdef UNDER_CE
        if (osvi.UnderlyingPlatform == COSVersion::WindowsCE)
        {
            sText <<  _T("Model: "); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            sText <<  osvi.szOEMInfo); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            sText <<  _T("\nDevice Type: "); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
            sText <<  osvi.szPlatformType); //NOLINT(clang-analyzer-security.insecureAPI.strcpy)
        }
#endif //#ifdef UNDER_CE
    }
    else
        sText << _T("Failed in call to GetOSVersion\n");
    return sText.str();
}
