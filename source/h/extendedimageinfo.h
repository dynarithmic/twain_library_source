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
#ifndef EXTENDEDIMAGEINFO_H
#define EXTENDEDIMAGEINFO_H

#include <vector>
#include <string>
#include <array>
#include <twain.h>

/* The ExtendedImageInfo and helper classes maintain a cache of the TWAIN Source's
   "Extended Image Information.  For more information, see the TWAIN Specification 2.5, 
    Chapter 9, "Extended Image Information Definitions*/
namespace dynarithmic
{
    struct ExtendedImageInfo_Barcode
    {
        TW_UINT32 count = 0;
        std::vector<TW_UINT32> vConfidence;
        std::vector<TW_UINT32> vRotation;
        std::vector<TW_UINT32> vLength;
        std::vector<TW_UINT32> vXCoordinate;
        std::vector<TW_UINT32> vYCoordinate;
        std::vector<TW_UINT32> vType;
        std::vector<std::string> vText;
    };

    struct ExtendedImageInfo_PageSourceInfo
    {
        std::string bookname;
        TW_UINT32 chapterNumber = 0;
        TW_UINT32 documentNumber = 0;
        TW_UINT32 pageNumber = 0;
        std::string camera;
        TW_UINT32 frameNumber = 0;
        TW_FRAME frame{};
        TW_UINT16 pixelFlavor = 0;
    };

    struct ExtendedImageInfo_SkewDetectionInfo
    {
        TW_UINT32 DeskewStatus{};
        TW_UINT32 OriginalAngle{};
        TW_UINT32 FinalAngle{};
        TW_UINT32 Confidence{};
        TW_UINT32 WindowX1{};
        TW_UINT32 WindowX2{};
        TW_UINT32 WindowX3{};
        TW_UINT32 WindowX4{};
        TW_UINT32 WindowY1{};
        TW_UINT32 WindowY2{};
        TW_UINT32 WindowY3{};
        TW_UINT32 WindowY4{};
    };

    struct ExtendedImageInfo_ShadedAreaDetectionInfo
    {
        TW_UINT32 top = 0;
        TW_UINT32 left = 0;
        TW_UINT32 height = 0;
        TW_UINT32 width = 0;
        TW_UINT32 size = 0;
        TW_UINT32 blackCountOld = 0;
        TW_UINT32 blackCountNew = 0;
        TW_UINT32 blackRLMin = 0;
        TW_UINT32 blackRLMax = 0;
        TW_UINT32 whiteCountOld = 0;
        TW_UINT32 whiteCountNew = 0;
        TW_UINT32 whiteRLMin = 0;
        TW_UINT32 whiteRLMax = 0;
        TW_UINT32 whiteRLAvg = 0;
    };

    struct ExtendedImageInfo_ShadedAreaDetectionInfoV
    {
        TW_UINT32 count = 0;
        std::vector<TW_UINT32> topV;
        std::vector<TW_UINT32> leftV;
        std::vector<TW_UINT32> heightV;
        std::vector<TW_UINT32> widthV;
        std::vector<TW_UINT32> sizeV;
        std::vector<TW_UINT32> blackCountOldV;
        std::vector<TW_UINT32> blackCountNewV;
        std::vector<TW_UINT32> blackRLMinV;
        std::vector<TW_UINT32> blackRLMaxV;
        std::vector<TW_UINT32> whiteCountOldV;
        std::vector<TW_UINT32> whiteCountNewV;
        std::vector<TW_UINT32> whiteRLMinV;
        std::vector<TW_UINT32> whiteRLMaxV;
        std::vector<TW_UINT32> whiteRLAvgV;
    };

    struct ExtendedImageInfo_ShadedAreaDetection
    {
        std::vector<ExtendedImageInfo_ShadedAreaDetectionInfo> m_vShadeInfos;
    };

    struct ExtendedImageInfo_SpeckleRemovalInfo
    {
        TW_UINT32 specklesRemoved = 0;
        TW_UINT32 whiteSpecklesRemoved = 0;
        TW_UINT32 blackSpecklesRemoved = 0;
    };

    struct ExtendedImageInfo_LineDetectionInfo
    {
        TW_UINT32 xCoordinate = 0;
        TW_UINT32 yCoordinate = 0;
        TW_UINT32 length = 0;
        TW_UINT32 thickness = 0;
    };

    struct ExtendedImageInfo_LineDetection
    {
        std::vector<ExtendedImageInfo_LineDetectionInfo> m_vLineInfo;
    };

    struct ExtendedImageInfo_FormsRecognition
    {
        std::vector<TW_UINT32> m_vConfidence;
        std::vector<std::string> m_vTemplateMatch;
        std::vector<TW_UINT32> m_vTemplatePageMatch;
        std::vector<TW_UINT32> m_vHorizontalDocOffset;
        std::vector<TW_UINT32> m_vVerticalDocOffset;
    };

    struct ExtendedImageInfo_ImageSegmentationInfo
    {
        std::string m_sICCProfile;
        TW_BOOL m_bLastSegment = {};
        TW_UINT32 m_segmentNumber = {};
    };

    struct ExtendedImageInfo_EndorsedTextInfo
    {
        std::string m_sEndorsedText;
    };

    struct ExtendedImageInfo_ExtendedImageInfo20
    {
        TW_UINT16 m_magType = {};
    };

    struct ExtendedImageInfo_ExtendedImageInfo21
    {
        std::string m_fileSystemSource;
        TW_BOOL m_imageMerged = {};
        std::vector<char> m_magData;
        TW_UINT32 m_magDataLength = {};
        TW_UINT16 m_pageSide = {};
    };

    struct ExtendedImageInfo_ExtendedImageInfo22
    {
        TW_UINT32 m_PaperCount = {};
    };

    struct ExtendedImageInfo_ExtendedImageInfo23
    {
        std::string m_PrinterText;
    };

    struct ExtendedImageInfo_ExtendedImageInfo24
    {
        std::string m_twainDirectMetaData;
    };

    struct ExtendedImageInfo_ExtendedImageInfo25
    {
        struct ImageAddressing
        {
            std::array<std::string, 5> m_AddressInfo = {};
            TW_UINT16 m_iaLevel = {};
        };
        TW_UINT16 m_printer = {};
        std::vector<std::string> m_barcodeText;
        ImageAddressing m_ImageAddressing;
    };

    struct ExtendedImageInfo_PatchCode
    {
        TW_UINT32 m_patchCode = {};
    };

    struct ExtendedImageInfoBlock
    {
        ExtendedImageInfo_SkewDetectionInfo m_skewDetection;
        ExtendedImageInfo_PageSourceInfo m_pageSource;
        ExtendedImageInfo_Barcode m_barcodeInfo;
        ExtendedImageInfo_ShadedAreaDetectionInfoV m_shadedInfo;
        ExtendedImageInfo_SpeckleRemovalInfo m_speckleRemoval;
        ExtendedImageInfo_LineDetection m_horizontalLineInfo;
        ExtendedImageInfo_LineDetection m_verticalLineInfo;
        ExtendedImageInfo_FormsRecognition m_formsRecognitionInfo;
        ExtendedImageInfo_ImageSegmentationInfo m_imageSementationInfo;
        ExtendedImageInfo_EndorsedTextInfo m_endorsedTextInfo;
        ExtendedImageInfo_ExtendedImageInfo20 m_extendedImageInfo20;
        ExtendedImageInfo_ExtendedImageInfo21 m_extendedImageInfo21;
        ExtendedImageInfo_ExtendedImageInfo22 m_extendedImageInfo22;
        ExtendedImageInfo_ExtendedImageInfo23 m_extendedImageInfo23;
        ExtendedImageInfo_ExtendedImageInfo24 m_extendedImageInfo24;
        ExtendedImageInfo_ExtendedImageInfo25 m_extendedImageInfo25;
        ExtendedImageInfo_PatchCode m_extendedImagePatchCode;
    };

    class ExtendedImageInformation
    {
    private:
        bool GenericFillLineInfo(ExtendedImageInfo_LineDetection& allInfo, int32_t itemCountType, const std::array<int32_t, 4>& itemsToGet);

    public:
        CTL_ITwainSource* m_theSource = nullptr;
        std::vector<LONG> m_vFoundTypes;
        bool infoRetrieved = false;
        bool m_bIsFillingInfo = false;

        ExtendedImageInfoBlock m_InfoBlock;
        ExtendedImageInformation(CTL_ITwainSource* theSource);
        ~ExtendedImageInformation() = default;
        void ClearInfo();
        bool BeginRetrieval();
        bool IsInfoRetrieved() const { return infoRetrieved; }
        void SetInfoRetrieved(bool bSet) { infoRetrieved = bSet; }
        bool IsInfoBeingFilled() const { return m_bIsFillingInfo; }

        /* The "Fill" functions get the extended information from the Source
         * and fills in the requisite structures with the information found */
        bool FillAllInfo();
        bool FillBarcodeInfo();
        bool FillPageSourceInfo();
        bool FillSkewInfo();
        bool FillShadedAreaInfo();
        bool FillSpeckleRemovalInfo();
        bool FillHorizontalLineInfo();
        bool FillVerticalLineInfo();
        bool FillFormsRecognitionInfo();
        bool FillImageSegmentationInfo();
        bool FillEndorsedTextInfo();
        bool FillExtendedImageInfo20();
        bool FillExtendedImageInfo21();
        bool FillExtendedImageInfo22();
        bool FillExtendedImageInfo23();
        bool FillExtendedImageInfo24();
        bool FillExtendedImageInfo25();
        bool FillPatchCodeInfo();

        /* The "Get" functions takes the information from the structs,
           and returns it in the form of a DTWAIN_ARRAY for the application
           to query and process */
        DTWAIN_ARRAY GetBarcodeInfo(long nWhichInfo);
        DTWAIN_ARRAY GetPageSourceInfo(long nWhichInfo);
        DTWAIN_ARRAY GetSkewInfo(long nWhichInfo);
        DTWAIN_ARRAY GetShaderAreaInfo(long nWhichInfo);
        DTWAIN_ARRAY GetSpeckleRemovalInfo(long nWhichInfo);
        DTWAIN_ARRAY GetHorizontalVerticalLineInfo(long nWhichInfo);
        DTWAIN_ARRAY GetFillFormsRecognitionInfo(long nWhichInfo);
        DTWAIN_ARRAY GetImageSegmentationInfo(long nWhichInfo);
        DTWAIN_ARRAY GetEndorsedTextInfo(long nWhichInfo);
        DTWAIN_ARRAY GetExtendedImageInfo20(long nWhichInfo);
        DTWAIN_ARRAY GetExtendedImageInfo21(long nWhichInfo);
        DTWAIN_ARRAY GetExtendedImageInfo22(long nWhichInfo);
        DTWAIN_ARRAY GetExtendedImageInfo23(long nWhichInfo);
        DTWAIN_ARRAY GetExtendedImageInfo24(long nWhichInfo);
        DTWAIN_ARRAY GetExtendedImageInfo25(long nWhichInfo);
        DTWAIN_ARRAY GetPatchCodeInfo(long nWhichInfo);
    };
}
#endif

