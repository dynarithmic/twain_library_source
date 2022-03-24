/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2022 Dynarithmic Software.

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
#include "ctlres.h"

#include "cppfunc.h"
#include "ctltwmgr.h"
#include "dtwain_verinfo.h"
#include "dtwstrfn.h"
using namespace dynarithmic;

#ifdef TWAINSAVE_STATIC
LONG  TS_Command(LPCTSTR lpCommand);
#endif

namespace dynarithmic
{
    static bool load2valueMap(std::ifstream& ifs, CTL_TwainLongToStringMap& theMap)
    {
        int value1;
        std::string value2;
        while (ifs >> value1 >> value2)
        {
            if (value1 == -1000 && value2 == "END")
                break;
            theMap.insert({ value1, value2 });
        }
        return true;
    }

    static CTL_StringType createResourceFileName(LPCTSTR resName)
    {
        CTL_StringType sPath;
        if ( CTL_TwainDLLHandle::s_strResourcePath.empty())
            sPath = GetDTWAINExecutionPath();
        else
            sPath = StringWrapper::RemoveBackslashFromDirectory(CTL_TwainDLLHandle::s_strResourcePath);
        sPath = StringWrapper::AddBackslashToDirectory(sPath);
        return sPath + resName;
    }

    bool LoadTwainResources()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        CTL_ErrorStruct ErrorStruct;
        int dg, dat, msg, structtype, retcode, successcode;
        auto sPath = createResourceFileName(DTWAINRESOURCEINFOFILE);
        auto sPathA = StringConversion::Convert_Native_To_Ansi(sPath);
        StringWrapperA::traits_type::inputfile_type ifs(sPathA);
        if (!ifs)
            return false;

        while (ifs >> dg >> dat >> msg >> structtype >> retcode >> successcode)
        {
            if (dg == -1000 && dat == -1000)
                break;
            auto structKey = CTL_GeneralErrorInfo::key_type{ dg,dat,msg };
            ErrorStruct.SetKey(structKey);
            ErrorStruct.SetDataType(structtype);
            ErrorStruct.SetFailureCodes(retcode);
            ErrorStruct.SetSuccessCodes(successcode);
            CTL_TwainDLLHandle::s_mapGeneralErrorInfo.insert({ structKey, ErrorStruct });
        }

        // Load the TWAIN data resources
        int resourceID;
        int twainID;
        std::string twainName;
        while (ifs >> resourceID >> twainID >> twainName)
        {
            if (resourceID == -1000 && twainID == -1000)
                break;
            CTL_TwainDLLHandle::s_TwainNameMap.insert({ { resourceID,twainID }, twainName });
        }

        if (!load2valueMap(ifs, CTL_TwainDLLHandle::GetTwainLanguageMap()))
            return false;
        if (!load2valueMap(ifs, CTL_TwainDLLHandle::GetTwainCountryMap()))
            return false;

        decltype(CTL_CapStruct::m_strCapName) capName;
        decltype(CTL_CapStruct::m_nDataType)  capType;
        decltype(CTL_CapStruct::m_nGetContainer)  capGet;
        decltype(CTL_CapStruct::m_nSetContainer) capSet;
        LONG lCap;

        while (ifs >> lCap >> capName >> capType >> capGet >> capSet)
        {
            if (lCap == -1000 && capName == "END")
                break;
            CTL_CapStruct cStruct;
            cStruct.m_nDataType = capType;
            cStruct.m_nGetContainer = capGet;
            cStruct.m_nSetContainer = capSet;
            cStruct.m_strCapName = capName;
            CTL_TwainDLLHandle::s_mapGeneralCapInfo.insert({ static_cast<TW_UINT16>(lCap), cStruct });
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

        auto& mediamap = CTL_TwainDLLHandle::GetPDFMediaMap();
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
        std::string extList;
        auto& availableFileMap = CTL_TwainDLLHandle::GetAvailableFileFormatsMap();
        while (std::getline(ifs, line))
        {
            StringStreamInA strm(line);
            LONG fileType;
            strm >> fileType;
            std::string name;
            strm >> name;
            name = StringWrapperA::TrimAll(name);
            std::vector<std::string> vExt;
            std::string ext;
            while (strm >> ext)
                vExt.push_back(ext);
            availableFileMap.insert({ fileType, {name,vExt} });
        }

        LOG_FUNC_EXIT_PARAMS(true)
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

    size_t GetResourceStringA(UINT nError, LPSTR buffer, LONG bufSize)
    {
        const auto found = CTL_TwainDLLHandle::s_ResourceStrings.find(nError);
        if (found != CTL_TwainDLLHandle::s_ResourceStrings.end())
            return StringWrapperA::CopyInfoToCString(found->second, buffer, bufSize);
        return 0;
    }

    static bool LoadLanguageResourceFromFileA(const std::string& sPath)
    {
        std::string::value_type sVersion[100];
        DTWAIN_GetShortVersionStringA(sVersion, 100);
        std::ifstream ifs(sPath);
        bool open = false;
        if (ifs)
        {
            std::string descr;
            int resourceID;
            CTL_TwainDLLHandle::s_ResourceStrings.clear();
            open = true;
            std::string line;
            while (getline(ifs, line))
            {
                std::istringstream strm(line);
                while (strm >> resourceID)
                {
                    getline(strm, descr);
                    StringWrapperA::TrimAll(descr);
                    descr = StringWrapperA::ReplaceAll(descr, "{short_version}", sVersion);
                    descr = StringWrapperA::ReplaceAll(descr, "{company_name}", DTWAIN_VERINFO_COMPANYNAME);
                    descr = StringWrapperA::ReplaceAll(descr, "{copyright}", DTWAIN_VERINFO_LEGALCOPYRIGHT);
                    CTL_TwainDLLHandle::s_ResourceStrings.insert({ resourceID, descr });
                }
            }
        }
        return open;
    }

    std::string GetResourceFileNameA(LPCSTR lpszName)
    {
        const auto resPath = createResourceFileName(DTWAINLANGRESOURCEFILE);
        const std::string sPathA = StringConversion::Convert_Native_To_Ansi(resPath);
        return sPathA + lpszName + ".txt";
    }

    bool LoadLanguageResourceA(LPCSTR lpszName, const CTL_ResourceRegistryMap& registryMap)
    {
        LOG_FUNC_ENTRY_PARAMS((lpszName))
        const auto iter = registryMap.find(lpszName);
        if (iter != registryMap.end())
        {
            if ( !iter->second )
                LOG_FUNC_EXIT_PARAMS(false)
        }
        const bool retVal = LoadLanguageResourceA(lpszName);
        LOG_FUNC_EXIT_PARAMS(retVal)
        CATCH_BLOCK(false)
    }

    bool LoadLanguageResourceA(LPCSTR lpszName)
    {
        LOG_FUNC_ENTRY_PARAMS((lpszName))
        const bool bReturn = LoadLanguageResourceFromFileA(GetResourceFileNameA(lpszName));
        LOG_FUNC_EXIT_PARAMS(bReturn)
        CATCH_BLOCK(false)
    }

    bool LoadLanguageResourceA(const std::string& lpszName, const CTL_ResourceRegistryMap& registryMap)
    {
        return LoadLanguageResourceA(lpszName.c_str(), registryMap);
    }

    bool LoadLanguageResourceA(const std::string& lpszName)
    {
        return LoadLanguageResourceA(lpszName.c_str());
    }

    bool LoadLanguageResourceXML(LPCTSTR sLangDLL)
    {
        // Load the XML version of the language resources
        if ( !filesys::exists( sLangDLL))
        {
            return false;
        }
        if ( LoadLanguageResourceXMLImpl(sLangDLL) )
            CTL_TwainDLLHandle::s_UsingCustomResource = true;
        return true;
    }

    void UnloadStringResources()
    {
        CTL_TwainDLLHandle::s_mapGeneralCapInfo.clear();
    }

    void UnloadErrorResources()
    {
        CTL_TwainDLLHandle::s_mapGeneralErrorInfo.clear();
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
