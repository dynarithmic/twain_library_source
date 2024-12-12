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
#include "ctltr043.h"
#include "ctltwainmanager.h"
#include "dtwain.h"
#include "winbit32.h"
#include "logwriterutils.h"
using namespace dynarithmic;

CTL_ImageMemFileXferTriplet::CTL_ImageMemFileXferTriplet(CTL_ITwainSession *pSession,
                                                         CTL_ITwainSource* pSource,
                                                         TW_UINT32 numBytes,
                                                         bool bHandleMemory) :
            CTL_ImageXferTriplet(pSession, pSource, DAT_IMAGEMEMFILEXFER), m_ImageMemXferBuffer{}, m_nCompressPos{}, hLocalHandle{}
{
    InitXferBuffer();

    m_ImageMemXferBuffer.Compression = TWON_DONTCARE16;
    m_ImageMemXferBuffer.BytesPerRow =
    m_ImageMemXferBuffer.Columns =
    m_ImageMemXferBuffer.Rows =
    m_ImageMemXferBuffer.XOffset =
    m_ImageMemXferBuffer.YOffset =
    m_ImageMemXferBuffer.BytesWritten = TWON_DONTCARE32;

    InitVars( DAT_IMAGEMEMFILEXFER, CTL_GetTypeGET, &m_ImageMemXferBuffer );

    m_ImageMemXferBuffer.Memory.Length = numBytes;

    HANDLE hBuffer = pSource->GetUserStripBuffer();
    if (bHandleMemory)
    {
        hLocalHandle = ImageMemoryHandler::GlobalAlloc(GMEM_MOVEABLE, numBytes);
        m_ImageMemXferBuffer.Memory.TheMem = static_cast<TW_MEMREF>(ImageMemoryHandler::GlobalLock(hLocalHandle));
    }
    else
        m_ImageMemXferBuffer.Memory.TheMem = static_cast<TW_MEMREF>(ImageMemoryHandler::GlobalLock(hBuffer));
}

void CTL_ImageMemFileXferTriplet::InitXferBuffer()
{
    m_ImageMemXferBuffer.Compression = TWON_DONTCARE16;
    m_ImageMemXferBuffer.BytesPerRow =
        m_ImageMemXferBuffer.Columns =
        m_ImageMemXferBuffer.Rows =
        m_ImageMemXferBuffer.XOffset =
        m_ImageMemXferBuffer.YOffset =
        m_ImageMemXferBuffer.BytesWritten = TWON_DONTCARE32;
}

CTL_ImageMemFileXferTriplet::~CTL_ImageMemFileXferTriplet()
{
    CTL_ITwainSource *pSource = GetSourcePtr();
    HANDLE hBuffer = pSource->GetUserStripBuffer();
    if (hLocalHandle)
    {
        ImageMemoryHandler::GlobalUnlock(ImageMemoryHandler::GlobalHandle(hLocalHandle));
        ImageMemoryHandler::GlobalFree(ImageMemoryHandler::GlobalHandle(hLocalHandle));
    }
    else
    {
        ImageMemoryHandler::GlobalUnlock(ImageMemoryHandler::GlobalHandle(hBuffer));
    }
    pSource->SetBufferStripData(&m_ImageMemXferBuffer);
}

TW_UINT16 CTL_ImageMemFileXferTriplet::Execute()
{
    TW_UINT16 rc = 0;
    if ( !m_ImageMemXferBuffer.Memory.TheMem )
        return TWRC_FAILURE;
    // Get the app manager's AppID

    CTL_ITwainSource *pSource = GetSourcePtr();
    CTL_ITwainSession *pSession = GetSessionPtr();

    // Loop until strips have been transferred
    do
    {
        // Call base function
        rc = CTL_ImageXferTriplet::Execute();

        switch (rc)
        {
            case TWRC_SUCCESS:
                // Send message that a strip has been transferred successfully
                CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr,
                                            DTWAIN_TN_TRANSFERSTRIPDONE,
                                            reinterpret_cast<LPARAM>(pSource));
                InitXferBuffer();
            break;

            case TWRC_CANCEL:
            {
                ImageMemoryHandler::GlobalUnlock(m_ImageMemXferBuffer.Memory.TheMem);
                CancelAcquisition();
            }
            break;

            case TWRC_FAILURE:
            {
                ImageMemoryHandler::GlobalUnlock(m_ImageMemXferBuffer.Memory.TheMem);
                FailAcquisition();
                return rc;
            }

            case TWRC_XFERDONE:             // All strips transferred.  Process bitmap
            {
                // Let Source set the handle
                m_nCompressPos += m_ImageMemXferBuffer.BytesWritten;
                pSource->SetNumCompressBytes(m_nCompressPos);
                pSource->SetTransferDone(true);
            }
            break;
            default:
            {
                StringStreamA strm;
                strm << "Unknown return code " << rc << " from DSM during transfer!  Twain driver unstable!";
                LogWriterUtils::WriteLogInfoIndentedA(strm.str());
                break;
            }
        }
    } while (rc == TWRC_SUCCESS );

    // Delete the buffer if compression used and we have saved to a file
    ImageMemoryHandler::GlobalFreePr(m_ImageMemXferBuffer.Memory.TheMem);
    AbortTransfer(false);
    return rc;
}