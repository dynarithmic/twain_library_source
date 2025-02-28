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
#include "ctltmpl5.h"
#include "errorcheck.h"
#include "twain.h"
using namespace dynarithmic;
/* These functions can only be used in State 7   (when DTWAIN_TN_TRANSFERDONE notification is sent).
   This means that only languages that can utilize DTWAIN_SetCallback or can intercept Window's
   messages can use these functions. */

/* The source** must** have the right InfoType for the following extended information types.
   If not, then this is a violation of the TWAIN specification and retrieval cannot proceed. */
static constexpr std::array<TW_UINT16, 81> aRequiredTypeMatches = {TWEI_BARCODECOUNT,
                                                                   TWEI_BARCODEX,
                                                                   TWEI_BARCODEY,
                                                                   TWEI_BARCODETYPE,
                                                                   TWEI_BARCODETEXT,
                                                                   TWEI_BARCODEROTATION,
                                                                   TWEI_BARCODECONFIDENCE,
                                                                   TWEI_BARCODETEXTLENGTH,
                                                                   TWEI_CAMERA,
                                                                   TWEI_BOOKNAME,
                                                                   TWEI_CHAPTERNUMBER,
                                                                   TWEI_DOCUMENTNUMBER,
                                                                   TWEI_PAGENUMBER,
                                                                   TWEI_FRAMENUMBER,
                                                                   TWEI_PAGESIDE,
                                                                   TWEI_PIXELFLAVOR,
                                                                   TWEI_DESKEWSTATUS,
                                                                   TWEI_SKEWORIGINALANGLE,
                                                                   TWEI_SKEWFINALANGLE,
                                                                   TWEI_SKEWCONFIDENCE,
                                                                   TWEI_SKEWWINDOWX1,
                                                                   TWEI_SKEWWINDOWY1,
                                                                   TWEI_SKEWWINDOWX2,
                                                                   TWEI_SKEWWINDOWY2,
                                                                   TWEI_SKEWWINDOWX3,
                                                                   TWEI_SKEWWINDOWY3,
                                                                   TWEI_SKEWWINDOWX4,
                                                                   TWEI_SKEWWINDOWY4,
                                                                   TWEI_DESHADELEFT,
                                                                   TWEI_DESHADETOP,
                                                                   TWEI_DESHADEWIDTH,
                                                                   TWEI_DESHADEHEIGHT,
                                                                   TWEI_DESHADESIZE,
                                                                   TWEI_DESHADEBLACKCOUNTOLD,
                                                                   TWEI_DESHADEBLACKCOUNTNEW,
                                                                   TWEI_DESHADEBLACKRLMIN,
                                                                   TWEI_DESHADEBLACKRLMAX,
                                                                   TWEI_DESHADEWHITECOUNTOLD,
                                                                   TWEI_DESHADEWHITECOUNTNEW,
                                                                   TWEI_DESHADEWHITERLMIN,
                                                                   TWEI_DESHADEWHITERLMAX,
                                                                   TWEI_DESHADEWHITERLAVE,
                                                                   TWEI_SPECKLESREMOVED,
                                                                   TWEI_BLACKSPECKLESREMOVED,
                                                                   TWEI_WHITESPECKLESREMOVED,
                                                                   TWEI_HORZLINECOUNT,
                                                                   TWEI_HORZLINEXCOORD,
                                                                   TWEI_HORZLINEYCOORD,
                                                                   TWEI_HORZLINELENGTH,
                                                                   TWEI_HORZLINETHICKNESS,
                                                                   TWEI_VERTLINECOUNT,
                                                                   TWEI_VERTLINEXCOORD,
                                                                   TWEI_VERTLINEYCOORD,
                                                                   TWEI_VERTLINELENGTH,
                                                                   TWEI_VERTLINETHICKNESS,
                                                                   TWEI_FORMTEMPLATEMATCH,
                                                                   TWEI_FORMCONFIDENCE, 
                                                                   TWEI_FORMTEMPLATEPAGEMATCH, 
                                                                   TWEI_FORMHORZDOCOFFSET, 
                                                                   TWEI_FORMVERTDOCOFFSET,
                                                                   TWEI_ICCPROFILE, 
                                                                   TWEI_LASTSEGMENT,
                                                                   TWEI_SEGMENTNUMBER,
                                                                   TWEI_ENDORSEDTEXT,
                                                                   TWEI_MAGTYPE,
                                                                   TWEI_MAGDATALENGTH,
                                                                   TWEI_FILESYSTEMSOURCE,
                                                                   TWEI_PAGESIDE, 
                                                                   TWEI_IMAGEMERGED,
                                                                   TWEI_PAPERCOUNT,
                                                                   TWEI_PRINTERTEXT,
                                                                   TWEI_TWAINDIRECTMETADATA,
                                                                   TWEI_IAFIELDA_VALUE, 
                                                                   TWEI_IAFIELDB_VALUE, 
                                                                   TWEI_IAFIELDC_VALUE, 
                                                                   TWEI_IAFIELDD_VALUE, 
                                                                   TWEI_IAFIELDE_VALUE,
                                                                   TWEI_IALEVEL, 
                                                                   TWEI_PRINTER,
                                                                   TWEI_BARCODETEXT2,
                                                                   TWEI_PATCHCODE };

/* Return all the supported ExtImageInfo types.  This function is useful if your app
   wants to know what types of Extended Image Information is supported by the Source.
   This function does not need DTWAIN_InitExtImageInfo to execute correctly.  */
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumExtImageInfoTypes(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY Array)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Array))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    CTL_ITwainSource* pS = pSource;

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !pS->IsExtendedImageInfoSupported(); },
                                        DTWAIN_ERR_CAP_NO_SUPPORT, false, FUNC_MACRO);

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return pS->GetState() != SOURCE_STATE_TRANSFERRING; }, 
                                        DTWAIN_ERR_INVALID_STATE, false, FUNC_MACRO);
    // We clear the user array here, since we do not want to 
    // report information back to user if capability is not supported
    bool bArrayExists = pHandle->m_ArrayFactory->is_valid(*Array);
    if (bArrayExists)
        pHandle->m_ArrayFactory->clear(*Array);

    CTL_IntArray vExtImageInfo;
    if (pSource->EnumExtImageInfo(vExtImageInfo))
    {
        const size_t nCount = vExtImageInfo.size();
        DTWAIN_ARRAY ThisArray = CreateArrayFromContainer<std::vector<int>>(pHandle, vExtImageInfo);
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(ThisArray);

        // Dump contents of the enumerated values to the log
        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
        {
            DTWAIN_ARRAY aStrings = DTWAIN_ArrayCreate(DTWAIN_ARRAYANSISTRING, 0);
            DTWAINArrayLowLevel_RAII raii(pHandle, aStrings);
            auto& aValues = pHandle->m_ArrayFactory->underlying_container_t<std::string>(aStrings);
            for (auto val : vValues)
                aValues.push_back(CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWEI, val).second);
            LogWriterUtils::WriteLogInfoIndentedA("Supported Extended Image Info types:");
            DumpArrayContents(aStrings, 0);
        }
        dynarithmic::AssignArray(pHandle, Array, &ThisArray);
        return TRUE;
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}

/* Initialize the extimageinfo interface.  This will retrieve all the extended image info 
   in STATE 7 of the source */
static bool RetrieveExtImageInfo(CTL_TwainDLLHandle* pHandle, CTL_ITwainSource* pTheSource)
{
    auto* pExtendedImageInfo = pTheSource->GetExtendedImageInfo();
    bool bOk = true;
    pExtendedImageInfo->SetInfoRetrieved(false);
    pTheSource->InitExtImageInfo(0);
    bOk = pExtendedImageInfo->BeginRetrieval();
    return bOk;
}


/* Initialize the extimageinfo interface.  This must be called first! */
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_InitExtImageInfo(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    auto pTheSource = pSource;

    // This will only work in STATE 7 or the source!
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return pTheSource->GetState() != SOURCE_STATE_TRANSFERRING; }, 
                                        DTWAIN_ERR_INVALID_STATE, false, FUNC_MACRO);

    // Retrieve all of the extended image information now.
    auto val = RetrieveExtImageInfo(pHandle, pTheSource);
    LOG_FUNC_EXIT_NONAME_PARAMS(val)
    CATCH_BLOCK_LOG_PARAMS(false)
}

/* !!DEPRECATED as of version 5.6.2 !!
   Application adds an item to query the image information.  Before getting the Extended
   Image Information, the application will call DTWAIN_AddExtImageInfoQuery multiple times,
   each time for each Image Information desired  */
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddExtImageInfoQuery(DTWAIN_SOURCE Source, LONG ExtImageInfo)
{
    LOG_FUNC_ENTRY_PARAMS((Source, ExtImageInfo))
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

/* !!DEPRECATED as of version 5.6.2 !!
   This function actually initiates the querying of the ext image information.  This function
   will query the TWAIN Source.  If your TWAIN Source has bugs, this will be where any problem
   will exist */
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetExtImageInfo(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

/* This returns the information pertaining to a certain item in the list.  The application
   will call this for each information retrieved from the Source.  This function does not
   return the actual data, only the information as to the number of items, data type, etc.
   that the Source reports for the data item.  Use DTWAIN_GetExtImageInfoData to get the
   data */
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetExtImageInfoItem(DTWAIN_SOURCE Source,
                                                    LONG nWhich,
                                                    LPLONG InfoID,
                                                    LPLONG NumItems,
                                                    LPLONG Type)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nWhich, InfoID, NumItems, Type))

    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    CTL_ITwainSource* pTheSource = pSource;
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !pTheSource->IsExtendedImageInfoSupported(); },
                                        DTWAIN_ERR_CAP_NO_SUPPORT, false, FUNC_MACRO);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return pTheSource->GetState() != SOURCE_STATE_TRANSFERRING; }, 
                                        DTWAIN_ERR_INVALID_STATE, false, FUNC_MACRO);
    const TW_INFO Info = pTheSource->GetExtImageInfoItem(nWhich, DTWAIN_BYID);
    if ( InfoID )
        *InfoID = Info.InfoID;
    if ( NumItems )
        *NumItems = Info.NumItems;
    if ( Type )
        *Type = Info.ItemType;

    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((InfoID, NumItems, Type))
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

/* This returns the information pertaining to a certain item in the list.  The application
   will call this for each information retrieved from the Source.  This function does not
   return the actual data, only the information as to the number of items, data type, etc.
   that the Source reports for the data item.  Use DTWAIN_GetExtImageInfoData to get the
   data */
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetExtImageInfoItemEx(DTWAIN_SOURCE Source, LONG nWhich,LPLONG InfoID,
                                                      LPLONG NumItems,LPLONG Type, LPLONG ReturnCode)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nWhich, InfoID, NumItems, Type, ReturnCode))
    auto bRet = DTWAIN_GetExtImageInfoItem(Source, nWhich, InfoID, NumItems, Type);
    if (bRet)
    {
        auto pTheSource = static_cast<CTL_ITwainSource*>(Source);
        const TW_INFO Info = pTheSource->GetExtImageInfoItem(nWhich, DTWAIN_BYID);
        if (ReturnCode)
            *ReturnCode = Info.ReturnCode;
    }
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((InfoID, NumItems, Type, ReturnCode))
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

/* Returns the data for a specified Extended Image Info type.  Note that this can only be used in State 7 of the source,
   and after DTWAIN_InitExtImageInfo() has been called*/
static std::pair<bool, int> GetCachedExtImageInfoData(CTL_TwainDLLHandle* pHandle, CTL_ITwainSource* pSource, LONG nWhich, LPDTWAIN_ARRAY Data)
{
    LONG ItemType = 0;
    LONG ReturnCode = 0;

    // If the ext image info item does not exist for the source, return an error.
    DTWAIN_GetExtImageInfoItemEx(pSource, nWhich, nullptr, nullptr, &ItemType, &ReturnCode);
    if (ReturnCode == TWRC_DATANOTAVAILABLE)
        return { false, DTWAIN_ERR_UNAVAILABLE_EXTINFO };
    else
    if (ReturnCode == TWRC_INFONOTSUPPORTED)
        return { false, DTWAIN_ERR_UNSUPPORTED_EXTINFO };

    // Get the extended image info block containing the cached data.
    auto* pExtendedImageInfo = pSource->GetExtendedImageInfo();
    DTWAIN_ARRAY theArray = nullptr;

    // Get the Extended Image Info item.
    switch (nWhich)
    {
        case TWEI_BARCODECONFIDENCE:
        case TWEI_BARCODECOUNT:
        case TWEI_BARCODEX:
        case TWEI_BARCODEY:
        case TWEI_BARCODEROTATION:
        case TWEI_BARCODETEXT:
        case TWEI_BARCODETYPE:
            theArray = pExtendedImageInfo->GetBarcodeInfo(nWhich);
        break;

        case TWEI_CAMERA:
        case TWEI_BOOKNAME:
        case TWEI_CHAPTERNUMBER:
        case TWEI_DOCUMENTNUMBER:
        case TWEI_PAGENUMBER:
        case TWEI_FRAMENUMBER:
        case TWEI_PIXELFLAVOR:
        case TWEI_FRAME:
            theArray = pExtendedImageInfo->GetPageSourceInfo(nWhich);
        break;

        case TWEI_DESKEWSTATUS:
        case TWEI_SKEWORIGINALANGLE:
        case TWEI_SKEWFINALANGLE:
        case TWEI_SKEWCONFIDENCE:
        case TWEI_SKEWWINDOWX1:
        case TWEI_SKEWWINDOWX2:
        case TWEI_SKEWWINDOWX3:
        case TWEI_SKEWWINDOWX4:
        case TWEI_SKEWWINDOWY1:
        case TWEI_SKEWWINDOWY2:
        case TWEI_SKEWWINDOWY3:
        case TWEI_SKEWWINDOWY4:
            theArray = pExtendedImageInfo->GetSkewInfo(nWhich);
        break;

        case TWEI_DESHADECOUNT:
        case TWEI_DESHADELEFT:
        case TWEI_DESHADETOP:
        case TWEI_DESHADEWIDTH:
        case TWEI_DESHADEHEIGHT:
        case TWEI_DESHADESIZE:
        case TWEI_DESHADEBLACKCOUNTOLD:
        case TWEI_DESHADEBLACKCOUNTNEW:
        case TWEI_DESHADEBLACKRLMIN:
        case TWEI_DESHADEBLACKRLMAX:
        case TWEI_DESHADEWHITECOUNTOLD:
        case TWEI_DESHADEWHITECOUNTNEW:
        case TWEI_DESHADEWHITERLMIN:
        case TWEI_DESHADEWHITERLMAX:
        case TWEI_DESHADEWHITERLAVE:
            theArray = pExtendedImageInfo->GetShaderAreaInfo(nWhich);
        break;

        case TWEI_SPECKLESREMOVED:
        case TWEI_BLACKSPECKLESREMOVED:
        case TWEI_WHITESPECKLESREMOVED:
            theArray = pExtendedImageInfo->GetSpeckleRemovalInfo(nWhich);
        break;

        case TWEI_HORZLINECOUNT:
        case TWEI_HORZLINEXCOORD:
        case TWEI_HORZLINEYCOORD:
        case TWEI_HORZLINELENGTH:
        case TWEI_HORZLINETHICKNESS:
        case TWEI_VERTLINEXCOORD:
        case TWEI_VERTLINEYCOORD:
        case TWEI_VERTLINELENGTH:
        case TWEI_VERTLINETHICKNESS:
            theArray = pExtendedImageInfo->GetHorizontalVerticalLineInfo(nWhich);
        break;

        case TWEI_FORMTEMPLATEMATCH:
        case TWEI_FORMTEMPLATEPAGEMATCH:
        case TWEI_FORMHORZDOCOFFSET:
        case TWEI_FORMVERTDOCOFFSET:
            theArray = pExtendedImageInfo->GetFillFormsRecognitionInfo(nWhich);
        break;

        case TWEI_ICCPROFILE:
        case TWEI_LASTSEGMENT:
        case TWEI_SEGMENTNUMBER:
            theArray = pExtendedImageInfo->GetImageSegmentationInfo(nWhich);
        break;

        case TWEI_ENDORSEDTEXT:
            theArray = pExtendedImageInfo->GetEndorsedTextInfo(nWhich);
        break;

        case TWEI_MAGTYPE:
            theArray = pExtendedImageInfo->GetExtendedImageInfo20(nWhich);
        break;

        case TWEI_MAGDATALENGTH:
        case TWEI_MAGDATA:
        case TWEI_FILESYSTEMSOURCE:
        case TWEI_PAGESIDE:
        case TWEI_IMAGEMERGED:
            theArray = pExtendedImageInfo->GetExtendedImageInfo21(nWhich);
        break;

        case TWEI_PAPERCOUNT:
            theArray = pExtendedImageInfo->GetExtendedImageInfo22(nWhich);
        break;

        case TWEI_PRINTERTEXT:
            theArray = pExtendedImageInfo->GetExtendedImageInfo23(nWhich);
        break;

        case TWEI_TWAINDIRECTMETADATA:
            theArray = pExtendedImageInfo->GetExtendedImageInfo24(nWhich);
        break;

        case TWEI_IAFIELDA_VALUE:
        case TWEI_IAFIELDB_VALUE:
        case TWEI_IAFIELDC_VALUE:
        case TWEI_IAFIELDD_VALUE:
        case TWEI_IAFIELDE_VALUE:
        case TWEI_IALEVEL:
        case TWEI_PRINTER:
        case TWEI_BARCODETEXT2:
            theArray = pExtendedImageInfo->GetExtendedImageInfo25(nWhich);
        break;

        case TWEI_PATCHCODE:
            theArray = pExtendedImageInfo->GetPatchCodeInfo(nWhich);
        break;
    }

    if (theArray)
    {
        *Data = theArray;
        return { true, DTWAIN_NO_ERROR };
    }
    return { false, DTWAIN_ERR_MEM };
}

/* This returns the data that the Source returned when the item is queried.  Application
   must make sure that the LPVOID passed in fits the data that is returned from the Source.
   Use DTWAIN_GetExtImageInfoItem to determine the type of data.   */
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetExtImageInfoData(DTWAIN_SOURCE Source, LONG nWhich, LPDTWAIN_ARRAY Data)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nWhich, Data))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    CTL_ITwainSource* pTheSource = pSource;
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !pTheSource->IsExtendedImageInfoSupported(); },
                                        DTWAIN_ERR_CAP_NO_SUPPORT, false, FUNC_MACRO);

    // Must be in State 7 for this function to be called
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return pTheSource->GetState() != SOURCE_STATE_TRANSFERRING; },
                                        DTWAIN_ERR_INVALID_STATE, false, FUNC_MACRO);

    // Check if array exists
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{ return !Data;}, DTWAIN_ERR_BAD_ARRAY, false, FUNC_MACRO);

    // We clear the user array here, since we do not want to 
    // report information back to user if capability is not supported
    bool bArrayExists = pHandle->m_ArrayFactory->is_valid(*Data);
    if (bArrayExists)
        pHandle->m_ArrayFactory->clear(*Data);

    auto* pExtendedImageInfo = pTheSource->GetExtendedImageInfo();

    // The if() is only true on the initial filling of the extended image inforamtion
    if ( pExtendedImageInfo->IsInfoBeingFilled() )
    {
        // Get the info 
        TW_INFO Info = pTheSource->GetExtImageInfoItem(nWhich, DTWAIN_BYID);

        // Check if the info type is supported
        bool bNotSupported = false;
        int nErrorCode = 0;
        if (Info.ReturnCode == TWRC_DATANOTAVAILABLE)
        {
            nErrorCode = DTWAIN_ERR_UNAVAILABLE_EXTINFO;
            bNotSupported = true;
        }
        else
        if (Info.ReturnCode == TWRC_INFONOTSUPPORTED)
        {
            nErrorCode = DTWAIN_ERR_UNSUPPORTED_EXTINFO;
            bNotSupported = true;
        }
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return bNotSupported; }, nErrorCode, false, FUNC_MACRO);

        // Check if type returned by device is what the TWAIN specification indicates
        auto lTypeReportedByDevice = Info.ItemType;
        auto lTypeRequiredByTWAIN = CTL_TwainAppMgr::GetGeneralCapInfo(nWhich + CTL_StaticData::GetExtImageInfoOffset()).m_nDataType;
        bool bTypesMatch = true;
        if (lTypeReportedByDevice != static_cast<LONG>(lTypeRequiredByTWAIN))
        {
            bTypesMatch = false;
            // Log this condition.  We *may* still get the data, even though TWAIN spec was violated.
            if (CTL_StaticData::GetLogFilterFlags())
            {
                StringTraitsA::string_type sBadType = GetResourceStringFromMap(IDS_DTWAIN_ERROR_REPORTED_TYPE_MISMATCH);
                sBadType += "  Extended Image Info Value: " + CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWEI, nWhich).second;
                sBadType += " - {Device Type=" + CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWTY, lTypeReportedByDevice).second;
                sBadType += ", Twain Required Type=" + CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWTY, lTypeRequiredByTWAIN).second + "}";
                LogWriterUtils::WriteLogInfoIndentedA(sBadType);
            }

            // We return an error if the TWAIN spec has been violated for any required types that have to match
            auto iter = std::find(aRequiredTypeMatches.begin(), aRequiredTypeMatches.end(), nWhich);
            if (iter != aRequiredTypeMatches.end())
                DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return true; }, DTWAIN_ERR_EXTIMAGEINFO_DATATYPE_MISMATCH, false, FUNC_MACRO);

            // If the mismatch type is TW_FRAME, we should fake it out and pretend that the types match
            if (Info.InfoID == TWEI_FRAME)
                Info.ItemType = TWTY_FRAME;
        }

        DTWAIN_ARRAY ExtInfoArray = CreateArrayFromFactory(pHandle, ExtImageInfoArrayType(Info.ItemType), 0);

        if (!ExtInfoArray)
        {
            // Check if array exists
            DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !ExtInfoArray; }, DTWAIN_ERR_BAD_ARRAY, false, FUNC_MACRO);
        }

        auto Count = Info.NumItems;

        const auto& factory = pHandle->m_ArrayFactory;
        auto eType = factory->tag_type(ExtInfoArray);

        // resize the array if this is not a frame
        if ( eType != CTL_ArrayFactory::arrayTag::FrameSingleType)
            factory->resize(ExtInfoArray, Count);

        std::pair<bool, int32_t> finalRet = { true, DTWAIN_NO_ERROR };
        for ( int i = 0; i < Count; ++i )
        {
            if (eType == CTL_ArrayFactory::arrayTag::StringType)
            {
                std::vector<char> Temp;
                size_t ItemSize;
                finalRet = pTheSource->GetExtImageInfoData(nWhich, DTWAIN_BYID, i, nullptr, nullptr, &ItemSize);
                if ( finalRet.first )
                {
                    Temp.resize(ItemSize);
                    finalRet = pTheSource->GetExtImageInfoData(nWhich, DTWAIN_BYID, i, Temp.data(), nullptr, nullptr);
                    if ( finalRet.first)
                        SetArrayValueFromFactory(pHandle, ExtInfoArray, i, Temp.data());
                }
            }
            else
            if (eType == CTL_ArrayFactory::arrayTag::VoidPtrType) // This is a handle
            {
                TW_HANDLE pDataHandle = nullptr;
                finalRet = pTheSource->GetExtImageInfoData(nWhich, DTWAIN_BYID, i, nullptr, &pDataHandle, nullptr);
                if (finalRet.first)
                {
                    auto& vValues = factory->underlying_container_t<void*>(ExtInfoArray);
                    vValues[i] = pDataHandle;
                }
            }
            else
            if (eType == CTL_ArrayFactory::arrayTag::FrameSingleType) // This is a frame
            {
                TW_FRAME oneFrame = {};
                if (bTypesMatch) // Only do this if the types match up.
                    finalRet = pTheSource->GetExtImageInfoData(nWhich, DTWAIN_BYID, i, &oneFrame, nullptr);
                dynarithmic::TWFRAMEToDTWAINFRAME(oneFrame, ExtInfoArray);
            }
            else
            {
                finalRet = pTheSource->GetExtImageInfoData(nWhich, DTWAIN_BYID, i, factory->get_buffer(ExtInfoArray, i), nullptr);
            }
        }
        dynarithmic::AssignArray(pHandle, Data, &ExtInfoArray);
        if (!finalRet.first)
        {
            // Error occurred
            CTL_TwainAppMgr::SetError(finalRet.second, "", false);
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
        }
    }
    else
    {
        // Info already filled, so get the cached information
        auto pr = GetCachedExtImageInfoData(pHandle, pSource, nWhich, Data);
        // Check if succeeded
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !pr.first; }, pr.second, false, FUNC_MACRO);
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

/* Uninitializes the Extended Image information interface.  This also must be called.  */
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FreeExtImageInfo(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    CTL_ITwainSource* pTheSource = pSource;
    
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !pTheSource->IsExtendedImageInfoSupported(); },
        DTWAIN_ERR_CAP_NO_SUPPORT, false, FUNC_MACRO);

    pTheSource->DestroyExtImageInfo();
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}
