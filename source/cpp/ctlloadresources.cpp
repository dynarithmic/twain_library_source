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
#include <sstream>
#include <boost/format.hpp>
#include "ctliface.h"
#include "ctlloadresources.h"

#include "cppfunc.h"
#include "ctltwainmanager.h"
#include "dtwain_verinfo.h"
#include "dtwstrfn.h"
#include "ctldefsource.h"
#include "resamplefactory.h"
#include "crc32_aux.h"

using namespace dynarithmic;

#ifdef TWAINSAVE_STATIC
LONG  TS_Command(LPCTSTR lpCommand);
#endif


namespace dynarithmic
{
    static std::string LoadResourceFromRC(unsigned resNum)
    {
        char szBuffer[DTWAIN_USERRES_MAXSIZE + 1];
        if (::LoadStringA(CTL_StaticData::s_DLLInstance, resNum, szBuffer, DTWAIN_USERRES_MAXSIZE))
            return szBuffer;
        return {};
    }

    static std::vector<std::pair<uint16_t, uint16_t>> parseBracketedPairs(std::string bracketedPairs)
    {
        std::vector<std::pair<uint16_t, uint16_t>> retVal;
        size_t pos = 0;
        while (true)
        {
            pos = bracketedPairs.find_first_of('[');
            if (pos != std::string::npos)
            {
                auto pos2 = bracketedPairs.find_first_of(']', pos + 1);
                if (pos2 != std::string::npos)
                {
                    std::string subPair = bracketedPairs.substr(pos + 1, pos2 - pos - 1);
                    StringWrapperA::TrimAll(subPair);
                    if (StringWrapperA::IsEmpty(subPair))
                        break;
                    std::istringstream strm(subPair);
                    uint16_t firstNum, secondNum;
                    strm >> firstNum >> secondNum;
                    retVal.push_back({ firstNum, secondNum });
                    bracketedPairs.erase(bracketedPairs.begin(), bracketedPairs.begin() + pos2 + 1);
                }
            }
            else
                break;
        }
        return retVal;
    }

    static std::vector<uint16_t> parseBracketedNumberList(std::string bracketedList)
    {
        std::vector<uint16_t> retVal;
        auto pos = bracketedList.find_first_of('[');
        if (pos == std::string::npos)
            return retVal;
        auto pos2 = bracketedList.find_first_of(']', pos + 1);
        if (pos2 == std::string::npos)
            return retVal;
        bracketedList.erase(bracketedList.begin() + pos2);
        bracketedList.erase(bracketedList.begin() + pos);
        std::istringstream strm(bracketedList);
        uint16_t num;
        while (strm >> num)
            retVal.push_back(num);
        return retVal;
    }

    static CTL_StringType createResourceFileName(LPCTSTR resName)
    {
        CTL_StringType sPath;
        if ( CTL_StaticData::GetResourcePath().empty())
            sPath = GetDTWAINExecutionPath();
        else
            sPath = StringWrapper::RemoveBackslashFromDirectory(CTL_StaticData::GetResourcePath());
        sPath = StringWrapper::AddBackslashToDirectory(sPath);
        if (CTL_StaticData::GetResourcePath().empty())
            CTL_StaticData::s_strResourcePath = sPath;
        return sPath + resName;
    }

    static bool GetDataCRC(std::ifstream& ifs, int numTrailers)
    {
        std::queue<std::string> lineQueue;
        std::string line;
        std::string totalBuf;
        for (int i = 0; i < numTrailers; ++i)
        {
            std::getline(ifs, line);
            boost::trim(line);
            lineQueue.push(line);
        }
        while (std::getline(ifs, line))
        {
            boost::trim(line);
            totalBuf += lineQueue.front();
            lineQueue.pop();
            lineQueue.push(line);
        }
        auto crcVal = crc32_aux(0, (unsigned char*)totalBuf.data(), static_cast<unsigned int>(totalBuf.size()));
        try
        {
            uint64_t crc = std::stoul(lineQueue.front());
            if (crc != crcVal)
                return false;
        }
        catch (...)
        {
            return false;
        }
        return true;
    }

    bool LoadTwainResources(ResourceLoadingInfo& retValue)
    {
        LOG_FUNC_ENTRY_NONAME_PARAMS()
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_LOADED] = true;
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INIFILE_LOADED] = true;
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_VERSION_READ] = true;
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_CRC_CHECK] = true;
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_NODUPLICATE_ID] = true;
        CTL_ErrorStruct ErrorStruct;
        int dg, dat, msg, structtype, retcode, successcode;
        auto sPath = createResourceFileName(DTWAINRESOURCEINFOFILE);
        auto sPathA = StringConversion::Convert_Native_To_Ansi(sPath);
        StringWrapperA::traits_type::inputfile_type ifs(sPathA);
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_LOADED] = ifs ? true : false;
        
        // Test for the INI file existing
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INIFILE_LOADED] = CTL_StaticData::s_bINIFileLoaded;

        // Error if twaininfo.txt or the INI file is missing or cannot be opened
        if (!retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_LOADED] || 
            !retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INIFILE_LOADED])
            return false;

        auto iniInterface = CTL_StaticData::GetINIInterface();

        // Read in warning
        std::string sWarning;
        std::getline(ifs, sWarning);

        // Read in the Twain triplet information
        while (ifs >> dg >> dat >> msg >> structtype >> retcode >> successcode)
        {
            if (dg == -1000 && dat == -1000)
                break;
            auto structKey = CTL_GeneralErrorInfo::key_type{ dg,dat,msg };
            ErrorStruct.SetKey(structKey);
            ErrorStruct.SetDataType(structtype);
            ErrorStruct.SetFailureCodes(retcode);
            ErrorStruct.SetSuccessCodes(successcode);
            CTL_StaticData::s_mapGeneralErrorInfo.insert({ structKey, ErrorStruct });
        }

        // Load the TWAIN data resources
        decltype(CTL_CapStruct::m_strCapName) capName;
        decltype(CTL_CapStruct::m_nDataType)  capType;
        decltype(CTL_CapStruct::m_nGetContainer)  capGet;
        decltype(CTL_CapStruct::m_nSetContainer) capSet;
        decltype(CTL_CapStruct::m_nGetCurrentContainer)  capGetCurrent;
        decltype(CTL_CapStruct::m_nGetDefaultContainer) capGetDefault;
        decltype(CTL_CapStruct::m_nSetConstraintContainer) capSetConstraint;
        decltype(CTL_CapStruct::m_nResetContainer) capReset;
        decltype(CTL_CapStruct::m_nQuerySupportContainer) capQuery;
        LONG lCap;
        auto& extendedImageInfoMap = CTL_StaticData::GetExtendedImageInfoMap();
        while (ifs >> lCap >> capName >> capType >> capGet >>
                capGetCurrent >> capGetDefault >> capSet >> capSetConstraint >>
                capReset >> capQuery)
        {
            if (lCap == -1000 && capName == "END")
                break;
            CTL_CapStruct cStruct;
            cStruct.m_nDataType = capType;
            cStruct.m_nGetContainer = capGet;
            cStruct.m_nSetContainer = capSet;
            cStruct.m_strCapName = capName;
            cStruct.m_nGetCurrentContainer = capGetCurrent;
            cStruct.m_nGetDefaultContainer = capGetDefault;
            cStruct.m_nSetConstraintContainer = capSetConstraint;
            cStruct.m_nResetContainer = capReset;
            cStruct.m_nQuerySupportContainer = capQuery;
            CTL_StaticData::GetGeneralCapInfo().insert({ static_cast<TW_UINT16>(lCap), cStruct });
            if (StringWrapperA::StartsWith(capName, "TWEI_"))
                extendedImageInfoMap.insert({ lCap - 1000 ,capName });
        }

        auto& bppMap = CTL_ImageIOHandler::GetSupportedBPPMap();
        std::string line;
        while (std::getline(ifs, line))
        {
            StringStreamInA strm(line);
            LONG imgType;
            strm >> imgType;
            if (imgType == -1)
                break;
            int bppValue;
            while (strm >> bppValue)
                bppMap.insert({ imgType, std::vector<int>() }).first->second.push_back(bppValue);
        }

        auto& mediamap = CTL_StaticData::GetPDFMediaMap();
        while (std::getline(ifs, line))
        {
            StringStreamInA strm(line);
            LONG pageType;
            strm >> pageType;
            std::string name;
            strm >> name;
            if ( pageType == -1 )
                break;
            name = StringWrapperA::TrimAll(name);
            std::string dimensions;
            std::getline(strm, dimensions);
            dimensions = StringWrapperA::TrimAll(dimensions);
            mediamap.insert({ pageType, {name, dimensions } });
        }

        // Read in the list of available file types.
        auto& availableFileMap = CTL_StaticData::GetAvailableFileFormatsMap();
        while (std::getline(ifs, line))
        {
            StringStreamInA strm(line);
            LONG fileType;
            strm >> fileType;
            if ( fileType == -1 )
                break;
            std::string name;
            strm >> name;
            name = StringWrapperA::TrimAll(name);
            std::vector<std::string> vExt;
            std::string ext;
            while (strm >> ext)
                vExt.push_back(ext);
            availableFileMap.insert({ fileType, {name,vExt} });
        }

        // Read in the TWAIN constants
        auto& constantsMap = CTL_StaticData::GetTwainConstantsMap();
        auto& stringToConstantMap = CTL_StaticData::GetStringToConstantMap();
        for ( int constantVal = 0; constantVal < CTL_TwainDLLHandle::NumTwainMapValues; ++constantVal)
        { 
            auto iter = constantsMap.insert({constantVal, {}}).first;
            while (std::getline(ifs, line))
            {
                StringStreamInA strm(line);
                std::string strTwainValue;
                strm >> strTwainValue;
                LONG twainValue = 0;
                if (StringWrapperA::StartsWith(strTwainValue, "0x"))
                    twainValue = stol(strTwainValue, nullptr, 16);
                else
                    twainValue = stol(strTwainValue);
                if (twainValue == -9999)
                    break;
                std::string name;
                strm >> name;
                std::replace(name.begin(), name.end(), '#', ' ');
                name = StringWrapperA::TrimAll(name);
                iter->second.insert({twainValue, name});
                if (stringToConstantMap.find(name) != stringToConstantMap.end())
                {
                    retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_NODUPLICATE_ID] = false;
                    return false;
                }
                stringToConstantMap.insert({ name, twainValue });
            }
        }

        // Read in the image resampling data
        auto& imageMap = CTL_StaticData::GetImageResamplerMap();
        imageMap.clear();
        std::string sImageConstants, sImageName, sNoResampleVals, sResampleVals;
        std::string totalLine;
        while (std::getline(ifs, totalLine))
        {
            // break up the line into the 4 components
            // image constants
            if (totalLine.compare(0, 3, "END") == 0)
                break;
            auto pos = totalLine.find_first_of('[');
            if (pos == std::string::npos)
                break;
            auto pos2 = totalLine.find_first_of(']', pos + 1);
            if (pos2 == std::string::npos)
                break;
            std::string imageConstants = totalLine.substr(pos, pos2 - pos + 1);
            auto vImageConstants = parseBracketedNumberList(imageConstants);

            // Get the image name
            totalLine.erase(totalLine.begin(), totalLine.begin() + pos2 + 1);
            pos = totalLine.find_first_of('[');
            if (pos == std::string::npos)
                break;
            std::string sImageTypeName = totalLine.substr(0, pos);
            StringWrapperA::TrimAll(sImageTypeName);

            // Get the bits-per-pixel that are "good"
            totalLine.erase(totalLine.begin(), totalLine.begin() + pos);
            pos = totalLine.find_first_of('[');
            if (pos == std::string::npos)
                break;
            pos2 = totalLine.find_first_of(']', pos + 1);
            if (pos2 == std::string::npos)
                break;
            std::string sgoodBits = totalLine.substr(pos, pos2 - pos + 1);
            auto vGoodBits = parseBracketedNumberList(sgoodBits);

            // Get the resample to-from information
            totalLine.erase(totalLine.begin(), totalLine.begin() + pos2 + 1);
            pos = totalLine.find_first_of('[');
            if (pos == std::string::npos)
                break;
            pos2 = totalLine.find_last_of(']');
            if (pos2 == std::string::npos)
                break;
            std::string sBracketList = totalLine.substr(pos, pos2 - pos + 1);
            pos = sBracketList.find_first_of('[', 1);
            if (pos == std::string::npos)
                break;
            sBracketList.pop_back();
            sBracketList = sBracketList.substr(pos);
            auto vResampleData = parseBracketedPairs(sBracketList);

            // Add data to resample image map
            for (auto c : vImageConstants)
            {
                auto iter = imageMap.insert({ c, {} }).first;
                auto& imgNode = iter->second;
                imgNode.m_sImageType = sImageTypeName;
                imgNode.m_vNoSamples = vGoodBits;
                for (auto& pr : vResampleData)
                    imgNode.m_mapFromTo.insert(pr);
            }
        }
        // Read in the minimum version number for this resource
        // Check if resource version if >= running version
        std::getline(ifs, totalLine);
        bool goodVersion = (DTWAIN_TEXTRESOURCE_FILEVERSION == totalLine);
        if (!goodVersion)
        {
            retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_VERSION_READ] = false;
            retValue.errorMessage = StringConversion::Convert_Ansi_To_Native(totalLine);
            return false;
        }
        else
            retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_VERSION_READ] = true;

        CTL_StaticData::s_ResourceVersion = StringConversion::Convert_Ansi_To_Native(DTWAIN_TEXTRESOURCE_FILEVERSION);
        bool doResourceCheck = iniInterface->GetBoolValue("Miscellaneous", "resourcecheck", true);
        if (doResourceCheck)
        {
            ifs.close();
            ifs.open(sPathA);
            retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_CRC_CHECK] = GetDataCRC(ifs, 2);
        }
        else
            retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_CRC_CHECK] = true;

        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }

    std::vector<std::string> GetLangResourceNames()
    {
        std::vector<std::string> ret;
        const auto sPath = createResourceFileName(DTWAINLANGRESOURCENAMESFILE);
        const std::string sPathA = StringConversion::Convert_Native_To_Ansi(sPath);
        std::ifstream ifs(sPathA);
        if (!ifs)
            return ret;
        std::string s;
        while (ifs >> s)
            ret.push_back(s);
        return ret;
    }

    static std::string GetResourceString_Internal(UINT nResNumber)
    {
        auto str = GetResourceStringFromMap(nResNumber);
        // First check the external  resources
        if (str.empty())
            // Try the internal resources
            str = LoadResourceFromRC(nResNumber);
        return str;
    }
    
    size_t GetResourceStringA(UINT nResNumber, LPSTR buffer, LONG bufSize)
    {
        auto str = GetResourceString_Internal(nResNumber);
        return StringWrapperA::CopyInfoToCString(str, buffer, bufSize);
    }

    size_t GetResourceStringW(UINT nResNumber, LPWSTR buffer, LONG bufSize)
    {
        auto str = GetResourceString_Internal(nResNumber);
        auto native_str = StringConversion::Convert_Ansi_To_Wide(str);
        return StringWrapperW::CopyInfoToCString(native_str, buffer, bufSize);
    }

    size_t GetResourceString(UINT nResNumber, LPTSTR buffer, LONG bufSize)
    {
        auto str = GetResourceString_Internal(nResNumber);
        auto native_str = StringConversion::Convert_Ansi_To_Native(str);
        return StringWrapper::CopyInfoToCString(native_str, buffer, bufSize);
    }

    CTL_StringType GetResourceStringFromMap_Native(LONG nError)
    {
        return StringConversion::Convert_Ansi_To_Native(GetResourceStringFromMap(nError));
    }

    std::string& GetResourceStringFromMap(LONG nError)
    {
        static std::string retString;

        // Check if string is in cache
        auto& sKey = CTL_StaticData::GetCurrentLanguageResourceKey();
        auto& resCache = CTL_StaticData::GetResourceCache();
        auto iter = resCache.find({ nError, sKey });
        if (iter != resCache.end())
            return iter->second;

        // Get string the long way
        auto currentResource = CTL_StaticData::GetCurrentLanguageResource();
        if (currentResource)
        {
            auto iterFound = currentResource->find(nError);
            if (iterFound != currentResource->end())
            {
                resCache.insert({{ nError, sKey }, iterFound->second});
                return iterFound->second;
            }
        }
        return retString;
    }


    static void ClearMapEntries(CTL_LongToStringMap& resourceMap, LONG border, bool deleteBeforeBorder = true)
    {
        if (resourceMap.empty())
            return;
        auto iterFirst = resourceMap.lower_bound(border);
        if (iterFirst == resourceMap.end())
        {
            if (deleteBeforeBorder)
                resourceMap.clear();
            return;
        }

        if (deleteBeforeBorder)
            resourceMap.erase(resourceMap.begin(), iterFirst);
        else
            resourceMap.erase(iterFirst, resourceMap.end());
        return;
    }

    static bool LoadLanguageResourceFromFileA(const char* szLangName, const std::string& sPath, bool clearEntry, bool bIsCustom)
    {
        auto& allLanguages = CTL_StaticData::GetAllLanguagesResourceMap();

        // Search for language already loaded
        auto iterLang = allLanguages.find(szLangName);
        if (iterLang != allLanguages.end())
        {
            // language was already loaded, so set as the current language
            CTL_StaticData::SetCurrentLanguageResourceKey(iterLang->first);

            if (clearEntry)
                ClearMapEntries(iterLang->second, DTWAIN_USERRES_START, !bIsCustom);
            else
                return true;
        }

        // Create an empty map
        CTL_StringToMapLongToStringMap::mapped_type resourceMap;
        std::string::value_type sVersion[100];
        DTWAIN_GetShortVersionStringA(sVersion, 100);
        std::ifstream ifs(sPath);
        bool open = false;
        if (ifs)
        {
            std::string descr;
            int resourceID;
            open = true;
            std::string line;
            while (getline(ifs, line))
            {
                std::istringstream strm(line);
                while (strm >> resourceID)
                {
                    getline(strm, descr);
                    if (resourceID == IDS_DTWAIN_APPTITLE)
                        descr = StringConversion::Convert_Native_To_Ansi(
                            CTL_StaticData::GetTwainNameFromConstant(DTWAIN_CONSTANT_DLLINFO, IDS_DTWAIN_APPTITLE));
                    StringWrapperA::TrimAll(descr);
                    descr = StringWrapperA::ReplaceAll(descr, "{short_version}", sVersion);
                    descr = StringWrapperA::ReplaceAll(descr, "{company_name}", DTWAIN_VERINFO_COMPANYNAME);
                    descr = StringWrapperA::ReplaceAll(descr, "{copyright}", DTWAIN_VERINFO_LEGALCOPYRIGHT);
                    resourceMap.insert({ resourceID, descr });
                }
            }
            allLanguages.insert({ szLangName, resourceMap });
            CTL_StaticData::SetCurrentLanguageResourceKey(szLangName);
            auto& info = CTL_StaticData::GetGeneralResourceInfo();
            info.sResourceName = StringConversion::Convert_AnsiPtr_To_Native(szLangName);
            info.bIsFromRC = false;
        }
        return open;
    }

    // Only works for english language
    bool LoadLanguageResourceFromRC()
    {
        const char* lang = "english";
        auto& allLanguages = CTL_StaticData::GetAllLanguagesResourceMap();

        // Search for language already loaded
        auto iterLang = allLanguages.find(lang);
        if (iterLang != allLanguages.end())
        {
            // language was already loaded, so set as the current language
            CTL_StaticData::SetCurrentLanguageResourceKey(lang);

            // Clean out the string resources
            iterLang->second.clear();
        }
        CTL_StringToMapLongToStringMap::mapped_type resourceMap;
        std::string::value_type sVersion[100];
        DTWAIN_GetShortVersionStringA(sVersion, 100);
        // Assume resource ID's are numbered from 0 to 20000
        char szBuffer[DTWAIN_USERRES_MAXSIZE + 1];
        for (int i = 0; i < DTWAIN_USERRES_START; ++i)
        {
            if (::LoadStringA(CTL_StaticData::s_DLLInstance, i, szBuffer, DTWAIN_USERRES_MAXSIZE))
            {
                std::string descr = szBuffer;
                StringWrapperA::TrimAll(descr);
                descr = StringWrapperA::ReplaceAll(descr, "{short_version}", sVersion);
                descr = StringWrapperA::ReplaceAll(descr, "{company_name}", DTWAIN_VERINFO_COMPANYNAME);
                descr = StringWrapperA::ReplaceAll(descr, "{copyright}", DTWAIN_VERINFO_LEGALCOPYRIGHT);
                resourceMap.insert({ i, descr });
            }
        }
        allLanguages.insert({ lang, resourceMap});
        CTL_StaticData::SetCurrentLanguageResourceKey(lang);
        auto& info = CTL_StaticData::GetGeneralResourceInfo();
        info.sResourceName = _T("english");
        info.bIsFromRC = true;
        return true;
    }

    std::string GetResourceFileNameA(LPCSTR lpszName, LPCTSTR szPrefix)
    {
        const auto resPath = createResourceFileName(szPrefix);
        const std::string sPathA = StringConversion::Convert_Native_To_Ansi(resPath);
        return sPathA + lpszName + (std::string)".txt";
    }

    bool LoadLanguageResourceA(LPCSTR lpszName, const CTL_ResourceRegistryMap& registryMap, bool bClear)
    {
        LOG_FUNC_ENTRY_PARAMS((lpszName))
        const auto iter = registryMap.find(lpszName);
        if (iter != registryMap.end())
        {
            if ( !iter->second )
                LOG_FUNC_EXIT_NONAME_PARAMS(false)
        }
        const bool retVal = LoadLanguageResourceA(lpszName, bClear);
        if (!retVal)
        {
            auto& info = CTL_StaticData::GetGeneralResourceInfo();
            info.sResourceName = {};
            info.bIsFromRC = false;
        }

        // Regenerate the cached version information, since the language has changed
        CTL_StaticData::s_VersionString.clear();
        GetVersionString();

        LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
        CATCH_BLOCK(false)
    }

    bool LoadLanguageResourceA(LPCSTR lpszName, bool bClear)
    {
        LOG_FUNC_ENTRY_PARAMS((lpszName))
        bool bReturn = LoadLanguageResourceFromFileA(lpszName, GetResourceFileNameA(lpszName, DTWAINLANGRESOURCEFILE), bClear, false);
        LoadLanguageResourceFromFileA(lpszName, GetResourceFileNameA(lpszName, DTWAINCUSTOMRESOURCESFILE), bClear, true);
        LOG_FUNC_EXIT_NONAME_PARAMS(bReturn)
        CATCH_BLOCK(false)
    }

    bool LoadLanguageResourceA(const std::string& lpszName, const CTL_ResourceRegistryMap& registryMap, bool bClear)
    {
        return LoadLanguageResourceA(lpszName.c_str(), registryMap, bClear);
    }

    bool LoadLanguageResourceA(const std::string& lpszName, bool bClear)
    {
        return LoadLanguageResourceA(lpszName.c_str(), bClear);
    }

    bool LoadLanguageResourceXML(LPCTSTR sLangDLL)
    {
        // Load the XML version of the language resources
        if ( !filesys::exists( sLangDLL))
        {
            return false;
        }
        return true;
    }

    void UnloadStringResources()
    {
        CTL_StaticData::s_mapGeneralCapInfo.clear();
    }

    void UnloadErrorResources()
    {
        CTL_StaticData::s_mapGeneralErrorInfo.clear();
    }

    /////////////////////////////////////////////////////////////////////
    CTL_CapStruct::operator std::string() const
    {
            return m_strCapName;
    }
    ////////////////////////////////////////////////////////////////////
    bool CTL_ErrorStruct::IsFailureMatch(TW_UINT16 cc) const
    {
        return 1L << cc & m_nTWCCErrorCodes?true:false;
    }

    bool CTL_ErrorStruct::IsSuccessMatch(TW_UINT16 rc) const
    {
        if (rc == TWRC_SUCCESS)
            return true;
        return 1L << rc & m_nTWRCCodes?true:false;
    }
}


