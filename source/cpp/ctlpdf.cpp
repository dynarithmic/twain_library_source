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
#include <algorithm>
#include "cppfunc.h"
#include "dtwain.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "dtwain_resource_constants.h"
#include "errorcheck.h"
#include "dtwstrfn.h"

using namespace dynarithmic;
static std::pair<bool, CTL_TEXTELEMENTPTRLIST::iterator>
    CheckPDFTextElement(DTWAIN_PDFTEXTELEMENT TextElement, LONG& ConditionCode);

/////////////////////////////  PDF Settings ///////////////////////////////
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFAuthor(DTWAIN_SOURCE Source, LPCTSTR lpAuthor)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpAuthor))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFAUTHORKEY, lpAuthor);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFCreator(DTWAIN_SOURCE Source, LPCTSTR lpCreator)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpCreator))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFCREATORKEY, lpCreator);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFProducer(DTWAIN_SOURCE Source, LPCTSTR lpProducer)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpProducer))
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFEncryption(DTWAIN_SOURCE Source, DTWAIN_BOOL bUseEncryption,
                                                 LPCTSTR lpszUser, LPCTSTR lpszOwner,
                                                 LONG Permissions, DTWAIN_BOOL UseStrongEncryption)
{
    LOG_FUNC_ENTRY_PARAMS((Source, bUseEncryption, lpszUser, lpszOwner, Permissions, UseStrongEncryption))
    auto [pHandle, pSource] = VerifyHandles(Source);
    const CTL_StringType owner = lpszOwner?lpszOwner:_T("");
    const CTL_StringType user = lpszUser?lpszUser:_T("");

    pSource->SetPDFEncryption(bUseEncryption?true:false, user, owner, Permissions, UseStrongEncryption?true:false);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTitle(DTWAIN_SOURCE Source, LPCTSTR lpTitle)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpTitle))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFTITLEKEY, lpTitle);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)

}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFSubject(DTWAIN_SOURCE Source, LPCTSTR lpSubject)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpSubject))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFSUBJECTKEY, lpSubject);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFKeywords(DTWAIN_SOURCE Source, LPCTSTR lpKeyWords)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpKeyWords))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFKEYWORDSKEY, lpKeyWords);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)

}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFOrientation(DTWAIN_SOURCE Source, LONG Orientation)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Orientation))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFORIENTATIONKEY, Orientation);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFJpegQuality(DTWAIN_SOURCE Source, LONG Quality)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Quality))
    auto [pHandle, pSource] = VerifyHandles(Source);
    Quality = (std::max)(static_cast<LONG>(1), (std::min)(static_cast<LONG>(100), Quality));
    pSource->SetPDFValue(PDFJPEGQUALITYKEY, Quality);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

typedef DTWAIN_BOOL (DLLENTRY_DEF *SetPDFFn)(DTWAIN_SOURCE, LONG, DTWAIN_FLOAT, DTWAIN_FLOAT);

static DTWAIN_BOOL SetPDFStringFunc(DTWAIN_SOURCE Source, LONG value, LPCTSTR val1, LPCTSTR val2, SetPDFFn fn)
{
    const DTWAIN_FLOAT value1 = StringWrapper::ToDouble(val1);
    const DTWAIN_FLOAT value2 = StringWrapper::ToDouble(val2);
    return fn(Source, value, value1, value2);
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageSizeString(DTWAIN_SOURCE Source, LONG PageSize,
                                                     LPCTSTR CustomWidth, LPCTSTR CustomHeight)
{
    LOG_FUNC_ENTRY_PARAMS((Source, PageSize, CustomWidth, CustomHeight))
    const DTWAIN_BOOL bRet = SetPDFStringFunc(Source, PageSize, CustomWidth, CustomHeight, &DTWAIN_SetPDFPageSize);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageScaleString(DTWAIN_SOURCE Source, LONG nOptions,
                                                      LPCTSTR xScale, LPCTSTR yScale)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nOptions,xScale, yScale))
    const DTWAIN_BOOL bRet = SetPDFStringFunc(Source, nOptions, xScale, yScale, &DTWAIN_SetPDFPageScale);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageSize(DTWAIN_SOURCE Source, LONG PageSize,
                                               DTWAIN_FLOAT CustomWidth,
                                               DTWAIN_FLOAT CustomHeight)
{
    LOG_FUNC_ENTRY_PARAMS((Source, PageSize,CustomWidth, CustomHeight))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFPageSize(PageSize, CustomWidth, CustomHeight);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
 }


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageScale(DTWAIN_SOURCE Source, LONG nOptions, DTWAIN_FLOAT xScale,
                                                DTWAIN_FLOAT yScale)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nOptions,xScale, yScale))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFSCALINGKEY, nOptions);
    if ( nOptions == DTWAIN_PDF_CUSTOMSCALE )
        pSource->SetPDFValue(PDFSCALINGKEY, xScale/100.0, yScale/100.0);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFCompression(DTWAIN_SOURCE Source, DTWAIN_BOOL bCompression)
{
    LOG_FUNC_ENTRY_PARAMS((Source, bCompression))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFCOMPRESSIONKEY, static_cast<LONG>(bCompression));
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFAESEncryption(DTWAIN_SOURCE Source, DTWAIN_BOOL bUseAES)
{
    LOG_FUNC_ENTRY_PARAMS((Source, bUseAES))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFAESKEY, static_cast<LONG>(bUseAES));
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFASCIICompression(DTWAIN_SOURCE Source, DTWAIN_BOOL bCompression)
{
    LOG_FUNC_ENTRY_PARAMS((Source, bCompression))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFASCIICOMPRESSKEY, static_cast<LONG>(bCompression));
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPostScriptTitle(DTWAIN_SOURCE Source, LPCTSTR szTitle)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szTitle))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PSTITLEKEY, szTitle);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPostScriptType(DTWAIN_SOURCE Source, LONG PSType)
{
    LOG_FUNC_ENTRY_PARAMS((Source, PSType))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PSTYPEKEY, PSType);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFOCRMode(DTWAIN_SOURCE Source, DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((Source, bSet))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFOCRMODE, bSet);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

LONG DLLENTRY_DEF DTWAIN_GetPDFType1FontName(LONG FontVal, LPTSTR szFont, LONG nChars)
{
    LOG_FUNC_ENTRY_PARAMS((FontVal, szFont, nChars))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    auto st = CTL_StaticData::GetTwainNameFromConstant(DTWAIN_CONSTANT_FONTNAME, FontVal + DTWAIN_FONT_START_);
    auto numChars = StringWrapper::CopyInfoToCString(st, szFont, nChars);
    LOG_FUNC_EXIT_NONAME_PARAMS(numChars)
    CATCH_BLOCK(-1)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddPDFTextEx(DTWAIN_SOURCE Source, DTWAIN_PDFTEXTELEMENT TextElement, LONG Flags)
{
    LOG_FUNC_ENTRY_PARAMS((Source, TextElement))
    auto [pHandle, pSource] = VerifyHandles(Source);
    const PDFTextElement* pElement = static_cast<PDFTextElement*>(TextElement);
    const DTWAIN_BOOL retVal = DTWAIN_AddPDFText(Source,
                                                 StringConversion::Convert_Ansi_To_Native(pElement->m_text).c_str(),
                                                 static_cast<LONG>(pElement->xpos),
                                                 static_cast<LONG>(pElement->ypos),
                                                 StringConversion::Convert_Ansi_To_Native(pElement->m_font.m_fontName).c_str(),
                                                 pElement->fontSize,
                                                 static_cast<LONG>(pElement->colorRGB),
                                                 static_cast<LONG>(pElement->renderMode),
                                                 pElement->scaling,
                                                 pElement->charSpacing,
                                                 pElement->wordSpacing,
                                                 pElement->strokeWidth,
                                                 Flags);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddPDFText(DTWAIN_SOURCE Source,
                                                 LPCTSTR szText, LONG xPos, LONG yPos,
                                                 LPCTSTR fontName, DTWAIN_FLOAT fontSize, LONG colorRGB,
                                                 LONG renderMode, DTWAIN_FLOAT scaling,
                                                 DTWAIN_FLOAT charSpacing, DTWAIN_FLOAT wordSpacing,
                                                 LONG strokeWidth, LONG Flags)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szText, xPos, yPos, fontName, fontSize, colorRGB,
                                               renderMode, scaling, charSpacing, wordSpacing, strokeWidth, Flags))
    auto [pHandle, pSource] = VerifyHandles(Source);
    struct DefaultValSetterLONG
    {
        LONG DefaultSetting;
        int DefaultValue;
        LONG pSource;
        int *pDestination;
        LONG flagValue;
    };

    struct DefaultValSetterDOUBLE
    {
        double DefaultSetting;
        double DefaultValue;
        double pSource;
        double *pDestination;
        LONG flagValue;
    };

    PDFTextElement element;
    constexpr LONG riseValue = 0;
    const DefaultValSetterLONG defVals[] = {

                        {DTWAIN_DEFAULT, 0 , renderMode, &element.renderMode,DTWAIN_PDFTEXT_NORENDERMODE},
                        {riseValue, 0 , riseValue, &element.riseValue,1},
                        {DTWAIN_DEFAULT, 1 , strokeWidth, &element.strokeWidth,DTWAIN_PDFTEXT_NOSTROKEWIDTH},
                        {DTWAIN_DEFAULT, 0 , colorRGB, &element.colorRGB, DTWAIN_PDFTEXT_NORGBCOLOR},
                        {Flags, static_cast<int>(Flags), Flags, &element.displayFlags, 1}
                     };

   const DefaultValSetterDOUBLE defValsDOUBLE[] = {
                        {fontSize, 10.0 , fontSize, &element.fontSize, DTWAIN_PDFTEXT_NOFONTSIZE},
                        {DTWAIN_FLOATDEFAULT, 100.0 , scaling, &element.scaling, DTWAIN_PDFTEXT_NOSCALING},
                        {DTWAIN_FLOATDEFAULT, 0.0 , wordSpacing, &element.wordSpacing, DTWAIN_PDFTEXT_NOWORDSPACING},
                        {DTWAIN_FLOATDEFAULT, 0.0 , charSpacing, &element.charSpacing, DTWAIN_PDFTEXT_NOCHARSPACING},
                     };

     constexpr size_t numDefVals = std::size(defVals);
     constexpr size_t numDefValsDOUBLE = std::size(defValsDOUBLE);

    if (pSource)
    {
        element.m_text = StringConversion::Convert_NativePtr_To_Ansi(szText);
        element.xpos = xPos;
        element.ypos = yPos;
        std::string sFontName = "Helvetica";
        if ( !fontName )
            element.m_font.m_fontName = sFontName;
        else
            element.m_font.m_fontName = StringConversion::Convert_NativePtr_To_Ansi(fontName); 
        for (int i = 0; i < numDefVals; ++i )
        {
            if ( Flags & defVals[i].flagValue )
                *defVals[i].pDestination = defVals[i].DefaultValue;
            else
                *defVals[i].pDestination = defVals[i].pSource;
        }

        for (int i = 0; i < numDefValsDOUBLE; ++i )
        {
            if ( Flags & defValsDOUBLE[i].flagValue )
                *defValsDOUBLE[i].pDestination = defValsDOUBLE[i].DefaultValue;
            else
                *defValsDOUBLE[i].pDestination = defValsDOUBLE[i].pSource;
        }

        /* Now get the position */
        if ( Flags & DTWAIN_PDFTEXT_NOABSPOSITION )
            element.stockPosition = Flags & 0x000FFF00;

        PDFTextElementPtr pPtr = std::make_shared<PDFTextElement>();
        *pPtr = element;
        pSource->SetPDFValue(PDFTEXTELEMENTKEY, pPtr);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ClearPDFText(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->ClearPDFText();
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_PDFTEXTELEMENT DLLENTRY_DEF DTWAIN_CreatePDFTextElement(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    PDFTextElementPtr pPtr = std::make_shared<PDFTextElement>();
    const DTWAIN_PDFTEXTELEMENT pdfHandle = pPtr.get();
    PDFTextElement* rawPtr = static_cast<PDFTextElement*>(pdfHandle);
    rawPtr->pTwainSource = pSource;
    pSource->SetPDFValue(PDFTEXTELEMENTKEY, pPtr);
    LOG_FUNC_EXIT_NONAME_PARAMS(pdfHandle)
    CATCH_BLOCK_LOG_PARAMS(DTWAIN_PDFTEXTELEMENT())
}

struct FindPDFTextHandle
{
    FindPDFTextHandle(PDFTextElement* pEl) : m_Element(pEl) { }
    bool operator() (const PDFTextElementPtr& pEl) const { return pEl.get() == m_Element; }

private:
    PDFTextElement* m_Element;
};


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_DestroyPDFTextElement(DTWAIN_PDFTEXTELEMENT TextElement)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LONG ConditionCode;
    const auto itEx = CheckPDFTextElement(TextElement, ConditionCode);
    if ( !itEx.first )
    {
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return TRUE;} , ConditionCode, false, FUNC_MACRO);
    }
    const PDFTextElement* pPtr = static_cast<PDFTextElement*>(TextElement);

    pHandle->m_mapPDFTextElement[pPtr->pTwainSource].erase(itEx.second);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTextElementFloat(DTWAIN_PDFTEXTELEMENT TextElement, DTWAIN_FLOAT val1, DTWAIN_FLOAT val2, LONG Flags)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement, val1, val2, Flags))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LONG ConditionCode;
    const auto it = CheckPDFTextElement(TextElement, ConditionCode);
    if ( !it.first )
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1;}, ConditionCode, false, FUNC_MACRO);

    auto* pPtr = static_cast<PDFTextElement*>(TextElement);

    switch (Flags)
    {
        case DTWAIN_PDFTEXTELEMENT_SCALINGXY:
            pPtr->scalingX = val1;
            pPtr->scalingY = val2;
        break;

        case DTWAIN_PDFTEXTELEMENT_FONTHEIGHT:
            pPtr->fontSize = val1;
        break;

        case DTWAIN_PDFTEXTELEMENT_ROTATIONANGLE:
            pPtr->rotationAngle = val1;
        break;

        case DTWAIN_PDFTEXTELEMENT_SCALING:
            pPtr->scaling = val1;
        break;

        case DTWAIN_PDFTEXTELEMENT_CHARSPACING:
            pPtr->charSpacing = val1;
        break;

        case DTWAIN_PDFTEXTELEMENT_WORDSPACING:
            pPtr->wordSpacing = val1;
        break;

        case DTWAIN_PDFTEXTELEMENT_SKEWANGLES:
            pPtr->skewAngleX = val1;
            pPtr->skewAngleY = val2;
        break;

        default:
            LOG_FUNC_EXIT_NONAME_PARAMS(false)

    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTextElementLong(DTWAIN_PDFTEXTELEMENT TextElement, LONG val1, LONG val2, LONG Flags)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement, val1, val2, Flags))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LONG ConditionCode;
    const auto it = CheckPDFTextElement(TextElement, ConditionCode);
    if ( !it.first )
    {
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, []{ return 1; }, ConditionCode, false, FUNC_MACRO);
    }

    auto pPtr = static_cast<PDFTextElement*>(TextElement);

    switch (Flags)
    {
        case DTWAIN_PDFTEXTELEMENT_POSITION:
            pPtr->xpos = val1;
            pPtr->ypos = val2;
        break;

        case DTWAIN_PDFTEXTELEMENT_COLOR:
            pPtr->colorRGB = val1;
        break;

        case DTWAIN_PDFTEXTELEMENT_STROKEWIDTH:
            pPtr->strokeWidth = val1;
        break;

        case DTWAIN_PDFTEXTELEMENT_DISPLAYFLAGS:
            pPtr->displayFlags = val1;
        break;

        case DTWAIN_PDFTEXTELEMENT_RENDERMODE:
            pPtr->renderMode = val1;
        break;

        case DTWAIN_PDFTEXTELEMENT_TRANSFORMORDER:
            pPtr->textTransform = (std::max)(static_cast<LONG>(0), (std::min)(val1, static_cast<LONG>(DTWAIN_PDFTEXTTRANSFORM_KTRS)));
        break;

        default:
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTextElementString(DTWAIN_PDFTEXTELEMENT TextElement, LPCTSTR val1, LONG Flags)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement, val1, Flags))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LONG ConditionCode;
    const auto it = CheckPDFTextElement(TextElement, ConditionCode);
    if ( !it.first )
    {
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1;}, ConditionCode, false, FUNC_MACRO);
    }

    auto* pPtr = static_cast<PDFTextElement*>(TextElement);

    switch (Flags)
    {
        case DTWAIN_PDFTEXTELEMENT_FONTNAME:
            pPtr->m_font.m_fontName = StringConversion::Convert_NativePtr_To_Ansi(val1);
        break;

        case DTWAIN_PDFTEXTELEMENT_TEXT:
            pPtr->m_text = StringConversion::Convert_NativePtr_To_Ansi(val1);
        break;

        default:
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetPDFTextElementFloat(DTWAIN_PDFTEXTELEMENT TextElement, LPDTWAIN_FLOAT val1, LPDTWAIN_FLOAT val2, LONG Flags)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement, val1, val2, Flags))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LONG ConditionCode;

    const auto it = CheckPDFTextElement(TextElement, ConditionCode);
    if ( !it.first )
    {
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1; }, ConditionCode, false, FUNC_MACRO);
    }

    const PDFTextElement* pPtr = static_cast<PDFTextElement*>(TextElement);

    switch (Flags)
    {
        case DTWAIN_PDFTEXTELEMENT_SCALINGXY:
            *val1 = pPtr->scalingX;
            *val2 = pPtr->scalingY;
        break;

        case DTWAIN_PDFTEXTELEMENT_FONTHEIGHT:
            *val1 = pPtr->fontSize;
        break;

        case DTWAIN_PDFTEXTELEMENT_ROTATIONANGLE:
            *val1 = pPtr->rotationAngle;
        break;

        case DTWAIN_PDFTEXTELEMENT_SCALING:
            *val1 = pPtr->scaling;
        break;

        case DTWAIN_PDFTEXTELEMENT_CHARSPACING:
            *val1 = pPtr->charSpacing;
        break;

        case DTWAIN_PDFTEXTELEMENT_WORDSPACING:
            *val1 = pPtr->wordSpacing;
        break;

        case DTWAIN_PDFTEXTELEMENT_SKEWANGLES:
            *val1 = pPtr->skewAngleX;
            *val2 = pPtr->skewAngleY;
        break;

        default:
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetPDFTextElementLong(DTWAIN_PDFTEXTELEMENT TextElement, LPLONG val1, LPLONG val2, LONG Flags)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement, val1, val2, Flags))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LONG ConditionCode;
    const auto it = CheckPDFTextElement(TextElement, ConditionCode);
    if ( !it.first )
    {
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1; }, ConditionCode, false, FUNC_MACRO);
    }

    const PDFTextElement* pPtr = static_cast<PDFTextElement*>(TextElement);

    switch (Flags)
    {
        case DTWAIN_PDFTEXTELEMENT_POSITION:
            *val1 = static_cast<LONG>(pPtr->xpos);
            *val2 = static_cast<LONG>(pPtr->ypos);
        break;

        case DTWAIN_PDFTEXTELEMENT_COLOR:
            *val1 = pPtr->colorRGB;
        break;

        case DTWAIN_PDFTEXTELEMENT_STROKEWIDTH:
            *val1 = pPtr->strokeWidth;
        break;

        case DTWAIN_PDFTEXTELEMENT_DISPLAYFLAGS:
            *val1 = pPtr->displayFlags;
        break;

        case DTWAIN_PDFTEXTELEMENT_RENDERMODE:
            *val1 = pPtr->renderMode;
        break;

        case DTWAIN_PDFTEXTELEMENT_TEXTLENGTH:
            *val1 = static_cast<long>(pPtr->m_text.size());
        break;

        default:
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetPDFTextElementString(DTWAIN_PDFTEXTELEMENT TextElement, LPTSTR lpszStr, LONG maxLen, LONG Flags)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement, lpszStr, maxLen, Flags))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LONG ConditionCode;
    const auto it = CheckPDFTextElement(TextElement, ConditionCode);
    if ( !it.first )
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1; }, ConditionCode, false, FUNC_MACRO);

    const PDFTextElement* pPtr = static_cast<PDFTextElement*>(TextElement);

    switch (Flags)
    {
        case DTWAIN_PDFTEXTELEMENT_FONTNAME:
            StringWrapper::SafeStrcpy(lpszStr, StringConversion::Convert_Ansi_To_Native(pPtr->m_font.m_fontName).c_str(), maxLen);
        break;

        case DTWAIN_PDFTEXTELEMENT_TEXT:
            StringWrapper::SafeStrcpy(lpszStr, StringConversion::Convert_Ansi_To_Native(pPtr->m_text).c_str(), maxLen);
        break;

        default:
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ResetPDFTextElement(DTWAIN_PDFTEXTELEMENT TextElement)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LONG ConditionCode;
    const auto it = CheckPDFTextElement(TextElement, ConditionCode);
    if ( !it.first )
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1; }, ConditionCode, false, FUNC_MACRO);

    const auto pPtr = static_cast<PDFTextElement*>(TextElement);
    *pPtr = {};
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

std::pair<bool, CTL_TEXTELEMENTPTRLIST::iterator> CheckPDFTextElement(DTWAIN_PDFTEXTELEMENT TextElement, LONG& ConditionCode)
{
    ConditionCode = 0;
    const auto pPtr = static_cast<PDFTextElement*>(TextElement);
    auto [pHandle, pSource] = VerifyHandles(pPtr->pTwainSource, DTWAIN_TEST_NOTHROW);

    // First check if source handle is still open/valid (note that this only does a read, not write
    // so if an invalid pointer is passed, we won't care at this point)
    if ( !pSource )
    {
        ConditionCode = DTWAIN_ERR_INVALID_PARAM;
        return { false, pHandle->m_mapPDFTextElement[nullptr].end() };
    }

    // Now check if the source really has this pointer
    const auto it = pHandle->m_mapPDFTextElement.find(pSource);
    if ( it == pHandle->m_mapPDFTextElement.end())
    {
        ConditionCode = DTWAIN_ERR_INVALID_PARAM;
        return { false, pHandle->m_mapPDFTextElement[nullptr].end() };
    }

    auto it2 = std::find_if(it->second.begin(), it->second.end(), [&](const auto& ptr) { return ptr->pTwainSource == pPtr->pTwainSource; });

    if ( CTL_StaticData::GetLogFilterFlags() )
    {
        std::string sOut = "PDF TextElement Info: \n";
        sOut += CTL_ErrorStructDecoder::DecodePDFTextElement(pPtr);
        CTL_TwainAppMgr::WriteLogInfoA(sOut);
    }
    return { true, it2 };
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPolarity(DTWAIN_SOURCE Source, LONG Polarity)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Polarity))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFPOLARITYKEY, Polarity);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}
