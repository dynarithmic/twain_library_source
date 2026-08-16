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
#include <sstream>
#include <array>

#include "ctltwainmanager.h"
#include "ctltwaindecoder.h"
#include "ctlglobalhandletraits.h"
#include "ctlstaticdata.h"
#include "dtwain_resource_constants2.h"
#include "ctlstringutils.h"
/*#define FMT_HEADER_ONLY
#include "../fmt/format.h"
*/
using namespace dynarithmic;
namespace stringutils = basicstringutils;

constexpr const char *indenter = "    ";

#define ADD_ERRORCODE_TO_MAP2(x, y) {(x) + (y), #y}

static constexpr std::array<std::pair<uint32_t, const char*>, 41> mapTwainDSMReturnCodes =
{ {
    ADD_ERRORCODE_TO_MAP2(IDS_TWRC_ERRORSTART, TWRC_SUCCESS),
    ADD_ERRORCODE_TO_MAP2(IDS_TWRC_ERRORSTART, TWRC_FAILURE),
    ADD_ERRORCODE_TO_MAP2(IDS_TWRC_ERRORSTART, TWRC_CHECKSTATUS),
    ADD_ERRORCODE_TO_MAP2(IDS_TWRC_ERRORSTART, TWRC_CANCEL),
    ADD_ERRORCODE_TO_MAP2(IDS_TWRC_ERRORSTART, TWRC_DSEVENT),
    ADD_ERRORCODE_TO_MAP2(IDS_TWRC_ERRORSTART, TWRC_NOTDSEVENT),
    ADD_ERRORCODE_TO_MAP2(IDS_TWRC_ERRORSTART, TWRC_XFERDONE),
    ADD_ERRORCODE_TO_MAP2(IDS_TWRC_ERRORSTART, TWRC_ENDOFLIST),
    ADD_ERRORCODE_TO_MAP2(IDS_TWRC_ERRORSTART, TWRC_INFONOTSUPPORTED),
    ADD_ERRORCODE_TO_MAP2(IDS_TWRC_ERRORSTART, TWRC_DATANOTAVAILABLE),
    ADD_ERRORCODE_TO_MAP2(IDS_TWRC_ERRORSTART, TWRC_BUSY),
    ADD_ERRORCODE_TO_MAP2(IDS_TWRC_ERRORSTART, TWRC_SCANNERLOCKED),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_SUCCESS),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_BUMMER),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_LOWMEMORY),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_NODS),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_MAXCONNECTIONS),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_OPERATIONERROR),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_BADCAP),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_BADPROTOCOL),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_BADVALUE),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_SEQERROR),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_BADDEST),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_CAPUNSUPPORTED),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_CAPBADOPERATION),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_CAPSEQERROR),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_DENIED),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_FILEEXISTS),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_FILENOTFOUND),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_NOTEMPTY),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_PAPERJAM),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_PAPERDOUBLEFEED),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_FILEWRITEERROR),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_CHECKDEVICEONLINE),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_INTERLOCK),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_DAMAGEDCORNER),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_FOCUSERROR),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_DOCTOOLIGHT),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_DOCTOODARK),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_NOMEDIA),
    ADD_ERRORCODE_TO_MAP2(IDS_TWCC_ERRORSTART, TWCC_DOCTOOLIGHT)
} };

static constexpr std::array<std::pair<uint32_t, const char*>, 6> mapSupportedGroups =
{ {
    ADD_ERRORCODE_TO_MAP2(0, DG_CONTROL),
    ADD_ERRORCODE_TO_MAP2(0, DG_IMAGE),
    ADD_ERRORCODE_TO_MAP2(0, DG_AUDIO),
    ADD_ERRORCODE_TO_MAP2(0, DF_DSM2),
    ADD_ERRORCODE_TO_MAP2(0, DF_APP2),
    ADD_ERRORCODE_TO_MAP2(0, DF_DS2)
} };

namespace
{
    std::string DecodeTWMemory(TW_MEMREF pData);   
    std::string DecodeSupportedGroups(TW_UINT32 SupportedGroups);
    std::string DecodeCustomDSData(TW_MEMREF pData);
    std::string DecodeSourceInfo(pTW_IDENTITY pIdentity, LPCSTR sPrefix, int indentLevel = 1);
    std::string DecodeTW_MEMORY(pTW_MEMORY pMemory, LPCSTR pMem);
    std::string DecodeTW_ELEMENT8(pTW_ELEMENT8 pEl, LPCSTR pMem);
    std::string DecodeDeviceEvent(TW_MEMREF pData);
    std::string DecodeTWEvent(TW_MEMREF pData); 
    std::string DecodeFileSystem(TW_MEMREF pData);
    std::string DecodeTWCapability(TW_MEMREF pData);
    std::string DecodeTWIdentity(TW_MEMREF pData);
    std::string DecodeUserInterface(TW_MEMREF pData); 
    std::string DecodeTWEntryPoint(TW_MEMREF pData);
    std::string DecodeTWImageLayout(TW_MEMREF pData);
    std::string DecodeTWImageInfo(TW_MEMREF pData);
    std::string DecodeHWND(TW_MEMREF pData);
    std::string DecodeTWPassthru(TW_MEMREF pData);
    std::string DecodePendingXFers(TW_MEMREF pData);
    std::string DecodeSetupFileXFer(TW_MEMREF pData);
    std::string DecodeSetupMemXFer(TW_MEMREF pData);
    std::string DecodeStatusUTF8(TW_MEMREF pData);
    std::string DecodeTWStatus(TW_MEMREF pData);
    std::string DecodeImageMemXfer(TW_MEMREF pData);
    std::string DecodeTWCIEColor(TW_MEMREF pData);
    std::string DecodeJPEGCompression(TW_MEMREF pData);
    std::string DecodeHDIB(TW_MEMREF pData);
    std::string DecodeTWPalette8(TW_MEMREF pData);
    std::string DecodeTWUINT32(TW_MEMREF pData);
    std::string DecodeTWGrayResponse(TW_MEMREF pData);
    std::string DecodeTWRGBResponse(TW_MEMREF pData);
    std::string DecodeExtImageInfo(TW_MEMREF pData);
    std::string DecodeTWUNKIdentity(TW_MEMREF pData);
    std::string DecodeTWAudioInfo(TW_MEMREF pData);
    std::string DecodeTWCallback(TW_MEMREF pData);
    std::string DecodeTWCallback2(TW_MEMREF pData);
    std::string DecodeTWMetrics(TW_MEMREF pData);
    std::string DecodeTwainDirect(TW_MEMREF pData);

    std::string DecodeSupportedGroups(TW_UINT32 SupportedGroups)
    {
        std::vector<std::string> allGroups;
        constexpr unsigned int numberOfBits = sizeof(TW_UINT32) << 3;
        for (unsigned int i = 0; i < numberOfBits; ++i)
        {
            const unsigned int curGroup = static_cast<TW_UINT32>(1) << i;
            if (SupportedGroups & curGroup)
            {
                auto it = generic_array_finder_if(mapSupportedGroups, [&](const auto& pr) { return pr.first == curGroup; });
                if (it.first)
                    allGroups.push_back(mapSupportedGroups[it.second].second);
                else
                    allGroups.push_back("Unknown (" + std::to_string(curGroup) + ")");
            }
        }
        return stringutils::Join<std::string>(allGroups, ", ");
    }

    std::string DecodeSourceInfo(pTW_IDENTITY pIdentity, LPCSTR sPrefix, int indentLevel)
    {
        if (pIdentity)
        {
            std::string indenter1;
            std::string indenter2;
            if ( indentLevel == 1 )
            {
                indenter1 = indenter;
            }
            else
            {
                for (int i = 0; i < indentLevel; ++i)
                    indenter1 += indenter;
                for (int i = 0; i < indentLevel - 1; ++i)
                    indenter2 += indenter;
            }

            std::ostringstream sBuffer;
            sBuffer << indenter2 << "Decoded " << sPrefix << ":\n" << indenter2 << "{\n" << indenter1 << 
                "Id=" << pIdentity->Id << "\n" << indenter1 <<
                "Version Number=" << pIdentity->Version.MajorNum << "." << pIdentity->Version.MinorNum << "\n" << indenter1 <<
                "Version Language=" << pIdentity->Version.Language << "\n" << indenter1 <<
                "Version Country=" << pIdentity->Version.Country << "\n" << indenter1 << 
                "Version Info=" << pIdentity->Version.Info << "\n" << indenter1 << 
                "ProtocolMajor=" << pIdentity->ProtocolMajor << "\n" << indenter1 <<
                "ProtocolMinor=" << pIdentity->ProtocolMinor << "\n" << indenter1 <<
                "SupportedGroups=" << DecodeSupportedGroups(pIdentity->SupportedGroups) << "\n" << indenter1 <<
                "Manufacturer=" << pIdentity->Manufacturer << "\n" << indenter1 <<
                "Product Family=" << pIdentity->ProductFamily << "\n" << indenter1 <<
                "Product Name=" << pIdentity->ProductName << "\n" << indenter2 << "}";
            return sBuffer.str();
        }
        return "\nNo information for " + std::string(sPrefix);
    }

    std::string DecodeTW_MEMORY(pTW_MEMORY pMemory, LPCSTR pMem)
    {
        std::ostringstream sBuffer;
        sBuffer << "{Flags=" <<
            pMemory->Flags << ", " <<
            "Length=" <<
            pMemory->Length << ", " <<
            "TheMem=" << basicstringutils::PointerToString<std::string>(pMemory->TheMem) << "}";
        std::string sTemp = sBuffer.str();
        if (pMem)
        {
            sTemp = pMem;
            sTemp += "=";
        }
        sTemp += sBuffer.str();
        return sTemp;
    }


    std::string DecodeTW_ELEMENT8(pTW_ELEMENT8 pEl, LPCSTR pMem)
    {
        std::ostringstream sBuffer;

        sBuffer << "{Index=" << static_cast<int>(pEl->Index) << ", " <<
            "Channel1=" << static_cast<int>(pEl->Channel1) << ", " <<
            "Channel2=" << static_cast<int>(pEl->Channel3) << "}";
        std::string sTemp = sBuffer.str();
        if (pMem)
        {
            std::string sTemp2 = pMem;
            sTemp2 += "=";
            sTemp = sTemp2 + sTemp;
        }
        return sTemp;
    }

    std::string DecodeCustomDSData(TW_MEMREF pData)
    {
        auto pCUSTOMDSDATA = static_cast<pTW_CUSTOMDSDATA>(pData);
        std::ostringstream strm;
        strm << "\nTW_MEMREF <==> TW_CUSTOMDSDATA:\n{\n" << indenter <<
            "InfoLength=" << pCUSTOMDSDATA->InfoLength << "\n" << indenter <<
            "hData=" << basicstringutils::PointerToString<std::string>(pCUSTOMDSDATA->hData) << "\n}";
        return strm.str();
    }

    std::string DecodeDeviceEvent(TW_MEMREF pData)
    {
        auto pDEVICEEVENT = static_cast<pTW_DEVICEEVENT>(pData);
        std::ostringstream sBuffer;
        sBuffer << "\nTW_MEMREF <==> TW_DEVICEEVENT:\n{\n" << indenter << 
            "Event=" << CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWDE, pDEVICEEVENT->Event).second << "\n" << indenter << 
            "DeviceName=" << pDEVICEEVENT->DeviceName << "\n" << indenter << 
            "BatteryMinutes=" << pDEVICEEVENT->BatteryMinutes << "\n" << indenter << 
            "BatteryPercentage=" << pDEVICEEVENT->BatteryPercentage << "\n" << indenter << 
            "PowerSupply=" << pDEVICEEVENT->PowerSupply << "\n" << indenter << 
            "XResolution=" << Fix32ToFloat(pDEVICEEVENT->XResolution) << "\n" << indenter << 
            "YResolution=" << Fix32ToFloat(pDEVICEEVENT->YResolution) << "\n" << indenter << 
            "FlashUsed2=" << pDEVICEEVENT->FlashUsed2 << "\n" << indenter << 
            "AutomaticCapture=" << pDEVICEEVENT->AutomaticCapture << "\n" << indenter << 
            "TimeBeforeFirstCapture=" << pDEVICEEVENT->TimeBeforeFirstCapture << "\n" << indenter << 
            "TimeBetweenCaptures=" << pDEVICEEVENT->TimeBetweenCaptures << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWEvent(TW_MEMREF pData)
    {
        auto logFilterFlags = CTL_StaticData::GetLogFilterFlags();
        if (logFilterFlags & DTWAIN_LOG_DECODE_TWEVENT)
        {
            std::ostringstream sBuffer;
            sBuffer << "\nTW_MEMREF <==> TW_EVENT:\n{\n" <<
                indenter << "pEvent has MSG structure:\n" <<
                indenter << "MSG Values\n" <<
                indenter << "{\n";

            auto p = static_cast<pTW_EVENT>(pData);
            if (!p || !p->pEvent )
            {
                sBuffer << indenter << indenter << "Invalid MSG (null)" << "\n" << indenter << "}\n}";
                return sBuffer.str();
            }
            MSG* pmsg = static_cast<MSG*>(p->pEvent);
            sBuffer << indenter << indenter << 
                    "hwnd=" << pmsg->hwnd << "\n" << indenter << indenter <<
                    "message=" << pmsg->message << "\n" << indenter << indenter <<
                    "wParam=" << pmsg->wParam << "\n" << indenter << indenter <<
                    "lParam=" << pmsg->lParam << "\n" << indenter << indenter <<
                    "time=" << pmsg->time << "\n" << indenter << indenter <<
                    "point.x=" << pmsg->pt.x << "\n" << indenter << indenter <<
                    "point.y=" << pmsg->pt.y << "\n" << indenter <<
                " }\n" <<
                indenter << "DS Message=" << p->TWMessage << "\n}";
            return sBuffer.str();
        }
        return {};
    }

    std::string DecodeFileSystem(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        auto pFILESYSTEM = static_cast<pTW_FILESYSTEM>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_FILESYSTEM:\n{\n" <<
            indenter << "InputName=" << pFILESYSTEM->InputName << "\n" <<
            indenter << "OutputName=" << pFILESYSTEM->OutputName << "\n" <<
            indenter << "Context=" << basicstringutils::PointerToString<std::string>(pFILESYSTEM->Context) << "\n" <<
            indenter << "Recursive=" << pFILESYSTEM->Recursive << "\n" <<
            indenter << "FileType=" << pFILESYSTEM->FileType << "\n" <<
            indenter << "Size=" << pFILESYSTEM->Size << "\n" <<
            indenter << "CreateTimeDate=" << pFILESYSTEM->CreateTimeDate << "\n" <<
            indenter << "ModifiedTimeDate=" << pFILESYSTEM->ModifiedTimeDate << "\n" <<
            indenter << "FreeSpace=" << pFILESYSTEM->FreeSpace << "\n" <<
            indenter << "NewImageSize=" << pFILESYSTEM->NewImageSize << "\n" <<
            indenter << "NumberOfFiles=" << pFILESYSTEM->NumberOfFiles << "\n" <<
            indenter << "NumberOfSnippets=" << pFILESYSTEM->NumberOfSnippets << "\n}\n";
        return sBuffer.str();
    }

    std::string DecodeTWCapability(TW_MEMREF pData)
    {
        
        std::ostringstream sBuffer;
        auto pCAPABILITY = static_cast<pTW_CAPABILITY>(pData);
        std::string s = "Unspecified (TWON_DONTCARE)";
        std::string container_type = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWON, pCAPABILITY->ConType).second;
        if (!container_type.empty())
            s = container_type;
        sBuffer << "\nTW_MEMREF <==> TW_CAPABILITY:\n{\n" <<
            indenter << "Cap=" << CTL_TwainAppMgr::GetCapNameFromCap(pCAPABILITY->Cap) << "\n" <<
            indenter << "ContainerType=" << s << "\n" <<
            indenter << "hContainer=" << basicstringutils::PointerToString<std::string>(pCAPABILITY->hContainer) << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWIdentity(TW_MEMREF pData)
    {
        return "\nTW_MEMREF <==> TW_IDENTITY:\n" + DecodeSourceInfo(static_cast<pTW_IDENTITY>(pData), "TW_MEMREF");
    }

    std::string DecodeTWMemory(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pMEMORY = static_cast<TW_MEMORY*>(pData);
        sBuffer <<
            "\nTW_MEMREF <==> TW_MEMORY:\n{\n" <<
            indenter << "Flags=" << pMEMORY->Flags << "\n" <<
            indenter << "Length=" << pMEMORY->Length << "\n" <<
            indenter << "TheMem=" << basicstringutils::PointerToString<std::string>(pMEMORY->TheMem) << "\n}";
        return sBuffer.str();
    }

    std::string DecodeUserInterface(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
#ifdef _WIN32
        
        auto pUSERINTERFACE = static_cast<pTW_USERINTERFACE>(pData);
        TCHAR sz[256];
        RECT r;
        SetRect(&r, 0, 0, 0, 0);
        std::array<LONG, 4> aRect = { r.left, r.top, r.right, r.bottom };
        sz[0] = _T('\0');
        sBuffer << "\nTW_MEMREF <==> TW_USERINTERFACE:\n{\n" <<
            indenter << "ShowUI=" << (pUSERINTERFACE->ShowUI ? "TRUE" : "FALSE") << "\n" <<
            indenter << "ModalUI=" << (pUSERINTERFACE->ModalUI ? "TRUE" : "FALSE") << "\n" <<
            indenter << "hParent=" << basicstringutils::PointerToString<std::string>(pUSERINTERFACE->hParent) << "\n" <<
            indenter << "hParent.Title=" << stringconversion::Convert_NativePtr_To_Ansi(sz) << "\n" <<
            indenter << "hParent.ScreenPos= {" << stringutils::Join<std::string>(aRect, ",") << "}\n}";
#endif
        return sBuffer.str();
    }

    std::string DecodeTWEntryPoint(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pENTRYPOINT = static_cast<TW_ENTRYPOINT*>(pData);
        sBuffer <<
            "\nTW_MEMREF <==> TW_ENTRYPOINT:\n{\n" <<
            indenter << "Size=" << pENTRYPOINT->Size << "\n" <<
            indenter << "DSMEntry=" << basicstringutils::PointerToString<std::string>(&pENTRYPOINT->DSM_Entry) << "\n" <<
            indenter << "DSMMemAllocate=" << basicstringutils::PointerToString<std::string>(&pENTRYPOINT->DSM_MemAllocate) << "\n" <<
            indenter << "DSMMemLock=" << basicstringutils::PointerToString<std::string>(&pENTRYPOINT->DSM_MemLock) << "\n" <<
            indenter << "DSMMemUnlock=" << basicstringutils::PointerToString<std::string>(&pENTRYPOINT->DSM_MemUnlock) << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWImageLayout(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pIMAGELAYOUT = static_cast<pTW_IMAGELAYOUT>(pData);
        sBuffer <<
            "\nTW_MEMREF <==> TW_IMAGELAYOUT:\n{\n" <<
            indenter << "Frame=" <<
            Fix32ToFloat(pIMAGELAYOUT->Frame.Left) << "," <<
            Fix32ToFloat(pIMAGELAYOUT->Frame.Top) << "-" <<
            Fix32ToFloat(pIMAGELAYOUT->Frame.Right) << "," <<
            Fix32ToFloat(pIMAGELAYOUT->Frame.Bottom) << "\n" <<
            indenter << "DocmentNumber=" << pIMAGELAYOUT->DocumentNumber << "\n" <<
            indenter << "PageNumber=" << pIMAGELAYOUT->PageNumber << "\n" <<
            indenter << "FrameNumber=" << pIMAGELAYOUT->FrameNumber << "\n}";
        return sBuffer.str();
    }


    std::string DecodeTWImageInfo(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pIMAGEINFO = static_cast<pTW_IMAGEINFO>(pData);
        auto allSamples = "{" + basicstringutils::Join<std::string>(pIMAGEINFO->BitsPerSample, pIMAGEINFO->BitsPerSample + 8, ",") + "}";
        sBuffer << "\nTW_MEMREF <==> TW_IMAGEINFO:\n{\n" <<
            indenter << "XResolution=" << Fix32ToFloat(pIMAGEINFO->XResolution) << "\n" <<
            indenter << "YResolution=" << Fix32ToFloat(pIMAGEINFO->YResolution) << "\n" <<
            indenter << "ImageWidth=" << pIMAGEINFO->ImageWidth << "\n" <<
            indenter << "ImageLength=" << pIMAGEINFO->ImageLength << "\n" <<
            indenter << "SamplesPerPixel=" << pIMAGEINFO->SamplesPerPixel << "\n" <<
            indenter << "BitsPerSample=" << allSamples << "\n" << 
            indenter << "BitsPerPixel=" << pIMAGEINFO->BitsPerPixel << "\n" <<
            indenter << "Planar=" << (pIMAGEINFO->Planar ? "TRUE" : "FALSE") << "\n" <<
            indenter << "PixelType=" << pIMAGEINFO->PixelType << "\n" <<
            indenter << "Compression=" << pIMAGEINFO->Compression << "\n}";
        return sBuffer.str();
    }

    std::string DecodeHWND(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
#ifdef _WIN32
        RECT r;
        HWND* pHWND = static_cast<HWND*>(pData);
        if (*pHWND && IsWindow(*pHWND))
        {
            GetWindowRect(*pHWND, &r);
            std::array<LONG, 4> aRect = { r.left, r.top, r.right, r.bottom };
            sBuffer <<
                "\nTW_MEMREF <==> handle to window (HWND):\n{\n" <<
                indenter << "HWND=" << *pHWND << "\n" <<
                indenter << "Screen Pos.=" << stringutils::Join<std::string>(aRect, ",") << "\n}";
        }
        else
        {
            sBuffer << "\nTW_MEMREF <==> handle to window (HWND):\n{\n" << indenter << "Null or invalid window handle\n}";
        }
#endif
        return sBuffer.str();
    }

    std::string DecodeTWPassthru(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pPASSTHRU = static_cast<pTW_PASSTHRU>(pData);
        sBuffer <<
            "\nTW_MEMREF <==> TW_PASSTHRU:\n{\n" <<
            indenter << "Command=" << basicstringutils::PointerToString<std::string>(pPASSTHRU->pCommand) << "\n" <<
            indenter << "CommandBytes=" << pPASSTHRU->CommandBytes << "\n" <<
            indenter << "Direction=" << pPASSTHRU->Direction << "\n" <<
            indenter << "pDataBuffer=" << basicstringutils::PointerToString<std::string>(pPASSTHRU->pData) << "\n" <<
            indenter << "DataBytes=" << pPASSTHRU->DataBytes << "\n" <<
            indenter << "DataBytesXfered=" << pPASSTHRU->DataBytesXfered << "\n}";
        return sBuffer.str();
    }

    std::string DecodePendingXFers(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pPENDINGXFERS = static_cast<pTW_PENDINGXFERS>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_PENDINGXFERS:\n{\n" <<
            indenter << "Count=" << pPENDINGXFERS->Count << "\n" <<
            indenter << "EOJ=" << pPENDINGXFERS->EOJ << "\n}";
        return sBuffer.str();
    }

    std::string DecodeSetupFileXFer(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pSETUPFILEXFER = static_cast<pTW_SETUPFILEXFER>(pData);
        sBuffer <<
        "\nTW_MEMREF <==> TW_SETUPFILEXFER:\n{\n" <<
        indenter << "FileName=" << pSETUPFILEXFER->FileName << "\n" <<
        indenter << "Format=" << pSETUPFILEXFER->Format << "\n" <<
        indenter << "VRefNum=" << pSETUPFILEXFER->VRefNum << "\n}";
        return sBuffer.str();
    }

    std::string DecodeSetupMemXFer(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pSETUPMEMXFER = static_cast<pTW_SETUPMEMXFER>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_SETUPMEMXFER:\n{\n" <<
                indenter << "MinBufSize=" << pSETUPMEMXFER->MinBufSize << "\n" <<
                indenter << "MaxBufSize=" << pSETUPMEMXFER->MaxBufSize << "\n" <<
                indenter << "Preferred=" << pSETUPMEMXFER->Preferred << "\n}";
        return sBuffer.str();
    }

    std::string DecodeStatusUTF8(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pSTATUSUTF8 = static_cast<pTW_STATUSUTF8>(pData);
        pTW_STATUS pStatus = &pSTATUSUTF8->Status;
        sBuffer << "\nTW_MEMREF <==> TW_STATUSUTF8:\n{\n" <<
            indenter << "Status ConditionCode=" << pStatus->ConditionCode << "\n" <<
            indenter << "Size=" << pSTATUSUTF8->Size << "\n" <<
            indenter << "UTF8string=" << basicstringutils::PointerToString<std::string>(pSTATUSUTF8->UTF8string) << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWStatus(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pSTATUS = static_cast<pTW_STATUS>(pData);
        std::string sConditionCode = "(Unknown)";
        uint32_t finderValue = IDS_TWCC_ERRORSTART + pSTATUS->ConditionCode;
        auto it = generic_array_finder_if(mapTwainDSMReturnCodes, [&](const auto& pr) { return pr.first == finderValue; });
        if (it.first)
            sConditionCode = std::string() + mapTwainDSMReturnCodes[it.second].second + "";
        sBuffer << "\nTW_MEMREF <==> TW_STATUS:\n{\n" <<
                indenter << "ConditionCode=" << pSTATUS->ConditionCode << "  (" << sConditionCode << ")\n}";
        return sBuffer.str();
    }

    std::string DecodeImageMemXfer(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pIMAGEMEMXFER = static_cast<pTW_IMAGEMEMXFER>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_IMAGEMEMXFER:\n{\n" <<
                    indenter << "Compression=" << pIMAGEMEMXFER->Compression << "\n" <<
                    indenter << "BytesPerRow=" << pIMAGEMEMXFER->BytesPerRow << "\n" <<
                    indenter << "Columns=" << pIMAGEMEMXFER->Columns << "\n" <<
                    indenter << "Rows=" << pIMAGEMEMXFER->Rows << "\n" <<
                    indenter << "XOffset=" << pIMAGEMEMXFER->XOffset << "\n" <<
                    indenter << "YOffset=" << pIMAGEMEMXFER->YOffset << "\n" <<
                    indenter << "BytesWritten=" << pIMAGEMEMXFER->BytesWritten << "\n" <<
                    indenter << DecodeTW_MEMORY(&pIMAGEMEMXFER->Memory,"Memory") << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWCIEColor(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        static constexpr std::array<const char *, 4> CIEPointNames = {"WhitePoint", "BlackPoint", "WhitePaper", "BlackInk"};
        auto pCIECOLOR = static_cast<pTW_CIECOLOR>(pData);
        std::array<pTW_CIEPOINT, CIEPointNames.size()> aPoints = {&pCIECOLOR->WhitePoint, &pCIECOLOR->BlackPoint, &pCIECOLOR->WhitePaper, &pCIECOLOR->BlackInk};
        
        sBuffer << "\nTW_MEMREF <==> TW_CIECOLOR:\n{\n" <<
                    indenter << "ColorSpace=" << pCIECOLOR->ColorSpace << "\n" <<
                    indenter << "LowEndian=" << pCIECOLOR->LowEndian << "\n" <<
                    indenter << "DeviceDependent=" << pCIECOLOR->DeviceDependent << "\n" <<
                    indenter << "VersionNumber=" << pCIECOLOR->VersionNumber << "\n\n";
        
        sBuffer << indenter << "Transform Stage Info:\n" << indenter << "{\n";

        pTW_TRANSFORMSTAGE pCurTransform{};
        size_t numDecodes = std::size(TW_TRANSFORMSTAGE().Decode);
        size_t numMixes = std::size(TW_TRANSFORMSTAGE().Mix[0]);
        for ( int nTransform = 0; nTransform < 2; nTransform++)
        {
            if ( nTransform == 0 )
                pCurTransform = &pCIECOLOR->StageABC;
            else
                pCurTransform = &pCIECOLOR->StageLMN;
            for ( size_t i = 0; i < numDecodes; i++ )
            {
                sBuffer << indenter << indenter << "Decode Value[" << i << "] =\n";
                sBuffer << indenter << indenter << "{\n" <<
                    indenter << indenter << indenter << "StartIn=" << Fix32ToFloat(pCurTransform->Decode[i].StartIn) << "\n" <<
                    indenter << indenter << indenter << "BreakIn=" << Fix32ToFloat(pCurTransform->Decode[i].BreakIn) << "\n" <<
                    indenter << indenter << indenter << "EndIn="   << Fix32ToFloat(pCurTransform->Decode[i].EndIn) << "\n" <<
                    indenter << indenter << indenter << "StartOut=" <<Fix32ToFloat(pCurTransform->Decode[i].StartOut) << "\n" <<
                    indenter << indenter << indenter << "BreakOut=" << Fix32ToFloat(pCurTransform->Decode[i].BreakOut) << "\n" <<
                    indenter << indenter << indenter << "EndOut=" << Fix32ToFloat(pCurTransform->Decode[i].EndOut) << "\n" <<
                    indenter << indenter << indenter << "Gamma=" << Fix32ToFloat(pCurTransform->Decode[i].Gamma) << "\n" <<
                    indenter << indenter << indenter << "SampleCount=" << Fix32ToFloat(pCurTransform->Decode[i].SampleCount) << "\n" <<
                    indenter << indenter << "}\n";
            }
            for ( size_t i = 0; i < numMixes; i++ )
            {
                for ( size_t j = 0; j < numMixes; j++ )
                {
                    sBuffer << indenter << indenter << "MixValue[" << i << "][" << j << "]=" <<
                            Fix32ToFloat(pCurTransform->Mix[i][j]) << "\n";
                }
            }
        }

        sBuffer << indenter << "}\n";
        // Get the CIE info
        for ( size_t i = 0; i < CIEPointNames.size(); i++ )
        {
            sBuffer << indenter << "CIEPoint " << CIEPointNames[i] << "={" <<
                       indenter <<  Fix32ToFloat(aPoints[i]->X) << "," <<
                       indenter << Fix32ToFloat(aPoints[i]->Y) << "," <<
                       indenter << Fix32ToFloat(aPoints[i]->Z) << "}\n";
        }

        sBuffer << "\nSample is user-defined and can't be determined \n}\n";
        return sBuffer.str();
    }

    std::string DecodeJPEGCompression(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pJPEGCOMPRESSION = static_cast<pTW_JPEGCOMPRESSION>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_JPEGCOMPRESSION:\n{\n" <<
                    indenter << "ColorSpace=" << pJPEGCOMPRESSION->ColorSpace << "\n" <<
                    indenter << "SubSampling=" << pJPEGCOMPRESSION->SubSampling << "\n" <<
                    indenter << "NumComponents=" << pJPEGCOMPRESSION->NumComponents << "\n" <<
                    indenter << "RestartFrequency=" << pJPEGCOMPRESSION->RestartFrequency << "\n" <<
                    indenter << "QuantMap={" <<
                    pJPEGCOMPRESSION->QuantMap[0] << "," <<
                    pJPEGCOMPRESSION->QuantMap[1] << "," <<
                    pJPEGCOMPRESSION->QuantMap[2] << "," <<
                    pJPEGCOMPRESSION->QuantMap[3] << "}\n" <<
                    indenter << DecodeTW_MEMORY(&pJPEGCOMPRESSION->QuantTable[0],"QuantTable[0]") <<
                    "\n" <<
                    indenter << DecodeTW_MEMORY(&pJPEGCOMPRESSION->QuantTable[1],"QuantTable[1]")<<
                    "\n" <<
                    indenter << DecodeTW_MEMORY(&pJPEGCOMPRESSION->QuantTable[2],"QuantTable[2]")<<
                    "\n" <<
                    indenter << DecodeTW_MEMORY(&pJPEGCOMPRESSION->QuantTable[3],"QuantTable[3]")<<
                    "\n" <<
                    indenter << "HuffmanMap={" <<
                    pJPEGCOMPRESSION->HuffmanMap[0] << "," <<
                    pJPEGCOMPRESSION->HuffmanMap[1] << "," <<
                    pJPEGCOMPRESSION->HuffmanMap[2] << "," <<
                    pJPEGCOMPRESSION->HuffmanMap[3] << "}\n" <<
                    indenter << DecodeTW_MEMORY(&pJPEGCOMPRESSION->HuffmanDC[0],"HuffmanDC[0]") <<
                    "\n" <<
                    indenter << DecodeTW_MEMORY(&pJPEGCOMPRESSION->HuffmanDC[1],"HuffmanDC[1]") <<
                    "\n" <<
                    indenter << DecodeTW_MEMORY(&pJPEGCOMPRESSION->HuffmanAC[0],"HuffmanAC[0]") <<
                    "\n" <<
                    indenter << DecodeTW_MEMORY(&pJPEGCOMPRESSION->HuffmanAC[1],"HuffmanAC[1]") <<
                    "\n}";
        return sBuffer.str();
    }

    std::string DecodeHDIB(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto handle = static_cast<HANDLE>(pData);
        sBuffer << "\nTW_MEMREF <==> a DIB:\n{\n" <<
                    indenter << "DIB Handle=" << basicstringutils::PointerToString<std::string>(handle) << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWPalette8(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pPALETTE8 = static_cast<pTW_PALETTE8>(pData);
        sBuffer << "\nTW_MEMREF <==> a TW_PALETTE8:\n{\n" <<
                    indenter << "NumColors=" << pPALETTE8->NumColors << "\n" <<
                    indenter << "PaletteType=" << pPALETTE8->PaletteType << "\n";
        for ( int i = 0; i < 256; i++ )
        {
            sBuffer << indenter << "ColorInfo[" << i << "]" <<
                    " - Index=" << static_cast<int>(pPALETTE8->Colors[i].Index) <<
                    ", Channel1=" << static_cast<int>(pPALETTE8->Colors[i].Channel1) <<
                    ", Channel2=" << static_cast<int>(pPALETTE8->Colors[i].Channel2) <<
                    ", Channel3=" << static_cast<int>(pPALETTE8->Colors[i].Channel3) << "\n";
        }
        sBuffer << "}\n";
        return sBuffer.str();
    }

    std::string DecodeTWUINT32(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        sBuffer << "\nTW_MEMREF <==> TW_UINT32 pointer:\n{\n" <<
                    indenter << "Address=" << basicstringutils::PointerToString<std::string>(pData) << "\n" <<
                    indenter << "Value at Address=" << *static_cast<TW_UINT32*>(pData) << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWGrayResponse(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        auto pGRAYRESPONSE = static_cast<pTW_GRAYRESPONSE>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_GRAYRESPONSE:\n{\n" << indenter <<
                    DecodeTW_ELEMENT8(&pGRAYRESPONSE->Response[0], "Response[0]") <<
                    "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWRGBResponse(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        auto pRGBRESPONSE = static_cast<pTW_RGBRESPONSE>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_RGBRESPONSE:\n{\n" << indenter << 
                    DecodeTW_ELEMENT8(&pRGBRESPONSE->Response[0], "Response[0]") <<
                    "\n}";
        return sBuffer.str();
    }

    std::string DecodeExtImageInfo(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        auto pEXTIMAGEINFO = static_cast<pTW_EXTIMAGEINFO>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_EXTIMAGINFO:\n{\n" << indenter << "NumInfos=" << pEXTIMAGEINFO->NumInfos << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWUNKIdentity(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pTWUNKIDENTITY = static_cast<pTW_TWUNKIDENTITY>(pData);
        pTW_IDENTITY pIdentity = &pTWUNKIDENTITY->identity;
        std::string dsPath = " ";
            dsPath = pTWUNKIDENTITY->dsPath;
        sBuffer << "\nTW_MEMREF <==> TW_TWUNKIDENTITY:\n{\n" << 
                    DecodeSourceInfo(pIdentity, "TW_TWUNKIDENTITY", 2) << "\n" <<
                    indenter << "dsPath=" << dsPath << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWAudioInfo(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pAUDIOINFO = static_cast<pTW_AUDIOINFO>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_AUDIOINFO:\n{\n" <<
            indenter << "Name=" << pAUDIOINFO->Name << "\n" <<
            indenter << "Reserved=" << pAUDIOINFO->Reserved << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWCallback(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pCALLBACK = static_cast<pTW_CALLBACK>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_CALLBACK:\n{\n";
        #if defined(__APPLE__)
            sBuffer << indenter << "Refcon=" << pCALLBACK->RefCon << "\n";
        #endif
            sBuffer << indenter << "Message=" << pCALLBACK->Message << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWCallback2(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pCALLBACK2 = static_cast<pTW_CALLBACK2>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_CALLBACK2:\n{\n";
        sBuffer << indenter << "CallbackProc=" << basicstringutils::PointerToString<std::string>(pCALLBACK2->CallBackProc) << "\n";
        sBuffer << indenter << "Refcon=" << pCALLBACK2->RefCon << "\n";
        sBuffer << indenter << "Message=" << pCALLBACK2->Message << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTWMetrics(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pMETRICS = static_cast<pTW_METRICS>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_METRICS:\n{\n";
        sBuffer << indenter << "SizeOf=" << pMETRICS->SizeOf << "\n";
        sBuffer << indenter << "ImageCount=" << pMETRICS->ImageCount << "\n";
        sBuffer << indenter << "SheetCount=" << pMETRICS->SheetCount << "\n}";
        return sBuffer.str();
    }

    std::string DecodeTwainDirect(TW_MEMREF pData)
    {
        std::ostringstream sBuffer;
        
        auto pTWAINDIRECT = static_cast<pTW_TWAINDIRECT>(pData);
        sBuffer << "\nTW_MEMREF <==> TW_TWAINDIRECT:\n{\n";
        sBuffer << indenter << "SizeOf=" << pTWAINDIRECT->SizeOf << "\n";
        sBuffer << indenter << "CommunicationManager=" << pTWAINDIRECT->CommunicationManager << "\n";
        sBuffer << indenter << "Send=" << basicstringutils::PointerToString<std::string>(pTWAINDIRECT->Send) << "\n";
        sBuffer << indenter << "SendSize=" << pTWAINDIRECT->SendSize << "\n";
        sBuffer << indenter << "Receive=" << basicstringutils::PointerToString<std::string>(pTWAINDIRECT->Receive) << "\n";
        sBuffer << indenter << "ReceiveSize=" << pTWAINDIRECT->ReceiveSize << "\n}";
        return sBuffer.str();
    }

    using TwainDecodeFn = std::function<std::string(TW_MEMREF)>;
    using MapTwainDecodeFn = std::map<DecoderStructTypes, TwainDecodeFn>;

    MapTwainDecodeFn mapTwainCodes = {
        {DECODERSTRUCT_TW_CUSTOMDSDATA, DecodeCustomDSData},
        {DECODERSTRUCT_TW_DEVICEEVENT, DecodeDeviceEvent},
        {DECODERSTRUCT_TW_EVENT, DecodeTWEvent},
        {DECODERSTRUCT_TW_FILESYSTEM, DecodeFileSystem},
        {DECODERSTRUCT_TW_CAPABILITY, DecodeTWCapability},
        {DECODERSTRUCT_TW_IDENTITY, DecodeTWIdentity},
        {DECODERSTRUCT_TW_MEMORY, DecodeTWMemory},
        {DECODERSTRUCT_TW_USERINTERFACE, DecodeUserInterface},
        {DECODERSTRUCT_TW_ENTRYPOINT, DecodeTWEntryPoint},
        {DECODERSTRUCT_TW_IMAGEINFO, DecodeTWImageInfo},
        {DECODERSTRUCT_TW_IMAGELAYOUT, DecodeTWImageLayout},
        {DECODERSTRUCT_LPHWND, DecodeHWND},
        {DECODERSTRUCT_TW_PASSTHRU, DecodeTWPassthru},
        {DECODERSTRUCT_TW_PENDINGXFERS, DecodePendingXFers},
        {DECODERSTRUCT_TW_SETUPFILEXFER, DecodeSetupFileXFer },
        {DECODERSTRUCT_TW_SETUPMEMXFER, DecodeSetupMemXFer} ,
        {DECODERSTRUCT_TW_STATUSUTF8, DecodeStatusUTF8} ,
        {DECODERSTRUCT_TW_STATUS, DecodeTWStatus} ,
        {DECODERSTRUCT_TW_IMAGEMEMXFER, DecodeImageMemXfer} ,
        {DECODERSTRUCT_TW_CIECOLOR, DecodeTWCIEColor} ,
        {DECODERSTRUCT_TW_JPEGCOMPRESSION, DecodeJPEGCompression},
        {DECODERSTRUCT_HDIB, DecodeHDIB} ,
        {DECODERSTRUCT_TW_PALETTE8, DecodeTWPalette8} ,
        {DECODERSTRUCT_pTW_UINT32, DecodeTWUINT32} ,
        {DECODERSTRUCT_TW_GRAYRESPONSE, DecodeTWGrayResponse} ,
        {DECODERSTRUCT_TW_RGBRESPONSE, DecodeTWRGBResponse} ,
        {DECODERSTRUCT_TW_EXTIMAGEINFO, DecodeExtImageInfo} ,
        {DECODERSTRUCT_TW_TWUNKIDENTITY, DecodeTWUNKIdentity} ,
        {DECODERSTRUCT_TW_AUDIOINFO, DecodeTWAudioInfo} ,
        {DECODERSTRUCT_TW_CALLBACK, DecodeTWCallback} ,
        {DECODERSTRUCT_TW_CALLBACK2, DecodeTWCallback2} ,
        {DECODERSTRUCT_TW_METRICS, DecodeTWMetrics} ,
        {DECODERSTRUCT_TW_TWAINDIRECT, DecodeTwainDirect} };
        
    std::string DecodeData(CTL_TWAINTypeDecoder* pDecoder, TW_MEMREF pData, DecoderStructTypes sType)
    {
        if ( !pData || sType == DECODERSTRUCT_NONE)
            return "\nNo TW_MEMREF Data";
        else
        {
            auto mapIter = mapTwainCodes.find(sType);
            if (mapIter != mapTwainCodes.end())
                return mapIter->second(pData);
        }
        return {};
/*            switch (sType)
            {
                case DECODERSTRUCT_TW_TWUNKDSENTRYPARAMS: break;
                case DECODERSTRUCT_pWAV: break;
                case DECODERSTRUCT_DTWAIN_MESSAGE: break;
                default: ;
            }
        }
        sTemp = sBuffer.str();
        return sTemp;*/
    }
}

void CTL_TWAINTypeDecoder::StartMessageDecoder(HWND hWnd, UINT nMsg,
                                                 WPARAM wParam, LPARAM lParam)
{
    std::ostringstream sBuffer;
    m_pString.clear();
    auto notification_name = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_DTWAIN_TN, static_cast<int>(wParam)).second;
    if ( !notification_name.empty() )
        sBuffer << "DTWAIN Message(HWND=" << hWnd << ", " <<
                                    "MSG=" << nMsg << ", " <<
                                    "Notification code=" << notification_name << ", " <<
                                    "LPARAM=" << lParam << ")";
    else
        sBuffer << "DTWAIN Message(HWND=" << hWnd << ", " <<
                                    "MSG=" << nMsg << ", " <<
                                    "Notification code=" << wParam << ", " <<
                                    "LPARAM=" << lParam << ")";
    m_pString = sBuffer.str();
}

void CTL_TWAINTypeDecoder::StartDecoder(pTW_IDENTITY pSource, pTW_IDENTITY pDest,
                                         TW_UINT32 nDG, TW_UINT16 nDAT, TW_UINT16 nMSG, TW_MEMREF Data,
                                         DecoderStructTypes sType)
{
    std::ostringstream sBuffer;

    m_pString.clear();
    std::string s1;
    auto sDG = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_DG, nDG).second;
    auto sDAT = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_DAT, nDAT).second;
    auto sMSG = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_MSG, nMSG).second;

    sBuffer << "DSM_Entry(pSource=0x" << std::hex << pSource << ", " <<
        "pDest=0x" << std::hex << pDest << ", " << sDG << ", " << sDAT << ", " << sMSG << ", " <<
        "TW_MEMREF=0x" << std::hex << Data << ") " << GetResourceStringFromMap(IDS_LOGMSG_CALLEDTEXT) << "\n";

    s1 = sBuffer.str();

    std::string pSourceStr;
    std::string pDestStr;
    std::string pMemRefStr;

    // Decode the pSource argument
    auto logFilterFlags = CTL_StaticData::GetLogFilterFlags();
    if ( nDG == DG_CONTROL && nDAT == DAT_EVENT && nMSG == MSG_PROCESSEVENT )
    {
        if (!(logFilterFlags & DTWAIN_LOG_DECODE_TWEVENT) )
            return;
    }
    if ( logFilterFlags & DTWAIN_LOG_DECODE_SOURCE )
    {
        pSourceStr = DecodeSourceInfo(pSource, "pSource");
        pSourceStr += "\n";
    }

    // Decode the pDest argument
    if ( logFilterFlags & DTWAIN_LOG_DECODE_DEST)
    {
        pDestStr   = DecodeSourceInfo(pDest, "pDest");
        pDestStr += "\n";
    }

    // Decode the TW_MEMREF structure
    if ( logFilterFlags & DTWAIN_LOG_DECODE_TWMEMREF)
        pMemRefStr = DecodeData(this, Data, sType);

    m_pString = s1 + pSourceStr;
    m_pString += pDestStr + pMemRefStr;
}

std::string CTL_TWAINTypeDecoder::DecodeBitmap(HANDLE hBitmap)
{
    std::ostringstream sBuffer;
    if ( !hBitmap )
        return "\n(null bitmap)\n\n";
    const auto pbi = static_cast<LPBITMAPINFOHEADER>(ImageMemoryHandler::GlobalLock(hBitmap));
    DTWAINGlobalHandle_RAII dibHandle(hBitmap);
    sBuffer << "\nHandle=" << hBitmap << "\n" <<
            "biSize=" << pbi->biSize << "\n" <<
            "biWidth=" << pbi->biWidth << "\n" <<
            "biHeight=" << pbi->biHeight << "\n" <<
            "biPlanes=" << pbi->biPlanes << "\n" <<
            "biBitCount=" << pbi->biBitCount << "\n" <<
            "biCompression=" << pbi->biCompression << "\n" <<
            "biSizeImage=" << pbi->biSizeImage << "\n" <<
            "biXPelsPerMeter=" << pbi->biXPelsPerMeter << "\n" <<
            "biYPelsPerMeter=" << pbi->biYPelsPerMeter << "\n" <<
            "biClrUsed=" << pbi->biClrUsed << "\n" <<
            "biClrImportant=" << pbi->biClrImportant << "\n";
    return sBuffer.str();
}

std::string CTL_TWAINTypeDecoder::DecodePDFTextElement(PDFTextElement *pEl)
{
    std::ostringstream sBuffer;
    if ( !pEl )
        return "\n(null PDF Text Element)\n\n";

    // PDFTextHandle
    const std::string indent(42, ' ');
    sBuffer << indent << "text=\"" << pEl->m_text << "\"\n";
    sBuffer << indent <<  "(xpos,ypos)=" << pEl->xpos << "," << pEl->ypos << "\n";
    sBuffer <<  indent << "(scalex,scaley)=" << pEl->scalingX << "," << pEl->scalingY << "\n";
    sBuffer <<  indent << "generalScaling=" << pEl->scaling << "\n";
    sBuffer <<  indent << "font=" << pEl->m_font.m_fontName << "\n";
    sBuffer <<  indent << "fontSize=" << pEl->fontSize << "\n";
    const int r = GetRValue(pEl->colorRGB);
    const int g = GetBValue(pEl->colorRGB);
    const int b = GetGValue(pEl->colorRGB);
    sBuffer <<  indent << "RGBValue=" << r << "," << g <<"," << b << "\n";
    sBuffer <<  indent << "charSpacing=" << pEl->charSpacing << "\n";
    sBuffer <<  indent << "wordSpacing=" << pEl->wordSpacing << "\n";
    sBuffer <<  indent << "strokeWidth=" << pEl->strokeWidth << "\n";
    sBuffer <<  indent << "renderMode=" << pEl->renderMode << "\n\n";
    return sBuffer.str();
}

std::string CTL_TWAINTypeDecoder::DecodeTWAINReturnCode(TW_UINT16 retCode)
{
    return DecodeTWAINCode(retCode, IDS_TWRC_ERRORSTART,
                           GetErrorString_Internal(DTWAIN_ERR_UNKNOWN_TWAIN_RC));
}

std::string CTL_TWAINTypeDecoder::DecodeTWAINReturnCodeCC(TW_UINT16 retCode)
{
    return DecodeTWAINCode(retCode, IDS_TWCC_ERRORSTART,
                           GetErrorString_Internal(DTWAIN_ERR_UNKNOWN_TWAIN_CC));
}

std::string CTL_TWAINTypeDecoder::DecodeTWAINCode(TW_UINT16 retCode, TW_UINT16 errStart, std::string_view defMessage)
{
    const TW_UINT16 actualCode = retCode + errStart;
    const auto it = generic_array_finder_if(mapTwainDSMReturnCodes, [&](const auto& pr) { return pr.first == actualCode; });
    if (it.first)
        return mapTwainDSMReturnCodes[it.second].second;
    return defMessage.data();
}

std::string CTL_TWAINTypeDecoder::DecodeTW_INFO(pTW_INFO pInfo, LPCSTR pMem)
{
    std::ostringstream sBuffer;
    sBuffer << "{InfoId=" << CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWEI, pInfo->InfoID).second 
                << ", " <<
               "ItemType=" << CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWTY, pInfo->ItemType).second 
                << ", " <<
               "NumItems=" << pInfo->NumItems << ", " <<
               "ReturnCode=" << pInfo->ReturnCode << ", " <<
               "Item=" << pInfo->Item << "}";
    std::string sTemp = sBuffer.str();
    if ( pMem )
    {
        std::string sTemp2 = pMem;
        sTemp2 += "=";
        sTemp = sTemp2 + sTemp;
    }
    return sTemp;
}

std::string CTL_TWAINTypeDecoder::DecodeCapOperations(LONG nOps)
{
    std::string sReturn;

    static constexpr std::array<std::string_view, 6> sAllOps =
        { "TWQC_GET", "TWQC_GETCURRENT", "TWQC_GETDEFAULT", "TWQC_SET", "TWQC_RESET", "TWQC_SETCONSTRAINT" };

    static constexpr std::array<TW_UINT16, 6> nAllOps = 
        { TWQC_GET, TWQC_GETCURRENT, TWQC_GETDEFAULT, TWQC_SET, TWQC_RESET, TWQC_SETCONSTRAINT };

    std::vector<std::string> vOps;
    for (size_t i = 0; i < nAllOps.size(); ++i)
    {
        if (nOps & nAllOps[i])
            vOps.push_back(sAllOps[i].data());
    }
    return stringutils::Join<std::string>(vOps, " | ");
}
