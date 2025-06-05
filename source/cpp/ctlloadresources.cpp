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
#include <sstream>
#include <boost/format.hpp>
#include <string_view>
#include "ctliface.h"
#include "ctlloadresources.h"

#include "cppfunc.h"
#include "ctltwainmanager.h"
#include "dtwain_verinfo.h"
#include "dtwstrfn.h"
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
        if (::LoadStringA(CTL_StaticData::GetDLLInstanceHandle(), resNum, szBuffer, DTWAIN_USERRES_MAXSIZE))
            return szBuffer;
        return {};
    }

    static std::vector<std::pair<uint16_t, uint16_t>> parseBracketedPairs(std::string bracketedPairs)
    {
        std::vector<std::pair<uint16_t, uint16_t>> retVal;
        while (true)
        {
            size_t pos = bracketedPairs.find_first_of('[');
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
        auto& resourcePath = CTL_StaticData::GetResourcePath();
        if (resourcePath.empty())
            resourcePath = sPath;
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
        auto crcVal = crc32_aux(0, reinterpret_cast<unsigned char*>(totalBuf.data()), static_cast<unsigned int>(totalBuf.size()));
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
        LOG_FUNC_ENTRY_PARAMS(())
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_LOADED] = true;
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INIFILE_LOADED] = true;
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_VERSION_READ] = true;
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_CRC_CHECK] = true;
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_NODUPLICATE_ID] = true;
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_EXCEPTION_OK] = true;
        CTL_ErrorStruct ErrorStruct;
        TW_UINT32 dg;
        TW_UINT16 dat, msg;
        int structtype, retcode, successcode;
        auto sPath = createResourceFileName(DTWAINRESOURCEINFOFILE);
        retValue.resourcePath = sPath;
        auto sPathA = StringConversion::Convert_Native_To_Ansi(sPath, sPath.length());
        StringWrapperA::traits_type::inputfile_type ifs(sPathA);
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_LOADED] = ifs ? true : false;
        
        // Test for the INI file existing
        retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INIFILE_LOADED] = CTL_StaticData::IsINIFileLoaded();

        // Error if twaininfo.txt or the INI file is missing or cannot be opened
        if (!retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_LOADED] || 
            !retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INIFILE_LOADED])
            return false;

        auto iniInterface = CTL_StaticData::GetINIInterface();

        // Read in warning
        std::string sWarning;
        int curLine = 0;
        std::getline(ifs, sWarning);
        ++curLine;

        static constexpr TW_UINT32 stopper = 9999;
        // Read in the Twain triplet information
        auto& genralErrorMap = CTL_StaticData::GetGeneralErrorInfoMap();
        while (ifs >> dg >> dat >> msg >> structtype >> retcode >> successcode)
        { 
            ++curLine;
            if (dg == stopper)
                break;
            auto structKey = CTL_GeneralErrorInfo::key_type{ dg,dat,msg };
            ErrorStruct.SetKey(structKey);
            ErrorStruct.SetDataType(structtype);
            ErrorStruct.SetFailureCodes(retcode);
            ErrorStruct.SetSuccessCodes(successcode);
            genralErrorMap.insert({ structKey, ErrorStruct });
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

        // First load the offset for the extended information constants
        std::string sOffset;
        int32_t extOffset = 0;
        ifs >> sOffset;
        ++curLine;
        try
        {
            extOffset = stol(sOffset);
        }
        catch (...)
        {
            retValue.m_dupInfo.lineNumber = curLine;
            retValue.m_dupInfo.line = sOffset;
            retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_EXCEPTION_OK] = false;
            return false;
        }
        CTL_StaticData::SetExtImageInfoOffset(extOffset);

        std::string sCap;
        LONG lCap;
        auto& extendedImageInfoMap = CTL_StaticData::GetExtendedImageInfoMap();
        auto& intToInfoMap = CTL_StaticData::GetIntToTwainInfoMap();

        while (ifs >> sCap >> capName >> capType >> capGet >>
                capGetCurrent >> capGetDefault >> capSet >> capSetConstraint >>
                capReset >> capQuery)
        {
            ++curLine;
            try
            {
                if (StringWrapperA::StartsWith(sCap, "0x"))
                    lCap = std::stol(sCap, nullptr, 16);
                else
                    lCap = std::stol(sCap);
            }
            catch (...)
            {
                retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_EXCEPTION_OK] = false;
                retValue.m_dupInfo.lineNumber = curLine;
                retValue.m_dupInfo.line = sCap;
                return false;
            }

            if (lCap == -1000 && capName == "END")
                break;
            bool isTWEIName = StringWrapperA::StartsWith(capName, "TWEI_");
            if (isTWEIName)
            {
                extendedImageInfoMap.insert({ lCap ,capName });
                TW_INFO Info{};
                Info.InfoID = static_cast<TW_UINT16>(lCap);
                Info.ReturnCode = TWRC_DATANOTAVAILABLE;
                intToInfoMap.insert({ static_cast<TW_UINT16>(lCap), Info });
                lCap += extOffset;
            }
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
        }

        auto& bppMap = CTL_ImageIOHandler::GetSupportedBPPMap();
        std::string line;
        while (std::getline(ifs, line))
        {
            ++curLine;
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
            ++curLine;
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
            ++curLine;
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
                ++curLine;
                StringStreamInA strm(line);
                std::string strTwainValue;
                strm >> strTwainValue;
                int64_t twainValue = 0;
                try
                {
                    if (StringWrapperA::StartsWith(strTwainValue, "0x"))
                        twainValue = stoll(strTwainValue, nullptr, 16);
                    else
                        twainValue = stoll(strTwainValue);
                }
                catch (...)
                {
                    retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_EXCEPTION_OK] = false;
                    retValue.m_dupInfo.lineNumber = curLine;
                    retValue.m_dupInfo.line = line;
                    return false;
                }
                if (twainValue == -9999)
                    break;
                std::string name;
                strm >> name;
                std::replace(name.begin(), name.end(), '#', ' ');
                name = StringWrapperA::TrimAll(name);

                // Get all the names associated with this constant
                std::vector<std::string> saNames;
                if (twainValue == IDS_DTWAIN_APPTITLE)
                    saNames.push_back(name);
                else
                    StringWrapperA::Tokenize(name, ", ", saNames);
                iter->second.insert({twainValue, saNames});
                if (stringToConstantMap.find(name) != stringToConstantMap.end())
                {
                    retValue.m_dupInfo.line = line;
                    retValue.m_dupInfo.lineNumber = curLine;
                    retValue.m_dupInfo.duplicateID = twainValue;
                    retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_NODUPLICATE_ID] = false;
                    return false;
                }

                // Always insert the special name that has more than one entry
                if (saNames.size() > 1)
                    stringToConstantMap.insert({ name, twainValue });

                // Insert the actual entries
                for (auto& oneName : saNames)
                    stringToConstantMap.insert({ oneName, twainValue });
            }
        }

        // Read in the image resampling data
        auto& imageMap = CTL_StaticData::GetImageResamplerMap();
        imageMap.clear();
        std::string totalLine;
        while (std::getline(ifs, totalLine))
        {
            ++curLine;
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
        // Read in the file save constants when saving image to a file
        std::vector<std::string> vParsedComponents;
        auto& fileSaveMap = CTL_StaticData::GetFileSaveMap();
        int fileType = 0;
        while (std::getline(ifs, totalLine))
        {
            ++curLine;
            if (totalLine == "END")
                break;

            // Parse the line containing the file save dialog information for the 
            // file type being saved
            StringWrapperA::TokenizeQuoted(StringWrapperA::TrimAll(totalLine), " ", vParsedComponents);
            if (vParsedComponents.size() != 5)
            {
                retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_EXCEPTION_OK] = false;
                retValue.m_dupInfo.lineNumber = curLine;
                retValue.m_dupInfo.line = line;
                return false;
            }

            try
            {
                fileType = std::stoi(vParsedComponents[0]);
            }
            catch (...)
            {
                retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_EXCEPTION_OK] = false;
                retValue.m_dupInfo.lineNumber = curLine;
                retValue.m_dupInfo.line = sCap;
                return false;
            }

            fileSaveMap[fileType] = { fileType,
                StringConversion::Convert_Ansi_To_Native(vParsedComponents[2]),
                StringConversion::Convert_Ansi_To_Native(vParsedComponents[3]),
                StringConversion::Convert_Ansi_To_Native(vParsedComponents[4]) };
        }

        // Now read in the Twain compression -> image type mapping
        while (std::getline(ifs, totalLine))
        {
            ++curLine;
            if (totalLine == "END")
                break;
            // Get the compression type
            auto& compressionMap = CTL_StaticData::GetCompressionMap();
            auto bracketPosStart = totalLine.find_first_of('[');
            auto bracketPosEnd = totalLine.find_first_of(']');
            if ( bracketPosStart == std::string::npos ||
                 bracketPosEnd == std::string::npos)
            {
                retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_EXCEPTION_OK] = false;
                retValue.m_dupInfo.lineNumber = curLine;
                retValue.m_dupInfo.line = line;
                return false;
            }
            auto twainIDName = totalLine.substr(0, bracketPosStart);
            StringWrapperA::TrimAll(twainIDName);
            auto compressionValuePr = CTL_StaticData::GetIDFromTwainName(twainIDName);
            if ( !compressionValuePr.first)
            {
                retValue.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_EXCEPTION_OK] = false;
                retValue.m_dupInfo.lineNumber = curLine;
                retValue.m_dupInfo.line = line;
                return false;
            }
            auto compressionValue = compressionValuePr.second;
            auto allFileTypes = totalLine.substr(bracketPosStart+1);
            StringWrapperA::TrimAll(allFileTypes);
            allFileTypes.pop_back();

            // Parse the file types
            std::vector<std::string> sVector;
            StringWrapperA::Tokenize(allFileTypes, " ", sVector);
            for (auto& str : sVector)
            {
                auto oneVal = CTL_StaticData::GetIDFromTwainName(str);
                compressionMap[static_cast<int>(compressionValue)].push_back(static_cast<int>(oneVal.second));
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

        // Check the CRC value
        CTL_StaticData::GetResourceVersion() = StringConversion::Convert_Ansi_To_Native(DTWAIN_TEXTRESOURCE_FILEVERSION);
        bool doResourceCheck = iniInterface->GetBoolValue(CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_MISCELLANEOUS_KEY).data(),
                                                          CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_RESOURCECHECK_ITEM).data(), true);
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
        const std::string sPathA = StringConversion::Convert_Native_To_Ansi(sPath, sPath.length());
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
        // First check the external resources
        auto str = GetResourceStringFromMap(static_cast<LONG>(nResNumber));
        if (str.empty())
            // Try the internal resources
            str = LoadResourceFromRC(nResNumber);
        return str;
    }

    std::string GetErrorString_Internal(int nError)
    {
        UINT nRealError = std::abs(nError);
        return GetResourceString_Internal(nRealError);
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
        return StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(str), buffer, bufSize);
    }

    CTL_StringType GetResourceStringFromMap_Native(LONG nResourceID)
    {
        return StringConversion::Convert_Ansi_To_Native(GetResourceStringFromMap(nResourceID));
    }

    std::string& GetResourceStringFromMap(LONG nResourceID)
    {
        static std::string retString;

        // Check if string is in cache
        auto& sKey = CTL_StaticData::GetCurrentLanguageResourceKey();
        auto& resCache = CTL_StaticData::GetResourceCache();
        auto iter = resCache.find({ nResourceID, sKey });
        if (iter != resCache.end())
            return iter->second;

        // Get string the long way
        auto currentResource = CTL_StaticData::GetCurrentLanguageResource();
        if (currentResource)
        {
            auto iterFound = currentResource->find(nResourceID);
            if (iterFound != currentResource->end())
            {
                resCache.insert({{ nResourceID, sKey }, iterFound->second});
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

    static bool LoadLanguageResourceFromFileA(const char* szLangName, std::string_view sPath, bool clearEntry, bool bIsCustom)
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
        std::ifstream ifs(sPath.data());
        bool open = false;
        if (ifs)
        {
            CTL_StringToMapLongToStringMap::mapped_type resourceMap;
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
                            CTL_StaticData::GetTwainNameFromConstant(DTWAIN_CONSTANT_DLLINFO, IDS_DTWAIN_APPTITLE).second);
                    StringWrapperA::TrimAll(descr);
                    descr = StringWrapperA::ReplaceAll(descr, "{short_version}", DTWAIN_VERINFO_FILEVERSION);
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
            if (::LoadStringA(CTL_StaticData::GetDLLInstanceHandle(), i, szBuffer, DTWAIN_USERRES_MAXSIZE))
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

    bool LoadLanguageResourceA(const char* lpszName, const CTL_ResourceRegistryMap& registryMap, bool bClear)
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
        CTL_StaticData::GetVersionString().clear();
        GetVersionString();

        LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
        CATCH_BLOCK(false)
    }

    bool LoadLanguageResourceA(const char* lpszName, bool bClear)
    {
        LOG_FUNC_ENTRY_PARAMS((lpszName))
        bool bReturn = LoadLanguageResourceFromFileA(lpszName, GetResourceFileNameA(lpszName, DTWAINLANGRESOURCEFILE), bClear, false);
        LoadLanguageResourceFromFileA(lpszName, GetResourceFileNameA(lpszName, DTWAINCUSTOMRESOURCESFILE), bClear, true);
        LOG_FUNC_EXIT_NONAME_PARAMS(bReturn)
        CATCH_BLOCK(false)
    }

    bool LoadLanguageResourceA(std::string_view lpszName, const CTL_ResourceRegistryMap& registryMap, bool bClear)
    {
        return LoadLanguageResourceA(lpszName.data(), registryMap, bClear);
    }

    bool LoadLanguageResourceA(std::string_view lpszName, bool bClear)
    {
        return LoadLanguageResourceA(lpszName.data(), bClear);
    }

    void UnloadStringResources()
    {
        CTL_StaticData::GetGeneralCapInfo().clear();
    }

    void UnloadErrorResources()
    {
        CTL_StaticData::GetGeneralErrorInfoMap().clear();
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


