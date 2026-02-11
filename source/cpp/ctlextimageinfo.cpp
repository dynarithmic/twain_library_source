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
#include "errorcheck.h"
#include "ctltr038.h"
#include "twain.h"
#include "extendedimageinfo.h"
using namespace dynarithmic;
/* These functions can only be used in State 7   (when DTWAIN_TN_TRANSFERDONE notification is sent).
   This means that only languages that can utilize DTWAIN_SetCallback or can intercept Window's
   messages can use these functions. */


DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumSupportedExtImageInfoEx(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    DTWAIN_ARRAY arr = {};
    DTWAIN_EnumSupportedExtImageInfo(Source, &arr);
    LOG_FUNC_EXIT_NONAME_PARAMS(arr)
    CATCH_BLOCK(nullptr) 
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumSupportedExtImageInfo(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY Array)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Array))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
	DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !Array; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    CTL_ITwainSource* pS = pSource;
    CTL_TwainDLLHandle* pH = pHandle;
    // We clear the user array here, since we do not want to 
    // report information back to user if capability is not supported
    bool bArrayExists = pHandle->m_ArrayFactory->is_valid(*Array);
    if (bArrayExists)
        pHandle->m_ArrayFactory->clear(*Array);

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !pS->IsExtendedImageInfoSupported(); },
                                        DTWAIN_ERR_CAP_NO_SUPPORT, false, FUNC_MACRO);
    if (!pS->IsSupportedExtImageInfoCap())
    {
        // This is done for TWAIN 1.x sources or 2.x sources that do not have the ICAP_SUPPORTEDEXTIMAGEINFO
        // capability (but do have the ICAP_EXTIMAGEINFO available).
        // Note that for sources that do not have ICAP_SUPPORTEDEXTIMAGEINFO, the retrieval of
        // the Extended Image Info types must be done in State 7, thus the available ext image info
        // items can change, depending on various criteria while the source is transferring images.

        // Enumerate the types now
        DTWAIN_EnumExtImageInfoTypes(Source, Array);

        // If there is an issue with reporting the types, must return error
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return pH->m_lLastError != DTWAIN_NO_ERROR; },
                                          pH->m_lLastError, false, FUNC_MACRO);

        // Set the types supported into the Source object.
        pS->SetSupportedExtImageInfos(CreateContainerFromArray<std::vector<LONG>>(pHandle, Array));
    }
    else
    {
        // The source supports ICAP_SUPPORTEDEXTIMAGEINFO, so we are ok in just returning the
        // cached list of supported TWEI_x values.
        auto& vVect = pS->GetSupportedExtImageInfos();
        DTWAIN_ARRAY ThisArray = CreateArrayFromContainer<std::vector<LONG>>(pHandle, vVect);
        dynarithmic::MoveArray(pHandle, Array, &ThisArray);
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

 //  Return all the supported ExtImageInfo types.  This function is useful if your app
 //  wants to know what types of Extended Image Information is supported by the Source.
 //  This function does not need DTWAIN_InitExtImageInfo to execute correctly.  
 //  
 //  Note that this function checks if the source is in state 7 (transferring).
 //  If you want to know the ext image types outside of state 7, you must call
 //  DTWAIN_EnumSupportedExtImageInfo() on a source that supports the ICAP_SUPPORTEDEXTIMAGEINFO
 //  capability.

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumExtImageInfoTypesEx(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    DTWAIN_ARRAY arr = {};
    DTWAIN_EnumExtImageInfoTypes(Source, &arr);
    LOG_FUNC_EXIT_NONAME_PARAMS(arr)
    CATCH_BLOCK(nullptr)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumExtImageInfoTypes(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY Array)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Array))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
	DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !Array; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

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
        DTWAIN_ARRAY ThisArray = CreateArrayFromContainer<std::vector<int>>(pHandle, vExtImageInfo);
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !ThisArray; }, DTWAIN_ERR_OUT_OF_MEMORY, false, FUNC_MACRO);

        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(ThisArray);

        // Dump contents of the enumerated values to the log
        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
        {
            auto retVal = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYANSISTRING, 0);
            if (retVal.second)
            {
                auto aStrings = retVal.second;
                DTWAINArrayLowLevel_RAII raii(pHandle, aStrings);
                auto& aValues = pHandle->m_ArrayFactory->underlying_container_t<std::string>(aStrings);
                for (auto val : vValues)
                    aValues.push_back(CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWEI, val).second);
                LogWriterUtils::WriteLogInfoIndentedA("Supported Extended Image Info types:");
                DumpArrayContents(aStrings, 0);
            }
        }
        MoveArray(pHandle, Array, &ThisArray);
		LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }
    else
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return true; }, DTWAIN_ERR_EXTIMAGEINFO_RETRIEVAL, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}

/* Initialize the extimageinfo interface.  This will retrieve all the extended image info 
   in STATE 7 of the source */
static bool RetrieveExtImageInfo(CTL_TwainDLLHandle* pHandle, CTL_ITwainSource* pTheSource)
{
    struct RetrieveRAII
    {
        CTL_ITwainSource* m_p;
        RetrieveRAII(CTL_ITwainSource* p) : m_p(p) {}
        ~RetrieveRAII() 
        { 
            // It is safe to delete the original Extended Image Info retrieved from the 
            // TWAIN triplet, since we have cached all the information into our local containers
            auto* pTrip = m_p->GetExtImageInfoTriplet();
            pTrip->DestroyInfo();
        }
    };

    // Ensure we clean up the memory allocated for the Extended Image Info
    RetrieveRAII raii(pTheSource);

    auto* pExtendedImageInfo = pTheSource->GetExtendedImageInfo();
    pExtendedImageInfo->SetInfoRetrieved(false);
    pTheSource->InitExtImageInfo(0);
    return pExtendedImageInfo->BeginRetrieval();
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


/* This returns the data that the Source returned when the item is queried.
   Use DTWAIN_GetExtImageInfoItem to determine the type of data.   */
DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_GetExtImageInfoDataEx(DTWAIN_SOURCE Source, LONG nWhich)
{
	LOG_FUNC_ENTRY_PARAMS((Source, nWhich))
    DTWAIN_ARRAY Data = {};
    DTWAIN_GetExtImageInfoData(Source, nWhich, &Data);
	LOG_FUNC_EXIT_NONAME_PARAMS(Data)
	CATCH_BLOCK(nullptr)
}

/* This returns the data that the Source returned when the item is queried.
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

	DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !Data; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    auto retValue = GetExtImageInfoDataInternal(Source, nWhich, Data);
    LOG_FUNC_EXIT_NONAME_PARAMS(retValue.first)
    CATCH_BLOCK(false)
}


// Uninitializes the Extended Image information interface.  This also must be called. 
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
