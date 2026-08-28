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
#ifndef CTLTWAINDECODER_H
#define CTLTWAINDECODER_H

#include <tuple>
#include <unordered_map>
#include <string>
#include <boost/functional/hash.hpp>
#include "twain.h"

/* Structure types are as follows
   0 - NONE
   1 - TW_CUSTOMDSDATA
   2 - TW_DEVICEEVENT
   3 - TW_EVENT
   4 - TW_FILESYSTEM
   5 - TW_IDENTITY
   6 - LPHWND (Pointer to window handle)
   7 - TW_PASSTHRU
   8 - TW_PENDINGXFERS
   9 - TW_SETUPFILEXFER
  10 - TW_SETUPMEMXFER
  11 - TW_STATUS
  12 - TW_USERINTERFACE
  13 - pTW_UINT32
  14 - TW_CIECOLOR
  15 - TW_EXTIMAGEINFO
  16 - TW_GRAYRESPONSE
  17 - TW_IMAGEINFO
  18 - TW_IMAGELAYOUT
  19 - TW_IMAGEMEMXFER
  20 - Dib Pointer
  21 - TW_JPEGCOMPRESSION
  22 - TW_PALETTE8
  23 - TW_RGBRESPONSE
  24 - TW_TWUNKIDENTITY
  25 - TW_TWUNKDSENTRYPARAMS
  26 - TW_AUDIOINFO
  27 - pointer to WAV file
  28 - TW_CAPABILITY
*/
namespace dynarithmic
{
   enum DecoderStructTypes {DECODERSTRUCT_NONE,
                            DECODERSTRUCT_TW_CUSTOMDSDATA           ,
                            DECODERSTRUCT_TW_DEVICEEVENT            ,
                            DECODERSTRUCT_TW_EVENT                  ,
                            DECODERSTRUCT_TW_FILESYSTEM             ,
                            DECODERSTRUCT_TW_IDENTITY               ,
                            DECODERSTRUCT_LPHWND                    ,
                            DECODERSTRUCT_TW_PASSTHRU               ,
                            DECODERSTRUCT_TW_PENDINGXFERS           ,
                            DECODERSTRUCT_TW_SETUPFILEXFER          ,
                            DECODERSTRUCT_TW_SETUPMEMXFER           ,
                            DECODERSTRUCT_TW_STATUS                 ,
                            DECODERSTRUCT_TW_USERINTERFACE          ,
                            DECODERSTRUCT_pTW_UINT32                ,
                            DECODERSTRUCT_TW_CIECOLOR               ,
                            DECODERSTRUCT_TW_EXTIMAGEINFO           ,
                            DECODERSTRUCT_TW_GRAYRESPONSE           ,
                            DECODERSTRUCT_TW_IMAGEINFO              ,
                            DECODERSTRUCT_TW_IMAGELAYOUT            ,
                            DECODERSTRUCT_TW_IMAGEMEMXFER           ,
                            DECODERSTRUCT_HDIB                      ,
                            DECODERSTRUCT_TW_JPEGCOMPRESSION        ,
                            DECODERSTRUCT_TW_PALETTE8               ,
                            DECODERSTRUCT_TW_RGBRESPONSE            ,
                            DECODERSTRUCT_TW_TWUNKIDENTITY          ,
                            DECODERSTRUCT_TW_TWUNKDSENTRYPARAMS     ,
                            DECODERSTRUCT_TW_AUDIOINFO              ,
                            DECODERSTRUCT_pWAV                      ,
                            DECODERSTRUCT_TW_CAPABILITY,
                            DECODERSTRUCT_DTWAIN_MESSAGE,
                            DECODERSTRUCT_TW_STATUSUTF8,
                            DECODERSTRUCT_TW_MEMORY,
                            DECODERSTRUCT_TW_ENTRYPOINT,
                            DECODERSTRUCT_TW_CALLBACK,
                            DECODERSTRUCT_TW_CALLBACK2,
                            DECODERSTRUCT_TW_TWAINDIRECT,
                            DECODERSTRUCT_TW_METRICS
   };

   struct PDFTextElement;

    class CTL_TWAINTypeDecoder {
        public:
            CTL_TWAINTypeDecoder() = default;
            void StartDecoder(pTW_IDENTITY pSource, pTW_IDENTITY pDest, TW_UINT32 nDG, TW_UINT16 nDAT, TW_UINT16 nMSG, TW_MEMREF Data,
                              DecoderStructTypes sType);

            static std::string DecodeBitmap(HANDLE hBitmap);
            static std::string DecodePDFTextElement(PDFTextElement* pEl);
            static std::string DecodeTWAINReturnCode(TW_UINT16 retCode);
            static std::string DecodeTWAINCode(TW_UINT16 retCode, TW_UINT16 errStart, std::string_view defMessage);
            static std::string DecodeTWAINReturnCodeCC(TW_UINT16 retCode);
            static std::string DecodeTWAINIdentity(pTW_IDENTITY pId);
            static std::string DecodeTW_INFO(pTW_INFO pInfo, LPCSTR pMem);
            void StartMessageDecoder(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
            const std::string& GetDecodedString() const { return m_pString; }
            static std::string DecodeCapOperations(LONG capOps);
        protected:
            std::string m_pString;
    };

    // Define the cap info structure used
    class CTL_TWAINDecoderStruct
    {
        public:
            using key_type = std::tuple<TW_UINT32, TW_UINT16, TW_UINT16>;
            CTL_TWAINDecoderStruct() :
                m_nStructType(0),
                m_nTWCCErrorCodes(0),
                m_nTWRCCodes(0),
                m_pOrigin(nullptr),
                m_pDest(nullptr),
                m_pData(nullptr),
                m_Key({}, {}, {}) {
            }

            CTL_TWAINDecoderStruct(TW_UINT32 nDG, TW_UINT16 nDAT, TW_UINT16 nMsg) :
                    m_nStructType(0),
                    m_nTWCCErrorCodes(0),
                    m_nTWRCCodes(0),
                    m_pOrigin(nullptr),
                    m_pDest(nullptr),
                    m_pData(nullptr),
                    m_Key{nDG,nDAT,nMsg}
            {}

            void    SetKey(const key_type& nVal) { m_Key = nVal; }
            const   key_type&  GetKey() const { return m_Key; }
            TW_UINT32    GetDG() const { return std::get<0>(m_Key); }
            TW_UINT16    GetDAT() const { return std::get<1>(m_Key); }
            TW_UINT16    GetMSG() const { return std::get<2>(m_Key); }
            UINT    GetDataType() const { return m_nStructType; }
            void    SetDataType(UINT nType) { m_nStructType = nType; }
            LONG    GetFailureCodes() const { return m_nTWCCErrorCodes; }
            void    SetFailureCodes(LONG lFailureCodes) { m_nTWCCErrorCodes = lFailureCodes; }
            LONG    GetSuccessCodes() const { return m_nTWRCCodes; }
            void    SetSuccessCodes(LONG lSuccessCodes) { m_nTWRCCodes = lSuccessCodes; }
            bool    IsFailureMatch(TW_UINT16 cc) const;
            bool    IsSuccessMatch(TW_UINT16 rc) const;
            bool    IsValid() const { return GetDG() || GetDAT() || GetMSG(); }

            std::string GetIdentityAndDataInfo(pTW_IDENTITY pOrigin, pTW_IDENTITY pDest, TW_MEMREF pData)
            {
                m_pOrigin = pOrigin; m_pDest = pDest; m_pData = pData;
                m_Decoder.StartDecoder(m_pOrigin, m_pDest, GetDG(),
                                        GetDAT(), GetMSG(), m_pData,
                                        static_cast<DecoderStructTypes>(m_nStructType));
                return m_Decoder.GetDecodedString();
            }

            std::string GetDTWAINMessageAndDataInfo(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
            {
                m_Decoder.StartMessageDecoder(hWnd, nMsg, wParam, lParam);
                return m_Decoder.GetDecodedString();
            }

            static std::string GetTWAINDSMError(TW_UINT16 retcode)
            {
                return CTL_TWAINTypeDecoder::DecodeTWAINReturnCode(retcode);
            }

            static std::string GetTWAINDSMErrorCC(TW_UINT16 retcode)
            {
                return CTL_TWAINTypeDecoder::DecodeTWAINReturnCodeCC(retcode);
            }

        private:
            friend class CTL_TWAINTypeDecoder;
            UINT       m_nStructType;
            LONG       m_nTWCCErrorCodes;
            LONG       m_nTWRCCodes;
            CTL_TWAINTypeDecoder m_Decoder;
            pTW_IDENTITY m_pOrigin, m_pDest;
            TW_MEMREF  m_pData;
            key_type m_Key;
    };
    using CTL_GeneralErrorInfo = std::unordered_map<CTL_TWAINDecoderStruct::key_type, CTL_TWAINDecoderStruct, boost::hash<CTL_TWAINDecoderStruct::key_type>>;
}
#endif


