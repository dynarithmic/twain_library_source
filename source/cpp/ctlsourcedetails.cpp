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

#include "../nlohmann/json.hpp"
#include <boost/range/adaptor/transformed.hpp>
#include "ctltwainmanager.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#include "ctlsetgetcaps.h"

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

template <typename Iter>
static std::string join_string(Iter it1, Iter it2, const char* val = ",")
{
    return StringWrapperA::Join(it1, it2, val);
}

static std::string remove_quotes(std::string s)
{
    s.erase(std::remove(s.begin(), s.end(), '\"'), s.end());
    return s;
}

template <typename T>
static void create_stream(std::ostringstream& strm, DTWAIN_SOURCE Source, LONG capValue, LONG twainConstantID, bool useTwainName = false)
{
    DTWAIN_ARRAY arr = nullptr;
    DTWAIN_GetCapValuesEx2(Source, capValue, DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &arr);
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE | DTWAIN_TEST_NOTHROW);
    if (pHandle && arr)
    {
        DTWAINArrayPtr_RAII raii(pHandle, &arr);
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<T>(arr);
        auto nCount = vValues.size();
        if (nCount == 0)
            strm << "\"<not available>\"";
        else
        {
            strm << "{";

            // check if range
            LONG status;
            bool isRange = DTWAIN_GetCapContainer(Source, capValue, DTWAIN_CAPGET) == DTWAIN_CONTRANGE;
            if (isRange && DTWAIN_RangeIsValid(arr, &status))
                strm << "\"data-type\":\"range\",";
            else
                strm << "\"data-type\":\"discrete\",";
            std::string joined_names;
            if constexpr (std::is_integral_v<T>)
            {
                char szConstantName[100] = {};
                if (!isRange && useTwainName)
                {
                    std::vector<std::string> vTwainNames;
                    for (auto& val : vValues)
                    {
                        DTWAIN_GetTwainNameFromConstantA(twainConstantID, val, szConstantName, 99);
                        std::vector<std::string> saParsedNames;
                        StringWrapperA::Tokenize(szConstantName, ", ", saParsedNames);
                        for (auto& sName : saParsedNames)
                        {
                            std::string sTotalName = "\"" + std::string(sName) + "\"";
                            vTwainNames.push_back(sTotalName);
                        }
                    }
                    joined_names = join_string(vTwainNames.begin(), vTwainNames.end());
                }
                else
                    joined_names = join_string(vValues.begin(), vValues.end());
            }
            else
                joined_names = join_string(vValues.begin(), vValues.end());
            strm << "\"data-values\":[" << joined_names << "]}";
        }
    }
    else
        strm << "\"<not available>\"";
}

struct DefaultStringFnGetter
{
    static DTWAIN_ARRAY GetAllStringValues(DTWAIN_SOURCE Source, LONG capValue)
    {
        DTWAIN_ARRAY arr = nullptr;
        DTWAIN_GetCapValuesEx2(Source, capValue, DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &arr);
        return arr;
    }
};

struct CameraSystemStringFnGetter
{
    static DTWAIN_ARRAY GetAllStringValues(DTWAIN_SOURCE Source, LONG)
    {
        DTWAIN_ARRAY arr = nullptr;
        if (DTWAIN_IsFileSystemSupported(Source))
        {
            DTWAIN_EnumCameras(Source, &arr);
        }
        return arr;
    }
};

template <typename Fn>
static void create_stream_from_strings(std::ostringstream& strm, DTWAIN_SOURCE Source, LONG capValue)
{
    const auto pHandle = static_cast<CTL_ITwainSource*>(Source)->GetDTWAINHandle();
    std::vector<std::string> imageVals;
    DTWAIN_ARRAY arr = Fn::GetAllStringValues(Source, capValue);
    DTWAINArrayPtr_RAII raii(pHandle, &arr);
    if (arr)
    {
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<std::string>(arr);
        auto nCount = vValues.size();
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
    // get all the image file formats
    const auto pHandle = static_cast<CTL_ITwainSource*>(Source)->GetDTWAINHandle();
    DTWAIN_ARRAY aFileFormats = nullptr;
    auto bOk = DTWAIN_GetCapValuesEx2(Source, ICAP_IMAGEFILEFORMAT, DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &aFileFormats);
    if (!bOk || !aFileFormats)
        return {};

        DTWAINArrayPtr_RAII raii1(pHandle, &aFileFormats);
        auto& vFileFormats = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aFileFormats);
    if (vFileFormats.empty())
    return {};

    std::vector<std::string> vRetVal;
    char szFileFormat[100];
    for (auto curFormat : vFileFormats)
    {
        DTWAIN_GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWFF, curFormat, szFileFormat, 99);
        vRetVal.push_back(StringWrapperA::QuoteString(szFileFormat)); 
    }
    return join_string(vRetVal.begin(), vRetVal.end());
}

using pixelMap = std::map<LONG, std::vector<LONG>>;

static pixelMap get_pixel_bitdepth_info(CTL_ITwainSource* pSource)
{
    const auto pHandle = pSource->GetDTWAINHandle();
    // Get the pixel information
    DTWAIN_ARRAY aPixelTypes = nullptr;
    DTWAIN_EnumPixelTypes(pSource, &aPixelTypes);
    DTWAINArrayPtr_RAII raii(pHandle, &aPixelTypes);
    auto& pixInfo = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aPixelTypes);
    pixelMap pMap;
    for (auto curPixInfo : pixInfo)
    {
        auto iter = pMap.insert({curPixInfo, {}}).first;
        DTWAIN_ARRAY aBitDepthInfo = nullptr;
        DTWAINArrayPtr_RAII raii2(pHandle, &aBitDepthInfo);
        DTWAIN_SetPixelType(pSource, curPixInfo, DTWAIN_DEFAULT, TRUE);
        DTWAIN_EnumBitDepths(pSource, &aBitDepthInfo);
        auto& aBitDepthInfoPtr = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aBitDepthInfo);
        for (auto curBitDepth : aBitDepthInfoPtr)
            iter->second.push_back(curBitDepth);
    }
    return pMap;
}

template <typename FnAdjuster>
static std::vector<std::string> getNamesFromConstants(CTL_ITwainSource *pSource, LONG capValue, LONG twainconstantID, FnAdjuster fn)
{
    DTWAIN_ARRAY arr = nullptr;
    const auto pHandle = pSource->GetDTWAINHandle();
    BOOL bRet = DTWAIN_GetCapValuesEx2(pSource, capValue, DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &arr);
    std::vector<std::string> allNames;
    if (bRet)
    {
        DTWAINArrayPtr_RAII raii(pHandle, &arr);
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(arr);
        for (LONG value : vValues)
        {
            value = fn(value);
            allNames.push_back(CTL_StaticData::GetTwainNameFromConstantA(twainconstantID, value).second);
        }
    }
    return allNames;
}

static std::vector<std::string> getDGDATNamesFromConstants(CTL_ITwainSource* pSource)
{
    DTWAIN_ARRAY arr = nullptr;
    const auto pHandle = pSource->GetDTWAINHandle();
    BOOL bRet = DTWAIN_GetCapValuesEx2(pSource, CAP_SUPPORTEDDATS, DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &arr);
    std::vector<std::string> allNames;
    if (bRet)
    {
        DTWAINArrayPtr_RAII raii(pHandle, &arr);
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<TW_UINT32>(arr);
        for (auto value : vValues)
        {
            auto valueDG = (value & 0xFFFF0000) >> 16;
            auto valueDAT = value & 0x0000FFFF;
            std::string dgName = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_DG, valueDG).second;
            std::string datName = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_DAT, valueDAT).second;
            allNames.push_back(dgName + " / " + datName); 
        }
    }
    return allNames;
}

static std::vector<std::string> getCompressionNames(CTL_ITwainSource* pSource)
{
    const auto pHandle = pSource->GetDTWAINHandle();
    std::set<LONG> compressionSets;

    // Enumerate all the file types available for file transfer
    DTWAIN_ARRAY aFileTypes = DTWAIN_EnumFileXferFormatsEx(pSource);
    if (!aFileTypes)
        return {};

    DTWAINArrayLowLevel_RAII raii(pHandle, aFileTypes);

    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aFileTypes);

    if (vValues.empty())
        return {};
    std::vector<std::string> allNames;

    for (auto& fileType : vValues)
    {
        // Call function to enumerate the compressions types for the fileType image type
        auto allCompressionTypes = DTWAIN_EnumCompressionTypesEx2(pSource, fileType, false);
        if (!allCompressionTypes)
            continue;
        DTWAINArrayLowLevel_RAII raii2(pHandle, allCompressionTypes);
        auto& vCompression = pHandle->m_ArrayFactory->underlying_container_t<LONG>(allCompressionTypes);
        compressionSets.insert(vCompression.begin(), vCompression.end());
    }
    for (auto value : compressionSets)
    {
        std::string compressName = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWCP, value).second;
        allNames.push_back(compressName);
    }
    return allNames;
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
    const auto pHandle = pSource->GetDTWAINHandle();
    ResInfoMap resMap;

    // get units of measure
    DTWAIN_ARRAY aUnits = DTWAIN_EnumSourceUnitsEx(pSource);
    DTWAINArrayPtr_RAII raii(pHandle, &aUnits);
    if ( aUnits )
    {
        auto& pUnitsVals = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aUnits);
        size_t sizeLen = pUnitsVals.size();
        DTWAIN_ARRAY aSetUnit = CreateArrayFromCap(pHandle, nullptr, ICAP_UNITS, 1);
        auto& pSetUnitsVal = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aSetUnit);
        DTWAINArrayPtr_RAII raii2(pHandle, &aSetUnit);
        DTWAIN_ARRAY curUnit = nullptr;
        GetCapValuesEx2_Internal(pSource, ICAP_UNITS, DTWAIN_CAPGETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &curUnit);
        if ( curUnit )
        {
            auto& pCurUnit = pHandle->m_ArrayFactory->underlying_container_t<LONG>(curUnit);
            if ( !pCurUnit.empty())
            {
                DTWAINArrayPtr_RAII raiiDefault(pHandle, &curUnit);
                for (size_t i = 0; i < sizeLen; ++i)
                {
                    resMap.insert({pUnitsVals[i],{}});
                    // Set the current unit of measure
                    pSetUnitsVal[0] = pUnitsVals[i];
                    if (SetCapValuesEx2_Internal(pSource, ICAP_UNITS, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, aSetUnit))
                    {
                        // Get the resolution values for this unit of measure
                        DTWAIN_ARRAY aResolutions = {};
                        DTWAINArrayPtr_RAII raii3(pHandle, &aResolutions);
                        GetCapValuesEx2_Internal(pSource, ICAP_XRESOLUTION, DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &aResolutions);
                        if ( aResolutions )
                        {
                            auto& pResolutions = pHandle->m_ArrayFactory->underlying_container_t<double>(aResolutions);
                            LONG nStatus = 0;
                            bool isValidRange = DTWAIN_RangeIsValid(aResolutions, &nStatus);
                            auto iter = resMap.find(pUnitsVals[i]);
                            auto& vect = iter->second.m_AllRes;
                            std::copy(pResolutions.begin(), pResolutions.end(), std::back_inserter(vect));
                            iter->second.m_bIsRange = isValidRange;
                        }
                    }
                }

                // Set the unit back to the original
                SetCapValuesEx2_Internal(pSource, ICAP_UNITS, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, curUnit);
            }
        }
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
   std::array<size_t, 3> mapCounts;
   AllCapInfo() : mapCounts{} {}
};

static AllCapInfo getAllCapInfo(CTL_ITwainSource* pSource)
{
    const auto pHandle = pSource->GetDTWAINHandle();
    AllCapInfo allCapInfo;
    AllCapInfoMap& capInfo = allCapInfo.m_infoMap;

    // get the capabilities
    DTWAIN_ARRAY aAllCaps = DTWAIN_EnumSupportedCapsEx2(pSource);
    DTWAINArrayPtr_RAII aAllCapsraii(pHandle, &aAllCaps);
    auto& vCapBuf = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aAllCaps);

    DTWAIN_ARRAY aExtendedCaps = DTWAIN_EnumExtendedCapsEx2(pSource);
    DTWAINArrayPtr_RAII aExtendedraii(pHandle, &aExtendedCaps);
    std::vector<LONG> vExtBuf;
    if ( aExtendedCaps )
        vExtBuf = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aExtendedCaps);

    DTWAIN_ARRAY aCustomCaps = DTWAIN_EnumCustomCapsEx2(pSource);
    DTWAINArrayPtr_RAII aCustomraii(pHandle, &aCustomCaps);
    std::vector<LONG> vCustomBuf;
    if ( aCustomCaps)
        vCustomBuf = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aCustomCaps);

    // Fill in the general info
    for (auto capVal : vCapBuf)
    {
        auto sz = CTL_TwainAppMgr::GetCapNameFromCap(capVal);
        std::string quouteString = "\"" + sz + "\"";
        auto iter = capInfo.insert({ capVal, {quouteString, capVal, "\"standard\""}}).first;
        iter->second.capName = sz;
        iter->second.value = capVal;
        iter->second.capType = "\"standard\"";
    }

    // Fill in extended cap info
    for (auto capVal : vExtBuf)
    {
        auto iter = capInfo.find(capVal);
        if ( iter != capInfo.end())
            iter->second.capType = "\"standard, extended\"";
    }

    // Fill in custom cap info
    for (auto capVal : vCustomBuf)
    {
        auto iter = capInfo.find(capVal);
        if (iter != capInfo.end())
            iter->second.capType = "\"custom\"";
        // Check if this is also an extended cap
        auto iter2 = std::find(vExtBuf.begin(), vExtBuf.end(), capVal);
        if ( iter2 != vExtBuf.end())
            iter->second.capType = "\"custom, extended\"";
    }
    allCapInfo.mapCounts = {vCapBuf.size(), vExtBuf.size(), vCustomBuf.size()};
    return allCapInfo;
}


static std::string generate_details(CTL_ITwainSession& ts, const std::vector<std::string>& allSources, LONG indentFactor, bool bWeOpenSource=false)
{
    static constexpr int numOneValueDeviceInfo = 12;
    static constexpr int numImageInfoString = 13;
    const auto pHandle = ts.GetTwainDLLHandle();
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
    glob_json["dtwain-version"] = DTWAIN_VERINFO_FILEVERSION;
    glob_json["session-twain-identity"] = json::parse(CTL_TwainIdentity(ts.GetAppIDPtr()).to_json());
    glob_json["device-count"] = allSources.size();
    json array_twain_identity;
    json array_source_names;
    std::vector<std::string> sNames = allSources;
    glob_json["device-names"] = sNames;
    std::string jsonString;
    std::array<std::string, numImageInfoString> imageInfoString;
    std::array<std::string, numOneValueDeviceInfo> deviceInfoString;

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

    struct SourceSelectStatusRAII
    {
        bool m_bSelectStatus;
        CTL_TwainDLLHandle* m_pHandle;
        SourceSelectStatusRAII(CTL_TwainDLLHandle* pHandle, bool bStatus) : m_pHandle(pHandle),  m_bSelectStatus(bStatus) {}
        ~SourceSelectStatusRAII() 
        { 
            m_pHandle->m_bOpenSourceOnSelect = m_bSelectStatus;
        }
    };

    SourceSelectStatusRAII sourceSelectraii(pHandle, pHandle->m_bOpenSourceOnSelect);
    pHandle->m_bOpenSourceOnSelect = false;

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
        deviceInfoString[9] = "\"extendedimageinfo-supported\":false"; 
        deviceInfoString[10] = "\"filesystem-supported\":false";
        deviceInfoString[11] = "\"progressindicator-supported\":false";
        bool devOpen[] = { false, false };

        // Check if we need to select and open the source to see
        // the details
        bool bMustClose = false;
        bool bNullSource = false;
        CTL_ITwainSource* pCurrentSourcePtr = nullptr;

        auto iter = sourceStatusMap.find(curSource);
        if (iter == sourceStatusMap.end())
        {
            jsonString = CTL_TwainIdentity().to_json();
            bNullSource = true;
        }            

        if ( !bNullSource )
        {
            if (iter->second.IsClosed() || iter->second.IsUnknown())
            {
                DTWAIN_SOURCE tempSource = DTWAIN_SelectSourceByNameA(curSource.c_str());
                if (tempSource)
                {
                    pCurrentSourcePtr = (CTL_ITwainSource*)tempSource;
                    bMustClose = true;
                    DTWAIN_OpenSource(tempSource);
                }
            }
            else
            {
                // Source already opened
                DTWAIN_SOURCE openedSource = iter->second.GetSourceHandle();
                pCurrentSourcePtr = (CTL_ITwainSource*)openedSource;
            }
        }

        CloserRAII closer(pCurrentSourcePtr, bMustClose);

        auto curIter = sourceStatusMap.find(curSource);
        if (bNullSource || curIter != sourceStatusMap.end())
        {
            if (bNullSource || curIter->second.IsSelected())
            {
                devOpen[0] = !bNullSource;
                if (!bNullSource && curIter->second.IsOpen())
                {
                    devOpen[1] = true;
                    jsonString = pCurrentSourcePtr->GetSourceInfo();
                    jColorInfo = "\"color-info\":{";
                    std::ostringstream strm;
                    std::ostringstream strm2;
                    std::string allbdepths;

                    // Get the pixel information
                    {
                        auto pixInfo = get_pixel_bitdepth_info(pCurrentSourcePtr);
                        std::vector<LONG> allPixInfo;
                        std::vector<std::string> vPixNames;
                        std::vector<std::string> vPixNamesEx;
                        for (auto& pr : pixInfo)
                        {
                            allPixInfo.push_back(pr.first);
                            char szName[20];
                            DTWAIN_GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWPT, pr.first, szName, 20);
                            std::string sName = szName;
                            vPixNames.push_back(sName);
                            vPixNamesEx.push_back("\"" + vPixNames.back() + "\"");
                        }
                        strm << "\"num-colors\":" << pixInfo.size() << ",";
                        jColorInfo += strm.str();

                        strm.str("");
                        std::string joinStr = join_string(vPixNamesEx.begin(), vPixNamesEx.end());
                        strm << "\"color-types\":[" << joinStr << "],";

                        strm2 << "\"bitdepth-info\":{";
                        int depthCount = 0;
                        for (auto& pr : pixInfo)
                        {
                            strm2 << "\"" << vPixNames[depthCount] << "\":[";
                            std::string bdepthStr = join_string(pr.second.begin(), pr.second.end());
                            strm2 << bdepthStr << "],";
                            ++depthCount;
                        }
                        allbdepths = strm2.str();
                        allbdepths.pop_back();
                        allbdepths += "}";
                    }

                    jColorInfo += strm.str() + allbdepths + "},";

                    struct TwainDataItems
                    {
                        LONG cap;
                        LONG capConstant;
                        const char* name;
                        int prefixCount;
                        const char* prefix;
                        bool usefullName;
                    };

                    std::array<TwainDataItems, 6> otherData = { {
                        { ICAP_SUPPORTEDSIZES, DTWAIN_CONSTANT_TWSS, "\"paper-sizes\":", 5, "TWSS_", true },
                        { ICAP_SUPPORTEDBARCODETYPES, DTWAIN_CONSTANT_TWBT, "\"barcode-supported-types\":", 5, "TWBT_", true },
                        { ICAP_SUPPORTEDPATCHCODETYPES,DTWAIN_CONSTANT_TWPCH, "\"patchcode-supported-types\":", 6, "TWPCH_", true },
                        { ICAP_SUPPORTEDEXTIMAGEINFO,DTWAIN_CONSTANT_TWEI, "\"extendedimageinfo-supported-types\":", 5, "TWEI_", true},
                        { CAP_SUPPORTEDDATS,DTWAIN_CONSTANT_DAT, "\"supported-dat-types\":", 4, "DAT_", true },
                        { ICAP_COMPRESSION, DTWAIN_CONSTANT_TWCP, "\"filecompression-types\":", 5, "TWCP_", true }
                    } };
                    for (auto& oneData : otherData)
                    {
                        strm2.str("");
                        {
                            std::vector<std::string> vNames;
                            if ( oneData.cap == CAP_SUPPORTEDDATS)
                                vNames = getDGDATNamesFromConstants(pCurrentSourcePtr);
                            else
                            if (oneData.cap == ICAP_COMPRESSION)
                                vNames = getCompressionNames(pCurrentSourcePtr);
                            else
                                vNames = getNamesFromConstants(pCurrentSourcePtr, oneData.cap, oneData.capConstant, [](LONG v) { return v; });
                            std::string allSizes;
                            std::vector<std::string> vAdjustedNames;
                            std::transform(vNames.begin(), vNames.end(), std::back_inserter(vAdjustedNames),
                                [&](auto& origName)
                                {
                                    if (!StringWrapperA::StartsWith(origName, oneData.prefix))
                                        return "\"" + origName + "\"";
                                    if (oneData.usefullName)
                                        return "\"" + origName + "\"";
                                    return "\"" +  origName.substr(oneData.prefixCount) + "\"";
                                });
                            std::sort(vAdjustedNames.begin(), vAdjustedNames.end());
                            std::string resultStr = join_string(vAdjustedNames.begin(), vAdjustedNames.end());
                            if (!vNames.empty())
                                strm2 << oneData.name << "[" << resultStr << "],";
                            else
                                strm2 << oneData.name << "\"<unsupported>\",";
                            allSizes = strm2.str();
                            jColorInfo += allSizes;
                        }
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
                            vSizeNames.push_back(StringWrapperA::LowerCase(std::string(buf)).substr(5));
                        }
                        strm << "\"resolution-count\":" << resMap.size() << ",";
                        strm << "\"resolution-units\":";

                        std::vector<std::string> unitNameV;
                        std::transform(vSizeNames.begin(), vSizeNames.end(), std::back_inserter(unitNameV),
                            [](auto& p) { return "\"" + p + "\""; });
                        std::string unitNameStr = "[" + join_string(unitNameV.begin(), unitNameV.end(), ",") + "]";
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

                    static constexpr int SPECIAL_FILESYSTEM = -999;
                    std::array<int, 11> imageInfoCaps = { ICAP_BRIGHTNESS, ICAP_CONTRAST, ICAP_GAMMA, ICAP_HIGHLIGHT, ICAP_SHADOW,
                                                          ICAP_THRESHOLD, ICAP_ROTATION, ICAP_ORIENTATION, ICAP_OVERSCAN, ICAP_HALFTONES, SPECIAL_FILESYSTEM };
                    std::array<std::string, 11> imageInfoCapsStr = { "\"brightness-values\":", "\"contrast-values\":", "\"gamma-values\":",
                        "\"highlight-values\":", "\"shadow-values\":", "\"threshold-values\":",
                        "\"rotation-values\":", "\"orientation-values\":", "\"overscan-values\":", "\"halftone-values\":", "\"filesystem-camera-values\":" };
                    for (size_t curImageCap = 0; curImageCap < imageInfoCaps.size(); ++curImageCap)
                    {
                        strm.str("");
                        strm << imageInfoCapsStr[curImageCap];
                        if (imageInfoCaps[curImageCap] == ICAP_ORIENTATION)
                            create_stream<LONG>(strm, pCurrentSourcePtr, ICAP_ORIENTATION, DTWAIN_CONSTANT_TWOR, true);
                        else
                        if (imageInfoCaps[curImageCap] == ICAP_OVERSCAN)
                            create_stream<LONG>(strm, pCurrentSourcePtr, ICAP_OVERSCAN, DTWAIN_CONSTANT_TWOV, true);
                        else
                        if (imageInfoCaps[curImageCap] == ICAP_HALFTONES)
                            create_stream_from_strings<DefaultStringFnGetter>(strm, pCurrentSourcePtr, ICAP_HALFTONES);
                        else
                        if ( imageInfoCaps[curImageCap] == SPECIAL_FILESYSTEM)
                        {
                            create_stream_from_strings<CameraSystemStringFnGetter>(strm, pCurrentSourcePtr, 0);
                        }
                        else
                            create_stream<double>(strm, pCurrentSourcePtr, imageInfoCaps[curImageCap], 0);
                        imageInfoString[curImageCap] = strm.str();
                    }

                    strm2.str("");
                    // get the capability string
                    auto capInfo = getAllCapInfo(pCurrentSourcePtr);
                    std::vector<OneCapInfo> vCapInfos;
                    for ( auto& pr : capInfo.m_infoMap )
                        vCapInfos.push_back(pr.second);
                    std::sort(vCapInfos.begin(), vCapInfos.end(),
                        [&](const auto& val1, const auto& val2) { return val1.capName < val2.capName;  });
                    for (auto& oneVal : vCapInfos)
                        strm2 << "{ \"name\":\"" << oneVal.capName << "\",\"value\":" << oneVal.value << ",\"type\":" << oneVal.capType << "},";
                    capabilityString = strm2.str();
                    capabilityString.pop_back();
                    capabilityString = "[" + capabilityString + "]";

                    std::ostringstream tempStrm;
                    tempStrm << "\"capability-count\":[{\"all\":" << capInfo.mapCounts[0] << ","
                        "\"custom\":" << capInfo.mapCounts[2] << ","
                        "\"extended\":" << capInfo.mapCounts[1] << "}],\"capability-values\":" << capabilityString;
                    imageInfoString[11] = tempStrm.str();

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
                    std::string customTypes = get_source_file_types(pCurrentSourcePtr); // .str();
                    if (!customTypes.empty())
                    {
//                        customTypes = "\"device-filetype-info\":" + customTypes;
                        allFileTypes += "," + customTypes;
                    }

                    tempStrm << "\"filetype-info\":[" << allFileTypes << "]";
                    imageInfoString[12] = tempStrm.str();

                    strm.str("");

                    std::array<int, numOneValueDeviceInfo> deviceInfoCaps = { CAP_FEEDERENABLED, CAP_FEEDERLOADED, CAP_UICONTROLLABLE,
                                                          ICAP_AUTOBRIGHT, ICAP_AUTOMATICDESKEW,
                                                          CAP_PRINTER, CAP_DUPLEX, CAP_JOBCONTROL, ICAP_LIGHTPATH, ICAP_EXTIMAGEINFO, 
                                                          0, CAP_INDICATORS};

                    std::array<std::string, numOneValueDeviceInfo> deviceInfoCapsStr;
                    std::copy(deviceInfoString.begin(), deviceInfoString.end(), deviceInfoCapsStr.begin());
                    for (auto& s : deviceInfoCapsStr)
                        s.resize(s.size() - 5);
                    for (size_t curDevice = 0; curDevice < deviceInfoCaps.size(); ++curDevice)
                    {
                        if (curDevice > 0)
                            strm << ",";
                        bool value = false;
                        if (deviceInfoCaps[curDevice] == ICAP_EXTIMAGEINFO)
                            value = DTWAIN_IsExtImageInfoSupported(pCurrentSourcePtr) ? true : false;
                        else
                        if (deviceInfoCaps[curDevice] == CAP_FEEDERENABLED)
                            value = DTWAIN_IsFeederSupported(pCurrentSourcePtr) ? true : false;
                        else
                        if (deviceInfoCaps[curDevice] == CAP_UICONTROLLABLE)
                            value = DTWAIN_IsUIControllable(pCurrentSourcePtr) ? true : false;
                        else
                        if (deviceInfoCaps[curDevice] == CAP_PRINTER)
                        {
                            DTWAIN_ARRAY aPrinter = nullptr;
                            DTWAIN_GetCapValuesEx2(pCurrentSourcePtr, CAP_PRINTER, DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &aPrinter);
                            DTWAINArrayPtr_RAII aPrinterRaii(pHandle, &aPrinter);
                            if ( aPrinter )
                            {
                                auto& vBuf = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aPrinter);
                                value = (!vBuf.empty() && vBuf.front() != TWDX_NONE);
                            }
                        }
                        else
                        if (deviceInfoCaps[curDevice] == CAP_JOBCONTROL)
                        {
                            auto vValue = DTWAIN_EnumJobControlsEx(pCurrentSourcePtr);
                            DTWAINArrayPtr_RAII aJob(pHandle, &vValue);
                            if (vValue)
                            {
                                auto& vBuf = pHandle->m_ArrayFactory->underlying_container_t<LONG>(vValue);
                                value = (!vBuf.empty() && vBuf.front() != TWJC_NONE);
                            }
                        }
                        else
                        if ( deviceInfoCaps[curDevice] == 0)
                        {
                            // This is a place holder for file system support
                            value = DTWAIN_IsFileSystemSupported(pCurrentSourcePtr);
                        }
                        else
                            value = DTWAIN_IsCapSupported(pCurrentSourcePtr, deviceInfoCaps[curDevice]) ? true : false;
                        strm << deviceInfoCapsStr[curDevice] << (value ? "true" : "false");
                        deviceInfoString[curDevice] = strm.str();
                    }
                }
                else
                {
                    if ( bNullSource )
                    {
                        CTL_TwainIdentity ti;
                        ti.set_product_name(curSource);
                        jsonString = ti.to_json();
                    }
                    else
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
                    imageInfoString[12] = "\"filesystem-info\":\"<not available>\"";

                    std::string sStatus = "false";
                    if ( bNullSource )
                        sStatus = "<not available>";
                    deviceInfoString[0] = "\"feeder-supported\":\"" + sStatus + "\"";
                    deviceInfoString[1] = "\"feeder-sensitive\":\"" + sStatus + "\""; 
                    deviceInfoString[2] = "\"ui-controllable\":\"" + sStatus + "\""; 
                    deviceInfoString[3] = "\"autobright-supported\":\"" + sStatus + "\""; 
                    deviceInfoString[4] = "\"autodeskew-supported\":\"" + sStatus + "\""; 
                    deviceInfoString[5] = "\"imprinter-supported\":\"" + sStatus + "\""; 
                    deviceInfoString[6] = "\"duplex-supported\":\"" + sStatus + "\""; 
                    deviceInfoString[7] = "\"jobcontrol-supported\":\"" + sStatus + "\""; 
                    deviceInfoString[8] = "\"transparencyunit-supported\":\"" + sStatus + "\""; 
                    deviceInfoString[9] = "\"extendedimageinfo-supported\":\"" + sStatus + "\"";
                    deviceInfoString[10] = "\"filesystem-supported\":\"" + sStatus + "\"";
                    deviceInfoString[11] = "\"progressindicator-supported\":\"" + sStatus + "\"";
                }
                std::string partString = "\"device-name\":\"" + curSource + "\",";
                std::string strStatus;
                if (devOpen[0] && devOpen[1])
                    strStatus = "\"<selected, opened>\"";
                else
                if (!devOpen[0] && !devOpen[1])
                {
                    if ( bNullSource )
                        strStatus = "\"<not installed/detected>\"";
                    else
                        strStatus = "\"<error>\"";
                }
                else
                if (devOpen[0] && !devOpen[1])
                    strStatus = "\"<selected, open_error>\"";

                partString += "\"device-status\":" + strStatus + ",";
                std::string imageInfoStringVal = join_string(imageInfoString.begin(), imageInfoString.end()) + ",";
                std::string deviceInfoStringVal = join_string(deviceInfoString.begin(), deviceInfoString.end()) + ",";
                if (jsonString.empty())
                    jsonString = " }";
                jsonString = "{" + partString + jColorInfo + resUnitInfo + imageInfoStringVal + deviceInfoStringVal + jsonString.substr(1);
                array_twain_identity.push_back(json::parse(jsonString));
            }
        }
    }
    glob_json["device-info"] = array_twain_identity;
    return glob_json.dump(indentFactor);
}

LONG DLLENTRY_DEF DTWAIN_GetSessionDetails(LPTSTR szBuf, LONG nSize, LONG indentFactor, BOOL bRefresh)
{
    LOG_FUNC_ENTRY_PARAMS((szBuf, nSize, indentFactor))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    CTL_StringType details;
    if (!bRefresh &&  !pHandle->m_strSessionDetails.empty())
        details = pHandle->m_strSessionDetails;
    else
    {
        const auto& vBuf = pHandle->m_pTwainSession->GetTwainSources();
        if ( vBuf.empty() )
            LOG_FUNC_EXIT_NONAME_PARAMS(0)
        std::vector<std::string> vAllSources;
        for (auto& theSource : vBuf)
            vAllSources.push_back(theSource->GetProductNameA());
        details = StringConversion::Convert_Ansi_To_Native(generate_details(*pHandle->m_pTwainSession, vAllSources, indentFactor));
        pHandle->m_strSessionDetails = details;
    }
    LONG retVal = StringWrapper::CopyInfoToCString(details, szBuf, nSize);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szBuf))
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_GetSourceDetails(LPCTSTR szSources, LPTSTR szBuf, LONG nSize, LONG indentFactor, BOOL bRefresh)
{
    LOG_FUNC_ENTRY_PARAMS((szSources, szBuf, nSize, indentFactor))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    if (bRefresh)
    {
        CTL_StringArrayType vAllSourcesT;
        std::vector<std::string> vAllSources;
        StringWrapper::TokenizeEx(szSources, _T("|"), vAllSourcesT, false);
        for (auto& name : vAllSourcesT)
            vAllSources.push_back(StringConversion::Convert_Native_To_Ansi(name, name.length()));
        auto genDetails = generate_details(*pHandle->m_pTwainSession, vAllSources, indentFactor);
        #ifdef UNICODE
        pHandle->m_strSourceDetails = StringConversion::Convert_Ansi_To_Native(genDetails, genDetails.length());
        #else
        pHandle->m_strSourceDetails = genDetails;
        #endif
    }
    LONG retVal = StringWrapper::CopyInfoToCString(pHandle->m_strSourceDetails, szBuf, nSize);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szBuf))
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(0)
}
