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
#include "pcxfun32.h"
#include "ctliface.h"
#include <dtwain_filesystem.h>
#ifdef _MSC_VER
#pragma warning (disable:4244)
#endif
using namespace dynarithmic;
std::string CPCXImageHandler::GetFileExtension() const
{
    return "PCX";
}

HANDLE CPCXImageHandler::GetFileInformation(LPCSTR /*path*/)
{
    return nullptr;
}

bool CPCXImageHandler::OpenOutputFile(LPCTSTR pFileName)
{
    if (m_MultiPageStruct.Stage == DIB_MULTI_FIRST || m_MultiPageStruct.Stage == 0)
    {
        m_hFile = std::make_unique<std::ofstream>(StringConversion::Convert_NativePtr_To_Ansi(pFileName).c_str(), std::ios::binary);
        if (!*m_hFile.get())
        {
            SetError(DTWAIN_ERR_FILEOPEN);
            return false;
    }
    }
    return true;
}

bool CPCXImageHandler::CloseOutputFile()
{
    return true;
}

int CPCXImageHandler::WriteGraphicFile(CTL_ImageIOHandler* ptrHandler, LPCTSTR path, HANDLE bitmap, void *pUserInfo/*=NULL*/)
{
    return CDibInterface::WriteGraphicFile(ptrHandler, path, bitmap, pUserInfo);
}

void CPCXImageHandler::SetMultiPageStatus(DibMultiPageStruct *pStruct)
{
    if ( pStruct )
        m_MultiPageStruct = *pStruct;
}

void CPCXImageHandler::GetMultiPageStatus(DibMultiPageStruct *pStruct)
{
    *pStruct = m_MultiPageStruct;
}

int CPCXImageHandler::WriteImage(CTL_ImageIOHandler* ptrHandler, BYTE *pImage2, UINT32 wid, UINT32 ht,
                                 UINT32 bpp, UINT32 /*nColors*/, RGBQUAD * /*pPal*/, void * /*pUserInfo*/)
{
    struct DestroyObjectHandler
    {
        bool doDestroy;
        CPCXImageHandler* m_pThis;
        std::shared_ptr<DibMultiPageData> m_pMultiPageData;
        DestroyObjectHandler(CPCXImageHandler* pThis) : doDestroy(true), m_pThis(pThis) {}
        void setDoDestroy(bool bSet) { doDestroy = bSet;  }
        void setDCXInfo(const std::shared_ptr<DibMultiPageData>& ptrData) { m_pMultiPageData = ptrData; }
        ~DestroyObjectHandler()
        {
            if (doDestroy)
            {
                m_pMultiPageData.reset();
                try
                {
                    m_pThis->DestroyAllObjects();
                }
                catch (...) {}
            }
        }
    };

    DestroyObjectHandler destroyHandler(this);

    m_bWriteOk = false;
    LPBITMAPINFOHEADER lpbi= nullptr;
    PCXHEAD pcx{};
    BYTE *ps;
    unsigned int linewidth;
    int width = 0;
    int depth = 0;
    int bits = 0;
    int a;
    int i;
    int j;
    int k;

    std::ofstream* fh = nullptr;
    if ( m_MultiPageStruct.Stage == DIB_MULTI_FIRST)
    {
        // This is a multi-page PCX (DCX file)
        // create a DCXINFO
        m_pDCXInfo = std::make_shared<DCXINFO>();
        destroyHandler.setDCXInfo(m_pDCXInfo);
        m_MultiPageStruct.pUserData = m_pDCXInfo;
        auto dcxPtr = std::dynamic_pointer_cast<DCXINFO>(m_pDCXInfo);
        dcxPtr->nCurrentPage = 0;
        dcxPtr->fh = std::move(m_hFile);
        fh = dcxPtr->fh.get();

        // Set all elements to 0
        memset(&dcxPtr->DCXHeader, 0, sizeof(DCXHEADER));

        // Set the DCXHeader Id
        dcxPtr->DCXHeader.Id = DCXHEADER_ID;

        // Write offset of first PCX page now
        dcxPtr->DCXHeader.nOffsets[0] = sizeof(DCXHEADER);

        // Save current offset
        dcxPtr->nCurrentOffset = sizeof(DCXHEADER);

        // Write the DCX Header to the file
        fh->write(reinterpret_cast<char *>(&dcxPtr->DCXHeader), sizeof(DCXHEADER));
    }

    else
    if ( m_MultiPageStruct.Stage == DIB_MULTI_NEXT )
    {
        // Retrieve handle and write the directory
        m_pDCXInfo = m_MultiPageStruct.pUserData;
        destroyHandler.setDCXInfo(m_pDCXInfo);
        auto dcxPtr = std::dynamic_pointer_cast<DCXINFO>(m_pDCXInfo);

        // Increment the page
        dcxPtr->nCurrentPage++;

        // Get the file handle
        fh = dcxPtr->fh.get();

        // Get current position in file
        std::streampos current_pos = fh->tellp();

        // Write the info to the file
        long dcxheaderpos = dcxPtr->nCurrentPage * sizeof(DWORD) + sizeof(DWORD);

        // First, go to position in the DCX header
        fh->seekp(dcxheaderpos, std::ios_base::beg);

        // Write the offset into the header
        fh->write(reinterpret_cast<char*>(&dcxPtr->nCurrentOffset), sizeof(DWORD));

        // return to current end of file
        fh->seekp(current_pos, std::ios_base::beg);
    }
    else
    if (m_MultiPageStruct.Stage == DIB_MULTI_LAST)
    {
        m_bWriteOk = true;
        if (m_MultiPageStruct.pUserData)
        {
            m_pDCXInfo = m_MultiPageStruct.pUserData;
            destroyHandler.setDCXInfo(m_pDCXInfo);
            auto dcxPtr = std::dynamic_pointer_cast<DCXINFO>(m_pDCXInfo);
            m_hFile = std::move(dcxPtr->fh);
        }
        return 0; // All OK
    }

    if ( !fh )
        fh = m_hFile.get();
    lpbi = reinterpret_cast<LPBITMAPINFOHEADER>(pImage2);
    fipImage fimage;
    fipImageUtility::copyFromHandle(fimage,lpbi,false);

    linewidth = LPBlinewidth(lpbi);

    m_plinebuffer.resize(linewidth + 1024);
    m_pextrabuffer.resize(linewidth + 1024);

    width = wid;
    depth = ht;
    bits  = bpp;

    pcx = {};

    if(bits < 4)
        GetDibPalette(fimage,pcx.palette);

    // Translate to inches if necessary
    double factor = GetScaleFactorPerInch(m_ImageInfoEx.UnitOfMeasure);

    // set the DPI
    pcx.hres = static_cast<short>(m_ImageInfoEx.ResolutionX * factor);
    pcx.vres = static_cast<short>(m_ImageInfoEx.ResolutionY * factor);

    if(bits==1)
    {
        pcx.manufacturer=10;
        pcx.encoding=1;
        pcx.xmin=pcx.ymin=0;
        pcx.xmax=static_cast<short>(width - 1);
        pcx.ymax=static_cast<short>(depth - 1);
        pcx.palette_type=1;
        pcx.bits=1;
        pcx.version=5;
        pcx.colour_planes=1;
        pcx.bytes_per_line=static_cast<short>(PIXELS2BYTES(width));
    }
    else
    if(bits > 1 && bits <=4)
    {
        pcx.manufacturer=10;
        pcx.encoding=1;
        pcx.xmin=pcx.ymin=0;
        pcx.xmax=static_cast<short>(width)-1;
        pcx.ymax=static_cast<short>(depth)-1;
        pcx.palette_type=1;
        pcx.bits=1;
        pcx.version=5;
        pcx.colour_planes=static_cast<char>(bits);
        pcx.bytes_per_line=static_cast<short>(PIXELS2BYTES(width));
    }
    else
    if(bits > 4 && bits <=8)
    {
        pcx.manufacturer=10;
        pcx.encoding=1;
        pcx.xmin=pcx.ymin=0;
        pcx.xmax=static_cast<short>(width)-1;
        pcx.ymax=static_cast<short>(depth)-1;
        pcx.palette_type=1;
        pcx.bits=8;
        pcx.version=5;
        pcx.colour_planes=1;
        pcx.bytes_per_line=static_cast<short>(width);
    }
    else
    {
        pcx.manufacturer=10;
        pcx.encoding=1;
        pcx.xmin=0;
        pcx.ymin=0;
        pcx.xmax=static_cast<short>(width)-1;
        pcx.ymax=static_cast<short>(depth)-1;
        pcx.colour_planes=3;
        pcx.bytes_per_line=static_cast<short>(width);
        pcx.bits=8;
        pcx.version=5;
    }

    if (bits <= 4)
        GetDibPalette(fimage, pcx.palette);

    fh->write(reinterpret_cast<char*>(&pcx), sizeof(PCXHEAD));

    int nLineWidth = static_cast<DWORD>(LPBlinewidth(lpbi));
    ps = GetDibBits(pImage2) + nLineWidth * static_cast<DWORD>(depth);

    resetbuffer();

    for(i=0;i<depth;++i)
    {
        ps -= nLineWidth;
        memcpy(m_plinebuffer.data(),ps,linewidth);
        if(bits==1)
        {
            if(!PCXWriteLine(m_plinebuffer.data(),*fh,pcx.bytes_per_line))
                return DTWAIN_ERR_FILEWRITE;
        }
        else if(bits > 1 && bits <=4)
        {
            for(k=0;k<bits;++k)
            {
                for(j=0;j<width;++j)
                {
                    a = GetChunkyPixel(m_plinebuffer,j);
                    if(a & bittable[k])
                        m_pextrabuffer[j>>3] |=
                        masktable[j & 0x0007];
                    else
                        m_pextrabuffer[j>>3] &=
                        ~masktable[j & 0x0007];
                }

                if(!PCXWriteLine(m_pextrabuffer.data(),*fh,pcx.bytes_per_line))
                    return DTWAIN_ERR_FILEWRITE;
            }
        }
        else if(bits > 4 && bits <= 8)
        {
            if(!PCXWriteLine(m_plinebuffer.data(),*fh,pcx.bytes_per_line))
                return DTWAIN_ERR_FILEWRITE;
        }
        else if(bits == 24)
        {
            for(j=0;j<width;++j)
                m_pextrabuffer[j]=m_plinebuffer[j*RGB_SIZE+WRGB_RED];
            if(!PCXWriteLine(m_pextrabuffer.data(),*fh,pcx.bytes_per_line))
                return DTWAIN_ERR_FILEWRITE;

            for(j=0;j<width;++j)
                m_pextrabuffer[j]=m_plinebuffer[j*RGB_SIZE+WRGB_GREEN];
            if(!PCXWriteLine(m_pextrabuffer.data(),*fh,pcx.bytes_per_line))
                return DTWAIN_ERR_FILEWRITE;

            for(j=0;j<width;++j)
                m_pextrabuffer[j]=m_plinebuffer[j*RGB_SIZE+WRGB_BLUE];
            if(!PCXWriteLine(m_pextrabuffer.data(),*fh,pcx.bytes_per_line))
                return DTWAIN_ERR_FILEWRITE;
        }
    }

    putbufferedbyte(static_cast<WORD>(EOF),*fh, TRUE);

    if(bits > 4 && bits <=8)
    {
        char palette[768];
        putbyte(12,*fh);
        GetDibPalette(fimage, palette);
        fh->write(reinterpret_cast<char*>(palette), 768);
    }

    m_bWriteOk = true;
    if ( m_MultiPageStruct.Stage == DIB_MULTI_FIRST || m_MultiPageStruct.Stage == DIB_MULTI_NEXT)
    {
        // Save the current position
        auto dcxPtr = std::dynamic_pointer_cast<DCXINFO>(m_pDCXInfo);
        dcxPtr->nCurrentOffset = fh->tellp(); 
        destroyHandler.setDoDestroy(false);
    }

    return 0;
}

void CPCXImageHandler::DestroyAllObjects()
{
    if (m_hFile && *m_hFile.get())
        m_hFile->close();
    if ( !m_bWriteOk )
       filesys::remove(GetOutputFileName().c_str());
}

WORD CPCXImageHandler::PCXWriteLine(LPSTR p, std::ofstream& fh,int n)
{
    unsigned short int j=0,t=0;
    int m_nStatus;
    do {
        unsigned short int i = 0;
        while(p[t+i]==p[t+i+1] && t+i < n && i < 63)++i;
        if(i>0)
        {
            putbufferedbyte(i | 0xc0, fh, false, &m_nStatus);
            if ( m_nStatus == -1 )
                return FALSE;
            putbufferedbyte(p[t], fh, false, &m_nStatus);
            if ( m_nStatus == -1 )
                return FALSE;
            t+=i;
            j+=2;
        }
        else
        {
            if((p[t] & 0xc0)==0xc0)
            {
                putbufferedbyte(0xc1, fh, false, &m_nStatus);
                if ( m_nStatus == -1 )
                    return FALSE;
                ++j;
            }
            putbufferedbyte(p[t++], fh, false, &m_nStatus);
            if ( m_nStatus == -1 )
                return FALSE;
            ++j;
        }
    } while(t<n);
    return TRUE;
}
