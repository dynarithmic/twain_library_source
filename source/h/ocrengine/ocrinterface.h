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
#ifndef OCRINTERFACE_H
#define OCRINTERFACE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <algorithm>

#include "ctlobstr.h"
#define OCROPTION_GETINFO         0
#define OCROPTION_STORECLEANTEXT1 1
#define OCROPTION_STORECLEANTEXT2 2
#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif
namespace dynarithmic
{
    class CTL_TwainDLLHandle;
    struct OCRJobOptions
    {
        std::string sFileName;
        UINT nFileType;
        void *pOtherOptions;
    };

    struct OCRVersionInfo
    {
        int MajorNum;  /* Major revision number of the software. */
        int MinorNum;  /* Incremental revision number of the software. */
        int Language;  /* e.g. TWLG_SWISSFRENCH */
        int Country;   /* e.g. TWCY_SWITZERLAND */
        std::string Info;      /* e.g. "1.0b3 Beta release" */
    };

    struct OCRVersionIdentity
    {
    //    TW_UINT32  Id;              /* Unique number.  In Windows, application hWnd      */
        OCRVersionInfo Version;         /* Identifies the piece of code              */
    //    TW_UINT16  ProtocolMajor;   /* Application and DS must set to TWON_PROTOCOLMAJOR */
    //    TW_UINT16  ProtocolMinor;   /* Application and DS must set to TWON_PROTOCOLMINOR */
    //    TW_UINT32  SupportedGroups; /* Bit field OR combination of DG_ constants */
        std::string Manufacturer;    /* Manufacturer name, e.g. "Hewlett-Packard" */
        std::string ProductFamily;   /* Product family name, e.g. "ScanJet"       */
        std::string ProductName;     /* Product name, e.g. "ScanJet Plus"         */
    };

    class OCREngine;
    typedef std::shared_ptr<OCREngine> OCREnginePtr;

    struct OCRCharacterInfo
    {
        std::vector<double> dConfidence;
        std::vector<int>    xPosition;
        std::vector<int>    yPosition;
        std::vector<int>    iChar;
        std::vector<int>    xWidth;
        std::vector<int>    yWidth;
        OCREngine *sourceEngine;
        int    nPage;
        OCRCharacterInfo(OCREngine* pEngine, int page = 0) : sourceEngine(pEngine),
                                                             nPage(page) { }

        void AddCharacterInfo(double dC, int xPos, int yPos, int iC, int xW, int yW)
        {
            dConfidence.push_back( dC );
            xPosition.push_back( xPos );
            yPosition.push_back( yPos );
            iChar.push_back( iC );
            xWidth.push_back( xW );
            yWidth.push_back( yW );
        }
    };

    struct OCRCapInfo
    {
        LONG CapValue;
        LONG CapDataType;
        LONG CapOperations;
        std::vector<LONG> CapContainerType;

        OCRCapInfo(LONG capValue,
                   LONG capDataType,
                   LONG capOperations,
                   bool isSingleValue) : CapValue(capValue),
                                         CapDataType(capDataType),
                                         CapOperations(capOperations),
                                         CapContainerType(4,0),
                                         doubleData{},
                                         longData{},
                                         m_bIsSingleValue(isSingleValue)
                    { }

        OCRCapInfo() : CapValue(0), CapDataType(-1), CapOperations(-1), doubleData{}, longData{}, m_bIsSingleValue(false) { }

        LONG GetCapDataType() const { return CapDataType; }
        LONG GetCapOperations() const { return CapOperations; }

        struct DoubleData
        {
            double defaultdata;
            double currentdata;
            double minvalue, maxvalue;
            std::vector<double> defaultdata_array;
            std::vector<double> currentdata_array;
        };

        struct LongData
        {
            LONG defaultdata;
            LONG currentdata;
            LONG minvalue, maxvalue;
            std::vector<LONG> defaultdata_array;
            std::vector<LONG> currentdata_array;
        };

        struct StringData
        {
            std::string defaultdata;
            std::string currentdata;
            std::vector<std::string> defaultdata_array;
            std::vector<std::string> currentdata_array;
        };

        DoubleData doubleData;
        LongData longData;
        StringData stringData;
        bool m_bIsSingleValue;

        bool IsSingleValue() const { return m_bIsSingleValue; }

        void SetCapInfo(LONG capValue, LONG capDataType, LONG capOps, bool bIsSingleValue=false)
        {
            CapValue = capValue;
            CapDataType = capDataType;
            CapOperations = capOps;
            m_bIsSingleValue = bIsSingleValue;
        }

        void SetCapContainerTypes(LONG GetType, LONG GetCurrentType, LONG GetDefaultType,
                                 LONG SetType)
        {
            CapContainerType[0] = GetType;
            CapContainerType[1] = GetCurrentType;
            CapContainerType[2] = GetDefaultType;
            CapContainerType[3] = SetType;
        }

        void SetCapDataInfo(double defaultval,
                            double currentval,
                            const std::vector<double>& defaultval_array,
                            const std::vector<double>& currentval_array,
                            double minvalue,
                            double maxvalue)
        {
            doubleData.defaultdata = defaultval;
            doubleData.currentdata = currentval;
            doubleData.currentdata_array = currentval_array;
            doubleData.defaultdata_array = defaultval_array;
            doubleData.minvalue = minvalue;
            doubleData.maxvalue = maxvalue;
        }

        void SetCapDataInfo(LONG defaultval,
                            LONG currentval,
                            const std::vector<LONG>& defaultval_array,
                            const std::vector<LONG>& currentval_array,
                            LONG minvalue,
                            LONG maxvalue)
        {
            longData.defaultdata = defaultval;
            longData.currentdata = currentval;
            longData.currentdata_array = currentval_array;
            longData.defaultdata_array = defaultval_array;
            longData.minvalue = minvalue;
            longData.maxvalue = maxvalue;
        }

        void SetCapDataInfo(const std::string& defaultval,
                            const std::string& currentval,
                            const std::vector<std::string>& defaultval_array,
                            const std::vector<std::string>& currentval_array)
        {
            stringData.defaultdata = defaultval;
            stringData.currentdata = currentval;
            stringData.currentdata_array = currentval_array;
            stringData.defaultdata_array = defaultval_array;
        }

        enum {GETCONTAINER, GETCURRENTCONTAINER, GETDEFAULTCONTAINER, SETCONTAINER};
    };

    struct OCRCacheInfo
    {
        std::string sOCRText;
        std::string  sOCRFileName;
    };

    struct OCRPDFInfo
    {
        typedef enum {PDFINFO_BW, PDFINFO_COLOR, PDFINFO_GRAY} enumPDFColorType;
        LONG FileType[3];
        LONG PixelType[3];
        LONG BitDepth[3];
        OCRPDFInfo()
        {
            std::fill(FileType, FileType + 3, 0);
            std::fill(PixelType, PixelType + 3, 0);
            std::fill(BitDepth, BitDepth + 3, 0);
        }
    };

    class OCREngine
    {
    public:
        typedef std::vector<LONG> OCRLongArrayValues;
        typedef std::vector<std::string> OCRStringArrayValues;
        typedef std::vector<UINT> FileTypeArray;
        typedef std::bitset<16> OptionList;
        typedef std::unordered_map<LONG, std::vector<OCRCharacterInfo> > OCRCharacterInfoMap;
        typedef std::unordered_map<LONG, std::string> OCRPageTextMap;

        std::unordered_map<LONG, OCRCapInfo> m_AllCapValues;
        std::unordered_map<LONG, LONG> m_mapOperations;
        OCRCacheInfo m_OCRCache;
        OCRPDFInfo m_OCRPDFInfo;
        OCRCapInfo& GetOCRCapInfo(LONG nCap);

        OCREngine(CTL_TwainDLLHandle* m_pHandle);
        virtual ~OCREngine();
        virtual bool IsInitialized() const;
        virtual bool SetOptions(OCRJobOptions& options);
        virtual LONG StartupOCREngine();
        virtual LONG StartOCR();
        virtual LONG StartOCR(CTL_StringType filename);
        virtual LONG GetLastError() const;
        virtual std::string GetErrorString(LONG errCode);
        virtual void SetOkErrorCode();
        virtual void SetLastError(LONG errCode);
        virtual bool IsReturnCodeOk(LONG returnCode);
        virtual std::string GetReturnCodeString(LONG returnCode);
        virtual bool SetFileType();
        virtual bool SetFileName();
        virtual std::string GetOCRText(LONG nPage=0);
        virtual std::string GetOCRVersionInfo();
        virtual bool SetOCRVersionIdentity(const OCRVersionIdentity& vIdentity);
        virtual bool SetOCRVersionIdentity();
        virtual bool ShutdownOCR(int& status);
        virtual int GetNumPagesInFile(CTL_StringType /*szFileName*/, int&) {return 1;}
        std::string GetManufacturer() const;
        std::string GetProductFamily() const;
        std::string GetProductName() const;
        LONG GetPDFColorFileType() { return m_OCRPDFInfo.FileType[OCRPDFInfo::PDFINFO_COLOR]; }
        LONG GetPDFBWFileType() { return m_OCRPDFInfo.FileType[OCRPDFInfo::PDFINFO_BW]; }

        std::string GetCachedFile() const;
        std::string GetCachedText() const;

        void AddCharacterInfo(LONG nPage, const OCRCharacterInfo& cInfo);
        void SetBaseOption(int nWhichOption, bool bSet=true);
        bool GetBaseOption(int nWhichOption) const;
        void SetCachedFile(const std::string& filename);
        void SetCachedText(const std::string& sText);
        void SetCurrentPageNumber(int PageNo);
        int GetCurrentPageNumber() const;
        bool IsActivated() const;
        void SetActivated(bool bActive=true);

        // OCR capability functions similar to TWAIN protocol
        bool GetCapValues(LONG nOCRCap, LONG CapType, OCRLongArrayValues& vals);
        bool GetCapValues(LONG nOCRCap, LONG CapType, OCRStringArrayValues& vals);
        bool SetCapValues(LONG nOCRCap, LONG CapType, const OCRLongArrayValues& vals);
        bool SetCapValues(LONG nOCRCap, LONG CapType, const OCRStringArrayValues& vals);
        bool IsCapSupported(LONG nOCRCap) const;
        bool GetSupportedCaps(OCRLongArrayValues& vals) const;
        bool SetPDFFileTypes(OCRPDFInfo::enumPDFColorType nWhich, LONG fileType, LONG pixelType, LONG bitDepth);
        bool SetAllFileTypes(const FileTypeArray& allTypes);
        FileTypeArray GetAllFileTypes() const;
        void AddCapValue(LONG nCap, const OCRCapInfo& capInfo);
        LONG GetCapDataType(LONG nOCRCap) const;
        void ClearCharacterInfoMap();
        std::vector<OCRCharacterInfo>& GetCharacterInfo(LONG nPage, int &status);
        void SetPageTextMap(LONG nPage, const std::string& sData);
        bool IsValidOCRPage(LONG nPage) const;
        CTL_TwainDLLHandle* GetDTWAINHandle() { return m_pHandle; }


    protected:
        virtual bool ProcessGetCapValues(LONG, LONG, OCRLongArrayValues&);
        virtual bool ProcessGetCapValues(LONG, LONG, OCRStringArrayValues&);
        virtual bool ProcessSetCapValues(LONG, LONG, const OCRLongArrayValues&);
        virtual bool ProcessSetCapValues(LONG, LONG, const OCRStringArrayValues&);

    private:
        FileTypeArray m_fileTypes;
        OCRVersionIdentity m_OCRIdentity;
        OptionList m_baseOptions;
        OCRCharacterInfoMap m_OCRCharMap;
        OCRPageTextMap      m_OCRPageTextMap;
        int m_nCurrentPage;
        std::vector<OCRCharacterInfo> m_InvalidOCRCharInfo;
        bool m_bIsActivated;
        LONG m_nLastOCRError;
        CTL_TwainDLLHandle* m_pHandle;
    };
}
#endif
