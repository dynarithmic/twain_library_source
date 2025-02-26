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
#include "dtwain.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "arrayfactory.h"
#include "extendedimageinfo.h"
#include "ctlarray.h"
using namespace dynarithmic;

ExtendedImageInformation::ExtendedImageInformation(CTL_ITwainSource* pSource) : m_theSource(pSource)
{
}

void ExtendedImageInformation::ClearInfo()
{
    m_InfoBlock = {};
    infoRetrieved = false;
}

bool ExtendedImageInformation::BeginRetrieval()
{
    struct IsFillingRAII
    {
        bool* m_pFilling = nullptr;
        IsFillingRAII(bool* pFilling) : m_pFilling(pFilling) {}
        ~IsFillingRAII() { *m_pFilling = false; }
    };

    IsFillingRAII raii(&m_bIsFillingInfo);

    ClearInfo();
    m_vFoundTypes.clear();
    bool bOk = m_theSource->GetExtImageInfo(true);
    if (!bOk)
        return false;
    infoRetrieved = false;
    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii2(m_theSource->GetDTWAINHandle(), &aValues);
    auto pHandle = m_theSource->GetDTWAINHandle();
    DTWAIN_EnumExtImageInfoTypes(m_theSource, &aValues);
    if (!aValues)
        return false;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
    for (auto val : vValues)
        m_vFoundTypes.push_back(val);
    if (!vValues.empty())
    {
        infoRetrieved = true;
        m_bIsFillingInfo = true;
        FillAllInfo();
    }
    return infoRetrieved;
}

bool ExtendedImageInformation::FillBarcodeInfo()
{
    if (!infoRetrieved)
        return false;

    auto pHandle = m_theSource->GetDTWAINHandle();
    DTWAIN_ARRAY aCountX = {}, aCountY = {}, aType = {}, aText = {}, aRotation = {}, aConfidence = {}, aTextLength = {}, aCount{};
    std::array<LPDTWAIN_ARRAY, 8> aVects = { &aCountX, &aCountY, &aType, &aText, &aRotation, &aConfidence, &aTextLength, &aCount };
    std::array<DTWAINArrayPtr_RAII, 8> aRAII;
    size_t curArray = 0;

    for (auto& val : aVects)
    {
        aRAII[curArray].SetHandle(pHandle);
        aRAII[curArray].SetArray(val);
        ++curArray;
    }

    // Get the barcode count information
    LONG barCodeCount = 0;
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_BARCODECOUNT, &aCount);
    if (!aCount)
        return true;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aCount);
    if (vValues.empty())
        return true;
    barCodeCount = vValues.front();
    if (barCodeCount == 0)
        return true;

    m_InfoBlock.m_barcodeInfo.m_vBarInfos.resize(barCodeCount);

    // Get the barcode details
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_BARCODEX, &aCountX);
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_BARCODEY, &aCountY);
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_BARCODETYPE, &aType);
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_BARCODETEXT, &aText);
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_BARCODEROTATION, &aRotation);
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_BARCODECONFIDENCE, &aConfidence);
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_BARCODETEXTLENGTH, &aTextLength);

    int lastLen = 0;

    // Fill in the barcode texts
    auto& vHandles = pHandle->m_ArrayFactory->underlying_container_t<DTWAIN_HANDLE>(aText);
    auto& vLengths = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aTextLength);
    for (int i = 0; i < barCodeCount; ++i)
    {
        std::string szBarText;
        DTWAIN_HANDLE sHandle = vHandles[i];
        LONG length = vLengths[i];
        HandleRAII raii(sHandle);
        char* pText = (char*)raii.getData();
        if (pText)
            szBarText = std::string(pText + lastLen, length);
        lastLen += length;
        m_InfoBlock.m_barcodeInfo.m_vBarInfos[i].text = szBarText;
    }

    // Fill in the other information
    std::array<LONG, 5> aCounts = { aCountX ? static_cast<LONG>(pHandle->m_ArrayFactory->underlying_container_t<LONG>(aCountX).size()) : 0,
                                    aCountY ? static_cast<LONG>(pHandle->m_ArrayFactory->underlying_container_t<LONG>(aCountY).size()) : 0,
                                    aType ? static_cast<LONG>(pHandle->m_ArrayFactory->underlying_container_t<LONG>(aType).size()) : 0,
                                    aRotation ? static_cast<LONG>(pHandle->m_ArrayFactory->underlying_container_t<LONG>(aRotation).size()) : 0,
                                    aConfidence ? static_cast<LONG>(pHandle->m_ArrayFactory->underlying_container_t<LONG>(aConfidence).size()) : 0 };
    
    LONG* pBufferX = aCounts[0] ? pHandle->m_ArrayFactory->underlying_container_t<LONG>(aCountX).data() : nullptr; 
    LONG* pBufferY = aCounts[1] ? pHandle->m_ArrayFactory->underlying_container_t<LONG>(aCountY).data() : nullptr;

    for (int i = 0; i < (std::min)(aCounts[0], aCounts[1]); ++i)
    {
        m_InfoBlock.m_barcodeInfo.m_vBarInfos[i].xCoordinate = pBufferX[i];
        m_InfoBlock.m_barcodeInfo.m_vBarInfos[i].yCoordinate = pBufferY[i];
    }

    LONG* pType = aCounts[2] ? pHandle->m_ArrayFactory->underlying_container_t<LONG>(aType).data() : nullptr; 
    for (int i = 0; i < aCounts[2]; ++i)
        m_InfoBlock.m_barcodeInfo.m_vBarInfos[i].type = pType[i];

    LONG* pRotation = aCounts[3] ? pHandle->m_ArrayFactory->underlying_container_t<LONG>(aRotation).data() : nullptr; 
    for (int i = 0; i < aCounts[3]; ++i)
        m_InfoBlock.m_barcodeInfo.m_vBarInfos[i].rotation = pRotation[i];

    LONG* pConfidence = aCounts[4] ? pHandle->m_ArrayFactory->underlying_container_t<LONG>(aConfidence).data() : nullptr;
    for (int i = 0; i < aCounts[4]; ++i)
        m_InfoBlock.m_barcodeInfo.m_vBarInfos[i].confidence = pConfidence[i];

    return true;
}

bool ExtendedImageInformation::FillPageSourceInfo()
{
    if (!infoRetrieved)
        return false;

    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);
    std::array<int32_t, 2> stringItems = { TWEI_CAMERA, TWEI_BOOKNAME };
    std::array<std::string*, 2> refStrings = { &m_InfoBlock.m_pageSource.camera, &m_InfoBlock.m_pageSource.bookname };
    auto pHandle = m_theSource->GetDTWAINHandle();

    for (size_t i = 0; i < stringItems.size(); ++i)
    {
        DTWAIN_GetExtImageInfoData(m_theSource, stringItems[i], &aValues);
        if (!aValues)
            continue;
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<std::string>(aValues);
        if (!vValues.empty())
            *(refStrings[i]) = vValues[i];
    }

    {
        std::array<int32_t, 4> intItems = { TWEI_CHAPTERNUMBER, TWEI_DOCUMENTNUMBER, TWEI_PAGENUMBER, TWEI_FRAMENUMBER };
        std::array<TW_UINT32*, 4> refInts = { &m_InfoBlock.m_pageSource.chapterNumber, &m_InfoBlock.m_pageSource.documentNumber, &m_InfoBlock.m_pageSource.pageNumber, &m_InfoBlock.m_pageSource.frameNumber };

        for (size_t i = 0; i < intItems.size(); ++i)
        {
            DTWAIN_GetExtImageInfoData(m_theSource, intItems[i], &aValues);
            if (!aValues)
                continue;
            auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
            if (!vValues.empty())
                *(refInts[i]) = vValues.front();
        }
    }

    {
        std::array<int32_t, 1> intItems = { TWEI_PIXELFLAVOR };
        std::array<TW_UINT16*, 1> refInts = { &m_InfoBlock.m_pageSource.pixelFlavor };

        for (size_t i = 0; i < intItems.size(); ++i)
        {
            DTWAIN_GetExtImageInfoData(m_theSource, intItems[i], &aValues);
            if (!aValues)
                continue;
            auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
            if (!vValues.empty())
            {
                LONG lVal = vValues.front();
                *(refInts[i]) = static_cast<TW_UINT16>(lVal);
            }
        }
    }

    // Get the frame
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_FRAME, &aValues);
    if (aValues)
    {
        auto& vFrames = pHandle->m_ArrayFactory->underlying_container_t<TwainFrameInternal>(aValues);
        if (vFrames.size() == 1) // There must be 1 item describing the frame
        {
            // Convert to TW_FIX32.  The resulting DTWAIN_ARRAY will consist of 4 TW_FIX32 values
            // describing left, top, right, and bottom of the frame
            DTWAIN_ARRAY aFix32 = DTWAIN_ArrayConvertFloatToFix32(aValues);
            DTWAINArrayPtr_RAII raii2(m_theSource->GetDTWAINHandle(), &aFix32);
            auto& vFix32 = pHandle->m_ArrayFactory->underlying_container_t<TW_FIX32>(aFix32);
            m_InfoBlock.m_pageSource.frame.Left.Whole = vFix32[TwainFrameInternal::FRAMELEFT].Whole;
            m_InfoBlock.m_pageSource.frame.Left.Frac = vFix32[TwainFrameInternal::FRAMELEFT].Frac;
            m_InfoBlock.m_pageSource.frame.Top.Whole = vFix32[TwainFrameInternal::FRAMETOP].Whole;
            m_InfoBlock.m_pageSource.frame.Top.Frac = vFix32[TwainFrameInternal::FRAMETOP].Frac;
            m_InfoBlock.m_pageSource.frame.Right.Whole = vFix32[TwainFrameInternal::FRAMERIGHT].Whole;
            m_InfoBlock.m_pageSource.frame.Right.Frac = vFix32[TwainFrameInternal::FRAMERIGHT].Frac;
            m_InfoBlock.m_pageSource.frame.Bottom.Whole = vFix32[TwainFrameInternal::FRAMEBOTTOM].Whole;
            m_InfoBlock.m_pageSource.frame.Bottom.Frac = vFix32[TwainFrameInternal::FRAMEBOTTOM].Frac;
        }
    }
    return true;
}

bool ExtendedImageInformation::FillSkewInfo()
{
    if (!infoRetrieved)
        return false;

    auto pHandle = m_theSource->GetDTWAINHandle();

    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);
    std::array<int32_t, 12> intItems = { TWEI_DESKEWSTATUS, TWEI_SKEWORIGINALANGLE,
                                        TWEI_SKEWFINALANGLE, TWEI_SKEWCONFIDENCE,
                                        TWEI_SKEWWINDOWX1, TWEI_SKEWWINDOWY1,
                                        TWEI_SKEWWINDOWX2, TWEI_SKEWWINDOWY2,
                                        TWEI_SKEWWINDOWX3, TWEI_SKEWWINDOWY3,
                                        TWEI_SKEWWINDOWX4, TWEI_SKEWWINDOWY4 };

    std::array<TW_UINT32*, 12> refInts = { &m_InfoBlock.m_skewDetection.DeskewStatus, &m_InfoBlock.m_skewDetection.OriginalAngle,
                                            &m_InfoBlock.m_skewDetection.FinalAngle, &m_InfoBlock.m_skewDetection.Confidence,
                                            &m_InfoBlock.m_skewDetection.WindowX1, &m_InfoBlock.m_skewDetection.WindowY1,
                                            &m_InfoBlock.m_skewDetection.WindowX2, &m_InfoBlock.m_skewDetection.WindowY2,
                                            &m_InfoBlock.m_skewDetection.WindowX3, &m_InfoBlock.m_skewDetection.WindowY3,
                                            &m_InfoBlock.m_skewDetection.WindowX4, &m_InfoBlock.m_skewDetection.WindowY4 };

    for (size_t i = 0; i < intItems.size(); ++i)
    {
        DTWAIN_GetExtImageInfoData(m_theSource, intItems[i], &aValues);
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<DWORD>(aValues);
        if (!vValues.empty())
            *(refInts[i]) = vValues.front();
    }
    return true;
}

bool ExtendedImageInformation::FillShadedAreaInfo()
{
    if (!infoRetrieved)
        return false;

    auto pHandle = m_theSource->GetDTWAINHandle();

    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);

    // Get the count information
    LONG shadeCount = 0;
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_DESHADECOUNT, &aValues);
    if (!aValues)
        return true;

    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);

    if ( vValues.empty())
        return true;
    shadeCount = vValues[0];
    if (shadeCount == 0)
        return true;

    m_InfoBlock.m_shadedInfo.count = shadeCount;

    std::array<int32_t, 14> intItems = { TWEI_DESHADELEFT,
                                        TWEI_DESHADETOP,
                                        TWEI_DESHADEWIDTH,
                                        TWEI_DESHADEHEIGHT,
                                        TWEI_DESHADESIZE,
                                        TWEI_DESHADEBLACKCOUNTOLD,
                                        TWEI_DESHADEBLACKCOUNTNEW,
                                        TWEI_DESHADEBLACKRLMIN,
                                        TWEI_DESHADEBLACKRLMAX,
                                        TWEI_DESHADEWHITECOUNTOLD,
                                        TWEI_DESHADEWHITECOUNTNEW,
                                        TWEI_DESHADEWHITERLMIN,
                                        TWEI_DESHADEWHITERLMAX,
                                        TWEI_DESHADEWHITERLAVE };

    std::array<std::vector<TW_UINT32>*, 14> ptrVect = { &m_InfoBlock.m_shadedInfo.leftV,
                                                        &m_InfoBlock.m_shadedInfo.topV,
                                                        &m_InfoBlock.m_shadedInfo.widthV,
                                                        &m_InfoBlock.m_shadedInfo.heightV,
                                                        &m_InfoBlock.m_shadedInfo.sizeV,
                                                        &m_InfoBlock.m_shadedInfo.blackCountOldV,
                                                        &m_InfoBlock.m_shadedInfo.blackCountNewV,
                                                        &m_InfoBlock.m_shadedInfo.blackRLMinV,
                                                        &m_InfoBlock.m_shadedInfo.blackRLMaxV,
                                                        &m_InfoBlock.m_shadedInfo.whiteCountOldV,
                                                        &m_InfoBlock.m_shadedInfo.whiteCountNewV,
                                                        &m_InfoBlock.m_shadedInfo.whiteRLMinV,
                                                        &m_InfoBlock.m_shadedInfo.whiteRLMaxV,
                                                        &m_InfoBlock.m_shadedInfo.whiteRLAvgV };
    for (size_t i = 0; i < intItems.size(); ++i)
    {
        DTWAIN_GetExtImageInfoData(m_theSource, intItems[i], &aValues);
        if (!aValues)
            continue;
        auto& vValues2 = pHandle->m_ArrayFactory->underlying_container_t<DWORD>(aValues);
        for (auto val : vValues2)
            ptrVect[i]->push_back(val);
    }
    std::for_each(ptrVect.begin(), ptrVect.end(), [&](auto* pVect) { pVect->resize(m_InfoBlock.m_shadedInfo.count);  });
    return true;
}

bool ExtendedImageInformation::FillSpeckleRemovalInfo()
{
    if (!infoRetrieved)
        return false;

    auto pHandle = m_theSource->GetDTWAINHandle();

    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);

    std::array<int32_t, 3> intItems = { TWEI_SPECKLESREMOVED,
                                        TWEI_BLACKSPECKLESREMOVED,
                                        TWEI_WHITESPECKLESREMOVED };
    std::array<TW_UINT32*, 3> refInts = { &m_InfoBlock.m_speckleRemoval.specklesRemoved,
                                          &m_InfoBlock.m_speckleRemoval.blackSpecklesRemoved,
                                          &m_InfoBlock.m_speckleRemoval.whiteSpecklesRemoved };
    for (size_t i = 0; i < intItems.size(); ++i)
    {
        DTWAIN_GetExtImageInfoData(m_theSource, intItems[i], &aValues);
        if (!aValues)
            continue;
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<DWORD>(aValues);
        if (!vValues.empty())
            *(refInts[i]) = vValues.front();
    }
    return true;
}

bool ExtendedImageInformation::GenericFillLineInfo(ExtendedImageInfo_LineDetection& allInfo,
                                                   int32_t itemCountType, const std::array<int32_t, 4>& intItems)
{
    if (!infoRetrieved)
        return false;

    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);

    // Get the count information
    LONG lineCount = 0;
    DTWAIN_GetExtImageInfoData(m_theSource, itemCountType, &aValues);
    if (!aValues)
        return true;
    auto pHandle = m_theSource->GetDTWAINHandle();

    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
    if (vValues.empty())
        return true;
    lineCount = vValues.front();
    if (lineCount == 0)
        return true;

    m_InfoBlock.m_horizontalLineInfo.m_vLineInfo.resize(lineCount);

    std::array<DTWAIN_ARRAY, 4> aAllValues;
    std::array<LPDTWAIN_ARRAY, 4> aVects = { &aAllValues[0], &aAllValues[1], &aAllValues[2], &aAllValues[3] };
    std::array<LONG, 4> allCounts{};
    std::array<DTWAINArrayPtr_RAII, 4> aRAII;
    std::array<std::vector<LONG>*, 4> vAllValues = { nullptr, nullptr, nullptr, nullptr };
    size_t curArray = 0;
    for (auto& val : aVects)
    {
        aRAII[curArray].SetHandle(pHandle);
        aRAII[curArray].SetArray(val);
        ++curArray;
    }

    for (size_t i = 0; i < intItems.size(); ++i)
    {
        DTWAIN_GetExtImageInfoData(m_theSource, intItems[i], &aAllValues[i]);
        if (aAllValues[i])
        {
            auto& vValues2 = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aAllValues[i]);
            allCounts[i] = vValues2.size();
            vAllValues[i] = &vValues2;
        }
        else
            allCounts[i] = 0;
    }
    LONG maxCount = *std::min_element(allCounts.begin(), allCounts.end());
    for (LONG i = 0; i < maxCount; ++i)
    {
        ExtendedImageInfo_LineDetectionInfo oneLine{};
        if (vAllValues[0])
            oneLine.xCoordinate = (*vAllValues[0])[i];
        if (vAllValues[1])
            oneLine.yCoordinate = (*vAllValues[1])[i];
        if (vAllValues[2])
            oneLine.length = (*vAllValues[2])[i]; 
        if (vAllValues[3])
            oneLine.thickness = (*vAllValues[3])[i]; 
        allInfo.m_vLineInfo.push_back(oneLine);
    }
    return true;
}

bool ExtendedImageInformation::FillHorizontalLineInfo()
{
    return GenericFillLineInfo(m_InfoBlock.m_horizontalLineInfo, TWEI_HORZLINECOUNT, { TWEI_HORZLINEXCOORD,
                                                                           TWEI_HORZLINEYCOORD,
                                                                           TWEI_HORZLINELENGTH,
                                                                           TWEI_HORZLINETHICKNESS });
}

bool ExtendedImageInformation::FillVerticalLineInfo()
{
    return GenericFillLineInfo(m_InfoBlock.m_verticalLineInfo, TWEI_VERTLINECOUNT, { TWEI_VERTLINEXCOORD,
                                                                           TWEI_VERTLINEYCOORD,
                                                                           TWEI_VERTLINELENGTH,
                                                                           TWEI_VERTLINETHICKNESS });
}

bool ExtendedImageInformation::FillFormsRecognitionInfo()
{
    if (!infoRetrieved)
        return false;

    auto pHandle = m_theSource->GetDTWAINHandle();

    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);

    // Get the template match information
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_FORMTEMPLATEMATCH, &aValues);
    if (!aValues)
        return true;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<std::string>(aValues);
    for (auto& str : vValues)
        m_InfoBlock.m_formsRecognitionInfo.m_vTemplateMatch.push_back(str);

    std::array<int32_t, 4> intTypes = { TWEI_FORMCONFIDENCE, TWEI_FORMTEMPLATEPAGEMATCH, TWEI_FORMHORZDOCOFFSET, TWEI_FORMVERTDOCOFFSET };
    std::array<std::vector<TW_UINT32>*, 4> ptrVects = { &m_InfoBlock.m_formsRecognitionInfo.m_vConfidence, &m_InfoBlock.m_formsRecognitionInfo.m_vTemplatePageMatch,
                                                        &m_InfoBlock.m_formsRecognitionInfo.m_vHorizontalDocOffset, &m_InfoBlock.m_formsRecognitionInfo.m_vVerticalDocOffset };
    for (size_t i = 0; i < intTypes.size(); ++i)
    {
        DTWAIN_GetExtImageInfoData(m_theSource, intTypes[i], &aValues);
        if (!aValues)
            continue;
        auto& vValues2 = pHandle->m_ArrayFactory->underlying_container_t<DWORD>(aValues);
        std::transform(vValues2.begin(), vValues2.end(), std::back_inserter(*(ptrVects[i])), [&](LONG val) { return static_cast<TW_UINT32>(val); });
    }
    return true;
}

bool ExtendedImageInformation::FillImageSegmentationInfo()
{
    if (!infoRetrieved)
        return false;

    auto pHandle = m_theSource->GetDTWAINHandle();

    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);

    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_ICCPROFILE, &aValues);
    if (aValues)
    {
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<std::string>(aValues);
        if (!vValues.empty())
            m_InfoBlock.m_imageSementationInfo.m_sICCProfile = vValues.front();
    }

    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_LASTSEGMENT, &aValues);
    if (aValues)
    {
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
        if (!vValues.empty())
            m_InfoBlock.m_imageSementationInfo.m_bLastSegment = static_cast<TW_BOOL>(vValues.front());
    }

    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_SEGMENTNUMBER, &aValues);
    if (aValues)
    {
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<DWORD>(aValues);
        if (!vValues.empty())
            m_InfoBlock.m_imageSementationInfo.m_segmentNumber = vValues.front();
    }
    return true;
}

bool ExtendedImageInformation::FillEndorsedTextInfo()
{
    if (!infoRetrieved)
        return false;

    auto pHandle = m_theSource->GetDTWAINHandle();

    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_ENDORSEDTEXT, &aValues);
    if (!aValues)
        return true;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<std::string>(aValues);
    if (!vValues.empty())
        m_InfoBlock.m_endorsedTextInfo.m_sEndorsedText = vValues.front();
    return true;
}

bool ExtendedImageInformation::FillExtendedImageInfo20()
{
    if (!infoRetrieved)
        return false;
    auto pHandle = m_theSource->GetDTWAINHandle();
    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);

    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_MAGTYPE, &aValues);
    if (!aValues)
        return true;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
    if (!vValues.empty())
        m_InfoBlock.m_extendedImageInfo20.m_magType = static_cast<TW_UINT16>(vValues.front());
    return true;
}

bool ExtendedImageInformation::FillExtendedImageInfo21()
{
    if (!infoRetrieved)
        return false;
    auto pHandle = m_theSource->GetDTWAINHandle();
    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);

    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_MAGDATALENGTH, &aValues);
    if (!aValues)
        return true;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
    LONG magDataLength = 0;
    if (!vValues.empty())
    {
        magDataLength = vValues[0];

        // Get the mag data
        DTWAIN_GetExtImageInfoData(m_theSource, TWEI_MAGDATA, &aValues);
        LONG nCount = DTWAIN_ArrayGetCount(aValues);
        DTWAIN_HANDLE sHandle = NULL;
        if (nCount > 0)
        {
            // Need to determine the array type
            auto arrayType = CTL_ArrayFactory::tagtype_to_arraytype(pHandle->m_ArrayFactory->tag_type(aValues));
            if (arrayType == DTWAIN_ARRAYANSISTRING)
            {
                auto& vValues2 = pHandle->m_ArrayFactory->underlying_container_t<std::string>(aValues);
                // Copy the string to the vector
                auto& szData = vValues2.front();
                m_InfoBlock.m_extendedImageInfo21.m_magData = std::vector<char>(szData.begin(), szData.end());
            }
            else
            {
                // The data is a handle, so maybe GlobalLock it?
                auto& vValues2 = pHandle->m_ArrayFactory->underlying_container_t<DTWAIN_HANDLE>(aValues);
                sHandle = vValues2.front();
                HandleRAII raii2(sHandle);
                char* pText = (char*)raii2.getData();
                if (pText)
                    m_InfoBlock.m_extendedImageInfo21.m_magData = std::vector<char>(pText, pText + magDataLength);
            }
            if (sHandle)
                GlobalFree(sHandle);
        }
    }
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_FILESYSTEMSOURCE, &aValues);
    if (aValues)
    {
        auto& vValues2 = pHandle->m_ArrayFactory->underlying_container_t<std::string>(aValues);
        if ( !vValues2.empty())
            m_InfoBlock.m_extendedImageInfo21.m_fileSystemSource = vValues2.front();
    }

    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_PAGESIDE, &aValues);
    if (aValues)
    {
        auto& vValues2 = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
        if (!vValues2.empty())
            m_InfoBlock.m_extendedImageInfo21.m_pageSide = static_cast<TW_UINT16>(vValues2.front());
    }

    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_IMAGEMERGED, &aValues);
    if (aValues)
    {
        auto& vValues2 = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
        if (!vValues2.empty())
            m_InfoBlock.m_extendedImageInfo21.m_imageMerged = static_cast<TW_UINT16>(vValues2.front());
    }
    return true;
}

bool ExtendedImageInformation::FillExtendedImageInfo22()
{
    if (!infoRetrieved)
        return false;
    auto pHandle = m_theSource->GetDTWAINHandle();
    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);

    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_PAPERCOUNT, &aValues);
    if (!aValues)
        return true;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
    if (!vValues.empty())
        m_InfoBlock.m_extendedImageInfo22.m_PaperCount = vValues.front();
    return true;
}

bool ExtendedImageInformation::FillExtendedImageInfo23()
{
    if (!infoRetrieved)
        return false;
    auto pHandle = m_theSource->GetDTWAINHandle();
    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);

    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_PRINTERTEXT, &aValues);
    if (!aValues)
        return true;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<std::string>(aValues);
    if (!vValues.empty())
        m_InfoBlock.m_extendedImageInfo23.m_PrinterText = vValues.front();
    return true;
}

bool ExtendedImageInformation::FillExtendedImageInfo24()
{
    if (!infoRetrieved)
        return false;
    auto pHandle = m_theSource->GetDTWAINHandle();
    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);

    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_TWAINDIRECTMETADATA, &aValues);
    if (!aValues)
        return true;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<DTWAIN_HANDLE>(aValues);

    if (!vValues.empty())
    {
        // This is a handle, so global lock it
        DTWAIN_HANDLE sHandle = vValues.front();
        if (sHandle)
        {
            HandleRAII raii2(sHandle);
            char* pData = (char*)raii2.getData();
            m_InfoBlock.m_extendedImageInfo24.m_twainDirectMetaData = pData;
        }
    }
    return true;
}

bool ExtendedImageInformation::FillExtendedImageInfo25()
{
    if (!infoRetrieved)
        return false;
    auto pHandle = m_theSource->GetDTWAINHandle();

    std::array<int32_t, 5> intTypes = { TWEI_IAFIELDA_VALUE, TWEI_IAFIELDB_VALUE, TWEI_IAFIELDC_VALUE, TWEI_IAFIELDD_VALUE, TWEI_IAFIELDE_VALUE };
    std::array<DTWAIN_ARRAY, 5> aValue;
    std::array<LPDTWAIN_ARRAY, 5> aVects = { &aValue[0], &aValue[1], &aValue[2], &aValue[3], &aValue[4] };
    std::array<DTWAINArrayPtr_RAII, 5> aRAII;
    size_t curArray = 0;

    for (auto& val : aVects)
    {
        aRAII[curArray].SetHandle(pHandle);
        aRAII[curArray].SetArray(val);
        ++curArray;
    }

    for (size_t i = 0; i < aValue.size(); ++i)
    {
        bool bOk = DTWAIN_GetExtImageInfoData(m_theSource, intTypes[i], &aValue[i]);
        if (!bOk)
            continue;
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<std::string>(aValue[i]);
        if (!vValues.empty())
            m_InfoBlock.m_extendedImageInfo25.m_ImageAddressing.m_AddressInfo[i] = vValues.front();
    }

    std::array<int32_t, 2> int16Types = { TWEI_IALEVEL, TWEI_PRINTER };
    std::array<TW_UINT16*, 2> ptrInt = { &m_InfoBlock.m_extendedImageInfo25.m_ImageAddressing.m_iaLevel, &m_InfoBlock.m_extendedImageInfo25.m_printer };
    for (size_t i = 0; i < int16Types.size(); ++i)
    {
        bool bOk = DTWAIN_GetExtImageInfoData(m_theSource, int16Types[i], &aValue[i]);
        if (!bOk)
            continue;
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValue[i]);
        if (!vValues.empty())
            *(ptrInt[i]) = static_cast<TW_UINT16>(vValues.front());
    }

    bool bOk = DTWAIN_GetExtImageInfoData(m_theSource, TWEI_BARCODETEXT2, &aValue[0]);
    if (!bOk)
        return true;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<DTWAIN_HANDLE>(aValue[0]);
    if (!vValues.empty())
    {
        // The first entry is a handle to either a string or more handles
        DTWAIN_HANDLE sHandle = vValues.front();
        if (vValues.size() == 1)
        {
            HandleRAII raii(sHandle);
            char* pData = (char*)raii.getData();
            m_InfoBlock.m_extendedImageInfo25.m_barcodeText.push_back(pData);
            return true;
        }
        else
        {
            // The first handle is a handle to nCount number of handles
            HandleRAII raii(sHandle);
            char* pData = (char*)raii.getData();
            for (size_t i = 0; i < vValues.size(); ++i)
            {
                // Get to each handle
                DTWAIN_HANDLE sHandle2 = ((DTWAIN_HANDLE*)pData)[i];
                if (sHandle2)
                {
                    // Lock handle
                    HandleRAII raii2(sHandle2);
                    char* pStrData = (char*)raii2.getData();
                    if (pStrData)
                        m_InfoBlock.m_extendedImageInfo25.m_barcodeText.push_back(pStrData);
                }
                else
                    m_InfoBlock.m_extendedImageInfo25.m_barcodeText.push_back({});
            }
        }
    }
    return true;
}

bool ExtendedImageInformation::FillPatchCodeInfo()
{
    if (!infoRetrieved)
        return false;
    auto pHandle = m_theSource->GetDTWAINHandle();
    DTWAIN_ARRAY aValues = {};
    DTWAINArrayPtr_RAII raii(m_theSource->GetDTWAINHandle(), &aValues);
    DTWAIN_GetExtImageInfoData(m_theSource, TWEI_PATCHCODE, &aValues);
    if (!aValues)
        return true;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
    if (!vValues.empty())
        m_InfoBlock.m_extendedImagePatchCode.m_patchCode = vValues.front();
    return true;
}

bool ExtendedImageInformation::FillAllInfo()
{
    using Fn = bool (ExtendedImageInformation::*)();
    std::array<Fn, 17> allFunctions = {
                                            &ExtendedImageInformation::FillBarcodeInfo,
                                            &ExtendedImageInformation::FillPageSourceInfo,
                                            &ExtendedImageInformation::FillSkewInfo,
                                            &ExtendedImageInformation::FillShadedAreaInfo,
                                            &ExtendedImageInformation::FillSpeckleRemovalInfo,
                                            &ExtendedImageInformation::FillHorizontalLineInfo,
                                            &ExtendedImageInformation::FillVerticalLineInfo,
                                            &ExtendedImageInformation::FillFormsRecognitionInfo,
                                            &ExtendedImageInformation::FillImageSegmentationInfo,
                                            &ExtendedImageInformation::FillEndorsedTextInfo,
                                            &ExtendedImageInformation::FillExtendedImageInfo20,
                                            &ExtendedImageInformation::FillExtendedImageInfo21,
                                            &ExtendedImageInformation::FillExtendedImageInfo22,
                                            &ExtendedImageInformation::FillExtendedImageInfo23,
                                            &ExtendedImageInformation::FillExtendedImageInfo24,
                                            &ExtendedImageInformation::FillExtendedImageInfo25,
                                            &ExtendedImageInformation::FillPatchCodeInfo };
    bool bOk = true;
    for (auto& curFn : allFunctions)
    {
        bool oneOk = (this->*curFn)();
        bOk = bOk || oneOk;
    }
    return bOk;
}

DTWAIN_ARRAY ExtendedImageInformation::GetBarcodeInfo(long nWhichInfo)
{
    std::vector<TW_UINT32> tempV;
    std::vector<std::string> tempVS;
    switch (nWhichInfo)
    {
        case TWEI_BARCODECOUNT:
            return CreateArrayFromContainer<std::vector<DWORD>>(m_theSource->GetDTWAINHandle(), 
                                                { static_cast<DWORD>(m_InfoBlock.m_barcodeInfo.m_vBarInfos.size()) });
        break;
        case TWEI_BARCODEX:
            std::transform(m_InfoBlock.m_barcodeInfo.m_vBarInfos.begin(),
                           m_InfoBlock.m_barcodeInfo.m_vBarInfos.end(),
                            std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.xCoordinate; });
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), tempV);
        break;
        case TWEI_BARCODEY:
            std::transform(m_InfoBlock.m_barcodeInfo.m_vBarInfos.begin(),
                m_InfoBlock.m_barcodeInfo.m_vBarInfos.end(),
                std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.yCoordinate; });
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), tempV);
        break;
        case TWEI_BARCODECONFIDENCE:
            std::transform(m_InfoBlock.m_barcodeInfo.m_vBarInfos.begin(),
                m_InfoBlock.m_barcodeInfo.m_vBarInfos.end(),
                std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.confidence; });
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), tempV);
        break;
        case TWEI_BARCODEROTATION:
            std::transform(m_InfoBlock.m_barcodeInfo.m_vBarInfos.begin(),
                m_InfoBlock.m_barcodeInfo.m_vBarInfos.end(),
                std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.rotation; });
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), tempV);
        break;
        case TWEI_BARCODETYPE:
            std::transform(m_InfoBlock.m_barcodeInfo.m_vBarInfos.begin(),
                m_InfoBlock.m_barcodeInfo.m_vBarInfos.end(),
                std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.type; });
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), tempV);
        break;
        case TWEI_BARCODETEXT:
            std::transform(m_InfoBlock.m_barcodeInfo.m_vBarInfos.begin(),
                m_InfoBlock.m_barcodeInfo.m_vBarInfos.end(),
                std::back_inserter(tempVS), [&](auto& oneInfo) { return oneInfo.text; });
            return CreateArrayFromContainer<std::vector<std::string>>(m_theSource->GetDTWAINHandle(), tempVS);
        break;
    }
    return nullptr;
}

DTWAIN_ARRAY ExtendedImageInformation::GetPageSourceInfo(long nWhichInfo)
{
    switch (nWhichInfo)
    {
        case TWEI_CAMERA:
            return CreateArrayFromContainer<std::vector<std::string>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_pageSource.camera });
        break;
        case TWEI_BOOKNAME:
            return CreateArrayFromContainer<std::vector<std::string>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_pageSource.bookname });
        break;
        case TWEI_CHAPTERNUMBER:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_pageSource.chapterNumber});
        break;
        case TWEI_DOCUMENTNUMBER:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_pageSource.documentNumber });
        break;
        case TWEI_PAGENUMBER:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_pageSource.pageNumber });
        break;
        case TWEI_FRAMENUMBER:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_pageSource.frameNumber });
        break;
        case TWEI_PIXELFLAVOR:
            return CreateArrayFromContainer<std::vector<TW_UINT16>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_pageSource.pixelFlavor });
        break;
        case TWEI_FRAME:
        {
            auto theFrame = DTWAIN_FrameCreate(0, 0, 0, 0);
            if (theFrame)
            {
                dynarithmic::TWFRAMEToDTWAINFRAME(m_InfoBlock.m_pageSource.frame, theFrame);
                return theFrame;
            }
        }
        break;
    }
    return nullptr;
}

DTWAIN_ARRAY ExtendedImageInformation::GetSkewInfo(long nWhichInfo)
{
    switch (nWhichInfo)
    {
        case TWEI_DESKEWSTATUS:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_skewDetection.DeskewStatus });
        break;
        case TWEI_SKEWORIGINALANGLE:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_skewDetection.OriginalAngle });
        break;
        case TWEI_SKEWFINALANGLE:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_skewDetection.FinalAngle });
        break;
        case TWEI_SKEWCONFIDENCE:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_skewDetection.Confidence });
        break;
        case TWEI_SKEWWINDOWX1:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_skewDetection.WindowX1 });
        break;
        case TWEI_SKEWWINDOWX2:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_skewDetection.WindowX2 });
        break;
        case TWEI_SKEWWINDOWX3:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_skewDetection.WindowX3 });
        break;
        case TWEI_SKEWWINDOWX4:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_skewDetection.WindowX4 });
        break;
        case TWEI_SKEWWINDOWY1:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_skewDetection.WindowY1 });
        break;
        case TWEI_SKEWWINDOWY2:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_skewDetection.WindowY2 });
        break;
        case TWEI_SKEWWINDOWY3:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_skewDetection.WindowY3 });
        break;
        case TWEI_SKEWWINDOWY4:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), { m_InfoBlock.m_skewDetection.WindowY4 });
        break;
    }
    return nullptr;
}

DTWAIN_ARRAY ExtendedImageInformation::GetShaderAreaInfo(long nWhichInfo)
{
    switch (nWhichInfo)
    {
        case TWEI_DESHADECOUNT:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(),
                                                    { static_cast<TW_UINT32>(m_InfoBlock.m_shadedInfo.count) });
        break;
        case TWEI_DESHADELEFT:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.leftV);
        break;
        case TWEI_DESHADETOP:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.topV);
        break;
        case TWEI_DESHADEWIDTH:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.widthV);
        break;
        case TWEI_DESHADEHEIGHT:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.heightV);
        break;
        case TWEI_DESHADESIZE:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.sizeV);
        break;
        case TWEI_DESHADEBLACKCOUNTOLD:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.blackCountOldV);
        break;
        case TWEI_DESHADEBLACKCOUNTNEW:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.blackCountNewV);
        break;
        case TWEI_DESHADEBLACKRLMIN:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.blackRLMinV);
        break;
        case TWEI_DESHADEBLACKRLMAX:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.blackRLMaxV);
        break;
        case TWEI_DESHADEWHITECOUNTOLD:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.whiteCountOldV);
        break;
        case TWEI_DESHADEWHITECOUNTNEW:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.whiteCountNewV);
        break;
        case TWEI_DESHADEWHITERLMIN:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.whiteRLMinV);
        break;
        case TWEI_DESHADEWHITERLMAX:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.whiteRLMaxV);
        break;
        case TWEI_DESHADEWHITERLAVE:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), m_InfoBlock.m_shadedInfo.whiteRLAvgV);
        break;
    }
    return nullptr;
}

DTWAIN_ARRAY ExtendedImageInformation::GetSpeckleRemovalInfo(long nWhichInfo)
{
    switch (nWhichInfo)
    {
        case TWEI_SPECKLESREMOVED:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(),
                { static_cast<TW_UINT32>(m_InfoBlock.m_speckleRemoval.specklesRemoved) });
        break;
        case TWEI_BLACKSPECKLESREMOVED:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(),
                { static_cast<TW_UINT32>(m_InfoBlock.m_speckleRemoval.blackSpecklesRemoved) });
        break;
        case TWEI_WHITESPECKLESREMOVED:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(),
                { static_cast<TW_UINT32>(m_InfoBlock.m_speckleRemoval.whiteSpecklesRemoved) });
        break;
    }
    return nullptr;
}

DTWAIN_ARRAY ExtendedImageInformation::GetHorizontalVerticalLineInfo(long nWhichInfo)
{
    std::vector<TW_UINT32> tempV;
    switch (nWhichInfo)
    {
        case TWEI_HORZLINECOUNT:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(),
                { static_cast<TW_UINT32>(m_InfoBlock.m_horizontalLineInfo.m_vLineInfo.size()) });
        break;
        case TWEI_HORZLINEXCOORD:
            std::transform(m_InfoBlock.m_horizontalLineInfo.m_vLineInfo.begin(),
                           m_InfoBlock.m_horizontalLineInfo.m_vLineInfo.end(),
                            std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.xCoordinate; });
        break;
        case TWEI_HORZLINEYCOORD:
            std::transform(m_InfoBlock.m_horizontalLineInfo.m_vLineInfo.begin(),
                           m_InfoBlock.m_horizontalLineInfo.m_vLineInfo.end(),
                            std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.yCoordinate; });
        break;
        case TWEI_HORZLINELENGTH:
            std::transform(m_InfoBlock.m_horizontalLineInfo.m_vLineInfo.begin(),
                           m_InfoBlock.m_horizontalLineInfo.m_vLineInfo.end(),
                            std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.length; });
        break;
        case TWEI_HORZLINETHICKNESS:
            std::transform(m_InfoBlock.m_horizontalLineInfo.m_vLineInfo.begin(),
                           m_InfoBlock.m_horizontalLineInfo.m_vLineInfo.end(),
                            std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.thickness; });
        break;
        case TWEI_VERTLINEXCOORD:
            std::transform(m_InfoBlock.m_verticalLineInfo.m_vLineInfo.begin(),
                           m_InfoBlock.m_verticalLineInfo.m_vLineInfo.end(),
                            std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.xCoordinate; });
        break;
        case TWEI_VERTLINEYCOORD:
            std::transform(m_InfoBlock.m_verticalLineInfo.m_vLineInfo.begin(),
                           m_InfoBlock.m_verticalLineInfo.m_vLineInfo.end(),
                            std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.yCoordinate; });
        break;
        case TWEI_VERTLINELENGTH:
            std::transform(m_InfoBlock.m_verticalLineInfo.m_vLineInfo.begin(),
                           m_InfoBlock.m_verticalLineInfo.m_vLineInfo.end(),
                            std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.length; });
        break;
        case TWEI_VERTLINETHICKNESS:
            std::transform(m_InfoBlock.m_verticalLineInfo.m_vLineInfo.begin(),
                           m_InfoBlock.m_verticalLineInfo.m_vLineInfo.end(),
                            std::back_inserter(tempV), [&](auto& oneInfo) { return oneInfo.thickness; });
        break;
    }
    return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(), tempV);
}

DTWAIN_ARRAY ExtendedImageInformation::GetFillFormsRecognitionInfo(long nWhichInfo)
{
    switch (nWhichInfo)
    {
        case TWEI_FORMTEMPLATEMATCH:
            return CreateArrayFromContainer<std::vector<std::string>>(m_theSource->GetDTWAINHandle(),
                                                                      m_InfoBlock.m_formsRecognitionInfo.m_vTemplateMatch);
        break;
        case TWEI_FORMTEMPLATEPAGEMATCH:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(),
                                    m_InfoBlock.m_formsRecognitionInfo.m_vTemplatePageMatch);
        break;
        case TWEI_FORMHORZDOCOFFSET:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(),
                                    m_InfoBlock.m_formsRecognitionInfo.m_vHorizontalDocOffset);
        break;
        case TWEI_FORMVERTDOCOFFSET:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(),
                                    m_InfoBlock.m_formsRecognitionInfo.m_vVerticalDocOffset);
        break;
    }
    return nullptr;
}

DTWAIN_ARRAY ExtendedImageInformation::GetImageSegmentationInfo(long nWhichInfo)
{
    switch (nWhichInfo)
    {
        case TWEI_ICCPROFILE:
            return CreateArrayFromContainer<std::vector<std::string>>(m_theSource->GetDTWAINHandle(),
                                                          { m_InfoBlock.m_imageSementationInfo.m_sICCProfile });
        break;
        case TWEI_LASTSEGMENT:
            return CreateArrayFromContainer<std::vector<TW_BOOL>>(m_theSource->GetDTWAINHandle(),
                                                          { m_InfoBlock.m_imageSementationInfo.m_bLastSegment });
        break;
        case TWEI_SEGMENTNUMBER:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(),
                { m_InfoBlock.m_imageSementationInfo.m_segmentNumber });
    }
    return nullptr;
}

DTWAIN_ARRAY ExtendedImageInformation::GetEndorsedTextInfo(long nWhichInfo)
{
    switch (nWhichInfo)
    {
        case TWEI_ENDORSEDTEXT:
            return CreateArrayFromContainer<std::vector<std::string>>(m_theSource->GetDTWAINHandle(),
                { m_InfoBlock.m_endorsedTextInfo.m_sEndorsedText });
        break;
    }
    return nullptr;
}

DTWAIN_ARRAY ExtendedImageInformation::GetExtendedImageInfo20(long nWhichInfo)
{
    switch (nWhichInfo)
    {
        case TWEI_MAGTYPE:
            return CreateArrayFromContainer<std::vector<TW_UINT16>>(m_theSource->GetDTWAINHandle(),
                { m_InfoBlock.m_extendedImageInfo20.m_magType });
        break;
    }
    return nullptr;
}

DTWAIN_ARRAY ExtendedImageInformation::GetExtendedImageInfo21(long nWhichInfo)
{
    switch (nWhichInfo)
    {
        case TWEI_MAGDATALENGTH:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(),
                { m_InfoBlock.m_extendedImageInfo21.m_magDataLength });
        break;
        case TWEI_MAGDATA:
        {
            std::string sTemp(m_InfoBlock.m_extendedImageInfo21.m_magData.data(),
                              m_InfoBlock.m_extendedImageInfo21.m_magData.size());
            sTemp += '\0';
            return CreateArrayFromContainer<std::vector<std::string>>(m_theSource->GetDTWAINHandle(), { sTemp });
        }
        break;
        case TWEI_FILESYSTEMSOURCE:
            return CreateArrayFromContainer<std::vector<std::string>>(m_theSource->GetDTWAINHandle(),
                { m_InfoBlock.m_extendedImageInfo21.m_fileSystemSource });
        break;
        case TWEI_PAGESIDE:
            return CreateArrayFromContainer<std::vector<TW_UINT16>>(m_theSource->GetDTWAINHandle(),
                { m_InfoBlock.m_extendedImageInfo21.m_pageSide });
        break;
        case TWEI_IMAGEMERGED:
            return CreateArrayFromContainer<std::vector<TW_BOOL>>(m_theSource->GetDTWAINHandle(),
                { m_InfoBlock.m_extendedImageInfo21.m_imageMerged });
        break;
     }
     return nullptr;
}

DTWAIN_ARRAY ExtendedImageInformation::GetExtendedImageInfo22(long nWhichInfo)
{
    switch (nWhichInfo)
    {
        case TWEI_PAPERCOUNT:
            return CreateArrayFromContainer<std::vector<TW_UINT32>>(m_theSource->GetDTWAINHandle(),
                { m_InfoBlock.m_extendedImageInfo22.m_PaperCount });
        break;
    }
    return nullptr;
}

DTWAIN_ARRAY ExtendedImageInformation::GetExtendedImageInfo23(long nWhichInfo)
{
    switch (nWhichInfo)
    {
        case TWEI_PRINTERTEXT:
            return CreateArrayFromContainer<std::vector<std::string>>(m_theSource->GetDTWAINHandle(),
                { m_InfoBlock.m_extendedImageInfo23.m_PrinterText });
        break;
    }
    return nullptr;
}

DTWAIN_ARRAY ExtendedImageInformation::GetExtendedImageInfo24(long nWhichInfo)
{
    switch (nWhichInfo)
    {
        case TWEI_TWAINDIRECTMETADATA:
            return CreateArrayFromContainer<std::vector<std::string>>(m_theSource->GetDTWAINHandle(),
                { m_InfoBlock.m_extendedImageInfo24.m_twainDirectMetaData});
        break;
    }
    return nullptr;
}


DTWAIN_ARRAY ExtendedImageInformation::GetExtendedImageInfo25(long nWhichInfo)
{
    static constexpr std::array<long, 5> aFields = {
        TWEI_IAFIELDA_VALUE, TWEI_IAFIELDB_VALUE, TWEI_IAFIELDC_VALUE, TWEI_IAFIELDD_VALUE, TWEI_IAFIELDE_VALUE };

    switch (nWhichInfo)
    {
        case TWEI_IAFIELDA_VALUE:
        case TWEI_IAFIELDB_VALUE:
        case TWEI_IAFIELDC_VALUE:
        case TWEI_IAFIELDD_VALUE:
        case TWEI_IAFIELDE_VALUE:
        {
            auto iter = std::find(aFields.begin(), aFields.end(), nWhichInfo);
            if (iter != aFields.end())
            {
                auto pos = std::distance(aFields.begin(), iter);
                return CreateArrayFromContainer<std::vector<std::string>>(m_theSource->GetDTWAINHandle(),
                    { m_InfoBlock.m_extendedImageInfo25.m_ImageAddressing.m_AddressInfo[pos] });
            }
        }
        break;
        case TWEI_IALEVEL:
            return CreateArrayFromContainer<std::vector<TW_UINT16>>(m_theSource->GetDTWAINHandle(),
                { m_InfoBlock.m_extendedImageInfo25.m_ImageAddressing.m_iaLevel});
        break;
        case TWEI_PRINTER:
            return CreateArrayFromContainer<std::vector<TW_UINT16>>(m_theSource->GetDTWAINHandle(),
                { m_InfoBlock.m_extendedImageInfo25.m_printer });
        break;
        case TWEI_BARCODETEXT2:
            return CreateArrayFromContainer<std::vector<std::string>>(m_theSource->GetDTWAINHandle(),
                                        m_InfoBlock.m_extendedImageInfo25.m_barcodeText);
        break;
    }
    return nullptr;
}
