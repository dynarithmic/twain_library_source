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

 #include "ctltwmgr.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#include "../nlohmann/json.hpp"
#include <boost/range/adaptor/transformed.hpp>

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

template <typename Iter>
static std::string join_string(Iter it1, Iter it2, char val = ',')
{
    std::stringstream strm;
    int i = 0;
    while (it1 != it2)
    {
        if (i != 0)
            strm << val;
        strm << *it1;
        ++it1;
        ++i;
    }
    return strm.str();
}

static std::string remove_quotes(std::string s)
{
    s.erase(std::remove(s.begin(), s.end(), '\"'), s.end());
    return s;
}

template <typename T>
static void create_stream(std::stringstream& strm, DTWAIN_SOURCE Source, LONG capValue)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    T* imageVals;
    DTWAIN_ARRAY arr = nullptr;
    DTWAIN_GetCapValues(Source, capValue, DTWAIN_CAPGET, &arr);
    LONG nCount = 0;
    DTWAINArrayPtr_RAII raii(&arr);
    if (arr)
    {
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<T>(arr);
        nCount = vValues.size();
        if (nCount == 0)
            strm << "\"<not available>\"";
        else
        {
            strm << "{";

            // check if range
            LONG status;
            if (DTWAIN_RangeIsValid(arr, &status))
                strm << "\"data-type\":\"range\",";
            else
                strm << "\"data-type\":\"discrete\",";
            strm << "\"data-values\":[" << join_string(vValues.begin(), vValues.end()) << "]}";
        }
    }
    else
        strm << "\"<not available>\"";
}

static void create_stream_from_strings(std::stringstream& strm, DTWAIN_SOURCE Source, LONG capValue)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    std::vector<std::string> imageVals;
    DTWAIN_ARRAY arr = nullptr;
    DTWAIN_GetCapValues(Source, capValue, DTWAIN_CAPGET, &arr); //capInfo.get_cap_values<std::vector<std::string>>(capValue);
    LONG nCount = 0;
    DTWAINArrayPtr_RAII raii(&arr);
    if (arr)
    {
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<std::string>(arr);
        nCount = vValues.size();
        if (nCount == 0)
            strm << "\"<not available>\"";
        else
        {
            std::transform(vValues.begin(), vValues.end(), 
                            std::back_inserter(imageVals), [](auto& s) { return "\"" + s + "\""; });
            strm << "{";
            strm << "\"data-values\":[" << join_string(imageVals.begin(), imageVals.end()) << "]}";
        }
    }
    else
        strm << "\"<not available>\"";
}

static std::string get_source_file_types(DTWAIN_SOURCE Source)
{
    using sourceMapType = std::unordered_map<LONG, std::string>;
    static sourceMapType source_map =
    {
        {DTWAIN_FF_BMP,"\"bmp1_mode2\""},
        {DTWAIN_FF_BMP,"\"bmp2_mode2\""},
        {DTWAIN_FF_BMP,"\"bmp3_mode2\""},
        {DTWAIN_FF_BMP,"\"bmp4_mode2\""},
        {DTWAIN_FF_DEJAVU,"\"dejavu_mode2\""},
        {DTWAIN_FF_EXIF,"\"exif_mode2\""},
        {DTWAIN_FF_FPX,"\"fpx_mode2\""},
        {DTWAIN_FF_JFIF,"\"jfif_mode2\""},
        {DTWAIN_JPEG,"\"jpeg_mode2\""},
        {DTWAIN_FF_JP2,"\"jp2_mode2\""},
        {DTWAIN_FF_JPX,"\"jpx_mode2\""},
        {DTWAIN_FF_PDF,"\"pdf_mode2\""},
        {DTWAIN_FF_PDFA,"\"pdfa1_mode2\""},
        {DTWAIN_FF_PDFA2,"\"pdfa2_mode2\""},
        {DTWAIN_FF_PICT,"\"pict_mode2\""},
        {DTWAIN_FF_PNG,"\"png_mode2\""},
        {DTWAIN_FF_SPIFF,"\"spiff1_mode2\""},
        {DTWAIN_FF_SPIFF,"\"spiff2_mode2\""},
        {DTWAIN_FF_TIFF,"\"tiff1_mode2\""},
        {DTWAIN_FF_TIFF,"\"tiff2_mode2\""},
        {DTWAIN_FF_TIFF,"\"tiff3_mode2\""},
        {DTWAIN_FF_TIFF,"\"tiff4_mode2\""},
        {DTWAIN_FF_TIFF,"\"tiff5_mode2\""},
        {DTWAIN_FF_TIFF,"\"tiff6_mode2\""},
        {DTWAIN_FF_TIFF,"\"tiff7_mode2\""},
        {DTWAIN_FF_TIFF,"\"tiff8_mode2\""},
        {DTWAIN_FF_TIFF,"\"tiff9_mode2\""},
        {DTWAIN_FF_XBM,"\"xbm_mode2\""}
    };

    static sourceMapType tiffMap =
    {
        {TWCP_NONE,"\"tiff1_mode2\""},
        {TWCP_GROUP31D,"\"tiff2_mode2\""},
        {TWCP_GROUP31DEOL,"\"tiff3_mode2\""},
        {TWCP_GROUP32D,"\"tiff4_mode2\""},
        {TWCP_GROUP4,"\"tiff5_mode2\""},
        {TWCP_JPEG,"\"tiff6_mode2\""},
        {TWCP_LZW,"\"tiff7_mode2\""},
        {TWCP_JBIG,"\"tiff8_mode2\""},
        {TWCP_ZIP,"\"tiff9_mode2\""}
    };

    static sourceMapType bmpMap =
    {
        {TWCP_NONE,"\"bmp1_mode2\""},
        {TWCP_RLE4,"\"bmp2_mode2\""},
        {TWCP_RLE8,"\"bmp3_mode2\""},
        {TWCP_BITFIELDS,"\"bmp4_mode2\""}
    };

    static sourceMapType spiffMap =
    {
        {TWCP_JPEG, "\"spiff1_mode2\""},
        {TWCP_JBIG, "\"spiff2_mode2\""}
    };

    std::map<LONG, const sourceMapType*> compToMap =
    { {TWFF_TIFF, &tiffMap}, {TWFF_BMP, &bmpMap}, {TWFF_SPIFF, &spiffMap} };

    struct resetAll
    {
        LONG curFormat;
        LONG curCompression;
        DTWAIN_SOURCE Source;
        resetAll(DTWAIN_SOURCE cInfo, LONG cF, LONG cC) : curFormat(cF), curCompression(cC), Source(cInfo) {}
        ~resetAll()
        {
            try
            {
                DTWAIN_ARRAY arr = DTWAIN_ArrayCreateFromCap(Source, ICAP_IMAGEFILEFORMAT, 1);
                if (arr)
                {
                    DTWAINArrayPtr_RAII raii(&arr);
                    LONG* buf = (LONG *)DTWAIN_ArrayGetBuffer(arr, 0);
                    buf[0] = curFormat;
                    DTWAIN_SetCapValues(Source, ICAP_IMAGEFILEFORMAT, DTWAIN_CAPSET, arr);
                    buf[0] = curCompression;
                    if (curCompression != -1)
                        DTWAIN_SetCapValues(Source, ICAP_COMPRESSION, DTWAIN_CAPSET, arr);
                }
            }
            catch (...)
            {
            }
        }
    };

    // get all the image file formats
    DTWAIN_ARRAY aFileFormats = nullptr;
    DTWAIN_ARRAY aCurrentFileFormat = nullptr;
    DTWAIN_GetCapValues(Source, ICAP_IMAGEFILEFORMAT, DTWAIN_CAPGET, &aFileFormats);
    LONG* vFileFormats = aFileFormats ? (LONG *)DTWAIN_ArrayGetBuffer(aFileFormats, 0) : nullptr;
    DTWAIN_GetCapValues(Source, ICAP_IMAGEFILEFORMAT, DTWAIN_CAPGETCURRENT, &aCurrentFileFormat);
    LONG* vCurrentFormat = aCurrentFileFormat ? (LONG *)DTWAIN_ArrayGetBuffer(aCurrentFileFormat, 0) : nullptr;
    DTWAIN_ARRAY aCurrentCompress = nullptr;
    DTWAIN_GetCapValues(Source, ICAP_COMPRESSION, DTWAIN_CAPGETCURRENT, &aCurrentCompress);
    LONG* vCurrentCompress = aCurrentCompress ? (LONG *)DTWAIN_ArrayGetBuffer(aCurrentCompress, 0) : nullptr;

    // get the current image file format
    if (!vCurrentFormat)
        return "";

    resetAll ra(Source, vCurrentFormat[0], !vCurrentCompress ? -1 : vCurrentCompress[0]);

    std::vector<std::string> returnFileTypes;
    LONG numFormats = DTWAIN_ArrayGetCount(aFileFormats);
    DTWAIN_ARRAY tempArray = DTWAIN_ArrayCreateFromCap(Source, ICAP_IMAGEFILEFORMAT, 1);
    DTWAINArrayPtr_RAII raii2(&tempArray);
    LONG* tempBuffer = (LONG*)DTWAIN_ArrayGetBuffer(tempArray, 0);
    for (int i = 0; i < numFormats; ++i)
    {
        LONG fformat = vFileFormats[i];
        tempBuffer[0] = fformat;
        auto compIter = compToMap.find(fformat);
        if (compIter != compToMap.end())
        {
            const std::unordered_map<LONG, std::string>* ptr = compIter->second;
            DTWAIN_SetCapValues(Source, ICAP_IMAGEFILEFORMAT, DTWAIN_CAPSET, tempBuffer);
            auto tempCompression = DTWAIN_EnumCompressionTypesEx(Source);
            DTWAINArrayPtr_RAII raii2(&tempCompression);
            LONG* compressBuf = (LONG *)DTWAIN_ArrayGetBuffer(tempCompression, 0);
            LONG nCompressions = DTWAIN_ArrayGetCount(tempCompression);
            for (LONG comp = 0; comp < nCompressions; ++comp)
            {
                auto iter = ptr->find(compressBuf[comp]);
                if (iter != ptr->end())
                    returnFileTypes.push_back(iter->second);
            }
        }
        else
        {
            auto sourceIter = source_map.find(fformat);
            if (sourceIter != source_map.end())
                returnFileTypes.push_back(sourceIter->second);
        }
    }
    return join_string(returnFileTypes.begin(), returnFileTypes.end());
}

using pixelMap = std::map<LONG, std::vector<LONG>>;

static pixelMap getPixelAndBitDepthInfo(CTL_ITwainSource* pSource)
{
    // Get the pixel information
    DTWAIN_ARRAY aPixelTypes = nullptr;
    DTWAIN_EnumPixelTypes(pSource, &aPixelTypes);
    DTWAINArrayPtr_RAII raii(&aPixelTypes);
    LONG* pixInfo = (LONG*)DTWAIN_ArrayGetBuffer(aPixelTypes, 0);
    LONG pixLen = DTWAIN_ArrayGetCount(aPixelTypes);
    pixelMap pMap;
    for (int curPixInfo = 0; curPixInfo < pixLen; ++curPixInfo)
    {
        auto iter = pMap.insert({pixInfo[curPixInfo], {}}).first;
        DTWAIN_ARRAY aBitDepthInfo = nullptr;
        DTWAINArrayPtr_RAII raii2(&aBitDepthInfo);
        DTWAIN_SetPixelType(pSource, pixInfo[curPixInfo], DTWAIN_DEFAULT, TRUE);
        DTWAIN_EnumBitDepths(pSource, &aBitDepthInfo);
        LONG* aBitDepthInfoPtr = (LONG*)DTWAIN_ArrayGetBuffer(aBitDepthInfo, 0);
        LONG aBitDepthInfoLen = DTWAIN_ArrayGetCount(aBitDepthInfo);
        for (int curBitDepth = 0; curBitDepth < aBitDepthInfoLen; ++curBitDepth)
            iter->second.push_back(aBitDepthInfoPtr[curBitDepth]);
    }
    return pMap;
}

static std::vector<std::string> getPageSizeInfo(CTL_ITwainSource* pSource)
{
    std::vector<std::string> vSizeNames;
    // get the paper sizes
    DTWAIN_ARRAY aSupportedSizes = DTWAIN_EnumPaperSizesEx(pSource);
    DTWAINArrayPtr_RAII raii(&aSupportedSizes);
    LONG* pSupportedSizes = (LONG*)DTWAIN_ArrayGetBuffer(aSupportedSizes, 0);
    LONG sizeLen = DTWAIN_ArrayGetCount(aSupportedSizes);
    for (LONG sz = 0; sz < sizeLen; ++sz)
    {
        char buf[100];
        DTWAIN_GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWSS, pSupportedSizes[sz], buf, 100);
        vSizeNames.push_back(buf);
    }
    return vSizeNames;
}

struct OneResInfo
{
    std::vector<double> m_AllRes;
    bool m_bIsRange;
    OneResInfo() : m_bIsRange{} {}
};


using ResInfoMap = std::map<LONG, OneResInfo>;

ResInfoMap getResolutionInfo(CTL_ITwainSource* pSource)
{
    ResInfoMap resMap;

    // get units of measure
    DTWAIN_ARRAY aUnits = DTWAIN_EnumSourceUnitsEx(pSource);
    DTWAINArrayPtr_RAII raii(&aUnits);
    LONG* pUnitsVals = (LONG*)DTWAIN_ArrayGetBuffer(aUnits, 0);
    LONG sizeLen = DTWAIN_ArrayGetCount(aUnits);
    DTWAIN_ARRAY aSetUnit = DTWAIN_ArrayCreateFromCap(NULL, ICAP_UNITS, 1);
    LONG *pSetUnitsVal = (LONG *)DTWAIN_ArrayGetBuffer(aSetUnit, 0);
    DTWAINArrayPtr_RAII raii2(&aSetUnit);
    DTWAIN_ARRAY curUnit = nullptr;
    DTWAIN_GetCapValues(pSource, ICAP_UNITS, DTWAIN_CAPGETCURRENT, &curUnit);
    if ( DTWAIN_ArrayGetCount(curUnit) > 0 )
    {
        DTWAINArrayPtr_RAII raiiDefault(&curUnit);
        LONG *pCurUnit = (LONG *)DTWAIN_ArrayGetBuffer(curUnit, 0);

        for (LONG i = 0; i < sizeLen; ++i)
        {
            resMap.insert({pUnitsVals[i],{}});
            // Set the current unit of measure
            pSetUnitsVal[0] = pUnitsVals[i];
            if (DTWAIN_SetCapValues(pSource, ICAP_UNITS, DTWAIN_CAPSET, aSetUnit))
            {
                // Get the resolution values for this unit of measure
                DTWAIN_ARRAY aResolutions;
                DTWAINArrayPtr_RAII raii3(&aResolutions);
                DTWAIN_GetCapValues(pSource, ICAP_XRESOLUTION, DTWAIN_CAPGET, &aResolutions);
                double *pResolutions = (double *)DTWAIN_ArrayGetBuffer(aResolutions, 0);
                LONG nResolutions = DTWAIN_ArrayGetCount(aResolutions);
                LONG nStatus = 0;
                bool isValidRange = DTWAIN_RangeIsValid(aResolutions, &nStatus);
                auto iter = resMap.find(pUnitsVals[i]);
                auto& vect = iter->second.m_AllRes;
                std::copy(pResolutions, pResolutions + nResolutions, std::back_inserter(vect));
                iter->second.m_bIsRange = isValidRange;
            }
        }

        // Set the unit back to the original
        DTWAIN_SetCapValues(pSource, ICAP_UNITS, DTWAIN_CAPSET, curUnit);
    }
    return resMap;
}

struct OneCapInfo
{
    std::string capName;
    LONG value;
    std::string capType;
    OneCapInfo(std::string name = "", LONG v = 0, std::string type = "") : capName(name), value(v), capType(type) {}
};

using AllCapInfoMap = std::map<LONG, OneCapInfo>;

struct AllCapInfo
{
   AllCapInfoMap m_infoMap;
   std::array<LONG, 3> mapCounts;
   AllCapInfo() : mapCounts{} {}
};

AllCapInfo getAllCapInfo(CTL_ITwainSource* pSource)
{
    AllCapInfo allCapInfo;
    AllCapInfoMap& capInfo = allCapInfo.m_infoMap;

    // get the capability string
    DTWAIN_ARRAY aAllCaps = DTWAIN_EnumSupportedCapsEx2(pSource);
    DTWAINArrayPtr_RAII aAllCapsraii(&aAllCaps);
    LONG* pCapBuf = (LONG*)DTWAIN_ArrayGetBuffer(aAllCaps, 0);
    LONG capLen = DTWAIN_ArrayGetCount(aAllCaps);

    DTWAIN_ARRAY aExtendedCaps = DTWAIN_EnumExtendedCapsEx2(pSource);
    DTWAINArrayPtr_RAII aExtendedraii(&aExtendedCaps);
    LONG* pExtBuf = (LONG*)DTWAIN_ArrayGetBuffer(aExtendedCaps, 0);
    LONG extLen = DTWAIN_ArrayGetCount(aExtendedCaps);

    DTWAIN_ARRAY aCustomCaps = DTWAIN_EnumCustomCapsEx2(pSource);
    DTWAINArrayPtr_RAII aCustomraii(&aCustomCaps);
    LONG* pCustomBuf = (LONG*)DTWAIN_ArrayGetBuffer(aCustomCaps, 0);
    LONG customLen = DTWAIN_ArrayGetCount(aCustomCaps);

    // Fill in the general info
    for (LONG curCap = 0; curCap < capLen; ++curCap)
    {
        char sz[100];
        DTWAIN_GetNameFromCapA(pCapBuf[curCap], sz, 100);
        std::string quouteString = "\"" + std::string(sz) + "\"";
        auto iter = capInfo.insert({ pCapBuf[curCap], {quouteString, pCapBuf[curCap], "\"standard\""}}).first;
        iter->second.capName = sz;
        iter->second.value = pCapBuf[curCap];
        iter->second.capType = "\"standard\"";
    }

    // Fill in extended cap info
    for (LONG curCap = 0; curCap < extLen; ++curCap)
    {
        auto iter = capInfo.find(pExtBuf[curCap]);
        if ( iter != capInfo.end())
            iter->second.capType = "\"standard, extended\"";
    }

    // Fill in custom cap info
    for (LONG curCap = 0; curCap < customLen; ++curCap)
    {
        auto iter = capInfo.find(pCustomBuf[curCap]);
        if (iter != capInfo.end())
            iter->second.capType = "\"custom\"";
    }
    allCapInfo.mapCounts = {capLen, extLen, customLen};
    return allCapInfo;
}


static std::string generate_details(CTL_ITwainSession& ts, const std::vector<std::string>& allSources, bool bWeOpenSource=false)
{
    using boost::algorithm::join;
    using boost::adaptors::transformed;
    using json = nlohmann::ordered_json;

    struct capabilityInfo
    {
        std::string name;
        int value;
        capabilityInfo(std::string n = "", int val = 0) : name(n), value(val) {}
    };

    struct sUniquePtrRAII
    {
        std::unique_ptr<CTL_ITwainSource*>* m_ptr;
        bool m_bReleaseOnDestruction;
        sUniquePtrRAII(std::unique_ptr<CTL_ITwainSource*>* p, bool isReleaseOnDestruction)
            : m_ptr(p), m_bReleaseOnDestruction(isReleaseOnDestruction) {}
        ~sUniquePtrRAII()
        {
            if (m_bReleaseOnDestruction)
                (*m_ptr).release();
        }
    };
    std::vector<capabilityInfo> vCapabilityInfo;

    json glob_json;
    glob_json["device-count"] = allSources.size();
    json array_twain_identity;
    json array_source_names;
    std::vector<std::string> sNames = allSources;
    glob_json["device-names"] = sNames;
    std::string jsonString;
    std::string imageInfoString[12];
    std::string deviceInfoString[9];

    struct CloserRAII
    {
        CTL_ITwainSource* p;
        bool bMustClose;
        CloserRAII(CTL_ITwainSource* pSource, bool bClose) : p(pSource), bMustClose(bClose) {}
        ~CloserRAII()
        {
            try
            {
                if (bMustClose)
                    DTWAIN_CloseSource(p);
            }
            catch (...) {}
        }
    };

    auto& sourceStatusMap = CTL_StaticData::GetSourceStatusMap();
    for (auto& curSource : allSources)
    {
        std::string jColorInfo;
        std::string resUnitInfo;
        std::string capabilityString;
        deviceInfoString[0] = "\"feeder-supported\":false";
        deviceInfoString[1] = "\"feeder-sensitive\":false";
        deviceInfoString[2] = "\"ui-controllable\":false";
        deviceInfoString[3] = "\"autobright-supported\":false";
        deviceInfoString[4] = "\"autodeskew-supported\":false";
        deviceInfoString[5] = "\"imprinter-supported\":false";
        deviceInfoString[6] = "\"duplex-supported\":false";
        deviceInfoString[7] = "\"jobcontrol-supported\":false";
        deviceInfoString[8] = "\"transparencyunit-supported\":false";
        bool devOpen[] = { false, false };

        // Check if we need to select and open the source to see
        // the details
        bool bMustClose = false;
        CTL_ITwainSource* pCurrentSourcePtr = nullptr;

        auto iter = sourceStatusMap.find(curSource);
        if (iter == sourceStatusMap.end())
            continue;
        auto sourceStatus = iter->second.IsClosed();
        if (iter->second.IsClosed() || iter->second.IsUnknown())
        {
            DTWAIN_SOURCE tempSource = DTWAIN_SelectSourceByNameA(curSource.c_str());
            if (tempSource)
            {
                pCurrentSourcePtr = (CTL_ITwainSource*)tempSource;
                bMustClose = true;
            }
        }
        else
        {
            // Source already opened
            DTWAIN_SOURCE openedSource = iter->second.GetSourceHandle();
            pCurrentSourcePtr = (CTL_ITwainSource*)openedSource;
        }

        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());

        if (!pCurrentSourcePtr && bWeOpenSource)
        {
            // try to select the source only
            bool oldSelect = pHandle->m_bOpenSourceOnSelect;
            pHandle->m_bOpenSourceOnSelect = false;
            DTWAIN_SOURCE tempSource = DTWAIN_SelectSourceByNameA(curSource.c_str());
            if (tempSource)
            {
                pCurrentSourcePtr = (CTL_ITwainSource*)tempSource;
                bMustClose = true;
            }
        }

        CloserRAII closer(pCurrentSourcePtr, bMustClose);

        if (pCurrentSourcePtr)
        {
            auto curIter = sourceStatusMap.find(pCurrentSourcePtr->GetProductNameA());
            if (curIter != sourceStatusMap.end())
            {
                if (curIter->second.IsSelected())
                {
                    devOpen[0] = true;
                    if (curIter->second.IsOpen())
                    {
                        devOpen[1] = true;
                        jsonString = pCurrentSourcePtr->GetSourceInfo();
                        jColorInfo = "\"color-info\":{";
                        std::stringstream strm;
                        std::stringstream strm2;
                        std::string allbdepths;

                        // Get the pixel information
                        {
                            auto pixInfo = getPixelAndBitDepthInfo(pCurrentSourcePtr);
                            std::vector<LONG> allPixInfo;
                            for (auto& pr : pixInfo)
                                allPixInfo.push_back(pr.first);
                            strm << "\"num-colors\":" << pixInfo.size() << ",";
                            jColorInfo += strm.str();

                            strm.str("");
                            std::string joinStr = join_string(allPixInfo.begin(), allPixInfo.end());
                            strm << "\"color-types\":[" << joinStr << "],";

                            strm2 << "\"bitdepthinfo\":{";
                            int depthCount = 0;
                            for (auto & pr : pixInfo)
                            {
                                strm2 << "\"depth_" << depthCount << "\":[";
                                std::string bdepthStr = join_string(pr.second.begin(), pr.second.end());
                                strm2 << bdepthStr << "],";
                                ++depthCount;
                            }
                            allbdepths = strm2.str();
                            allbdepths.pop_back();
                            allbdepths += "}";
                        }

                        strm2.str("");
                        // get the paper sizes
                        {
                            auto vSizeNames = getPageSizeInfo(pCurrentSourcePtr);
                            std::vector<std::string> vAdjustedNames;
                            std::transform(vSizeNames.begin(), vSizeNames.end(), std::back_inserter(vAdjustedNames),
                                [](auto& origName) { return "\"" + origName + "\""; });

                            std::string paperSizesStr = join_string(vAdjustedNames.begin(), vAdjustedNames.end());
                            strm2 << "\"paper-sizes\":[" << paperSizesStr << "],";
                            std::string allSizes = strm2.str();
                            jColorInfo += strm.str() + allbdepths + "}," + allSizes;
                        }

                        // get the resolution info
                        {
                            auto resMap = getResolutionInfo(pCurrentSourcePtr);

                            std::vector<std::string> vSizeNames;
                            strm.str("");
                            strm << "\"resolution-info\": {";
                            for (auto &pr : resMap)
                            {
                                char buf[100];
                                DTWAIN_GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWUN, pr.first, buf, 100);
                                vSizeNames.push_back(buf);
                            }
                            strm << "\"resolution-count\":" << resMap.size() << ",";
                            strm << "\"resolution-units\":";

                            std::vector<std::string> unitNameV;
                            std::transform(vSizeNames.begin(), vSizeNames.end(), std::back_inserter(unitNameV),
                                [](auto& p) { return "\"" + p + "\""; });
                            std::string unitNameStr = "[" + join_string(unitNameV.begin(), unitNameV.end(), ',') + "]";
                            strm << unitNameStr << ",";

                            int i = 0;
                            std::string resolutionTotalStr;
                            for (auto& pr : resMap)
                            {
                                strm2.str("");
                                if (i > 0)
                                    strm2 << ",";
                                strm2 << "\"resolution-" << remove_quotes(unitNameV[i]) << "\": {";

                                if (pr.second.m_bIsRange)
                                    strm2 << "\"data-type\":\"range\",";
                                else
                                    strm2 << "\"data-type\":\"discrete\",";
                                strm2 << "\"data-values\":[" << join_string(pr.second.m_AllRes.begin(), pr.second.m_AllRes.end()) << "]}";
                                resolutionTotalStr += strm2.str();
                                ++i;
                            }

                            resUnitInfo = strm.str() + resolutionTotalStr + "},";
                        }
                        int imageInfoCaps[] = { ICAP_BRIGHTNESS, ICAP_CONTRAST, ICAP_GAMMA, ICAP_HIGHLIGHT, ICAP_SHADOW,
                            ICAP_THRESHOLD, ICAP_ROTATION, ICAP_ORIENTATION, ICAP_OVERSCAN, ICAP_HALFTONES };
                        std::string imageInfoCapsStr[] = { "\"brightness-values\":", "\"contrast-values\":", "\"gamma-values\":",
                            "\"highlight-values\":", "\"shadow-values\":", "\"threshold-values\":",
                            "\"rotation-values\":", "\"orientation-values\":", "\"overscan-values\":", "\"halftone-values\":" };
                        for (int curImageCap = 0; curImageCap < sizeof(imageInfoCaps) / sizeof(imageInfoCaps[0]); ++curImageCap)
                        {
                            strm.str("");
                            strm << imageInfoCapsStr[curImageCap];
                            if (imageInfoCaps[curImageCap] == ICAP_ORIENTATION)
                                create_stream<LONG>(strm, pCurrentSourcePtr, ICAP_ORIENTATION);
                            else
                            if (imageInfoCaps[curImageCap] == ICAP_OVERSCAN)
                                create_stream<LONG>(strm, pCurrentSourcePtr, ICAP_OVERSCAN);
                            else
                            if (imageInfoCaps[curImageCap] == ICAP_HALFTONES)
                                create_stream_from_strings(strm, pCurrentSourcePtr, ICAP_HALFTONES);
                            else
                                create_stream<double>(strm, pCurrentSourcePtr, imageInfoCaps[curImageCap]);
                            imageInfoString[curImageCap] = strm.str();
                        }

                        strm2.str("");
                        // get the capability string
                        auto capInfo = getAllCapInfo(pCurrentSourcePtr);
                        for ( auto& pr : capInfo.m_infoMap )
                            strm2 << "{ \"name\":\"" << pr.second.capName << "\",\"value\":" << pr.second.value << ",\"type\":" << pr.second.capType << "},";
                        capabilityString = strm2.str();
                        capabilityString.pop_back();
                        capabilityString = "[" + capabilityString + "]";

                        std::ostringstream tempStrm;
                        tempStrm << "\"capability-count\":[{\"all\":" << capInfo.mapCounts[0] << ","
                            "\"custom\":" << capInfo.mapCounts[2] << ","
                            "\"extended\":" << capInfo.mapCounts[1] << "}],\"capability-values\":" << capabilityString;
                        imageInfoString[10] = tempStrm.str();

                        // Get the filetype info
                        tempStrm.str("");
                        std::vector<std::string> fileTypes = {
                                                                "\"bmp\",",
                                                                "\"gif\",",
                                                                "\"pcx\",",
                                                                "\"dcx\",",
                                                                "\"pdf\",",
                                                                "\"ico\",",
                                                                "\"png\",",
                                                                "\"tga\",",
                                                                "\"psd\",",
                                                                "\"emf\",",
                                                                "\"wbmp\",",
                                                                "\"wmf\",",
                                                                "\"jpeg\",",
                                                                "\"jp2\",",
                                                                "\"tif1\",",
                                                                "\"tif2\",",
                                                                "\"tif3\",",
                                                                "\"tif4\",",
                                                                "\"tif5\",",
                                                                "\"tif6\",",
                                                                "\"tif7\",",
                                                                "\"ps1\",",
                                                                "\"ps2\",",
                                                                "\"webp\"" };

                        std::string allFileTypes = std::accumulate(fileTypes.begin(), fileTypes.end(), std::string());
                        std::string customTypes = get_source_file_types(pCurrentSourcePtr);
                        if (!customTypes.empty())
                            allFileTypes += "," + customTypes;
                        tempStrm << "\"filetype-info\":[" << allFileTypes << "]";
                        imageInfoString[11] = tempStrm.str();

                        strm.str("");
                        int deviceInfoCaps[] = { CAP_FEEDERENABLED, CAP_FEEDERLOADED, CAP_UICONTROLLABLE,
                            ICAP_AUTOBRIGHT, ICAP_AUTOMATICDESKEW,
                            CAP_PRINTER, CAP_DUPLEX, CAP_JOBCONTROL, ICAP_LIGHTPATH
                        };

                        std::string deviceInfoCapsStr[sizeof(deviceInfoCaps) / sizeof(deviceInfoCaps[0])];
                        std::copy(deviceInfoString, deviceInfoString + sizeof(deviceInfoString) / sizeof(deviceInfoString[0]), deviceInfoCapsStr);
                        for (auto& s : deviceInfoCapsStr)
                            s.resize(s.size() - 5);
                        for (int curDevice = 0; curDevice < sizeof(deviceInfoCaps) / sizeof(deviceInfoCaps[0]); ++curDevice)
                        {
                            if (curDevice > 0)
                                strm << ",";
                            bool value = false;
                            if (deviceInfoCaps[curDevice] == CAP_FEEDERENABLED)
                                value = DTWAIN_IsFeederSupported(pCurrentSourcePtr) ? true : false;
                            else
                            if (deviceInfoCaps[curDevice] == CAP_UICONTROLLABLE)
                            {
                                auto vValue = DTWAIN_IsUIControllable(pCurrentSourcePtr) ? true : false;
                                value = vValue;
                            }
                            else
                            if (deviceInfoCaps[curDevice] == CAP_PRINTER)
                            {
                                DTWAIN_ARRAY aPrinter = nullptr;
                                DTWAIN_GetCapValues(pCurrentSourcePtr, CAP_PRINTER, DTWAIN_CAPGET, &aPrinter);
                                DTWAINArrayPtr_RAII aPrinterRaii(&aPrinter);
                                LONG* pBuf = (LONG*)DTWAIN_ArrayGetBuffer(aPrinter, 0);
                                value = (aPrinter && DTWAIN_ArrayGetCount(aPrinter) > 0 && pBuf[0] != TWDX_NONE);
                            }
                            else
                            if (deviceInfoCaps[curDevice] == CAP_JOBCONTROL)
                            {
                                auto vValue = DTWAIN_EnumJobControlsEx(pCurrentSourcePtr);
                                DTWAINArrayPtr_RAII aJob(&vValue);
                                LONG* pBuf = (LONG*)DTWAIN_ArrayGetBuffer(vValue, 0);
                                value = (vValue && DTWAIN_ArrayGetCount(vValue) > 0 && pBuf[0] != TWJC_NONE);
                            }
                            else
                                value = DTWAIN_IsCapSupported(pCurrentSourcePtr, deviceInfoCaps[curDevice]) ? true : false;
                            strm << deviceInfoCapsStr[curDevice] << (value ? "true" : "false");
                            deviceInfoString[curDevice] = strm.str();
                        }
                    }
                    else
                    {
                        jsonString = pCurrentSourcePtr->GetSourceInfo();
                        jColorInfo = "\"color-info\":\"<not available>\",";
                        resUnitInfo = "\"resolution-info\":\"<not available>\",";
                        imageInfoString[0] = "\"brightness-values\":\"<not available>\"";
                        imageInfoString[1] = "\"contrast-values\":\"<not available>\"";
                        imageInfoString[2] = "\"gamma-values\":\"<not available>\"";
                        imageInfoString[3] = "\"highlight-values\":\"<not available>\"";
                        imageInfoString[4] = "\"shadow-values\":\"<not available>\"";
                        imageInfoString[5] = "\"threshold-values\":\"<not available>\"";
                        imageInfoString[6] = "\"rotation-values\":\"<not available>\"";
                        imageInfoString[7] = "\"orientation-values\":\"<not available>\"";
                        imageInfoString[8] = "\"overscan-values\":\"<not available>\"";
                        imageInfoString[9] = "\"halftone-values\":\"<not available>\"";
                        imageInfoString[10] = "\"capability-info\":\"<not available>\"";
                        imageInfoString[11] = "\"filetype-info\":\"<not available>\"";

                        deviceInfoString[0] = "\"feeder-supported\":false";
                        deviceInfoString[1] = "\"feeder-sensitive\":false";
                        deviceInfoString[2] = "\"ui-controllable\":false";
                        deviceInfoString[3] = "\"autobright-supported\":false";
                        deviceInfoString[4] = "\"autodeskew-supported\":false";
                        deviceInfoString[5] = "\"imprinter-supported\":false";
                        deviceInfoString[6] = "\"duplex-supported\":false";
                        deviceInfoString[7] = "\"jobcontrol-supported\":false";
                        deviceInfoString[8] = "\"transparencyunit-supported\":false";
                    }
                }
                std::string partString = "\"device-name\":\"" + curSource + "\",";
                std::string strStatus;
                if (devOpen[0] && devOpen[1])
                    strStatus = "\"<selected, opened>\"";
                else
                if (!devOpen[0] && !devOpen[1])
                    strStatus = "\"<error>\"";
                else
                if (devOpen[0] && !devOpen[1])
                    strStatus = "\"<selected, unopened>\"";

                partString += "\"device-status\":" + strStatus + ",";
                std::string imageInfoStringVal = join_string(imageInfoString, imageInfoString +
                    sizeof(imageInfoString) / sizeof(imageInfoString[0])) + ",";
                std::string deviceInfoStringVal = join_string(deviceInfoString, deviceInfoString +
                    sizeof(deviceInfoString) / sizeof(deviceInfoString[0])) + ",";
                if (jsonString.empty())
                    jsonString = " }";
                jsonString = "{" + partString + jColorInfo + resUnitInfo + imageInfoStringVal + deviceInfoStringVal + jsonString.substr(1);
                array_twain_identity.push_back(json::parse(jsonString));
            }
        }
    }
    glob_json["device-info"] = array_twain_identity;
    return glob_json.dump(4);
}

LONG DLLENTRY_DEF DTWAIN_GetSessionDetails(LPTSTR szBuf, LONG nSize)
{
    LOG_FUNC_ENTRY_PARAMS((szBuf, nSize))
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());

    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    DTWAIN_ARRAY aAllSources = DTWAIN_EnumSourcesEx();
    DTWAINArrayPtr_RAII raii(&aAllSources);
    std::vector<std::string> vAllSources;
    LONG nSources = DTWAIN_ArrayGetCount(aAllSources);
    for (LONG i = 0; i < nSources; ++i)
    {
        DTWAIN_SOURCE theSource;
        DTWAIN_ArrayGetSourceAt(aAllSources, i, &theSource);
        char szName[100];
        DTWAIN_GetSourceProductNameA(theSource, szName, 100);
        vAllSources.push_back(szName);
    }
    CTL_StringType details = StringConversion::Convert_Ansi_To_Native(generate_details(*pHandle->m_pTwainSession, vAllSources));
    LONG retVal = StringWrapper::CopyInfoToCString(details, szBuf, nSize);
    LOG_FUNC_EXIT_PARAMS(retVal)
    CATCH_BLOCK(0)
}

#if 0

LONG DLLENTRY_DEF DTWAIN_GetSourceDetails(DTWAIN_SOURCE Source, LPTSTR szOut, LONG nLen)
{



    template <typename T, typename S>
    static void create_stream(std::stringstream& strm, DTWAIN_SOURCE Source, LONG capValue, bool createStringNames)
    {
        std::vector<T> imageVals;
        imageVals = capInfo.get_cap_values<std::vector<T>>(capValue);
        if (imageVals.empty())
        {
            strm << "\"<not available>\"";
        }
        else
        {
            strm << "{";
            // check if range
            if (is_valid_range(imageVals))
                strm << "\"data-type\":\"range\",";
            else
                strm << "\"data-type\":\"discrete\",";
            auto vInfo = S::to_twain_string(imageVals.begin(), imageVals.end());
            std::vector<std::string> vSizeNames;
            std::transform(vInfo.begin(), vInfo.end(), std::back_inserter(vSizeNames),
                [](const std::pair<const char*, const char*>& p) { return "\"" + std::string(p.second) + "\""; });
            std::string paperSizesStr = join_string(vSizeNames.begin(), vSizeNames.end());
            strm << "\"data-values\":[" << join_string(vSizeNames.begin(), vSizeNames.end()) << "]}";
        }
    }
#endif
#if 0
    static std::string get_source_file_types(const capability_interface& capInfo)
    {
        using sourceMapType = std::unordered_map<dynarithmic::twain::compression_value::value_type, std::string>;
        static sourceMapType source_map = {
                        {dynarithmic::twain::filetype_value::bmp_source_mode,"\"bmp1_mode2\""},
                        {dynarithmic::twain::filetype_value::bmp_source_mode,"\"bmp2_mode2\""},
                        {dynarithmic::twain::filetype_value::bmp_source_mode,"\"bmp3_mode2\""},
                        {dynarithmic::twain::filetype_value::bmp_source_mode,"\"bmp4_mode2\""},
                        {dynarithmic::twain::filetype_value::dejavu_source_mode,"\"dejavu_mode2\""},
                        {dynarithmic::twain::filetype_value::exif_source_mode,"\"exif_mode2\""},
                        {dynarithmic::twain::filetype_value::fpx_source_mode,"\"fpx_mode2\""},
                        {dynarithmic::twain::filetype_value::jfif_source_mode,"\"jfif_mode2\""},
                        {dynarithmic::twain::filetype_value::jpeg,"\"jpeg_mode2\""},
                        {dynarithmic::twain::filetype_value::jp2_source_mode,"\"jp2_mode2\""},
                        {dynarithmic::twain::filetype_value::jpx_source_mode,"\"jpx_mode2\""},
                        {dynarithmic::twain::filetype_value::pdf_source_mode,"\"pdf_mode2\""},
                        {dynarithmic::twain::filetype_value::pdfa_source_mode,"\"pdfa1_mode2\""},
                        {dynarithmic::twain::filetype_value::pdfa2_source_mode,"\"pdfa2_mode2\""},
                        {dynarithmic::twain::filetype_value::pict_source_mode,"\"pict_mode2\""},
                        {dynarithmic::twain::filetype_value::png_source_mode,"\"png_mode2\""},
                        {dynarithmic::twain::filetype_value::spiff_source_mode,"\"spiff1_mode2\""},
                        {dynarithmic::twain::filetype_value::spiff_source_mode,"\"spiff2_mode2\""},
                        {dynarithmic::twain::filetype_value::tiff_source_mode,"\"tiff1_mode2\""},
                        {dynarithmic::twain::filetype_value::tiff_source_mode,"\"tiff2_mode2\""},
                        {dynarithmic::twain::filetype_value::tiff_source_mode,"\"tiff3_mode2\""},
                        {dynarithmic::twain::filetype_value::tiff_source_mode,"\"tiff4_mode2\""},
                        {dynarithmic::twain::filetype_value::tiff_source_mode,"\"tiff5_mode2\""},
                        {dynarithmic::twain::filetype_value::tiff_source_mode,"\"tiff6_mode2\""},
                        {dynarithmic::twain::filetype_value::tiff_source_mode,"\"tiff7_mode2\""},
                        {dynarithmic::twain::filetype_value::tiff_source_mode,"\"tiff8_mode2\""},
                        {dynarithmic::twain::filetype_value::tiff_source_mode,"\"tiff9_mode2\""},
                        {dynarithmic::twain::filetype_value::xbm_source_mode,"\"xbm_mode2\""} };

        static sourceMapType tiffMap = {
                        {dynarithmic::twain::compression_value::none,"\"tiff1_mode2\""},
                        {dynarithmic::twain::compression_value::group31D,"\"tiff2_mode2\""},
                        {dynarithmic::twain::compression_value::group31DEOL,"\"tiff3_mode2\""},
                        {dynarithmic::twain::compression_value::group32D,"\"tiff4_mode2\""},
                        {dynarithmic::twain::compression_value::group4,"\"tiff5_mode2\""},
                        {dynarithmic::twain::compression_value::jpeg,"\"tiff6_mode2\""},
                        {dynarithmic::twain::compression_value::lzw,"\"tiff7_mode2\""},
                        {dynarithmic::twain::compression_value::jbig,"\"tiff8_mode2\""},
                        {dynarithmic::twain::compression_value::zip,"\"tiff9_mode2\""} };

        static sourceMapType bmpMap = {
                        {dynarithmic::twain::compression_value::none,"\"bmp1_mode2\""},
                        {dynarithmic::twain::compression_value::rle4,"\"bmp2_mode2\""},
                        {dynarithmic::twain::compression_value::rle8,"\"bmp3_mode2\""},
                        {dynarithmic::twain::compression_value::bitfields ,"\"bmp4_mode2\""} };

        static sourceMapType spiffMap = {
                        {dynarithmic::twain::compression_value::jpeg, "\"spiff1_mode2\""},
                        {dynarithmic::twain::compression_value::jbig, "\"spiff2_mode2\""} };

        std::map<dynarithmic::twain::compression_value::value_type, const sourceMapType*> compToMap =
        { {TWFF_TIFF, &tiffMap}, {TWFF_BMP, &bmpMap}, {TWFF_SPIFF, &spiffMap} };

        struct resetAll
        {
            dynarithmic::twain::filetype_value::value_type curFormat;
            dynarithmic::twain::compression_value::value_type curCompression;
            const capability_interface& capInfo;
            resetAll(const capability_interface& cInfo,
                dynarithmic::twain::filetype_value::value_type cF,
                dynarithmic::twain::compression_value::value_type cC) : curFormat(cF), curCompression(cC), capInfo(cInfo) {}
            ~resetAll()
            {
                capInfo.set_imagefileformat({ curFormat });
                if (curCompression != -1)
                    capInfo.set_compression({ curCompression });
            }
        };

        // get all the image file formats
        auto vFileFormats = capInfo.get_imagefileformat();

        // get the current image file format
        auto vCurrentFormat = capInfo.get_imagefileformat(capability_interface::get_current());
        auto vCurrentCompress = capInfo.get_compression(capability_interface::get_current());
        if (vCurrentFormat.empty())
            return "";

        resetAll ra(capInfo, vCurrentFormat.front(), vCurrentCompress.empty() ? -1 : vCurrentCompress.front());

        std::vector<std::string> returnFileTypes;
        for (auto fformat : vFileFormats)
        {
            auto compIter = compToMap.find(fformat);
            if (compIter != compToMap.end())
            {
                const std::unordered_map<dynarithmic::twain::filetype_value::value_type, std::string>* ptr = compIter->second;
                capInfo.set_imagefileformat({ fformat });
                auto vCompressions = capInfo.get_compression();
                for (auto compression : vCompressions)
                {
                    auto iter = ptr->find(compression);
                    if (iter != ptr->end())
                        returnFileTypes.push_back(iter->second);
                }
            }
            else
            {
                auto sourceIter = source_map.find(fformat);
                if (sourceIter != source_map.end())
                    returnFileTypes.push_back(sourceIter->second);
            }
        }
        return join_string(returnFileTypes.begin(), returnFileTypes.end());
    }

    std::string json_generator::generate_details(twain_session& ts, const std::vector<std::string>& allSources, bool bWeOpenSource/*=false*/)
    {
        using boost::algorithm::join;
        using boost::adaptors::transformed;
        using json = nlohmann::ordered_json;

        struct capabilityInfo
        {
            std::string name;
            int value;
            capabilityInfo(std::string n = "", int val = 0) : name(n), value(val) {}
        };

        struct sUniquePtrRAII
        {
            std::unique_ptr<twain_source>* m_ptr;
            bool m_bReleaseOnDestruction;
            sUniquePtrRAII(std::unique_ptr<twain_source>* p, bool isReleaseOnDestruction)
                : m_ptr(p), m_bReleaseOnDestruction(isReleaseOnDestruction) {}
            ~sUniquePtrRAII()
            {
                if (m_bReleaseOnDestruction)
                    (*m_ptr).release();
            }
        };
        std::vector<capabilityInfo> vCapabilityInfo;

        json glob_json;
        glob_json["device-count"] = allSources.size();
        json array_twain_identity;
        json array_source_names;
        std::vector<std::string> sNames = allSources;
        glob_json["device-names"] = sNames;
        std::string jsonString;
        std::string imageInfoString[12];
        std::string deviceInfoString[9];

        for (auto& curSource : allSources)
        {
            std::string jColorInfo;
            std::string resUnitInfo;
            std::string capabilityString;
            deviceInfoString[0] = "\"feeder-supported\":false";
            deviceInfoString[1] = "\"feeder-sensitive\":false";
            deviceInfoString[2] = "\"ui-controllable\":false";
            deviceInfoString[3] = "\"autobright-supported\":false";
            deviceInfoString[4] = "\"autodeskew-supported\":false";
            deviceInfoString[5] = "\"imprinter-supported\":false";
            deviceInfoString[6] = "\"duplex-supported\":false";
            deviceInfoString[7] = "\"jobcontrol-supported\":false";
            deviceInfoString[8] = "\"transparencyunit-supported\":false";
            bool devOpen[] = { false, false };

            // Check if we need to select and open the source to see
            // the details
            std::unique_ptr<twain_source> pCurrentSourcePtr;

            auto sourceStatus = ts.get_source_status(curSource);
            if (sourceStatus == twain_session::source_status::closed ||
                sourceStatus == twain_session::source_status::unknown)
            {
                auto select_info = ts.select_source(select_byname(curSource), bWeOpenSource);
                if (select_info.source_handle)
                {
                    pCurrentSourcePtr = std::make_unique<twain_source>(select_info);
                }
            }
            else
            {
                // Source already opened
                DTWAIN_SOURCE openedSource = ts.get_source_handle_from_name(curSource);
                pCurrentSourcePtr = std::make_unique<twain_source>();
                pCurrentSourcePtr->attach(ts, openedSource);
                pCurrentSourcePtr->make_weak();
            }

            if (!pCurrentSourcePtr && bWeOpenSource)
            {
                // try to select the source only
                auto select_info = ts.select_source(select_byname(curSource), false);
                if (select_info.source_handle)
                {
                    pCurrentSourcePtr = std::make_unique<twain_source>(select_info);
                }
            }
            if (pCurrentSourcePtr && pCurrentSourcePtr->is_selected())
            {
                devOpen[0] = true;
                if (pCurrentSourcePtr->is_open())
                {
                    devOpen[1] = true;
                    jsonString = pCurrentSourcePtr->get_source_info().to_json();
                    jColorInfo = "\"color-info\":{";
                    std::stringstream strm;

                    // Get the pixel information
                    auto& capInfo = pCurrentSourcePtr->get_capability_interface();
                    auto pixInfo = capInfo.get_pixeltype();
                    strm << "\"num-colors\":" << pixInfo.size() << ",";
                    jColorInfo += strm.str();

                    strm.str("");
                    std::string joinStr = join_string(pixInfo.begin(), pixInfo.end());
                    strm << "\"color-types\":[" << joinStr << "],";

                    std::stringstream strm2;
                    strm2 << "\"bitdepthinfo\":{";

                    for (auto p : pixInfo)
                    {
                        strm2 << "\"depth_" << p << "\":[";
                        capInfo.set_pixeltype({ p });
                        auto bdepth = capInfo.get_bitdepth();
                        std::string bdepthStr = join_string(bdepth.begin(), bdepth.end());
                        strm2 << bdepthStr << "],";
                    }
                    std::string allbdepths = strm2.str();
                    allbdepths.pop_back();
                    allbdepths += "}";

                    strm2.str("");

                    // get the paper sizes
                    auto paperSizes = capInfo.get_supportedsizes();
                    auto vSizeInfo = supportedsizes_value::to_twain_string(paperSizes.begin(), paperSizes.end());
                    std::vector<std::string> vSizeNames;
                    std::transform(vSizeInfo.begin(), vSizeInfo.end(), std::back_inserter(vSizeNames),
                        [](const std::pair<const char*, const char*>& p) { return "\"" + std::string(p.second) + "\""; });

                    std::string paperSizesStr = join_string(vSizeNames.begin(), vSizeNames.end());
                    strm2 << "\"paper-sizes\":[" << paperSizesStr << "],";
                    std::string allSizes = strm2.str();
                    jColorInfo += strm.str() + allbdepths + "}," + allSizes;

                    // get the resolution info
                    strm.str("");
                    strm << "\"resolution-info\": {";

                    auto allUnits = capInfo.get_units();
                    strm << "\"resolution-count\":" << allUnits.size() << ",";
                    strm << "\"resolution-units\":";
                    auto unitNames = units_value::to_twain_string(allUnits.begin(), allUnits.end());
                    std::vector<std::string> unitNameV;
                    std::transform(unitNames.begin(), unitNames.end(), std::back_inserter(unitNameV),
                        [](const std::pair<const char*, const char*>& p) { return "\"" + std::string(p.second) + "\""; });
                    std::string unitNameStr = "[" + join_string(unitNameV.begin(), unitNameV.end(), ',') + "]";
                    strm << unitNameStr << ",";
                    int i = 0;
                    std::string resolutionTotalStr;
                    for (auto u : allUnits)
                    {
                        strm2.str("");
                        if (i > 0)
                            strm2 << ",";
                        strm2 << "\"resolution-" << remove_quotes(unitNameV[i]) << "\": {";

                        // set the unit of measure
                        capInfo.set_units({ u });

                        // get all the values
                        auto allUnitValues = capInfo.get_xresolution();

                        // check if range
                        if (is_valid_range(allUnitValues))
                            strm2 << "\"data-type\":\"range\",";
                        else
                            strm2 << "\"data-type\":\"discrete\",";
                        strm2 << "\"data-values\":[" << join_string(allUnitValues.begin(), allUnitValues.end()) << "]}";
                        resolutionTotalStr += strm2.str();
                        ++i;
                    }

                    resUnitInfo = strm.str() + resolutionTotalStr + "},";

                    int imageInfoCaps[] = { ICAP_BRIGHTNESS, ICAP_CONTRAST, ICAP_GAMMA, ICAP_HIGHLIGHT, ICAP_SHADOW,
                        ICAP_THRESHOLD, ICAP_ROTATION, ICAP_ORIENTATION, ICAP_OVERSCAN, ICAP_HALFTONES };
                    std::string imageInfoCapsStr[] = { "\"brightness-values\":", "\"contrast-values\":", "\"gamma-values\":",
                        "\"highlight-values\":", "\"shadow-values\":", "\"threshold-values\":",
                        "\"rotation-values\":", "\"orientation-values\":", "\"overscan-values\":", "\"halftone-values\":" };
                    for (int i = 0; i < sizeof(imageInfoCaps) / sizeof(imageInfoCaps[0]); ++i)
                    {
                        strm.str("");
                        strm << imageInfoCapsStr[i];
                        if (imageInfoCaps[i] == ICAP_ORIENTATION)
                            create_stream<ICAP_ORIENTATION_::value_type>(strm, capInfo, ICAP_ORIENTATION);
                        else
                            if (imageInfoCaps[i] == ICAP_OVERSCAN)
                                create_stream<ICAP_OVERSCAN_::value_type, overscan_value>(strm, capInfo, ICAP_OVERSCAN, true);
                            else
                                if (imageInfoCaps[i] == ICAP_HALFTONES)
                                    create_stream_from_strings(strm, capInfo, ICAP_HALFTONES);
                                else
                                    create_stream<double>(strm, capInfo, imageInfoCaps[i]);
                        imageInfoString[i] = strm.str();
                    }

                    // get the capability string
                    auto allCaps = capInfo.get_caps();
                    for (auto& cap : allCaps)
                    {
                        vCapabilityInfo.push_back({ capability_interface::get_cap_name_s(cap), cap });
                    }

                    if (!vCapabilityInfo.empty())
                    {
                        strm2.str("");
                        for (auto& v : vCapabilityInfo)
                        {
                            std::string capType = "\"standard\"";
                            if (capInfo.is_custom_cap(v.value))
                                capType = "\"custom\"";
                            else
                                if (capInfo.is_extended_cap(v.value))
                                    capType = "\"standard, extended\"";
                            strm2 << "{ \"name\":\"" << v.name << "\",\"value\":" << v.value << ",\"type\":" << capType << "},";
                        }
                        capabilityString = strm2.str();
                        capabilityString.pop_back();
                        capabilityString = "[" + capabilityString + "]";
                    }
                    std::ostringstream tempStrm;
                    tempStrm << "\"capability-count\":[{\"all\":" << vCapabilityInfo.size() << ","
                        "\"custom\":" << capInfo.get_custom_caps().size() << ","
                        "\"extended\":" << capInfo.get_extended_caps().size() << "}],\"capability-values\":" << capabilityString;
                    imageInfoString[10] = tempStrm.str();

                    // Get the filetype info
                    tempStrm.str("");
                    std::vector<std::string> fileTypes = {
                                                            "\"bmp\",",
                                                            "\"gif\",",
                                                            "\"pcx\",",
                                                            "\"dcx\",",
                                                            "\"pdf\",",
                                                            "\"ico\",",
                                                            "\"png\",",
                                                            "\"tga\",",
                                                            "\"psd\",",
                                                            "\"emf\",",
                                                            "\"wbmp\",",
                                                            "\"wmf\",",
                                                            "\"jpeg\",",
                                                            "\"jp2\",",
                                                            "\"tif1\",",
                                                            "\"tif2\",",
                                                            "\"tif3\",",
                                                            "\"tif4\",",
                                                            "\"tif5\",",
                                                            "\"tif6\",",
                                                            "\"tif7\",",
                                                            "\"ps1\",",
                                                            "\"ps2\",",
                                                            "\"webp\"" };

                    std::string allFileTypes = std::accumulate(fileTypes.begin(), fileTypes.end(), std::string());
                    std::string customTypes = get_source_file_types(capInfo);
                    if (!customTypes.empty())
                        allFileTypes += "," + customTypes;
                    tempStrm << "\"filetype-info\":[" << allFileTypes << "]";
                    imageInfoString[11] = tempStrm.str();

                    strm.str("");
                    int deviceInfoCaps[] = { CAP_FEEDERENABLED, CAP_FEEDERLOADED, CAP_UICONTROLLABLE,
                        ICAP_AUTOBRIGHT, ICAP_AUTOMATICDESKEW,
                        CAP_PRINTER, CAP_DUPLEX, CAP_JOBCONTROL, ICAP_LIGHTPATH
                    };

                    std::string deviceInfoCapsStr[sizeof(deviceInfoCaps) / sizeof(deviceInfoCaps[0])];
                    std::copy(deviceInfoString, deviceInfoString + sizeof(deviceInfoString) / sizeof(deviceInfoString[0]), deviceInfoCapsStr);
                    for (auto& s : deviceInfoCapsStr)
                        s.resize(s.size() - 5);
                    paperhandling_info pinfo(*pCurrentSourcePtr);
                    for (int i = 0; i < sizeof(deviceInfoCaps) / sizeof(deviceInfoCaps[0]); ++i)
                    {
                        if (i > 0)
                            strm << ",";
                        bool value = false;
                        if (deviceInfoCaps[i] == CAP_FEEDERENABLED)
                            value = pinfo.is_feedersupported();
                        else
                            if (deviceInfoCaps[i] == CAP_UICONTROLLABLE)
                            {
                                auto vValue = capInfo.get_uicontrollable();
                                if (!vValue.empty())
                                    value = vValue.front();
                            }
                            else
                                if (deviceInfoCaps[i] == CAP_PRINTER)
                                {
                                    auto vValue = capInfo.get_printer();
                                    value = (!vValue.empty() && vValue.front() != TWDX_NONE);
                                }
                                else
                                    if (deviceInfoCaps[i] == CAP_JOBCONTROL)
                                    {
                                        auto vValue = capInfo.get_jobcontrol();
                                        value = (!vValue.empty() && vValue.front() != TWJC_NONE);
                                    }
                                    else
                                        value = capInfo.is_cap_supported(deviceInfoCaps[i]);
                        strm << deviceInfoCapsStr[i] << (value ? "true" : "false");
                        deviceInfoString[i] = strm.str();
                    }
                }
                else
                {
                    jsonString = pCurrentSourcePtr->get_source_info().to_json();
                    jColorInfo = "\"color-info\":\"<not available>\",";
                    resUnitInfo = "\"resolution-info\":\"<not available>\",";
                    imageInfoString[0] = "\"brightness-values\":\"<not available>\"";
                    imageInfoString[1] = "\"contrast-values\":\"<not available>\"";
                    imageInfoString[2] = "\"gamma-values\":\"<not available>\"";
                    imageInfoString[3] = "\"highlight-values\":\"<not available>\"";
                    imageInfoString[4] = "\"shadow-values\":\"<not available>\"";
                    imageInfoString[5] = "\"threshold-values\":\"<not available>\"";
                    imageInfoString[6] = "\"rotation-values\":\"<not available>\"";
                    imageInfoString[7] = "\"orientation-values\":\"<not available>\"";
                    imageInfoString[8] = "\"overscan-values\":\"<not available>\"";
                    imageInfoString[9] = "\"halftone-values\":\"<not available>\"";
                    imageInfoString[10] = "\"capability-info\":\"<not available>\"";
                    imageInfoString[11] = "\"filetype-info\":\"<not available>\"";

                    deviceInfoString[0] = "\"feeder-supported\":false";
                    deviceInfoString[1] = "\"feeder-sensitive\":false";
                    deviceInfoString[2] = "\"ui-controllable\":false";
                    deviceInfoString[3] = "\"autobright-supported\":false";
                    deviceInfoString[4] = "\"autodeskew-supported\":false";
                    deviceInfoString[5] = "\"imprinter-supported\":false";
                    deviceInfoString[6] = "\"duplex-supported\":false";
                    deviceInfoString[7] = "\"jobcontrol-supported\":false";
                    deviceInfoString[8] = "\"transparencyunit-supported\":false";
                }
            }
            std::string partString = "\"device-name\":\"" + curSource + "\",";
            std::string strStatus;
            if (devOpen[0] && devOpen[1])
                strStatus = "\"<selected, opened>\"";
            else
                if (!devOpen[0] && !devOpen[1])
                    strStatus = "\"<error>\"";
                else
                    if (devOpen[0] && !devOpen[1])
                        strStatus = "\"<selected, unopened>\"";

            partString += "\"device-status\":" + strStatus + ",";
            std::string imageInfoStringVal = join_string(imageInfoString, imageInfoString +
                sizeof(imageInfoString) / sizeof(imageInfoString[0])) + ",";
            std::string deviceInfoStringVal = join_string(deviceInfoString, deviceInfoString +
                sizeof(deviceInfoString) / sizeof(deviceInfoString[0])) + ",";
            if (jsonString.empty())
                jsonString = " }";
            jsonString = "{" + partString + jColorInfo + resUnitInfo + imageInfoStringVal + deviceInfoStringVal + jsonString.substr(1);
            array_twain_identity.push_back(json::parse(jsonString));
        }

        glob_json["device-info"] = array_twain_identity;
        return glob_json.dump(4);
    }
}
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsUIControllable(DTWAIN_SOURCE Source)
{
LOG_FUNC_ENTRY_PARAMS((Source))
const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

// See if DLL Handle exists
DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
CTL_ITwainSource *pSource = VerifySourceHandle(pHandle, Source);
if (!pSource)
    LOG_FUNC_EXIT_PARAMS(false)

// Open the source (if source is closed)
bool bSourceOpen = false;
if (CTL_TwainAppMgr::IsSourceOpen(pSource))
    bSourceOpen = true;
else
if (!CTL_TwainAppMgr::OpenSource(pHandle->m_pTwainSession, pSource))
    LOG_FUNC_EXIT_PARAMS(false)
bool bOk = false;

// Check if capability UICONTROLLABLE is supported
if (DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPUICONTROLLABLE))
{
    // Get the capability value
    DTWAIN_ARRAY CapArray = nullptr;
    BOOL bGetUI = DTWAIN_GetCapValuesEx(Source, DTWAIN_CV_CAPUICONTROLLABLE, DTWAIN_CAPGET, DTWAIN_CONTONEVALUE, &CapArray);
    if (bGetUI && CapArray && !pHandle->m_ArrayFactory->empty(CapArray))
    {
        DTWAINArrayLL_RAII arr(CapArray);
        bOk = pHandle->m_ArrayFactory->underlying_container_t<LONG>(CapArray)[0] ? true : false;
    }
}
else
{
    // Source UI must be tested
    bOk = CTL_TwainAppMgr::ShowUserInterface(pSource, true);
}
// Close source if opened in this function
if (!bSourceOpen)
    DTWAIN_CloseSource(Source);
LOG_FUNC_EXIT_PARAMS(bOk ? TRUE : FALSE)
CATCH_BLOCK(false)
}
#endif
