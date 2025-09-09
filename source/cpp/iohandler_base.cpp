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
#include "ctldib.h"
#include "ctliface.h"
#include "ctltwainmanager.h"

using namespace dynarithmic;

boost::container::flat_map<LONG, std::vector<int>> CTL_ImageIOHandler::s_supportedBitDepths;

CTL_ImageIOHandler::CTL_ImageIOHandler() : bytesleft(0), nextbyte(0),
bytebuffer{}, bittable{}, masktable{}, pMultiDibData(nullptr), m_nPage(0), m_bAllWritten(true), m_bOnePageWritten(false)
{
    m_pDib = nullptr;
}

CTL_ImageIOHandler::CTL_ImageIOHandler( CTL_TwainDib *pDib ): bytesleft(0), nextbyte(0),
bytebuffer{}, bittable{}, masktable{}, pMultiDibData(nullptr), m_nPage(0), m_bAllWritten(true), m_bOnePageWritten(false)
{
    m_pDib = pDib;
}

void CTL_ImageIOHandler::SetMultiDibInfo(const DibMultiPageStruct &s)
{
    m_DibMultiPageStruct = s;
}

DibMultiPageStruct CTL_ImageIOHandler::GetMultiDibInfo() const
{
    return m_DibMultiPageStruct;
}

void CTL_ImageIOHandler::resetbuffer()
{
    bytesleft=0;
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

int CTL_ImageIOHandler::SaveToFile() const
{
    #ifdef _WIN32
    fipImage fw;
    if (!fipImageUtility::copyFromHandle(fw, m_SaveParams.hDib))
        return 1;
    fipWinImage_RAII raii(&fw);
    #else
        fipImage fw;
        fipMemoryIO memIO((BYTE *)hDib, 0);
    fw.loadFromMemory(FIF_TIFF, memIO, flags);
    #endif

    double multiplier = 39.37 * std::get<0>(m_SaveParams.multiplier_pr);
    if (m_SaveParams.unitOfMeasure == DTWAIN_CENTIMETERS)
        multiplier = 100.0 * std::get<1>(m_SaveParams.multiplier_pr);

    fw.setHorizontalResolution(m_SaveParams.res.first * multiplier + std::get<2>(m_SaveParams.multiplier_pr));
    fw.setVerticalResolution(m_SaveParams.res.second * multiplier + std::get<3>(m_SaveParams.multiplier_pr));

    fipTag fp;
    fp.setKeyValue(m_SaveParams.commentKey, CTL_StaticData::GetAppTitle().c_str());
    fw.setMetadata(m_SaveParams.metaDataTag, m_SaveParams.commentKey, fp);
    return fw.save(m_SaveParams.fmt, StringConversion::Convert_NativePtr_To_Ansi(m_SaveParams.szFile).c_str(),
                   m_SaveParams.flags) ? 0 : 1;
}
