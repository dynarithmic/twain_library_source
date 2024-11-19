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
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "transym_ocrinterface.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#include "sourceselectopts.h"

#ifdef _MSC_VER
    #pragma warning (disable:4505)
#endif

static std::vector<TCHAR> GetDefaultOCRName(SelectStruct& selectTraits);
static std::vector<CTL_StringType> GetNameList(SelectStruct& selectTraits);

static CTL_StringType DTWAIN_LLSelectOCR(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions& opts)
{
    opts.getDefaultFunc = &GetDefaultOCRName;
    opts.getNameListFunc = &GetNameList;
    CTL_StringType actualSourceName = LLSelectionDialog(pHandle, opts);
    return actualSourceName;
}

static DTWAIN_OCRENGINE SelectOCREngine(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions& selectOptions)
{
    auto OCRName = DTWAIN_LLSelectOCR(pHandle, selectOptions);
    if (!OCRName.empty())
        return DTWAIN_SelectOCREngineByName(OCRName.c_str());
    return nullptr;
}

DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectDefaultOCREngine()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto [pHandle, pDummy] = VerifyOCRHandles();
    auto pH = pHandle;

    // Get the OCR engine associated with the name
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return pH->m_OCRInterfaceArray.empty(); },
        DTWAIN_ERR_OCR_NOTACTIVE, 0, FUNC_MACRO);
    const auto SelectedEngine = static_cast<DTWAIN_OCRENGINE>(pHandle->m_pOCRDefaultEngine.get());
    const auto pEngine = static_cast<OCREngine*>(SelectedEngine);
    if (!pEngine->IsActivated())
        pEngine->StartupOCREngine();

    LOG_FUNC_EXIT_NONAME_PARAMS(SelectedEngine)
    CATCH_BLOCK(DTWAIN_OCRENGINE())
}

DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngineByName(LPCTSTR lpszName)
{
    LOG_FUNC_ENTRY_PARAMS((lpszName))
    auto [pHandle, pEngine] = VerifyOCRHandles();

    const std::string sName = StringConversion::Convert_NativePtr_To_Ansi(lpszName);

    // Get the OCR engine associated with the name
    const auto it = pHandle->m_OCRProdNameToEngine.find(sName);
    OCREnginePtr SelectedEngine;
    DTWAIN_OCRENGINE ocrEngine_ = nullptr;
    if (it != pHandle->m_OCRProdNameToEngine.end())
    {
        SelectedEngine = it->second;
        if (SelectedEngine)
        {
            pHandle->m_pOCRDefaultEngine = SelectedEngine;

            if (!SelectedEngine->IsActivated())
                SelectedEngine->StartupOCREngine();
            ocrEngine_ = SelectedEngine.get();
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(ocrEngine_)
    CATCH_BLOCK(DTWAIN_OCRENGINE())
}

DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngine()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto [pHandle, pEngine] = VerifyOCRHandles();
    SourceSelectionOptions opts(SELECTSOURCE, IDS_SELECT_OCRENGINE_TEXT,
                                nullptr,
                                nullptr,
                                nullptr,
                                0, 0, nullptr, nullptr, nullptr, DTWAIN_DLG_CENTER_SCREEN);
    auto retVal = SelectOCREngine(pHandle, opts);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(DTWAIN_OCRENGINE())
}

DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngine2(HWND hWndParent, 
                                                      LPCTSTR szTitle,
                                                      LONG xPos,
                                                      LONG yPos,
                                                      LONG nOptions)
{
    LOG_FUNC_ENTRY_PARAMS((hWndParent, szTitle, xPos, yPos, nOptions))
    auto [pHandle, pEngine] = VerifyOCRHandles();
    SourceSelectionOptions opts(SELECTSOURCE, IDS_SELECT_OCRENGINE_TEXT,
                                nullptr,
                                hWndParent,
                                szTitle,
                                xPos, yPos, nullptr, nullptr, nullptr, nOptions);
    auto retVal = SelectOCREngine(pHandle, opts);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(DTWAIN_OCRENGINE())
}

DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngine2Ex(HWND hWndParent,
                                                        LPCTSTR szTitle,
                                                        LONG xPos,
                                                        LONG yPos,
                                                        LPCTSTR szIncludeFilter,
                                                        LPCTSTR szExcludeFilter,
                                                        LPCTSTR szNameMapping,
                                                        LONG nOptions)
{
    LOG_FUNC_ENTRY_PARAMS((hWndParent, szTitle, xPos, yPos, nOptions))
    auto [pHandle, pEngine] = VerifyOCRHandles();
    SourceSelectionOptions opts(SELECTSOURCE, IDS_SELECT_OCRENGINE_TEXT,
                                nullptr,
                                hWndParent,
                                szTitle,
                                xPos, yPos, szIncludeFilter, szExcludeFilter, szNameMapping, nOptions);

    auto retVal = SelectOCREngine(pHandle, opts);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(DTWAIN_OCRENGINE())
}

static std::vector<TCHAR> GetDefaultOCRName(SelectStruct& selectTraits)
{
    std::vector<TCHAR> DefName;
    TCHAR ProdName[256];
    const auto DefaultEngine = selectTraits.pHandle->m_pOCRDefaultEngine.get();
    GetOCRInfo(DefaultEngine, &OCREngine::GetProductName, ProdName, 255);
    CTL_StringType sourceName = ProdName;
    DefName = std::vector<TCHAR>(sourceName.begin(), sourceName.end());
    DefName.push_back(0);
    return DefName;
}

static std::vector<CTL_StringType> GetNameList(SelectStruct& selectTraits)
{
    // Fill the list box with the sources
    std::vector<CTL_StringType> vSourceNames;
    DTWAIN_ARRAY Array = nullptr;
    const auto pHandle = selectTraits.pHandle;
    DTWAIN_EnumOCRInterfaces(&Array);
    DTWAINArrayLowLevel_RAII arr(pHandle, Array);
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<OCREngine*>(Array);
    if (!vValues.empty())
    {
        TCHAR ProdName[256];

        std::transform(vValues.begin(), vValues.end(), std::back_inserter(vSourceNames),
            [&](OCREngine* ptr)
            {
                GetOCRInfo(ptr, &OCREngine::GetProductName, ProdName, 255);
                return ProdName;
            });
    }
    return vSourceNames;
}
