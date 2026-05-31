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
#include "ctldib.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "ctlloadresources.h"
#include "ctldib32ex.h"
#include "logwriterutils.h"

using namespace dynarithmic;

boost::container::flat_map<LONG, std::vector<uint16_t>> CTL_ImageIOHandler::s_supportedBitDepths;

CTL_ImageIOHandler::CTL_ImageIOHandler() : 
    pMultiDibData(nullptr), 
    m_nPage(0), 
    m_bOnePageWritten(false), 
    m_pDib(nullptr),
    m_sCopyright(GetResourceStringFromMap(IDS_DTWAIN_APPTITLE))
{}

CTL_ImageIOHandler::CTL_ImageIOHandler( CTL_TwainDib *pDib ): pMultiDibData(nullptr), m_nPage(0), m_bOnePageWritten(false), 
                                        m_pDib(pDib),
                                        m_sCopyright(GetResourceStringFromMap(IDS_DTWAIN_APPTITLE))
{}

void CTL_ImageIOHandler::SetMultiDibInfo(const DibMultiPageStruct &s)
{
    m_DibMultiPageStruct = s;
}

DibMultiPageStruct CTL_ImageIOHandler::GetMultiDibInfo() const
{
    return m_DibMultiPageStruct;
}

bool CTL_ImageIOHandler::IsValidBitDepth(LONG FileType, LONG bitDepth)
{
    const auto it = s_supportedBitDepths.find(FileType);
    if (it != s_supportedBitDepths.end())
    {
        const auto it2 = std::find(it->second.begin(), it->second.end(), bitDepth);
        if (it2 == it->second.end())
            return false;
    }
    return true;
}

int CTL_ImageIOHandler::WriteBitmapImpl(LPCTSTR szFile, int nFormat, bool bOpenFile, int fh, DibMultiPageStruct* pMultiDibStruct/* = nullptr*/)
{
    if (!m_pDib || !m_pDib->GetHandle())
        return DTWAIN_ERR_DIB;

    if (!IsValidBitDepth(nFormat, m_pDib->GetBitsPerPixel()))
        return DTWAIN_ERR_INVALID_BITDEPTH;

    return WriteBitmap(szFile, bOpenFile, fh, pMultiDibStruct);
}

void CTL_ImageIOHandler::SetPageWriteStatus(int nFormat, int Stage)
{
    if (CTL_StaticData::GetLogFilterFlags() != 0)
    {
        bool isFirstPage = (Stage == 0 || Stage == DIB_MULTI_FIRST);
        bool isLastPage = (Stage == 0 || Stage == DIB_MULTI_LAST);

        auto& availableFileTypes = CTL_StaticData::GetAvailableFileFormatsMap();
        auto iter = availableFileTypes.find(nFormat);
        std::string fileFormat = iter->second.m_formatName;

        if (isFirstPage)
        {
            LogWriterUtils::WriteLogInfoIndentedA("Writing " + fileFormat + " file");
            SetNumPagesWritten(1);
        }
        else
        if (!isLastPage)
        {
            auto numPages = GetNumPagesWritten();
            SetNumPagesWritten(numPages + 1);
            LogWriterUtils::WriteLogInfoIndentedA("Writing " + fileFormat + " page " + std::to_string(numPages + 1));
        }
    }
}
