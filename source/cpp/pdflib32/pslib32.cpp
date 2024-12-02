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
#ifdef _MSC_VER
#pragma warning (disable:4786)
#endif
#include <dtwainpdf.h>
#include <vector>


//////////////////////////////////
/// Postscript stuff
int PostscriptMain(int argc, const char**, const char *szTitle);

namespace dynarithmic
{
    LONG PDFInterface::DTWLIB_PSWriteFile(LPCTSTR szFileIn,
                                     LPCTSTR szFileOut,
                                     LONG PSType,
                                     LPCTSTR szTitle,
                                     bool bUseEncapsulated)
{
    const std::string sFileIn = StringConversion::Convert_Native_To_Ansi(szFileIn);
    const std::string sFileOut = StringConversion::Convert_Native_To_Ansi(szFileOut);
    const std::string sTitle  = StringConversion::Convert_Native_To_Ansi(szTitle);

    const char *argv[6];
    argv[0] = "DTWLIB";
    argv[1] = "-a";
    switch( PSType )
    {
        case 1:
            argv[2] = "-1";
        break;

        case 2:
            argv[2] = "-2";
        break;

        case 3:
            argv[2] = "-3";
        break;
    }
    std::string sCommandLine = "-O";
    sCommandLine += sFileOut;
    std::vector<char> sCommandV(sCommandLine.length() + 1, 0);
    std::copy(sCommandLine.begin(), sCommandLine.end(), sCommandV.begin());
    argv[3] = &sCommandV[0];

    if ( bUseEncapsulated )
        argv[4] = "-e";
    else
        argv[4] = "-p";

    argv[5] = const_cast<char*>(sFileIn.c_str());
    return PostscriptMain(6, argv, sTitle.c_str());
}
}