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
#include "txtfun32.h"
#include "winbit32.h"
#include "ctltwmgr.h"
#include "ctlfileutils.h"

using namespace dynarithmic;
std::string CTextImageHandler::GetFileExtension() const
{
    return "TXT";
}

HANDLE CTextImageHandler::GetFileInformation(LPCSTR /*path*/)
{
    return nullptr;
}

bool CTextImageHandler::OpenOutputFile(LPCTSTR pFileName)
{
    if (m_MultiPageStruct.Stage == DIB_MULTI_FIRST || m_MultiPageStruct.Stage == 0)
    {
        m_hFile = std::make_unique<std::ofstream>(StringConversion::Convert_NativePtr_To_Ansi(pFileName).c_str(), std::ios::binary);
        if (m_hFile.get())
            return true;
    }
    return false;
}

bool CTextImageHandler::CloseOutputFile()
{
    return true;
}

int CTextImageHandler::WriteGraphicFile(CTL_ImageIOHandler* ptrHandler, LPCTSTR path, HANDLE bitmap, void *pUserInfo/*=NULL*/)
{
    return CDibInterface::WriteGraphicFile(ptrHandler, path, bitmap, pUserInfo);
}

void CTextImageHandler::SetMultiPageStatus(DibMultiPageStruct *pStruct)
{
    if ( pStruct )
        m_MultiPageStruct = *pStruct;
}

void CTextImageHandler::GetMultiPageStatus(DibMultiPageStruct *pStruct)
{
    *pStruct = m_MultiPageStruct;
}

int CTextImageHandler::WriteImage(CTL_ImageIOHandler* ptrHandler, BYTE * /*pImage2*/, UINT32 /*wid*/, UINT32 /*ht*/,
                                  UINT32 /*bpp*/, UINT32 /*nColors*/, RGBQUAD * /*pPal*/, void * /*pUserInfo*/)
{
    struct DestroyObjectHandler
    {
        bool doDestroy;
        CTextImageHandler* m_pThis;
        std::shared_ptr<CTextPageInfo> m_pTextPageInfo;
        DestroyObjectHandler(CTextImageHandler* pThis) : doDestroy(true), m_pThis(pThis) {}
        void setDoDestroy(bool bSet) { doDestroy = bSet; }
        void setTextPageInfo(const std::shared_ptr<CTextPageInfo>& ptr) { m_pTextPageInfo = ptr; }
        ~DestroyObjectHandler()
        {
            // Always delete the temporary file
            if ( m_pTextPageInfo && !m_pTextPageInfo->szTempFile.empty())
                delete_file(m_pTextPageInfo->szTempFile.c_str());

            if (doDestroy)
            {
                m_pTextPageInfo.reset();
                try
                {
                    m_pThis->DestroyAllObjects();
                }
                catch (...) {}
            }
        }
    };

    DestroyObjectHandler destroyHandler(this);

    // If multi-page, make sure we remember original TIFF handle
    // First text page
    if ( m_MultiPageStruct.Stage == DIB_MULTI_FIRST || m_MultiPageStruct.Stage == 0 )
    {
        m_bWriteOk = false;
        m_pTextPageInfo = std::make_shared<CTextPageInfo>(0);
        destroyHandler.setTextPageInfo(m_pTextPageInfo);
        m_MultiPageStruct.pUserData = m_pTextPageInfo;
        m_pTextPageInfo->fh = std::move(m_hFile);

        // This is a temp file
        CTL_StringType szTempPath = GetDTWAINTempFilePath();
        if ( szTempPath.empty() )
            return DTWAIN_ERR_FILEWRITE;

        szTempPath += StringWrapper::GetGUID() +  _T("OCR");

        CTL_TwainAppMgr::WriteLogInfo(_T("Temporary Image File is ") + szTempPath + _T("\n"));

        // OK, now convert DIB to the correct file type -- only do this if the input format
        // is not DIB
        if ( m_InputFormat == DTWAIN_BMP)
            m_pTextPageInfo->m_pOrigHandler.reset(new CTL_BmpIOHandler(m_pDib));
        else
        if ( CTL_ITwainSource::IsFileTypeTIFF(static_cast<CTL_TwainFileFormatEnum>(m_InputFormat)))
        {
            m_ImageInfoEx.IsOCRTempImage = true;
            m_pTextPageInfo->m_pOrigHandler.reset(new CTL_TiffIOHandler(m_pDib, m_InputFormat, m_ImageInfoEx));
        }
        m_pTextPageInfo->szTempFile = szTempPath;

        const int retval = WriteTempFile();
        if ( retval != 0 )
            return retval;
        m_pOCREngine->ClearCharacterInfoMap(); // clear the character info map.
        m_pOCREngine->SetCurrentPageNumber(0);
        SaveOCR();
        m_bWriteOk = true;
        if (m_MultiPageStruct.Stage != 0)
            destroyHandler.setDoDestroy(false);
        return 0;
    }
    else
    if ( m_MultiPageStruct.Stage == DIB_MULTI_NEXT )
    {
        // Retrieve handle and write the directory
        m_pTextPageInfo = std::dynamic_pointer_cast<CTextPageInfo>(m_MultiPageStruct.pUserData);

        // Increment the page
        m_pTextPageInfo->curPageNum++;

        std::ofstream* ptrStream = m_pTextPageInfo->fh.get();

        // Write the new line at the end of the file
        char ff = '\x0c';
        ptrStream->write(reinterpret_cast<char*>(&ff), 1);

        // Write the temp file and save the OCR output
        m_pOCREngine->SetCurrentPageNumber(m_pOCREngine->GetCurrentPageNumber() + 1);
        WriteTempFile();
        SaveOCR();
        destroyHandler.setDoDestroy(false);
        return 0;
    }
    else
    if (m_MultiPageStruct.Stage == DIB_MULTI_LAST)
    {
        m_bWriteOk = true;
        if (m_MultiPageStruct.pUserData)
        {
            const auto pTextPageInfo = std::dynamic_pointer_cast<CTextPageInfo>(m_MultiPageStruct.pUserData);
            destroyHandler.setTextPageInfo(pTextPageInfo);
            szTempFile = pTextPageInfo->szTempFile;
            m_hFile = std::move(pTextPageInfo->fh);
        }
    }
    return 0; 
}

int CTextImageHandler::WriteTempFile()
{
    m_pTextPageInfo->m_pOrigHandler->SetDib(m_pDib);
    const int retval = m_pTextPageInfo->m_pOrigHandler->WriteBitmap(m_pTextPageInfo->szTempFile.c_str(), true, 0);
    return retval;
}

int CTextImageHandler::SaveOCR()
{
    // Now OCR the file and save to the text file specified by user
    const LONG bRet = m_pOCREngine->StartOCR(m_pTextPageInfo->szTempFile);
    if ( m_pOCREngine->IsReturnCodeOk(bRet) )
    {
        const std::string theText = m_pOCREngine->GetOCRText();
        m_pTextPageInfo->fh->write(reinterpret_cast<const char *>(theText.c_str()), theText.length()*sizeof(TCHAR));
    }
    return bRet;
}

void CTextImageHandler::DestroyAllObjects()
{
    if (m_hFile && *m_hFile.get())
        m_hFile->close();
    if (!m_bWriteOk)
        filesys::remove(GetOutputFileName().c_str());
}
