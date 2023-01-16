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
 #ifdef _MSC_VER
#pragma warning( disable : 4786)
#endif
#include <sstream>
#include <boost/format.hpp>
#include "ctlreg.h"

#include "ctliface.h"
#include "ctltwmgr.h"
#include "ctlobstr.h"
#include "../simpleini/simpleini.h"
#undef min
#undef max
using namespace dynarithmic;
static std::string NormalizeCapName(const std::string& sName);

bool SaveCapInfoToIni(const std::string& strSourceName, UINT nCap, const CTL_IntArray& rContainerTypes)
{
    #ifdef WIN32
    const char *szName = "dtwain32.ini";
    #else
    const char *szName = "dtwain64.ini";
    #endif

    // Saves the capability information to the DTWAIN16/32.INI
    std::string strKeyName;
    StringStreamOutA strm;
    strm << boost::format("%1%_CAP%2%") % strSourceName % nCap;
    strKeyName = strm.str();
    if ( rContainerTypes.size() != 5 )
        return false;

    // Create the entry string
    std::string strValues;
    std::string strTemp;
    StringArray aStr;
    for ( int i = 0; i < 5; i++ )
    {
        if ( i > 0 )
            strValues += ',';
        const int nValue = rContainerTypes[i];
        strTemp.clear();
        if ( nValue != 0 )
        {
            CTL_TwainAppMgr::GetContainerNamesFromType( nValue, aStr );
            if ( aStr.size() > 0 )
                strTemp = aStr[0];
        }
        strValues += strTemp;
    }

    // Get the section name
    CSimpleIniA customProfile;
    customProfile.LoadFile(szName);
    customProfile.SetValue("TwainControl", strKeyName.c_str(), strValues.c_str());
    return true;
}

bool dynarithmic::GetCapInfoFromIni(const std::string& strCapName,
                       const std::string& strSourceName,
                       UINT /* nCap*/,
                       UINT &rGetValues,
                       UINT &rGetValuesCurrent,
                       UINT &rGetValuesDefault,
                       UINT &rSetValuesCurrent,
                       UINT &rSetValuesAvailable,
                       UINT &rQuerySupport,
                       UINT &rEOJValue,
                       TW_UINT16 &rStateInfo,
                       UINT &rDataType,
                       UINT &rEntryFound,
                       bool &bContainerInfoFound,
                       const ContainerMap &mapContainer
                       )
{
    struct DataTypeToString
    {
        LPCSTR   name;
        unsigned int dataType;
    };

    static constexpr DataTypeToString DataTypeArray[] = {
                    {"TWTY_INT8",    TWTY_INT8},
                    {"TWTY_UINT8",   TWTY_UINT8},
                    {"TWTY_BOOL",    TWTY_BOOL},
                    {"TWTY_INT16",   TWTY_INT16},
                    {"TWTY_INT32",   TWTY_INT32},
                    {"TWTY_UINT16",  TWTY_UINT16},
                    {"TWTY_UINT32",  TWTY_UINT32},
                    {"TWTY_FIX32",   TWTY_FIX32},
                    {"TWTY_STR32",   TWTY_STR32},
                    {"TWTY_STR64",   TWTY_STR64},
                    {"TWTY_STR128",  TWTY_STR128},
                    {"TWTY_STR255",  TWTY_STR255},
                    {"TWTY_STR1024", TWTY_STR1024},
                    {"TWTY_UNI512",  TWTY_UNI512},
                    {"TWTY_FRAME",   TWTY_FRAME}
                    };
    static constexpr unsigned DataTypeArraySize = std::size(DataTypeArray);

    #ifdef WIN32
    CTL_StringType szName = _T("dtwain32.ini");
    #else
    CTL_StringType szName = _T("dtwain64.ini");
    #endif

    bContainerInfoFound = false;

    szName = CTL_TwainDLLHandle::s_sINIPath + szName;

    // Initialize State Info to indicate states 4 - 7 are negotiable for capability
    rStateInfo = 0xFF;

    // Get the capability information from DTWAIN16/32.INI
    std::string strKeyName = strSourceName;
    std::string strStates;

    // Check if profile string is present
    std::string szBuffer;

    // Check if there are any entries for the Source
    // Get the section name
    CSimpleIniA customProfile;
    auto sName = StringConversion::Convert_Native_To_Ansi(szName);
    customProfile.LoadFile(sName.c_str());

    // Check if MSG_QUERYSUPPORT is actually supported
    // First get a global setting
    rQuerySupport = customProfile.GetLongValue("AllSources", "QUERYSUPPORT", 1);

    // Now check the job control detector value
    rEOJValue     = customProfile.GetLongValue("AllSources", "EOJVALUE", 1);

    rEntryFound = 1;

    CSimpleIniA::TNamesDepend keys;
    customProfile.GetAllKeys(strKeyName.c_str(), keys);

    if ( keys.empty() )
    {
        rEntryFound = 0;
        return false;
    }

    std::string strNormalizedCapName = NormalizeCapName(strCapName);

    szBuffer =  customProfile.GetValue(strKeyName.c_str(), strNormalizedCapName.c_str(), " ");
    bool bFound = true;
    if ( szBuffer.empty() )
    {
        rGetValues = 0;
        rGetValuesCurrent = 0;
        rGetValuesDefault = 0;
        rSetValuesCurrent = 0;
        rSetValuesAvailable = 0;
        rDataType = std::numeric_limits<UINT>::max();
        bFound = false;
    }
    else
    {
        rGetValues = std::numeric_limits<UINT>::max();
        rGetValuesCurrent = std::numeric_limits<UINT>::max();
        rGetValuesDefault = std::numeric_limits<UINT>::max();
        rSetValuesCurrent = std::numeric_limits<UINT>::max();
        rSetValuesAvailable = std::numeric_limits<UINT>::max();
        rDataType = std::numeric_limits<UINT>::max();
        bContainerInfoFound = true;
    }
    // Now see if there is the Source has a QUERYSUPPORT setting.  If not, the default setting found
    // above will be used
    rQuerySupport = customProfile.GetLongValue(strKeyName.c_str(), "QUERYSUPPORT", rQuerySupport);

    // Now see if there is the Source has a QUERYSUPPORT setting.  If not, the default setting found
    // above will be used
    rEOJValue = customProfile.GetLongValue(strKeyName.c_str(), "EOJVALUE", rEOJValue);

    // Check the data type
    std::string strDataType = strNormalizedCapName + "_DATATYPE";
    std::string szDataType;
    szDataType = customProfile.GetValue(strKeyName.c_str(), strDataType.c_str(), "");

    // Trim the name found
    strDataType = StringWrapperA::TrimAll(strDataType);
    StringWrapperA::MakeUpperCase(strDataType);
    for (auto i : DataTypeArray)
    {
        if ( StringWrapperA::CompareNoCase(i.name, szDataType.c_str()) )
        {
            rDataType = i.dataType;
            break;
        }
    }

    // Check if there are is any state-related info
    std::string szStates;
    strStates = strCapName + "_STATES";
    szStates = customProfile.GetValue(strKeyName.c_str(), strStates.c_str(), "");

    if ( !bFound && szStates.empty() )
        return false;

    // Check the values in the Capability string (parse)
    StringArray aStr;

    // Make sure that you parse the NULL tokens
    StringWrapperA::Tokenize(szBuffer, ",", aStr, true );

    if (!aStr.empty())
    {
        std::string str;
        ContainerMap::const_iterator it;
        if ( !aStr[0].empty() )
        {
            str = aStr[0];
            it = static_cast<ContainerMap::const_iterator>(mapContainer.find(str));
            if ( it != mapContainer.end())
                rGetValues = (*it).second;
        }
        if ( !aStr[1].empty() )
        {
            str = aStr[1];
            it = static_cast<ContainerMap::const_iterator>(mapContainer.find(str));
            if ( it != mapContainer.end())
                rGetValuesCurrent = (*it).second;
        }
        if ( !aStr[2].empty() )
        {
            str = aStr[2];
            it = static_cast<ContainerMap::const_iterator>(mapContainer.find(str));
            if ( it != mapContainer.end())
                rGetValuesDefault = (*it).second;
        }
        if ( !aStr[3].empty() )
        {
            str = aStr[3];
            it = static_cast<ContainerMap::const_iterator>(mapContainer.find(str));
            if ( it != mapContainer.end())
                rSetValuesCurrent = (*it).second;
        }
        if ( !aStr[4].empty() )
        {
            str = aStr[4];
            it = static_cast<ContainerMap::const_iterator>(mapContainer.find(str));
            if ( it != mapContainer.end())
                rSetValuesAvailable = (*it).second;
        }
    }

    if ( !szStates.empty() )
    {
        // Make sure that you parse the NULL tokens
        StringWrapperA::Tokenize(szStates, ",", aStr, true );
        int nStates = static_cast<int>(aStr.size());
        if ( nStates > 0 )
        {
            std::string strNum;
            short int tempInfo = 0;
            bool bFoundNum = false;
            for ( int Count = 0; Count < nStates; Count++ )
            {
                strNum = StringWrapperA::TrimAll(aStr[Count]);
                if (strNum.length() == 1 )
                {
                    int nNum = stoi(strNum);
                    if ( nNum >= 4 && nNum <= 7 )
                    {
                        bFoundNum = true;
                        tempInfo |= (1 << (nNum-1));
                    }
                }
            }
            if ( bFoundNum )
                rStateInfo = tempInfo;
        }
    }
    return true;
}


std::string NormalizeCapName(const std::string& sName)
{
    // remove spaces in all the name
    StringArray arr;
    StringWrapperA::Tokenize(sName, " ", arr);
    return StringWrapperA::Join(arr);
}
