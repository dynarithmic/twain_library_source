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
#include <boost/format.hpp>
#include <sstream>
#include <array>
#include "ctliface.h"
#include "ctltr010.h"
#include "ctltwainmanager.h"
#include "errstruc.h"
#include "dtwain_resource_constants.h"
#include "twainfix32.h"

using namespace dynarithmic;

static std::string DecodeSourceInfo(pTW_IDENTITY pIdentity, LPCSTR sPrefix);
static std::string DecodeData(CTL_ErrorStructDecoder *pDecoder, TW_MEMREF pData, ErrorStructTypes sType);
static std::string DecodeTW_MEMORY(pTW_MEMORY pMemory, LPCSTR pMem);
static std::string DecodeTW_ELEMENT8(pTW_ELEMENT8 pEl, LPCSTR pMem);
static std::string DecodeTW_INFO(pTW_INFO pInfo, LPCSTR pMem);
static std::string DecodeSupportedGroups(TW_UINT32 SupportedGroups);
static std::string IndentDefinition() { return std::string(4, ' '); }

#define ADD_ERRORCODE_TO_MAP(theMap, start, x) theMap[(start) + x] = #x;
#define ADD_ERRORCODE_TO_MAP2(x, y) {x + y, #y}

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

static constexpr std::array<std::pair<uint32_t, const char*>, 62> mapNotificationType =
{ {
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_ACQUIREDONE),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_ACQUIREFAILED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_ACQUIRECANCELLED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_ACQUIRESTARTED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_PAGECONTINUE),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_PAGEFAILED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_PAGECANCELLED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_TRANSFERREADY),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_TRANSFERDONE),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_ACQUIREPAGEDONE),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_UICLOSING),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_UICLOSED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_UIOPENED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_CLIPTRANSFERDONE),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_INVALIDIMAGEFORMAT),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_ACQUIRETERMINATED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_TRANSFERSTRIPREADY),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_TRANSFERSTRIPDONE),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_IMAGEINFOERROR),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_DEVICEEVENT),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_FILESAVECANCELLED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_FILESAVEOK),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_FILESAVEERROR),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_FILEPAGESAVEOK),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_FILEPAGESAVEERROR),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_PROCESSEDDIB),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_PROCESSDIBACCEPTED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_PROCESSDIBFINALACCEPTED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_TRANSFERSTRIPFAILED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_IMAGEINFOERROR),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_TRANSFERCANCELLED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_UIOPENING),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_MANDUPFLIPPAGES),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_MANDUPSIDE1DONE),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_MANDUPSIDE2DONE),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_MANDUPPAGECOUNTERROR),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_MANDUPACQUIREDONE),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_MANDUPSIDE1START),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_MANDUPSIDE2START),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_MANDUPMERGEERROR),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_MANDUPMEMORYERROR),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_MANDUPFILEERROR),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_MANDUPFILESAVEERROR),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_ENDOFJOBDETECTED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_EOJDETECTED_XFERDONE),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_TWAINPAGECANCELLED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_TWAINPAGEFAILED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_QUERYPAGEDISCARD),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_PAGEDISCARDED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_APPUPDATEDDIB),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_FILEPAGESAVING),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_CROPFAILED),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_PROCESSEDDIBFINAL),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_BLANKPAGEDETECTED1),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_BLANKPAGEDETECTED2),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_BLANKPAGEDETECTED3),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_BLANKPAGEDISCARDED1),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_BLANKPAGEDISCARDED2),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_MESSAGELOOPERROR),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_SETUPMODALACQUISITION),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_TWAINTRIPLETBEGIN),
    ADD_ERRORCODE_TO_MAP2(0, DTWAIN_TN_TWAINTRIPLETEND)
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

static constexpr std::array<std::pair<uint32_t, const char*>, 4> mapContainerType =
{ {
    {TWON_ARRAY, "TW_ARRAY"},
    {TWON_ENUMERATION, "TW_ENUMERATION"},
    {TWON_ONEVALUE, "TW_ONEVALUE"},
    {TWON_RANGE, "TW_RANGE"}
} };

void CTL_ErrorStructDecoder::StartMessageDecoder(HWND hWnd, UINT nMsg,
                                                 WPARAM wParam, LPARAM lParam)
{
    StringStreamA sBuffer;

    m_pString.clear();
    auto it = dynarithmic::generic_array_finder_if(mapNotificationType, [&](const auto& pr) { return pr.first == wParam; });
    if ( it.first )
        sBuffer << "DTWAIN Message(HWND = " << hWnd << ", " <<
                                    "MSG = " << nMsg << ", " <<
                                    "Notification code = " << mapNotificationType[it.second].second << ", " <<
                                    "LPARAM = " << lParam;
    else
        sBuffer << "DTWAIN Message(HWND = " << hWnd << ", " <<
                                    "MSG = " << nMsg << ", " <<
                                    "Notification code = " << wParam << ", " <<
                                    "LPARAM = " << lParam;
    m_pString = sBuffer.str();
}

void CTL_ErrorStructDecoder::StartDecoder(pTW_IDENTITY pSource, pTW_IDENTITY pDest,
                                         LONG nDG, UINT nDAT, UINT nMSG, TW_MEMREF Data,
                                         ErrorStructTypes sType)
{
    StringStreamA sBuffer;

    m_pString.clear();
    std::string s1;
    auto sDG = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_DG, nDG);
    auto sDAT = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_DAT, nDAT);
    auto sMSG = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_MSG, nMSG);

    sBuffer << "DSM_Entry(pSource=" << pSource << "H, " <<
                "pDest=" << pDest << "H, " << sDG << ", " << sDAT << ", " << sMSG << ", " <<
                "TW_MEMREF=" << Data << "H) " << GetResourceStringFromMap(IDS_LOGMSG_CALLEDTEXT) << "\n";
    s1 = sBuffer.str();

    std::string pSourceStr;
    std::string pDestStr;
    std::string pMemRefStr;

    // Decode the pSource argument
    long lErrorFilter = CTL_StaticData::GetErrorFilterFlags();
    if ( nDG == DG_CONTROL && nDAT == DAT_EVENT && nMSG == MSG_PROCESSEVENT )
    {
        if (!(lErrorFilter & DTWAIN_LOG_DECODE_TWEVENT) )
            return;
    }
    if ( lErrorFilter & DTWAIN_LOG_DECODE_SOURCE )
    {
        pSourceStr = DecodeSourceInfo(pSource, "pSource");
        pSourceStr += "\n";
    }

    // Decode the pDest argument
    if ( lErrorFilter & DTWAIN_LOG_DECODE_DEST)
    {
        pDestStr   = DecodeSourceInfo(pDest, "pDest");
        pDestStr += "\n";
    }

    // Decode the TW_MEMREF structure
    if ( lErrorFilter & DTWAIN_LOG_DECODE_TWMEMREF)
        pMemRefStr = DecodeData(this, Data, sType);

    m_pString = s1 + pSourceStr;
    m_pString += pDestStr + pMemRefStr;
}

std::string CTL_ErrorStructDecoder::DecodeBitmap(HANDLE hBitmap)
{
    StringStreamA sBuffer;
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
            "biClrImportant=" << pbi->biClrImportant << "\n\n";
    return sBuffer.str();
}

std::string CTL_ErrorStructDecoder::DecodePDFTextElement(PDFTextElement *pEl)
{
    StringStreamA sBuffer;
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


std::string CTL_ErrorStructDecoder::DecodeTWAINReturnCode(TW_UINT16 retCode)
{
    return DecodeTWAINCode(retCode, IDS_TWRC_ERRORSTART,  "Unknown TWAIN Return Code");
}

std::string CTL_ErrorStructDecoder::DecodeTWAINReturnCodeCC(TW_UINT16 retCode)
{
    return DecodeTWAINCode(retCode, IDS_TWCC_ERRORSTART, "Unknown TWAIN Condition Code");
}

std::string CTL_ErrorStructDecoder::DecodeTWAINCode(TW_UINT16 retCode, TW_UINT16 errStart, const std::string& defMessage)
{
    const TW_UINT16 actualCode = retCode + errStart;
    const auto it = dynarithmic::generic_array_finder_if(mapTwainDSMReturnCodes, [&](const auto& pr) { return pr.first == actualCode; });
    if (it.first)
        return mapTwainDSMReturnCodes[it.second].second;
    return defMessage;
}

std::string DecodeData(CTL_ErrorStructDecoder* pDecoder, TW_MEMREF pData, ErrorStructTypes sType)
{
    StringStreamA sBuffer;
    std::string sTemp;
    std::string indenter = IndentDefinition();
    if ( !pData )
        sBuffer << "\nNo TW_MEMREF Data";
    else
    {
        switch (sType)
        {
            case ERRSTRUCT_NONE:
                sBuffer << "\nNo TW_MEMREF Data";
            break;

            case ERRSTRUCT_TW_CUSTOMDSDATA:
            {
                auto pCUSTOMDSDATA = static_cast<pTW_CUSTOMDSDATA>(pData);
                sBuffer << "\nTW_MEMREF is TW_CUSTOMDATA:\n{\n" <<
                            "InfoLength=" << pCUSTOMDSDATA->InfoLength << "\n" <<
                            "hData=" << pCUSTOMDSDATA->hData << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_DEVICEEVENT:
            {
                auto pDEVICEEVENT = static_cast<pTW_DEVICEEVENT>(pData);
                sBuffer << "\nTW_MEMREF is TW_DEVICEEVENT:\n{\n" <<
                            "Event=" << pDEVICEEVENT->Event << "\n" <<
                            "DeviceName=" << pDEVICEEVENT->DeviceName << "\n" <<
                            "BatteryMinutes=" << pDEVICEEVENT->BatteryMinutes << "\n" <<
                            "BatteryPercentage=" << pDEVICEEVENT->BatteryPercentage << "\n" <<
                            "PowerSupply=" << pDEVICEEVENT->PowerSupply << "\n" <<
                            "XResolution=" << dynarithmic::Fix32ToFloat(pDEVICEEVENT->XResolution) << "\n" <<
                            "YResolution=" << dynarithmic::Fix32ToFloat(pDEVICEEVENT->YResolution) << "\n" <<
                            "FlashUsed2=" << pDEVICEEVENT->FlashUsed2 << "\n" <<
                            "AutomaticCapture=" << pDEVICEEVENT->AutomaticCapture << "\n" <<
                            "TimeBeforeFirstCapture=" << pDEVICEEVENT->TimeBeforeFirstCapture << "\n" <<
                            "TimeBetweenCaptures=" << pDEVICEEVENT->TimeBetweenCaptures << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_EVENT:
            {
                LONG lErrorFlags = CTL_StaticData::GetErrorFilterFlags();
                if ( lErrorFlags & DTWAIN_LOG_DECODE_TWEVENT )
                {
                    auto p = static_cast<pTW_EVENT>(pData);
                    MSG *pmsg = static_cast<MSG*>(p->pEvent);
                    sBuffer << "\nTW_MEMREF is TW_EVENT:\n{\n" <<
                                indenter << "pEvent has MSG structure:\n" <<
                                indenter << "MSG Values\n" <<
                                indenter << "{"<<
                                " hwnd=" <<  pmsg->hwnd <<
                                ", message=" << pmsg->message <<
                                ", wParam=" << pmsg->wParam <<
                                ", lParam=" << pmsg->lParam <<
                                ", time=" << pmsg->time <<
                                ", point.x=" << pmsg->pt.x <<
                                ", point.y=" << pmsg->pt.y <<
                                " }\n" <<
                                indenter << "DS Message=" << p->TWMessage << "\n}\n";
                }
            }
            break;

            case ERRSTRUCT_TW_FILESYSTEM:
            {
                auto pFILESYSTEM = static_cast<pTW_FILESYSTEM>(pData);
                sBuffer << "\nTW_MEMREF is TW_FILESYSTEM:\n{\n" <<
                        indenter << "InputName=" << pFILESYSTEM->InputName << "\n" <<
                        indenter << "OutputName=" << pFILESYSTEM->OutputName << "\n" <<
                        indenter << "Context=" << pFILESYSTEM->Context << "H\n" <<
                        indenter << "Recursive=" << pFILESYSTEM->Recursive << "\n" <<
                        indenter << "FileType=" << pFILESYSTEM->FileType << "\n" <<
                        indenter << "Size=" << pFILESYSTEM->Size << "\n" <<
                        indenter << "CreateTimeDate=" << pFILESYSTEM->CreateTimeDate << "\n" <<
                        indenter << "ModifiedTimeDate=" << pFILESYSTEM->ModifiedTimeDate << "\n" <<
                        indenter << "FreeSpace=" << pFILESYSTEM->FreeSpace << "\n" <<
                        indenter << "NewImageSize=" << pFILESYSTEM->NewImageSize << "\n" <<
                        indenter << "NumberOfFiles=" << pFILESYSTEM->NumberOfFiles << "\n" <<
                        indenter << "NumberOfSnippets=" << pFILESYSTEM->NumberOfSnippets << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_IDENTITY:
            {
                sBuffer << "\nTW_MEMREF is TW_IDENTITY:\n" << DecodeSourceInfo(static_cast<pTW_IDENTITY>(pData), "TW_MEMREF");
            }
            break;

            case ERRSTRUCT_TW_MEMORY:
            {
                auto pMEMORY = static_cast<TW_MEMORY*>(pData);
                sBuffer <<
                    "\nTW_MEMREF is TW_MEMORY:\n{\n" <<
                    indenter << "Flags=" << pMEMORY->Flags<< "\n" <<
                    indenter << "Length=" << pMEMORY->Length << "\n" <<
                    indenter << "TheMem=" << pMEMORY->TheMem << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_ENTRYPOINT:
            {
                auto pENTRYPOINT = static_cast<TW_ENTRYPOINT*>(pData);
                sBuffer <<
                    "\nTW_MEMREF is TW_ENTRYPOINT:\n{\n" <<
                    indenter << "Size=" << pENTRYPOINT->Size << "\n" <<
                    indenter << "DSMEntry=" << &pENTRYPOINT->DSM_Entry << "\n" <<
                    indenter << "DSMMemAllocate=" << &pENTRYPOINT->DSM_MemAllocate << "\n" <<
                    indenter << "DSMMemLock=" << &pENTRYPOINT->DSM_MemLock << "\n" <<
                    indenter << "DSMMemUnlock=" << &pENTRYPOINT->DSM_MemUnlock << "\n}\n";
            }
            break;

            case ERRSTRUCT_LPHWND:
            {
            #ifdef _WIN32
                RECT r;
                HWND *pHWND = static_cast<HWND*>(pData);
                GetWindowRect(*pHWND, &r);
                std::array<LONG, 4> aRect = { r.left, r.top, r.right, r.bottom };
                sBuffer <<
                "\nTW_MEMREF is handle to window (HWND):\n{\n" <<
                indenter << "HWND=" << *pHWND << "\n" <<
                indenter << "Screen Pos.= " << StringWrapperA::Join(aRect, ",") << "\n}\n";
            #endif
            }
            break;

            case ERRSTRUCT_TW_PASSTHRU:
            {
                auto pPASSTHRU = static_cast<pTW_PASSTHRU>(pData);
                sBuffer <<
                "\nTW_MEMREF is TW_PASSTHRU:\n{\n" <<
                indenter << "Command=" << pPASSTHRU->pCommand << "H\n" <<
                indenter << "CommandBytes=" << pPASSTHRU->CommandBytes << "\n" <<
                indenter << "Direction=" << pPASSTHRU->Direction << "\n" <<
                indenter << "pDataBuffer=" << pPASSTHRU->pData << "H\n" <<
                indenter << "DataBytes=" << pPASSTHRU->DataBytes << "\n" <<
                indenter << "DataBytesXfered=" << pPASSTHRU->DataBytesXfered << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_PENDINGXFERS:
            {
                auto pPENDINGXFERS = static_cast<pTW_PENDINGXFERS>(pData);
                sBuffer << "\nTW_MEMREF is TW_PENDINGXFERS:\n{\n" <<
                            indenter << "Count=" << pPENDINGXFERS->Count << "\n" <<
                            indenter << "EOJ=" << pPENDINGXFERS->EOJ << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_SETUPFILEXFER:
            {
                auto pSETUPFILEXFER = static_cast<pTW_SETUPFILEXFER>(pData);
                sBuffer <<
                "\nTW_MEMREF is TW_SETUPFILEXFER:\n{\n" <<
                indenter << "FileName=" << pSETUPFILEXFER->FileName << "\n" <<
                indenter << "Format=" << pSETUPFILEXFER->Format << "\n" <<
                indenter << "VRefNum=" << pSETUPFILEXFER->VRefNum << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_SETUPMEMXFER:
            {
                auto pSETUPMEMXFER = static_cast<pTW_SETUPMEMXFER>(pData);
                sBuffer << "\nTW_MEMREF is TW_SETUPMEMXFER:\n{\n" <<
                        indenter << "MinBufSize=" << pSETUPMEMXFER->MinBufSize << "\n" <<
                        indenter << "MaxBufSize=" << pSETUPMEMXFER->MaxBufSize << "\n" <<
                        indenter << "Preferred=" << pSETUPMEMXFER->Preferred << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_CAPABILITY:
            {
                auto pCAPABILITY = static_cast<pTW_CAPABILITY>(pData);
                auto it = dynarithmic::generic_array_finder_if(mapContainerType, [&](const auto& pr) { return pr.first == pCAPABILITY->ConType; });
                std::string s = "Unspecified (TWON_DONTCARE)";
                if (it.first)
                    s = mapContainerType[it.second].second;

                sBuffer << "\nTW_MEMREF is TW_CAPABILITY:\n{\n" <<
                        indenter << "Cap=" << CTL_TwainAppMgr::GetCapNameFromCap(pCAPABILITY->Cap) << "\n" <<
                        indenter << "ContainerType=" << s << "\n" <<
                        indenter << "hContainer=" << pCAPABILITY->hContainer << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_STATUSUTF8:
            {
                auto pSTATUSUTF8 = static_cast<pTW_STATUSUTF8>(pData);
                pTW_STATUS pStatus = &pSTATUSUTF8->Status;
                sBuffer << "\nTW_MEMREF is TW_STATUSUTF8:\n{\n" <<
                    indenter << "Status ConditionCode=" << pStatus->ConditionCode << "\n" <<
                    indenter << "Size=" << pSTATUSUTF8->Size << "\n" <<
                    indenter << "UTF8string=" << pSTATUSUTF8->UTF8string << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_STATUS:
            {
                auto pSTATUS = static_cast<pTW_STATUS>(pData);
                std::string sConditionCode = "(Unknown)";
                uint32_t finderValue = IDS_TWCC_ERRORSTART + pSTATUS->ConditionCode;
                auto it = dynarithmic::generic_array_finder_if(mapTwainDSMReturnCodes, [&](const auto& pr) { return pr.first == finderValue; });
                if (it.first)
                    sConditionCode = std::string() + mapTwainDSMReturnCodes[it.second].second + "";
                sBuffer << "\nTW_MEMREF is TW_STATUS:\n{\n" <<
                        indenter << "ConditionCode=" << pSTATUS->ConditionCode << "  " << sConditionCode << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_USERINTERFACE:
            {
            #ifdef _WIN32
                auto pUSERINTERFACE = static_cast<pTW_USERINTERFACE>(pData);
                TCHAR sz[256];
                RECT r;
                SetRect(&r,0,0,0,0);
                std::array<LONG, 4> aRect = { r.left, r.top, r.right, r.bottom };
                sz[0] = _T('\0');
                sBuffer << "\nTW_MEMREF is TW_USERINTERFACE:\n{\n" <<
                        indenter << "ShowUI=" <<  (pUSERINTERFACE->ShowUI?"TRUE":"FALSE") << "\n" <<
                        indenter << "ModalUI=" << (pUSERINTERFACE->ModalUI?"TRUE":"FALSE") << "\n" <<
                        indenter << "hParent=" << pUSERINTERFACE->hParent << "\n" <<
                        indenter << "hParent.Title=" << sz << "\n" <<
                        indenter << "hParent.ScreenPos= {" << StringWrapperA::Join(aRect, ",") << "}\n}";
            #endif
            }
            break;

            case ERRSTRUCT_TW_IMAGEINFO:
            {
                auto pIMAGEINFO = static_cast<pTW_IMAGEINFO>(pData);
                sBuffer << "\nTW_MEMREF is TW_IMAGEINFO:\n{\n" <<
                        indenter << "XResolution=" << Fix32ToFloat(pIMAGEINFO->XResolution) << "\n" <<
                        indenter << "YResolution=" << Fix32ToFloat(pIMAGEINFO->YResolution) << "\n" <<
                        indenter << "ImageWidth=" << pIMAGEINFO->ImageWidth << "\n" <<
                        indenter << "ImageLength=" << pIMAGEINFO->ImageLength << "\n" <<
                        indenter << "SamplesPerPixel=" << pIMAGEINFO->SamplesPerPixel << "\n" <<
                        indenter << "BitsPerSample" <<
                        pIMAGEINFO->BitsPerSample[0] << ","  <<
                        pIMAGEINFO->BitsPerSample[1] << ","  <<
                        pIMAGEINFO->BitsPerSample[2] << ","  <<
                        pIMAGEINFO->BitsPerSample[3] << ","  <<
                        pIMAGEINFO->BitsPerSample[4] << ","  <<
                        pIMAGEINFO->BitsPerSample[5] << ","  <<
                        pIMAGEINFO->BitsPerSample[6] << ","  <<
                        pIMAGEINFO->BitsPerSample[7] << "\n"  <<
                        indenter << "BitsPerPixel=" << pIMAGEINFO->BitsPerPixel << "\n" <<
                        indenter << "Planar=" << (pIMAGEINFO->Planar?"TRUE":"FALSE") << "\n" <<
                        indenter << "PixelType=" << pIMAGEINFO->PixelType << "\n" <<
                        indenter << "Compression=" << pIMAGEINFO->Compression << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_IMAGELAYOUT:
            {
                auto pIMAGELAYOUT = static_cast<pTW_IMAGELAYOUT>(pData);
                sBuffer <<
                "\nTW_MEMREF is TW_IMAGELAYOUT:\n{\n" <<
                indenter << "Frame=" <<
                Fix32ToFloat(pIMAGELAYOUT->Frame.Left) << "," <<
                Fix32ToFloat(pIMAGELAYOUT->Frame.Top) << "-" <<
                Fix32ToFloat(pIMAGELAYOUT->Frame.Right) << "," <<
                Fix32ToFloat(pIMAGELAYOUT->Frame.Bottom) << "\n" <<
                indenter << "DocmentNumber=" << pIMAGELAYOUT->DocumentNumber << "\n" <<
                indenter << "PageNumber=" << pIMAGELAYOUT->PageNumber << "\n" <<
                indenter << "FrameNumber=" << pIMAGELAYOUT->FrameNumber << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_IMAGEMEMXFER:
            {
                auto pIMAGEMEMXFER = static_cast<pTW_IMAGEMEMXFER>(pData);
                sBuffer << "\nTW_MEMREF is TW_IMAGEMEMXFER:\n{\n" <<
                            indenter << "Compression=" << pIMAGEMEMXFER->Compression << "\n" <<
                            indenter << "BytesPerRow=" << pIMAGEMEMXFER->BytesPerRow << "\n" <<
                            indenter << "Columns=" << pIMAGEMEMXFER->Columns << "\n" <<
                            indenter << "Rows=" << pIMAGEMEMXFER->Rows << "\n" <<
                            indenter << "XOffset=" << pIMAGEMEMXFER->XOffset << "\n" <<
                            indenter << "YOffset=" << pIMAGEMEMXFER->YOffset << "\n" <<
                            indenter << "BytesWritten=" << pIMAGEMEMXFER->BytesWritten << "\n" <<
                            indenter << DecodeTW_MEMORY(&pIMAGEMEMXFER->Memory,"Memory") << "\n}\n";
            }
            break;

            case ERRSTRUCT_HDIB:
            {
                auto handle = static_cast<HANDLE>(pData);
                sBuffer << "\nTW_MEMREF is a DIB:\n{\n" <<
                            indenter << "DIB Handle=" << handle << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_PALETTE8:
            {
                auto pPALETTE8 = static_cast<pTW_PALETTE8>(pData);
                sBuffer << "\nTW_MEMREF is a TW_PALETTE8:\n{\n" <<
                            indenter << "NumColors=" << pPALETTE8->NumColors << "\n" <<
                            indenter << "PaletteType=" << pPALETTE8->PaletteType << "\n";
                for ( int i = 0; i < 256; i++ )
                {
                    sBuffer << "ColorInfo[" << i << "]" <<
                            " - Index=" << static_cast<int>(pPALETTE8->Colors[i].Index) <<
                            ", Channel1=" << static_cast<int>(pPALETTE8->Colors[i].Channel1) <<
                            ", Channel2=" << static_cast<int>(pPALETTE8->Colors[i].Channel2) <<
                            ", Channel3=" << static_cast<int>(pPALETTE8->Colors[i].Channel3) << "\n";
                }
                sBuffer << "}\n";
            }
            break;

            case ERRSTRUCT_pTW_UINT32:
            {
                sBuffer << "\nTW_MEMREF is TW_UINT32 pointer:\n{\n" <<
                            indenter << "Address=" << pData << "H\n" <<
                            indenter << "Value at Address=" << *static_cast<TW_UINT32*>(pData) << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_CIECOLOR:
            {
                static constexpr std::array<const char *, 4> CIEPointNames = {"WhitePoint", "BlackPoint", "WhitePaper", "BlackInk"};
                auto pCIECOLOR = static_cast<pTW_CIECOLOR>(pData);
                std::array<pTW_CIEPOINT, CIEPointNames.size()> aPoints = {&pCIECOLOR->WhitePoint, &pCIECOLOR->BlackPoint, &pCIECOLOR->WhitePaper, &pCIECOLOR->BlackInk};
                sBuffer << "\nTW_MEMREF is TW_CIECOLOR:\n{\n{\n" <<
                            "ColorSpace=" << pCIECOLOR->ColorSpace << ",\n" <<
                            "LowEndian=" << pCIECOLOR->LowEndian << ",\n" <<
                            "DeviceDependent=" << pCIECOLOR->DeviceDependent << ",\n" <<
                            "VersionNumber=" << pCIECOLOR->VersionNumber << "\n\nTransform Stage Info:\n}";

                pTW_TRANSFORMSTAGE pCurTransform;
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
                        sBuffer << "Decode Value[" << i << "] =";
                        sBuffer << "{\n" <<
                          "StartIn=" << Fix32ToFloat(pCurTransform->Decode[i].StartIn) << ", " <<
                          "BreakIn=" << Fix32ToFloat(pCurTransform->Decode[i].BreakIn) << ", " <<
                          "EndIn="   << Fix32ToFloat(pCurTransform->Decode[i].EndIn) << ",\n" <<
                          "StartOut=" <<Fix32ToFloat(pCurTransform->Decode[i].StartOut) << ", " <<
                          "BreakOut=" << Fix32ToFloat(pCurTransform->Decode[i].BreakOut) << ", " <<
                          "EndOut=" << Fix32ToFloat(pCurTransform->Decode[i].EndOut) << ", \n" <<
                          "Gamma=" << Fix32ToFloat(pCurTransform->Decode[i].Gamma) << ", " <<
                          "SampleCount=" << Fix32ToFloat(pCurTransform->Decode[i].SampleCount) <<
                          "\n}\n";
                    }
                    for ( size_t i = 0; i < numMixes; i++ )
                    {
                        for ( size_t j = 0; j < numMixes; j++ )
                        {
                            sBuffer << "MixValue[" << i << "][" << j << "]=" <<
                                    Fix32ToFloat(pCurTransform->Mix[i][j]) << "\n";
                        }
                    }
                }

                // Get the CIE info
                for ( size_t i = 0; i < CIEPointNames.size(); i++ )
                {
                    sBuffer << "CIEPoint " << CIEPointNames[i] << " = {" <<
                                Fix32ToFloat(aPoints[i]->X) << "," <<
                                Fix32ToFloat(aPoints[i]->Y) << "," <<
                                Fix32ToFloat(aPoints[i]->Z) << "}\n";
                }

                sBuffer << "\nSample is user-defined and can't be determined \n}\n";
            }
            break;

            case ERRSTRUCT_TW_GRAYRESPONSE:
            {
                auto pGRAYRESPONSE = static_cast<pTW_GRAYRESPONSE>(pData);
                sBuffer << "\nTW_MEMREF is TW_GRAYRESPONSE:\n{\n" <<
                            DecodeTW_ELEMENT8(&pGRAYRESPONSE->Response[0], "Response[0]") <<
                            "\n}\n";
            }
            break;
            case ERRSTRUCT_TW_RGBRESPONSE:
            {
                auto pRGBRESPONSE = static_cast<pTW_RGBRESPONSE>(pData);
                sBuffer << "\nTW_MEMREF is TW_RGBRESPONSE:\n{\n" <<
                            DecodeTW_ELEMENT8(&pRGBRESPONSE->Response[0], "Response[0]") <<
                            "\n}\n";
            }
            break;
            case ERRSTRUCT_TW_JPEGCOMPRESSION:
            {
                auto pJPEGCOMPRESSION = static_cast<pTW_JPEGCOMPRESSION>(pData);
                sBuffer << "\nTW_MEMREF is TW_JPEGCOMPRESSION:\n{\n" <<
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
                            "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_EXTIMAGEINFO:
            {
                StringStreamA TempStream;
                auto pEXTIMAGEINFO = static_cast<pTW_EXTIMAGEINFO>(pData);
                TempStream << "\nTW_MEMREF is TW_EXTIMAGINFO:\n{\n" << "NumInfos=" << pEXTIMAGEINFO->NumInfos << "\n";

                std::string sAllInfo = TempStream.str();
                StringStreamA strm;
                for (TW_UINT32 i = 0; i < pEXTIMAGEINFO->NumInfos; i++ )
                {
                    strm << boost::format("Info[%1%]=%2%\n") % i % DecodeTW_INFO(&pEXTIMAGEINFO->Info[i], nullptr);
                }
                sAllInfo += strm.str();
                sBuffer << sAllInfo << "}\n";
            }
            break;

            case ERRSTRUCT_TW_TWUNKIDENTITY:
            {
                auto pTWUNKIDENTITY = static_cast<pTW_TWUNKIDENTITY>(pData);
                pTW_IDENTITY pIdentity = &pTWUNKIDENTITY->identity;
                std::string dsPath = " ";
                    dsPath = pTWUNKIDENTITY->dsPath;
                sBuffer << "\nTW_MEMREF is TW_TWUNKIDENTITY:\n{\n" <<
                            indenter << DecodeSourceInfo(pIdentity, "TW_TWUNKIDENTITY") << "\n" <<
                            indenter << "dsPath = " << dsPath << "\n}";
            }
            break;

            case ERRSTRUCT_TW_AUDIOINFO:
            {
                auto pAUDIOINFO = static_cast<pTW_AUDIOINFO>(pData);
                sBuffer << "\nTW_MEMREF is TW_AUDIOINFO:\n{\n" <<
                    indenter << "Name=" << pAUDIOINFO->Name << "\n" <<
                    indenter << "Reserved=" << pAUDIOINFO->Reserved << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_CALLBACK:
            {
                auto pCALLBACK = static_cast<pTW_CALLBACK>(pData);
                sBuffer << "\nTW_MEMREF is TW_CALLBACK:\n{\n";
                #if defined(__APPLE__)
                    sBuffer << indenter << "Refcon=" << pCALLBACK->RefCon << "\n";
                #endif
                    sBuffer << indenter << "Message=" << pCALLBACK->Message << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_CALLBACK2:
            {
                auto pCALLBACK2 = static_cast<pTW_CALLBACK2>(pData);
                sBuffer << "\nTW_MEMREF is TW_CALLBACK2:\n{\n";
                sBuffer << indenter << "CallbackProc=" << pCALLBACK2->CallBackProc << "\n";
                sBuffer << indenter << "Refcon=" << pCALLBACK2->RefCon << "\n";
                sBuffer << indenter << "Message=" << pCALLBACK2->Message << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_METRICS:
            {
                auto pMETRICS = static_cast<pTW_METRICS>(pData);
                sBuffer << "\nTW_MEMREF is TW_METRICS:\n{\n";
                sBuffer << indenter << "SizeOf=" << pMETRICS->SizeOf << "\n";
                sBuffer << indenter << "ImageCount=" << pMETRICS->ImageCount << "\n";
                sBuffer << indenter << "SheetCount=" << pMETRICS->SheetCount << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_TWAINDIRECT:
            {
                auto pTWAINDIRECT = static_cast<pTW_TWAINDIRECT>(pData);
                sBuffer << "\nTW_MEMREF is TW_TWAINDIRECT:\n{\n";
                sBuffer << indenter << "SizeOf=" << pTWAINDIRECT->SizeOf << "\n";
                sBuffer << indenter << "CommunicationManager=" << pTWAINDIRECT->CommunicationManager << "\n";
                sBuffer << indenter << "Send=" << pTWAINDIRECT->Send << "\n";
                sBuffer << indenter << "SendSize=" << pTWAINDIRECT->SendSize << "\n";
                sBuffer << indenter << "Receive=" << pTWAINDIRECT->Receive << "\n";
                sBuffer << indenter << "ReceiveSize=" << pTWAINDIRECT->ReceiveSize << "\n}\n";
            }
            break;
            case ERRSTRUCT_TW_TWUNKDSENTRYPARAMS: break;
            case ERRSTRUCT_pWAV: break;
            case ERRSTRUCT_DTWAIN_MESSAGE: break;
            default: ;
        }
    }
    sTemp = sBuffer.str();
    return sTemp;
}

std::string DecodeSourceInfo(pTW_IDENTITY pIdentity, LPCSTR sPrefix)
{
    StringStreamA sBuffer;
    if ( pIdentity)
    {
        const std::string indenter = IndentDefinition();
        sBuffer << "Decoded " << sPrefix << ":\n{\n" <<

        indenter << "Id=" << pIdentity->Id << "\n" <<
        indenter << "Version Number=" << pIdentity->Version.MajorNum << "." << pIdentity->Version.MinorNum << "\n" <<
        indenter << "Version Language=" << pIdentity->Version.Language << "\n" <<
        indenter << "Version Country=" << pIdentity->Version.Country << "\n" <<
        indenter << "Version Info="   << pIdentity->Version.Info << "\n" <<
        indenter << "ProtocolMajor="  << pIdentity->ProtocolMajor << "\n"  <<
        indenter << "ProtocolMinor="  << pIdentity->ProtocolMinor << "\n"    <<
        indenter << "SupportedGroups=" << DecodeSupportedGroups(pIdentity->SupportedGroups) << "\n" <<
        indenter << "Manufacturer=" << pIdentity->Manufacturer << "\n" <<
        indenter << "Product Family=" << pIdentity->ProductFamily << "\n" <<
        indenter << "Product Name=" << pIdentity->ProductName <<

        "\n}\n";
    }
    else
    {
        sBuffer << "\nNo information for " << sPrefix << "\n";
    }
    return sBuffer.str();
}

std::string DecodeSupportedGroups(TW_UINT32 SupportedGroups)
{
    std::vector<std::string> allGroups;
    constexpr unsigned int numberOfBits = sizeof(TW_UINT32) << 3;
    for (unsigned int i = 0; i < numberOfBits; ++i)
    {
        const unsigned int curGroup = static_cast<TW_UINT32>(1) << i;
        if ( SupportedGroups & curGroup )
        {
            auto it = dynarithmic::generic_array_finder_if(mapSupportedGroups, [&](const auto& pr) { return pr.first == curGroup; });
            if ( it.first)
                allGroups.push_back(mapSupportedGroups[it.second].second);
            else
                allGroups.push_back("Unknown (" + std::to_string(curGroup) + ")");
        }
    }
    return StringWrapperA::Join(allGroups, ", ");
}

std::string DecodeTW_MEMORY(pTW_MEMORY pMemory, LPCSTR pMem)
{
    StringStreamA sBuffer;
    sBuffer << "{Flags=" <<
            pMemory->Flags << ", " <<
            "Length=" <<
            pMemory->Length << ", " <<
            "TheMem=" << pMemory->TheMem << "H}";
    std::string sTemp = sBuffer.str();
    if ( pMem )
    {
        sTemp = pMem;
        sTemp += "=";
    }
    sTemp += sBuffer.str();
    return sTemp;
}


std::string DecodeTW_ELEMENT8(pTW_ELEMENT8 pEl, LPCSTR pMem)
{
    StringStreamA sBuffer;

    sBuffer << "{Index=" << pEl->Index << ", " <<
               "Channel1=" << pEl->Channel1 << ", " <<
               "Channel2=" << pEl->Channel3 << "}";
    std::string sTemp = sBuffer.str();
    if ( pMem )
    {
        std::string sTemp2 = pMem;
        sTemp2 += "=";
        sTemp = sTemp2 + sTemp;
    }
    return sTemp;
}

std::string DecodeTW_INFO(pTW_INFO pInfo, LPCSTR pMem)
{
    StringStreamA sBuffer;
    sBuffer << "{InfoId=" << pInfo->InfoID << ", " <<
               "ItemType=" << pInfo->ItemType << ", " <<
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
