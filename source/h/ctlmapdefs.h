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
#ifndef CTLMAPDEFS_H
#define CTLMAPDEFS_H

#include <boost/functional/hash.hpp>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include "ctlsourceinfo.h"
#include "twain.h"
#include "mapdefs.h"
#include "capstructdefs.h"

namespace dynarithmic
{
    struct CacheKeyHash 
    {
        std::size_t operator()(const std::pair<LONG, std::string>& key) const 
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, key.first);
            boost::hash_combine(seed, key.second);
            return seed;
        }
    };

    typedef BASIC_MAPTYPE_<unsigned long, std::shared_ptr<CTL_TwainDLLHandle>> CTL_MapThreadToDLLHandle;
    typedef BASIC_MAPTYPE_<LONG, int> CTL_LongToIntMap;
    typedef BASIC_MAPTYPE_<CTL_StringType, CTL_ITwainSource*> CTL_StringToSourcePtrMap;
    typedef BASIC_MAPTYPE_<CTL_StringType, int> CTL_StringToIntMap;
    typedef BASIC_MAPTYPE_<LONG, HMODULE> CTL_LongToHMODULEMap;
    typedef BASIC_MAPTYPE_<TW_UINT16 , CTL_CapInfo> CTL_EnumCapToInfoMap;
    typedef BASIC_MAPTYPE_<LONG, CTL_StringType> CTL_StringToLongMap;
    typedef BASIC_MAPTYPE_<LONG, std::string> CTL_LongToStringMap;
    typedef BASIC_MAPTYPE_<std::string, CTL_LongToStringMap> CTL_StringToMapLongToStringMap;
    typedef BASIC_MAPTYPE_<LONG, std::vector<LONG>> CTL_LongToVectorLongMap;

    typedef std::vector<CTL_MapThreadToDLLHandle> CTL_HookInfoArray;

    using SourceStatusMap = BASIC_MAPTYPE_<std::string, SourceStatus>;
    using CTL_PDFMediaMap = BASIC_MAPTYPE_<int, std::pair<std::string, std::string>>;
    using TwainConstantType = int64_t;
    using CTL_TwainConstantToStringMapNode = BASIC_MAPTYPE_<TwainConstantType, std::vector<std::string>>;
    using CTL_TwainConstantsMap = BASIC_MAPTYPE_<int, CTL_TwainConstantToStringMapNode>;
    using CTL_TwainIDToStringMap = BASIC_MAPTYPE_<TwainConstantType, std::string>;
    using CTL_ErrorToExtraInfoMap = BASIC_MAPTYPE_<int32_t, std::string>;
    using CTL_ThreadMap = BASIC_MAPTYPE_<std::string, unsigned long>;
    using CTL_StringToConstantMap = BASIC_MAPTYPE_<std::string, TwainConstantType>;
    using CTL_UINT16ToInfoMap = BASIC_MAPTYPE_<TW_UINT16, TW_INFO>;
    using CTL_CompressionMap = BASIC_MAPTYPE_<int, std::vector<int>>;
    using SourceToUIAutocloseMap = BASIC_MAPTYPE_<std::string, bool>;
    using SourceToXferReadyList = std::vector<std::pair<std::string, uint32_t>>;
    using SourceFlatbedOnlyList = std::unordered_set<std::string>;
    using SourceGetMessageList = std::unordered_set<std::string>;
    using SourceSheetcountMap = std::vector<std::pair<std::string, std::string>>;
    using SourcePaperDetectableMap = BASIC_MAPTYPE_<std::string, bool>;
    using CTL_PairToStringMap = std::unordered_map<std::pair<int, std::string>, std::string, CacheKeyHash>;
}
#endif
