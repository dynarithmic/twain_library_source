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
#ifndef CTLFILEUTILS_H
#define CTLFILEUTILS_H

#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <boost/lexical_cast.hpp>
#include <iomanip>

#include "ctlstringutils.h"
#include "ctltimeutils.h"
#include "ctlstringdefs.h"
#include "dtwain_filesystem.h"

namespace dynarithmic
{
    namespace fileutils
    {
        bool delete_file(LPCTSTR filename);
        std::uintmax_t delete_directory(LPCTSTR directory);
        std::pair<bool, std::string> parent_directory_exists(LPCTSTR filename);
        bool file_exists(LPCTSTR filename);
        std::pair<bool, std::string> create_directory(LPCTSTR dirName);
        CTL_StringType temp_directory_path(bool bWithSeparator = true);
        CTL_StringType get_parent_directory(LPCTSTR filename, bool bAddBackSlash = true);
        bool directory_writeable(LPCTSTR filename);
    }

    namespace filenameutils
    {
        enum { DRIVE_POS, DRIVE_PATH, DIRECTORY_POS, NAME_POS, EXTENSION_POS };

        template <typename StringType, typename T>
        StringType PathGenericString(const T& x) 
        { 
            if constexpr (std::is_same_v<StringType, std::string>)
            {
                return x.generic_string();
            }
            else
            if constexpr (std::is_same_v<StringType, std::wstring>)
            { 
                return x.generic_wstring();
            }
            return {};
        }

        template <typename StringViewType, typename StringType>
        void SplitPath(StringViewType str, std::vector<typename StringType>& rArray)
        {
            static constexpr int numComponents = 5;
            using FILESYSTEM_PATHTYPE = filesys::path;
            FILESYSTEM_PATHTYPE p(str);
            rArray.clear();
            if (str.empty())
            {
                rArray.resize(numComponents);
                return;
            }
            rArray.emplace_back(PathGenericString<StringType>(p.root_name()));
            rArray.emplace_back(PathGenericString<StringType>(p.root_directory()));
            rArray.emplace_back(PathGenericString<StringType>(p.parent_path()));
            rArray.emplace_back(PathGenericString<StringType>(p.stem()));
            rArray.emplace_back(PathGenericString<StringType>(p.extension()));
            for (auto& name : rArray)
            {
                if (!name.empty())
                {
                    if constexpr (std::is_same_v<StringType, std::string>)
                        name = std::filesystem::path(name).make_preferred().string();
                    else
                        name = std::filesystem::path(name).make_preferred().native();
                }
            }
        }

        template <typename StringType>
        std::vector<StringType> SplitPath(const StringType& str)
        {
            std::vector<StringType> sArrType;
            SplitPath(str, sArrType);
            return sArrType;
        }

        template <typename CharType>
        void SplitPath(const CharType* str, std::vector<std::basic_string<CharType>>& rArray)
        {
            SplitPath(std::basic_string_view<CharType>(str), rArray);
        }

        template <typename StringType>
        StringType MakePath(const std::vector<StringType>& rArray)
        {
            if (rArray.size() < 5)
                return {};
            StringType s = rArray[NAME_POS] + rArray[EXTENSION_POS];
            const filesys::path dir(rArray[DIRECTORY_POS]);
            const filesys::path file = s;
            filesys::path full_path = dir / file;
            return PathGenericString<StringType>(full_path);
        }

        namespace
        {
            inline std::string CreateFileNameWithDateTime_Impl(std::string_view prefix, std::string_view ext, bool useUTC)
            {
                using namespace std::chrono;
                const auto now = system_clock::now();
                std::string result(prefix);

                if (useUTC)
                {
                    const auto utcMilliseconds = duration_cast<milliseconds>(now.time_since_epoch()).count();
                    result += std::to_string(utcMilliseconds);
                }
                else
                {
                    const auto localmilliseconds = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
                    const std::time_t timeValue = system_clock::to_time_t(now);

                    std::tm localTime{};
                    if (!timeutils::GetLocalTime(timeValue, localTime))
                        return {};

                    std::ostringstream os;

                    os << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S")
                        << '_'
                        << std::setw(3)
                        << std::setfill('0')
                        << localmilliseconds.count();

                    result += os.str();
                }

                if (!ext.empty())
                {
                    result += '.';
                    result.append(ext.data(), ext.size());
                }

                return result;
            }
        }


        template <typename StringType>
        StringType CreateFileNameFromNumber(const StringType& sFileName, int num, int nDigits)
        {
            using CharType = StringType::value_type;
            using StreamType = std::basic_ostringstream<CharType>;

            std::vector<StringType> rArray = {};
            SplitPath(sFileName, rArray);

            // Adjust the file name
            StreamType strm {};
            strm << std::setfill(CharType('0')) << std::setw(nDigits) << num;
            StringType szBuf = strm.str();
            StringType& sTemp = rArray[NAME_POS];
            sTemp = sTemp.substr(0, sTemp.length() - nDigits) + szBuf;
            return MakePath(rArray);
        }

        template <typename StringType>
        int GetInitialFileNumber(const StringType& sFileName, size_t& nDigits)
        {
            using CharType = StringType::value_type;
            std::vector<StringType> rArray = {};
            SplitPath(sFileName, rArray);
            nDigits = 0;
            StringType sTemp;
            const size_t nLen = rArray[NAME_POS].length();
            for (size_t i = nLen - 1; ; --i)
            {
                if (CharTraits<CharType>::IsDigit(rArray[NAME_POS][i]))
                {
                    sTemp = rArray[NAME_POS][i] + sTemp;
                    nDigits++;
                }
                else
                    break;
                if (i == 0)
                    break;
            }

            // now loop until we get a good cast from the string we have
            while (!sTemp.empty())
            {
                try
                {
                    return boost::lexical_cast<int>(sTemp);
                }
                catch (boost::bad_lexical_cast&)
                {
                    sTemp.erase(sTemp.begin());
                }
            }
            return 0;
        }


        // Create a file using the data and time within the file name
        template <typename StringType>
        StringType CreateFileNameWithDateTime(
            std::basic_string_view<typename StringType::value_type> prefix, 
            std::basic_string_view<typename StringType::value_type> ext, bool useUTC = false)
        {
            std::string fileName;
            if constexpr (std::is_same_v<StringType, std::string>)
            {
                fileName = CreateFileNameWithDateTime_Impl(prefix, ext, useUTC);
                return fileName;
            }
            else
            {
                fileName = CreateFileNameWithDateTime_Impl(basicstringutils::Narrow(prefix),
                                                           basicstringutils::Narrow(ext), useUTC);
                return basicstringutils::Widen(fileName);
            }
            return {};
        }

        ////////////////////////////////////////////////////////////////////////////
        template <typename StringType>
        StringType GetPageFileName(const StringType& strBase, int nCurImage, bool bUseLongNames)
        {
            StringType strFormat;
            using CharType = StringType::value_type;
            using StreamType = std::basic_ostringstream<CharType>;
            StreamType strm{};
            strm << nCurImage;
            strFormat = strm.str();
            const int nLenFormat = static_cast<int>(strFormat.length());

            std::vector<StringType> rName = {};
            SplitPath<StringType>(strBase, rName);

            auto strName = rName[NAME_POS];

            if (bUseLongNames)
                strName += strFormat;
            else
            {
                if ((strName + strFormat).length() > 8)
                {
                    int nBase = 8 - nLenFormat;
                    strName = basicstringutils::Left<StringType>(strName, nBase) + strFormat;
                }
                else
                    strName += strFormat;
            }
            rName[NAME_POS] = strName;
            return MakePath(rName);
        }
    }
}
#endif
