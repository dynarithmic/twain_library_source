/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2023 Dynarithmic Software.

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
#include "ctliface.h"
#include "ctltr010.h"
#include "ctltwmgr.h"
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

CTL_ContainerToNameMap CTL_ErrorStructDecoder::s_mapContainerType;
CTL_ContainerToNameMap CTL_ErrorStructDecoder::s_mapNotificationType;
std::unordered_map<TW_UINT32, std::string> CTL_ErrorStructDecoder::s_mapSupportedGroups;
std::unordered_map<TW_UINT16, std::string> CTL_ErrorStructDecoder::s_mapTwainDSMReturnCodes;

bool CTL_ErrorStructDecoder::s_bInit=false;

#define ADD_ERRORCODE_TO_MAP(theMap, start, x) theMap[(start) + x] = #x;

CTL_ErrorStructDecoder::CTL_ErrorStructDecoder()
{
    if ( !s_bInit )
    {
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWRC_ERRORSTART, TWRC_SUCCESS)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWRC_ERRORSTART, TWRC_FAILURE)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWRC_ERRORSTART, TWRC_CHECKSTATUS)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWRC_ERRORSTART, TWRC_CANCEL)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWRC_ERRORSTART, TWRC_DSEVENT)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWRC_ERRORSTART, TWRC_NOTDSEVENT)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWRC_ERRORSTART, TWRC_XFERDONE)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWRC_ERRORSTART, TWRC_ENDOFLIST)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWRC_ERRORSTART, TWRC_INFONOTSUPPORTED)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWRC_ERRORSTART, TWRC_DATANOTAVAILABLE)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWRC_ERRORSTART, TWRC_BUSY)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWRC_ERRORSTART, TWRC_SCANNERLOCKED)

        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_SUCCESS)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_BUMMER)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_LOWMEMORY)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_NODS)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_MAXCONNECTIONS)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_OPERATIONERROR)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_BADCAP)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_BADPROTOCOL)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_BADVALUE)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_SEQERROR)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_BADDEST)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_CAPUNSUPPORTED)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_CAPBADOPERATION)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_CAPSEQERROR)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_DENIED)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_FILEEXISTS)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_FILENOTFOUND)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_NOTEMPTY)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_PAPERJAM)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_PAPERDOUBLEFEED)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_FILEWRITEERROR)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_CHECKDEVICEONLINE)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_INTERLOCK)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_DAMAGEDCORNER)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_FOCUSERROR)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_DOCTOOLIGHT)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_DOCTOODARK)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_NOMEDIA)
        ADD_ERRORCODE_TO_MAP(s_mapTwainDSMReturnCodes,IDS_TWCC_ERRORSTART, TWCC_DOCTOOLIGHT)

        ADD_ERRORCODE_TO_MAP(s_mapSupportedGroups, 0, DG_CONTROL)
        ADD_ERRORCODE_TO_MAP(s_mapSupportedGroups, 0, DG_IMAGE)
        ADD_ERRORCODE_TO_MAP(s_mapSupportedGroups, 0, DG_AUDIO)
        ADD_ERRORCODE_TO_MAP(s_mapSupportedGroups, 0, DF_DSM2)
        ADD_ERRORCODE_TO_MAP(s_mapSupportedGroups, 0, DF_APP2)
        ADD_ERRORCODE_TO_MAP(s_mapSupportedGroups, 0, DF_DS2)

        s_mapContainerType[TWON_ARRAY]          = "TW_ARRAY";
        s_mapContainerType[TWON_ENUMERATION]    = "TW_ENUMERATION";
        s_mapContainerType[TWON_ONEVALUE]       = "TW_ONEVALUE";
        s_mapContainerType[TWON_RANGE]          = "TW_RANGE";

        // Map of DTWAIN window messages to strings
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_ACQUIREDONE       )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_ACQUIREFAILED     )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_ACQUIRECANCELLED  )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_ACQUIRESTARTED    )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_PAGECONTINUE      )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_PAGEFAILED        )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_PAGECANCELLED     )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_TRANSFERREADY     )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_TRANSFERDONE      )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_ACQUIREPAGEDONE   )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_UICLOSING         )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_UICLOSED          )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_UIOPENED          )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_CLIPTRANSFERDONE  )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_INVALIDIMAGEFORMAT)
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_ACQUIRETERMINATED )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_TRANSFERSTRIPREADY)
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_TRANSFERSTRIPDONE )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_IMAGEINFOERROR    )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_DEVICEEVENT       )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_FILESAVECANCELLED     )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_FILESAVEOK            )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_FILESAVEERROR         )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_FILEPAGESAVEOK        )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_FILEPAGESAVEERROR     )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_PROCESSEDDIB          )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_PROCESSDIBACCEPTED      )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_PROCESSDIBFINALACCEPTED )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_TRANSFERSTRIPFAILED   )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_IMAGEINFOERROR        )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_TRANSFERCANCELLED     )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_UIOPENING             )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_MANDUPFLIPPAGES       )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_MANDUPSIDE1DONE       )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_MANDUPSIDE2DONE       )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_MANDUPPAGECOUNTERROR  )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_MANDUPACQUIREDONE     )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_MANDUPSIDE1START      )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_MANDUPSIDE2START      )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_MANDUPMERGEERROR      )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_MANDUPMEMORYERROR     )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_MANDUPFILEERROR       )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_MANDUPFILESAVEERROR   )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_ENDOFJOBDETECTED      )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_EOJDETECTED_XFERDONE  )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_TWAINPAGECANCELLED    )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_TWAINPAGEFAILED       )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_QUERYPAGEDISCARD      )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_PAGEDISCARDED         )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_APPUPDATEDDIB         )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_FILEPAGESAVING        )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_CROPFAILED        )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_PROCESSEDDIBFINAL )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_BLANKPAGEDETECTED1    )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_BLANKPAGEDETECTED2    )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_BLANKPAGEDETECTED3    )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_BLANKPAGEDISCARDED1   )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_BLANKPAGEDISCARDED2   )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_MESSAGELOOPERROR   )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_SETUPMODALACQUISITION )
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_TWAINTRIPLETBEGIN)
        ADD_ERRORCODE_TO_MAP(s_mapNotificationType, 0,  DTWAIN_TN_TWAINTRIPLETEND)
        s_bInit = true;
    }
}

void CTL_ErrorStructDecoder::StartMessageDecoder(HWND hWnd, UINT nMsg,
                                                 WPARAM wParam, LPARAM lParam)
{
    StringStreamA sBuffer;

    m_pString.clear();
    if ( s_mapNotificationType.find(wParam) != s_mapNotificationType.end())
        sBuffer << "\nDTWAIN Message(HWND = " << hWnd << ", " <<
                                    "MSG = " << nMsg << ", " <<
                                    "Notification code = " << s_mapNotificationType[wParam] << ", " <<
                                    "LPARAM = " << lParam;
    else
        sBuffer << "\nDTWAIN Message(HWND = " << hWnd << ", " <<
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
    sBuffer << "\nDSM_Entry(pSource=" << pSource << "H, " <<
                "pDest=" << pDest << "H, " <<
           CTL_TwainDLLHandle::GetTwainNameFromResource(CTL_TwainDLLHandle::GetDGResourceID(),static_cast<int>(nDG)) << ", " <<
           CTL_TwainDLLHandle::GetTwainNameFromResource(CTL_TwainDLLHandle::GetDATResourceID(),static_cast<int>(nDAT)) << ", " <<
           CTL_TwainDLLHandle::GetTwainNameFromResource(CTL_TwainDLLHandle::GetMSGResourceID(),static_cast<int>(nMSG)) << ", " <<
           "TW_MEMREF=" << Data << "H) called\n";
    s1 = sBuffer.str();

    std::string pSourceStr;
    std::string pDestStr;
    std::string pMemRefStr;

    // Decode the pSource argument
    long lErrorFilter = CTL_TwainDLLHandle::GetErrorFilterFlags();
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
    const auto it = s_mapTwainDSMReturnCodes.find(actualCode);
    if ( it != s_mapTwainDSMReturnCodes.end() )
        return it->second;
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
                auto p = static_cast<pTW_CUSTOMDSDATA>(pData);
                sBuffer << "\nTW_MEMREF is TW_CUSTOMDATA:\n{\n" <<
                            "InfoLength=" << p->InfoLength << "\n" <<
                            "hData=" << p->hData << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_DEVICEEVENT:
            {
                auto p = static_cast<pTW_DEVICEEVENT>(pData);
                sBuffer << "\nTW_MEMREF is TW_DEVICEEVENT:\n{\n" <<
                            "Event=" << p->Event << "\n" <<
                            "DeviceName=" << p->DeviceName << "\n" <<
                            "BatteryMinutes=" << p->BatteryMinutes << "\n" <<
                            "BatteryPercentage=" << p->BatteryPercentage << "\n" <<
                            "PowerSupply=" << p->PowerSupply << "\n" <<
                            "XResolution=" << dynarithmic::Fix32ToFloat(p->XResolution) << "\n" <<
                            "YResolution=" << dynarithmic::Fix32ToFloat(p->YResolution) << "\n" <<
                            "FlashUsed2=" << p->FlashUsed2 << "\n" <<
                            "AutomaticCapture=" << p->AutomaticCapture << "\n" <<
                            "TimeBeforeFirstCapture=" << p->TimeBeforeFirstCapture << "\n" <<
                            "TimeBetweenCaptures=" << p->TimeBetweenCaptures << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_EVENT:
            {
                LONG lErrorFlags = CTL_TwainDLLHandle::GetErrorFilterFlags();
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
                auto p = static_cast<pTW_FILESYSTEM>(pData);
                sBuffer << "\nTW_MEMREF is TW_FILESYSTEM:\n{\n" <<
                        indenter << "InputName=" << p->InputName << "\n" <<
                        indenter << "OutputName=" << p->OutputName << "\n" <<
                        indenter << "Context=" << p->Context << "H\n" <<
                        indenter << "Recursive=" << p->Recursive << "\n" <<
                        indenter << "FileType=" << p->FileType << "\n" <<
                        indenter << "Size=" << p->Size << "\n" <<
                        indenter << "CreateTimeDate=" << p->CreateTimeDate << "\n" <<
                        indenter << "ModifiedTimeDate=" << p->ModifiedTimeDate << "\n" <<
                        indenter << "FreeSpace=" << p->FreeSpace << "\n" <<
                        indenter << "NewImageSize=" << p->NewImageSize << "\n" <<
                        indenter << "NumberOfFiles=" << p->NumberOfFiles << "\n" <<
                        indenter << "NumberOfSnippets=" << p->NumberOfSnippets << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_IDENTITY:
            {
                sBuffer << "\nTW_MEMREF is TW_IDENTITY:\n" << DecodeSourceInfo(static_cast<pTW_IDENTITY>(pData), "TW_MEMREF");
            }
            break;

            case ERRSTRUCT_TW_MEMORY:
            {
                auto pMemory = static_cast<TW_MEMORY*>(pData);
                sBuffer <<
                    "\nTW_MEMREF is TW_MEMORY:\n{\n" <<
                    indenter << "Flags=" << pMemory->Flags<< "\n" <<
                    indenter << "Length=" << pMemory->Length << "\n" <<
                    indenter << "TheMem=" << pMemory->TheMem << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_ENTRYPOINT:
            {
                auto pEntryPoint = static_cast<TW_ENTRYPOINT*>(pData);
                sBuffer <<
                    "\nTW_MEMREF is TW_ENTRYPOINT:\n{\n" <<
                    indenter << "Size=" << pEntryPoint->Size << "\n" <<
                    indenter << "DSMEntry=" << &pEntryPoint->DSM_Entry << "\n" <<
                    indenter << "DSMMemAllocate=" << &pEntryPoint->DSM_MemAllocate << "\n" <<
                    indenter << "DSMMemLock=" << &pEntryPoint->DSM_MemLock << "\n" <<
                    indenter << "DSMMemUnlock=" << &pEntryPoint->DSM_MemUnlock << "\n}\n";
            }
            break;

            case ERRSTRUCT_LPHWND:
            {
            #ifdef _WIN32
                RECT r;
                HWND *p = static_cast<HWND*>(pData);
                GetWindowRect(*p, &r);

                sBuffer <<
                "\nTW_MEMREF is handle to window (HWND):\n{\n" <<
                indenter << "HWND=" << *p << "\n" <<
                indenter << "Screen Pos.= " << r.left << "," << r.top << "-" <<
                                    r.right << "," << r.bottom << "\n}\n";
            #endif
            }
            break;

            case ERRSTRUCT_TW_PASSTHRU:
            {
                auto p = static_cast<pTW_PASSTHRU>(pData);
                sBuffer <<
                "\nTW_MEMREF is TW_PASSTHRU:\n{\n" <<
                indenter << "Command=" << p->pCommand << "H\n" <<
                indenter << "CommandBytes=" << p->CommandBytes << "\n" <<
                indenter << "Direction=" << p->Direction << "\n" <<
                indenter << "pDataBuffer=" << p->pData << "H\n" <<
                indenter << "DataBytes=" << p->DataBytes << "\n" <<
                indenter << "DataBytesXfered=" << p->DataBytesXfered << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_PENDINGXFERS:
            {
                auto p = static_cast<pTW_PENDINGXFERS>(pData);
                sBuffer << "\nTW_MEMREF is TW_PENDINGXFERS:\n{\n" <<
                            indenter << "Count=" << p->Count << "\n" <<
                            indenter << "EOJ=" << p->EOJ << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_SETUPFILEXFER:
            {
                auto p = static_cast<pTW_SETUPFILEXFER>(pData);
                sBuffer <<
                "\nTW_MEMREF is TW_SETUPFILEXFER:\n{\n" <<
                indenter << "FileName=" << p->FileName << "\n" <<
                indenter << "Format=" << p->Format << "\n" <<
                indenter << "VRefNum=" << p->VRefNum << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_SETUPMEMXFER:
            {
                auto p = static_cast<pTW_SETUPMEMXFER>(pData);
                sBuffer << "\nTW_MEMREF is TW_SETUPMEMXFER:\n{\n" <<
                        indenter << "MinBufSize=" << p->MinBufSize << "\n" <<
                        indenter << "MaxBufSize=" << p->MaxBufSize << "\n" <<
                        indenter << "Preferred=" << p->Preferred << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_CAPABILITY:
            {
                CTL_ContainerToNameMap::iterator it;
                auto p = static_cast<pTW_CAPABILITY>(pData);
                it = CTL_ErrorStructDecoder::s_mapContainerType.find(static_cast<int>(p->ConType));
                std::string s = "Unspecified (TWON_DONTCARE)";
                if (it != CTL_ErrorStructDecoder::s_mapContainerType.end() )
                    s = (*it).second;

                sBuffer << "\nTW_MEMREF is TW_CAPABILITY:\n{\n" <<
                        indenter << "Cap=" << CTL_TwainAppMgr::GetCapNameFromCap(p->Cap) << "\n" <<
                        indenter << "ContainerType=" << s << "\n" <<
                        indenter << "hContainer=" << p->hContainer << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_STATUSUTF8:
            {
                CTL_ContainerToNameMap::iterator it;
                auto p = static_cast<pTW_STATUSUTF8>(pData);
                pTW_STATUS pStatus = &p->Status;
                sBuffer << "\nTW_MEMREF is TW_STATUSUTF8:\n{\n" <<
                    indenter << "Status ConditionCode=" << pStatus->ConditionCode << "\n" <<
                    indenter << "Size=" << p->Size << "\n" <<
                    indenter << "UTF8string=" << p->UTF8string << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_STATUS:
            {
                auto p = static_cast<pTW_STATUS>(pData);
                std::string sConditionCode = "(Unknown)";
                if (CTL_ErrorStructDecoder::s_mapTwainDSMReturnCodes.find(IDS_TWCC_ERRORSTART + p->ConditionCode)
                    != CTL_ErrorStructDecoder::s_mapTwainDSMReturnCodes.end())
                    sConditionCode = "" + CTL_ErrorStructDecoder::s_mapTwainDSMReturnCodes[IDS_TWCC_ERRORSTART + p->ConditionCode] + "";
                sBuffer << "\nTW_MEMREF is TW_STATUS:\n{\n" <<
                        indenter << "ConditionCode=" << p->ConditionCode << "  " << sConditionCode << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_USERINTERFACE:
            {
            #ifdef _WIN32
                auto p = static_cast<pTW_USERINTERFACE>(pData);
                TCHAR sz[256];
                RECT r;
                SetRect(&r,0,0,0,0);
                sz[0] = _T('\0');
                sBuffer << "\nTW_MEMREF is TW_USERINTERFACE:\n{\n" <<
                        indenter << "ShowUI=" <<  (p->ShowUI?"TRUE":"FALSE") << "\n" <<
                        indenter << "ModalUI=" << (p->ModalUI?"TRUE":"FALSE") << "\n" <<
                        indenter << "hParent=" << p->hParent << "\n" <<
                        indenter << "hParent.Title=" << sz << "\n" <<
                        indenter << "hParent.ScreenPo.= {" <<
                        r.left << "," << r.top << "-" <<
                        r.right << "," << r.bottom << "\n}\n";
            #endif
            }
            break;

            case ERRSTRUCT_TW_IMAGEINFO:
            {
                auto p = static_cast<pTW_IMAGEINFO>(pData);
                sBuffer << "\nTW_MEMREF is TW_IMAGEINFO:\n{\n" <<
                        indenter << "XResolution=" << Fix32ToFloat(p->XResolution) << "\n" <<
                        indenter << "YResolution=" << Fix32ToFloat(p->YResolution) << "\n" <<
                        indenter << "ImageWidth=" << p->ImageWidth << "\n" <<
                        indenter << "ImageLength=" << p->ImageLength << "\n" <<
                        indenter << "SamplesPerPixel=" << p->SamplesPerPixel << "\n" <<
                        indenter << "BitsPerSample" <<
                        p->BitsPerSample[0] << ","  <<
                        p->BitsPerSample[1] << ","  <<
                        p->BitsPerSample[2] << ","  <<
                        p->BitsPerSample[3] << ","  <<
                        p->BitsPerSample[4] << ","  <<
                        p->BitsPerSample[5] << ","  <<
                        p->BitsPerSample[6] << ","  <<
                        p->BitsPerSample[7] << "\n"  <<
                        indenter << "BitsPerPixel=" << p->BitsPerPixel << "\n" <<
                        indenter << "Planar=" << (p->Planar?"TRUE":"FALSE") << "\n" <<
                        indenter << "PixelType=" << p->PixelType << "\n" <<
                        indenter << "Compression=" << p->Compression << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_IMAGELAYOUT:
            {
                auto p = static_cast<pTW_IMAGELAYOUT>(pData);
                sBuffer <<
                "\nTW_MEMREF is TW_IMAGELAYOUT:\n{\n" <<
                indenter << "Frame=" <<
                Fix32ToFloat(p->Frame.Left) << "," <<
                Fix32ToFloat(p->Frame.Top) << "-" <<
                Fix32ToFloat(p->Frame.Right) << "," <<
                Fix32ToFloat(p->Frame.Bottom) << "\n" <<
                indenter << "DocmentNumber=" << p->DocumentNumber << "\n" <<
                indenter << "PageNumber=" << p->PageNumber << "\n" <<
                indenter << "FrameNumber=" << p->FrameNumber << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_IMAGEMEMXFER:
            {
                auto p = static_cast<pTW_IMAGEMEMXFER>(pData);
                sBuffer << "\nTW_MEMREF is TW_IMAGEMEMXFER:\n{\n" <<
                            indenter << "Compression=" << p->Compression << "\n" <<
                            indenter << "BytesPerRow=" << p->BytesPerRow << "\n" <<
                            indenter << "Columns=" << p->Columns << "\n" <<
                            indenter << "Rows=" << p->Rows << "\n" <<
                            indenter << "XOffset=" << p->XOffset << "\n" <<
                            indenter << "YOffset=" << p->YOffset << "\n" <<
                            indenter << "BytesWritten=" << p->BytesWritten << "\n" <<
                            indenter << DecodeTW_MEMORY(&p->Memory,"Memory") << "\n}\n";
            }
            break;

            case ERRSTRUCT_HDIB:
            {
                auto h = static_cast<HANDLE>(pData);
                sBuffer << "\nTW_MEMREF is a DIB:\n{\n" <<
                            indenter << "DIB Handle=" << h << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_PALETTE8:
            {
                auto p = static_cast<pTW_PALETTE8>(pData);
                sBuffer << "\nTW_MEMREF is a TW_PALETTE8:\n{\n" <<
                            indenter << "NumColors=" << p->NumColors << "\n" <<
                            indenter << "PaletteType=" << p->PaletteType << "\n";
                for ( int i = 0; i < 256; i++ )
                {
                    sBuffer << "ColorInfo[" << i << "]" <<
                            " - Index=" << static_cast<int>(p->Colors[i].Index) <<
                            ", Channel1=" << static_cast<int>(p->Colors[i].Channel1) <<
                            ", Channel2=" << static_cast<int>(p->Colors[i].Channel2) <<
                            ", Channel3=" << static_cast<int>(p->Colors[i].Channel3) << "\n";
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
                const char *CIEPointNames[4] = {"WhitePoint", "BlackPoint", "WhitePaper", "BlackInk"};
                std::string str2;
                int i;
                auto p = static_cast<pTW_CIECOLOR>(pData);
                pTW_CIEPOINT aPoints[4] = {&p->WhitePoint, &p->BlackPoint, &p->WhitePaper, &p->BlackInk};
                sBuffer << "\nTW_MEMREF is TW_CIECOLOR:\n{\n{\n" <<
                            "ColorSpace=" << p->ColorSpace << ",\n" <<
                            "LowEndian=" << p->LowEndian << ",\n" <<
                            "DeviceDependent=" << p->DeviceDependent << ",\n" <<
                            "VersionNumber=" << p->VersionNumber << "\n\nTransform Stage Info:\n}";

                pTW_TRANSFORMSTAGE pCurTransform;
                for ( int nTransform = 0; nTransform < 2; nTransform++)
                {
                    if ( nTransform == 0 )
                        pCurTransform = &p->StageABC;
                    else
                        pCurTransform = &p->StageLMN;
                    for ( i = 0; i < 3; i++ )
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
                    int j;
                    str2.clear();
                    for ( i = 0; i < 3; i++ )
                    {
                        for ( j = 0; j < 3; j++ )
                        {
                            sBuffer << "MixValue[" << i << "][" << j << "]=" <<
                                    Fix32ToFloat(pCurTransform->Mix[i][j]) << "\n";
                        }
                    }
                }

                // Get the CIE info
                for ( i = 0; i < 4; i++ )
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
                auto p = static_cast<pTW_GRAYRESPONSE>(pData);
                sBuffer << "\nTW_MEMREF is TW_GRAYRESPONSE:\n{\n" <<
                            DecodeTW_ELEMENT8(&p->Response[0], "Response[0]") <<
                            "\n}\n";
            }
            break;
            case ERRSTRUCT_TW_RGBRESPONSE:
            {
                auto p = static_cast<pTW_RGBRESPONSE>(pData);
                sBuffer << "\nTW_MEMREF is TW_RGBRESPONSE:\n{\n" <<
                            DecodeTW_ELEMENT8(&p->Response[0], "Response[0]") <<
                            "\n}\n";
            }
            break;
            case ERRSTRUCT_TW_JPEGCOMPRESSION:
            {
                auto p = static_cast<pTW_JPEGCOMPRESSION>(pData);
                sBuffer << "\nTW_MEMREF is TW_JPEGCOMPRESSION:\n{\n" <<
                            indenter << "ColorSpace=" << p->ColorSpace << "\n" <<
                            indenter << "SubSampling=" << p->SubSampling << "\n" <<
                            indenter << "NumComponents=" << p->NumComponents << "\n" <<
                            indenter << "RestartFrequency=" << p->RestartFrequency << "\n" <<
                            indenter << "QuantMap={" <<
                            p->QuantMap[0] << "," <<
                            p->QuantMap[1] << "," <<
                            p->QuantMap[2] << "," <<
                            p->QuantMap[3] << "}\n" <<
                            indenter << DecodeTW_MEMORY(&p->QuantTable[0],"QuantTable[0]") <<
                            "\n" <<
                            indenter << DecodeTW_MEMORY(&p->QuantTable[1],"QuantTable[1]")<<
                            "\n" <<
                            indenter << DecodeTW_MEMORY(&p->QuantTable[2],"QuantTable[2]")<<
                            "\n" <<
                            indenter << DecodeTW_MEMORY(&p->QuantTable[3],"QuantTable[3]")<<
                            "\n" <<
                            indenter << "HuffmanMap={" <<
                            p->HuffmanMap[0] << "," <<
                            p->HuffmanMap[1] << "," <<
                            p->HuffmanMap[2] << "," <<
                            p->HuffmanMap[3] << "}\n" <<
                            indenter << DecodeTW_MEMORY(&p->HuffmanDC[0],"HuffmanDC[0]") <<
                            "\n" <<
                            indenter << DecodeTW_MEMORY(&p->HuffmanDC[1],"HuffmanDC[1]") <<
                            "\n" <<
                            indenter << DecodeTW_MEMORY(&p->HuffmanAC[0],"HuffmanAC[0]") <<
                            "\n" <<
                            indenter << DecodeTW_MEMORY(&p->HuffmanAC[1],"HuffmanAC[1]") <<
                            "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_EXTIMAGEINFO:
            {
                StringStreamA TempStream;
                auto p = static_cast<pTW_EXTIMAGEINFO>(pData);
                TempStream << "\nTW_MEMREF is TW_EXTIMAGINFO:\n{\n" << "NumInfos=" << p->NumInfos << "\n";

                std::string sAllInfo = TempStream.str();
                StringStreamA strm;
                for (TW_UINT32 i = 0; i < p->NumInfos; i++ )
                {
                    strm << boost::format("Info[%1%]=%2%\n") % i % DecodeTW_INFO(&p->Info[i], nullptr);
                }
                sAllInfo += strm.str();
                sBuffer << sAllInfo << "}\n";
            }
            break;

            case ERRSTRUCT_TW_TWUNKIDENTITY:
            {
                auto p = static_cast<pTW_TWUNKIDENTITY>(pData);
                pTW_IDENTITY pIdentity = &p->identity;
                std::string dsPath = " ";
                    dsPath = p->dsPath;
                sBuffer << "\nTW_MEMREF is TW_TWUNKIDENTITY:\n{\n" <<
                            indenter << DecodeSourceInfo(pIdentity, "TW_TWUNKIDENTITY") << "\n" <<
                            indenter << "dsPath = " << dsPath << "\n}";
            }
            break;

            case ERRSTRUCT_TW_AUDIOINFO:
            {
                auto p = static_cast<pTW_AUDIOINFO>(pData);
                sBuffer << "\nTW_MEMREF is TW_AUDIOINFO:\n{\n" <<
                    indenter << "Name=" << p->Name << "\n" <<
                    indenter << "Reserved=" << p->Reserved << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_CALLBACK:
            {
                auto p = static_cast<pTW_CALLBACK>(pData);
                sBuffer << "\nTW_MEMREF is TW_CALLBACK:\n{\n";
                #if defined(__APPLE__)
                    sBuffer << indenter << "Refcon=" << p->RefCon << "\n";
                #endif
                    sBuffer << indenter << "Message=" << p->Message << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_CALLBACK2:
            {
                auto p = static_cast<pTW_CALLBACK2>(pData);
                sBuffer << "\nTW_MEMREF is TW_CALLBACK2:\n{\n";
                sBuffer << indenter << "CallbackProc=" << p->CallBackProc << "\n";
                sBuffer << indenter << "Refcon=" << p->RefCon << "\n";
                sBuffer << indenter << "Message=" << p->Message << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_METRICS:
            {
                auto p = static_cast<pTW_METRICS>(pData);
                sBuffer << "\nTW_MEMREF is TW_METRICS:\n{\n";
                sBuffer << indenter << "SizeOf=" << p->SizeOf << "\n";
                sBuffer << indenter << "ImageCount=" << p->ImageCount << "\n";
                sBuffer << indenter << "SheetCount=" << p->SheetCount << "\n}\n";
            }
            break;

            case ERRSTRUCT_TW_TWAINDIRECT:
            {
                auto p = static_cast<pTW_TWAINDIRECT>(pData);
                sBuffer << "\nTW_MEMREF is TW_TWAINDIRECT:\n{\n";
                sBuffer << indenter << "SizeOf=" << p->SizeOf << "\n";
                sBuffer << indenter << "CommunicationManager=" << p->CommunicationManager << "\n";
                sBuffer << indenter << "Send=" << p->Send << "\n";
                sBuffer << indenter << "SendSize=" << p->SendSize << "\n";
                sBuffer << indenter << "Receive=" << p->Receive << "\n";
                sBuffer << indenter << "ReceiveSize=" << p->ReceiveSize << "\n}\n";
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
    const std::string indenter = IndentDefinition();
    StringStreamA sBuffer;
    if ( pIdentity)
    {
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
    StringStreamA sBuffer;
    constexpr unsigned int numberOfBits = sizeof(TW_UINT32) << 3;
    bool foundGroup = false;
    for (unsigned int i = 0; i < numberOfBits; ++i)
    {
        const unsigned int curGroup = static_cast<TW_UINT32>(1) << i;
        if ( SupportedGroups & curGroup )
        {
            auto it = CTL_ErrorStructDecoder::s_mapSupportedGroups.find( curGroup );
            if ( it != CTL_ErrorStructDecoder::s_mapSupportedGroups.end() )
            {
                if ( foundGroup )
                    sBuffer << ",";
                sBuffer << " " << it->second;
                foundGroup = true;
            }
            else
                sBuffer << " Unknown" << curGroup << "";
        }
    }
    return sBuffer.str();
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
