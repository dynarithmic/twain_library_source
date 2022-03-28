/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2021 Dynarithmic Software.

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
#ifndef SOURCEACQUIREOPTS_H
#define SOURCEACQUIREOPTS_H

#include "ctliface.h"
namespace dynarithmic
{
    enum { ACQUIRENATIVE=1, ACQUIREBUFFER, ACQUIREFILE, ACQUIRECLIPBOARD, ACQUIRENATIVEEX, ACQUIREBUFFEREX,
        ACQUIREAUDIONATIVE, ACQUIREAUDIOFILE, ACQUIREAUDIONATIVEEX};
    struct SourceAcquireOptions : NotImpl<SourceAcquireOptions>
    {
        DTWAIN_HANDLE DLLHandle;
        DTWAIN_SOURCE Source;
        LONG nPixelType;
        LONG nMaxPages;
        LONG nTransferMode;
        bool bShowUI;
        bool bRemainOpen;
        int nOrigAcquireType;
        int nActualAcquireType;
        LONG   return_status;
        DTWAIN_ARRAY UserArray; // = NULL,
        bool     bDiscardDibs; // = false,
        LONG     lFileType; // = -1,
        LONG     lFileFlags; // = 0,
        LPCTSTR   lpszFile; // = _T(""),
        DTWAIN_ARRAY  FileList; //

        SourceAcquireOptions(DTWAIN_HANDLE hnd=NULL,
                             DTWAIN_SOURCE src=NULL,
                             LONG pixType=0,
                             LONG maxPages=0,
                             LONG transferMode=0,
                             bool showui=true,
                             bool remainopen=true,
                             int whichTrans=0,
                             LONG status=0,
                             DTWAIN_ARRAY uArray=NULL,
                             bool discardPages=false,
                             LONG fileType=-1,
                             LONG fileflags=0,
                             LPCTSTR fileName= StringTraits::GetEmptyString(),
                             DTWAIN_ARRAY fList=NULL) :

                             DLLHandle(hnd),
                             Source(src),
                             nPixelType(pixType),
                             nMaxPages(maxPages),
                             nTransferMode(transferMode),
                             bShowUI(showui),
                             bRemainOpen(remainopen),
                             nOrigAcquireType(whichTrans),
                             nActualAcquireType(0),
                             return_status(status),
                             UserArray(uArray),
                             bDiscardDibs(discardPages),
                             lFileType(fileType),
                             lFileFlags(fileflags),
                             lpszFile(fileName),
                             FileList(fList)
                              {}

        SourceAcquireOptions& setHandle(DTWAIN_HANDLE handle) { DLLHandle = handle; return *this;}
        SourceAcquireOptions& setSource(DTWAIN_SOURCE src) { Source = src; return *this; }
        SourceAcquireOptions& setPixelType(LONG pixType) { nPixelType = pixType; return *this; }
        SourceAcquireOptions& setMaxPages(LONG pages) { nMaxPages = pages; return *this; }
        SourceAcquireOptions& setTransferMode(LONG transMode) { nTransferMode = transMode; return *this; }
        SourceAcquireOptions& setShowUI(bool bShow) { bShowUI = bShow; return *this; }
        SourceAcquireOptions& setRemainOpen(bool bOpen) { bRemainOpen = bOpen; return *this; }
        SourceAcquireOptions& setAcquireType(int nType) { nOrigAcquireType = nType; return *this; }
        SourceAcquireOptions& setUserArray(DTWAIN_ARRAY uArray) { UserArray = uArray; return *this; }
        SourceAcquireOptions& setDiscardDibs(bool bSet) { bDiscardDibs = bSet; return *this; }
        SourceAcquireOptions& setFileType(LONG nType) { lFileType = nType; return *this; }
        SourceAcquireOptions& setFileFlags(LONG nFlags) { lFileFlags = nFlags; return *this; }
        SourceAcquireOptions& setFileName(LPCTSTR szFile) { lpszFile = szFile; return *this; }
        SourceAcquireOptions& setFileList(DTWAIN_ARRAY fList) { FileList = fList; return *this; }
        SourceAcquireOptions& setStatus(LONG nStatus) { return_status = nStatus; return *this;}
        SourceAcquireOptions& setActualAcquireType(LONG acqType) { nActualAcquireType = acqType; return *this;}
        LONG getStatus() const { return return_status; }
        DTWAIN_SOURCE getSource() { return Source; }
        DTWAIN_HANDLE getHandle() { return DLLHandle; }
        LONG getPixelType() const { return nPixelType; }
        bool getDiscardDibs() const { return bDiscardDibs; }
        bool getRemainOpen() const { return bRemainOpen; }
        LONG getAcquireType() const { return nOrigAcquireType; }
        DTWAIN_ARRAY getUserArray() { return UserArray; }
        bool getShowUI() const { return bShowUI; }
        LONG getMaxPages() const { return nMaxPages; }
        DTWAIN_ARRAY getFileList() { return FileList; }
        LONG getFileFlags() const { return lFileFlags; }
        LONG getFileType() const { return lFileType; }
        LPCTSTR getFileName() const { return lpszFile; }
        LONG getTransferMode() const { return nTransferMode; }
        LONG getActualAcquireType() const { return nActualAcquireType; }

        friend std::wostream& operator << (std::wostream& strm, const SourceAcquireOptions& src);
        friend std::ostream& operator << (std::ostream& strm, const SourceAcquireOptions& src);
    };

    inline std::wostream& operator << (std::wostream& strm, const SourceAcquireOptions& src)
    {
        std::wstring nuller = L"null";
        strm << L"DLLHandle=" << src.DLLHandle
            << L", Source=" << src.Source
            << L", nPixelType=" << src.nPixelType
            << L", nMaxPages=" << src.nMaxPages
            << L", nTransferMode=" << src.nTransferMode
            << L", bShowUI=" << src.bShowUI
            << L", bRemainOpen=" << src.bRemainOpen
            << L", nOrigAcquireType=" << src.nOrigAcquireType
            << L", nActualAcquireType=" << src.nActualAcquireType
            << L", return_status=" << src.return_status
            << L", UserArray=" << src.UserArray
            << L", bDiscardDibs=" << src.bDiscardDibs
            << L", lFileType=" << src.lFileType
            << L", lFileFlags=" << src.lFileFlags
            << L", lpszFile=" << (src.lpszFile ? StringConversion::Convert_NativePtr_To_Wide(src.lpszFile) : nuller)
            << L", FileList=" << src.FileList;
        return strm;
    }

    inline std::ostream& operator << (std::ostream& strm, const SourceAcquireOptions& src)
    {
        std::string nuller = "null";
        strm << "DLLHandle=" << src.DLLHandle
            << ", Source=" << src.Source
            << ", nPixelType=" << src.nPixelType
            << ", nMaxPages=" << src.nMaxPages
            << ", nTransferMode=" << src.nTransferMode
            << ", bShowUI=" << src.bShowUI
            << ", bRemainOpen=" << src.bRemainOpen
            << ", nOrigAcquireType=" << src.nOrigAcquireType
            << ", nActualAcquireType=" << src.nActualAcquireType
            << ", return_status=" << src.return_status
            << ", UserArray=" << src.UserArray
            << ", bDiscardDibs=" << src.bDiscardDibs
            << ", lFileType=" << src.lFileType
            << ", lFileFlags=" << src.lFileFlags
            << ", lpszFile=" << (src.lpszFile ? StringConversion::Convert_NativePtr_To_Ansi(src.lpszFile) : nuller)
            << ", FileList=" << src.FileList;
        return strm;
    }

    DTWAIN_ACQUIRE    DTWAIN_LLAcquireNative( SourceAcquireOptions& opts );
    DTWAIN_ACQUIRE    DTWAIN_LLAcquireBuffered( SourceAcquireOptions& opts);
    DTWAIN_ACQUIRE    DTWAIN_LLAcquireFile( SourceAcquireOptions& opts );
    DTWAIN_ACQUIRE    DTWAIN_LLAcquireToClipboard( SourceAcquireOptions& opts);
    DTWAIN_ACQUIRE    DTWAIN_LLAcquireAudioNative(SourceAcquireOptions& opts);
    DTWAIN_ACQUIRE    DTWAIN_LLAcquireAudioFile(SourceAcquireOptions& opts);
}
#endif
