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
#include <string>
#include <iterator>
#include <algorithm>
#include "ctliface.h"

using namespace dynarithmic;

template <typename Obj, typename Container>
Container ReadData( std::istream& iFile)
{
    Container v;
    std::copy (std::istream_iterator<Obj> (iFile),
               std::istream_iterator<Obj>(), std::back_inserter(v));
    return v;
}

struct OneLineData
{
    int errnum{};
    std::string errcode;
    std::string errstring;
};

typedef std::vector<OneLineData> OneLineDataVector;

std::istream& operator >> (std::istream& theStream, OneLineData& theData)
{
    theStream >> theData.errnum >> theData.errcode;

   // read in the last name, which could contain spaces
   std::getline(theStream, theData.errstring);

   // set the error string
   StringWrapperA::TrimAll(theData.errstring);
   CTL_TwainDLLHandle::s_ErrorCodes[theData.errnum] = theData.errstring;

   // return the input stream
   return theStream;
}

bool dynarithmic::LoadLanguageResourceXMLImpl(LPCTSTR szFile)
{
    const std::string str = StringConversion::Convert_Native_To_Ansi(szFile);
    std::ifstream xmlFile(str.c_str()); //szFile);
    ReadData<OneLineData, OneLineDataVector>(xmlFile);
    return true;
}
