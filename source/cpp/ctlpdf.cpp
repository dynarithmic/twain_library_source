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
    CheckPDFTextElement(DTWAIN_PDFTEXTELEMENT TextElement, CTL_ITwainSource* pSource, LONG& ConditionCode);
static std::pair<bool, CTL_TEXTELEMENTPTRLIST::iterator>
    CheckGlobalPDFTextElement(DTWAIN_PDFTEXTELEMENT TextElement);


static constexpr std::array<LONG, 8> aPDFFloatTypes = {
													 DTWAIN_PDFTEXTELEMENT_SCALINGXY,
													 DTWAIN_PDFTEXTELEMENT_FONTHEIGHT,
													 DTWAIN_PDFTEXTELEMENT_ROTATIONANGLE,
													 DTWAIN_PDFTEXTELEMENT_SCALING,
													 DTWAIN_PDFTEXTELEMENT_STROKEWIDTH,
													 DTWAIN_PDFTEXTELEMENT_CHARSPACING,
													 DTWAIN_PDFTEXTELEMENT_WORDSPACING,
													 DTWAIN_PDFTEXTELEMENT_SKEWANGLES
                                                };

static constexpr std::array<LONG, 5> aPDFLongTypes= {
													DTWAIN_PDFTEXTELEMENT_POSITION,
													DTWAIN_PDFTEXTELEMENT_COLOR,
													DTWAIN_PDFTEXTELEMENT_DISPLAYFLAGS,
													DTWAIN_PDFTEXTELEMENT_RENDERMODE,
													DTWAIN_PDFTEXTELEMENT_TRANSFORMORDER
                                                };

static constexpr std::array<LONG, 2> aPDFStringTypes= {
        												DTWAIN_PDFTEXTELEMENT_FONTNAME,
		        										DTWAIN_PDFTEXTELEMENT_TEXT
                                                };

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
                                                 DWORD Permissions, DTWAIN_BOOL UseStrongEncryption)
{
    LOG_FUNC_ENTRY_PARAMS((Source, bUseEncryption, lpszUser, lpszOwner, Permissions, UseStrongEncryption))
    auto [pHandle, pSource] = VerifyHandles(Source);
    CTL_StringViewType owner = lpszOwner?lpszOwner:_T("");
    CTL_StringViewType user = lpszUser?lpszUser:_T("");

    // Even though the Permissions parameter is an unsigned 32-bit value from the user, 
    // this will be "converted" to a 32-bit signed integer internally, which is what the PDF 
    // specification requires.
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
    Quality = (std::max<LONG>)(1, (std::min<LONG>)(100, Quality));
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
    auto [pHandle, pSource] = VerifyHandles(Source);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return !CustomWidth || !CustomHeight; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    const DTWAIN_BOOL bRet = SetPDFStringFunc(Source, PageSize, CustomWidth, CustomHeight, &DTWAIN_SetPDFPageSize);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPageScaleString(DTWAIN_SOURCE Source, LONG nOptions,
                                                      LPCTSTR xScale, LPCTSTR yScale)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nOptions,xScale, yScale))
    auto [pHandle, pSource] = VerifyHandles(Source);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return !xScale|| !yScale; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
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

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFAESEncryption(DTWAIN_SOURCE Source, LONG nWhichEncryption, DTWAIN_BOOL bUseAES)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nWhichEncryption, bUseAES))
    #ifndef DTWAIN_SUPPORT_AES
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK_LOG_PARAMS(false)
    #else
    auto [pHandle, pSource] = VerifyHandles(Source);
    bool goodParam = (nWhichEncryption == DTWAIN_PDF_AES128 ||
                      nWhichEncryption == DTWAIN_PDF_AES256);

    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !goodParam; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    if (nWhichEncryption == DTWAIN_PDF_AES128)
    {
        pSource->SetPDFValue(PDFAESKEY, static_cast<LONG>(bUseAES));
    }
    else
    if (nWhichEncryption == DTWAIN_PDF_AES256)
    {
        pSource->SetPDFValue(PDFAES256KEY, static_cast<LONG>(bUseAES));
    }

    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
    #endif
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

template <typename wrapperType, typename strType, int tupleVal>
static LONG GetType1FontInternal(int FontVal, strType szFont, LONG nChars)
{
	auto st = GetType1FontNameFromType(FontVal);
    return wrapperType::CopyInfoToCString(std::get<tupleVal>(st), szFont, nChars);
}

LONG DLLENTRY_DEF DTWAIN_GetPDFType1FontName(LONG FontVal, LPTSTR szFont, LONG nChars)
{
    LOG_FUNC_ENTRY_PARAMS((FontVal, szFont, nChars))
    VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    int numChars = 0;
    #ifdef _UNICODE
    numChars = GetType1FontInternal<StringWrapperW, LPWSTR, 2>(FontVal, szFont, nChars); 
    #else
   	numChars = GetType1FontInternal<StringWrapperA, LPSTR, 1>(FontVal, szFont, nChars); 
    #endif
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szFont))
    LOG_FUNC_EXIT_NONAME_PARAMS(numChars)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetPDFType1FontNameA(LONG FontVal, LPSTR szFont, LONG nChars)
{
    LOG_FUNC_ENTRY_PARAMS((FontVal, szFont, nChars))
    VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
	int numChars = GetType1FontInternal<StringWrapperA, LPSTR, 1>(FontVal, szFont, nChars);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szFont))
    LOG_FUNC_EXIT_NONAME_PARAMS(numChars)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetPDFType1FontNameW(LONG FontVal, LPWSTR szFont, LONG nChars)
{
    LOG_FUNC_ENTRY_PARAMS((FontVal, szFont, nChars))
    VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
	int numChars = GetType1FontInternal<StringWrapperW, LPWSTR, 2>(FontVal, szFont, nChars);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szFont))
    LOG_FUNC_EXIT_NONAME_PARAMS(numChars)
    CATCH_BLOCK(-1)
}

static void GenericAddPDFText(CTL_ITwainSource *pSource,
                              LPCTSTR szText, LONG xPos, LONG yPos,
	                          LPCTSTR fontName, DTWAIN_FLOAT fontSize, LONG colorRGB,
	                          LONG renderMode, DTWAIN_FLOAT scaling,
	                          DTWAIN_FLOAT charSpacing, DTWAIN_FLOAT wordSpacing,
	                          DTWAIN_FLOAT strokeWidth, DWORD Flags, std::shared_ptr<PDFTextElement> pTextElement = nullptr)
{
	struct DefaultValSetterLONG
	{
		LONG DefaultSetting;
		int DefaultValue;
		LONG pSource;
		int* pDestination;
		DWORD flagValue;
	};

	struct DefaultValSetterDOUBLE
	{
		double DefaultSetting;
		double DefaultValue;
		double pSource;
		double* pDestination;
		DWORD flagValue;
	};

	PDFTextElement element;
	constexpr LONG riseValue = 0;
	const DefaultValSetterLONG defVals[] = {
						{DTWAIN_DEFAULT, 0 , renderMode, &element.renderMode,DTWAIN_PDFTEXT_NORENDERMODE},
						{riseValue, 0 , riseValue, &element.riseValue,1},
						{DTWAIN_DEFAULT, 0 , colorRGB, &element.colorRGB, DTWAIN_PDFTEXT_NORGBCOLOR},
						{static_cast<LONG>(Flags), static_cast<int>(Flags), static_cast<LONG>(Flags), &element.displayFlags, 1}
	};

	const DefaultValSetterDOUBLE defValsDOUBLE[] = {
						 {fontSize, 10.0 , fontSize, &element.fontSize, DTWAIN_PDFTEXT_NOFONTSIZE},
						 {DTWAIN_FLOATDEFAULT, 100.0 , scaling, &element.scaling, DTWAIN_PDFTEXT_NOSCALING},
						 {DTWAIN_FLOATDEFAULT, 0.0 , wordSpacing, &element.wordSpacing, DTWAIN_PDFTEXT_NOWORDSPACING},
						 {DTWAIN_FLOATDEFAULT, 0.0 , charSpacing, &element.charSpacing, DTWAIN_PDFTEXT_NOCHARSPACING},
	 					 {DTWAIN_FLOATDEFAULT, 1.0 , strokeWidth, &element.strokeWidth,DTWAIN_PDFTEXT_NOSTROKEWIDTH},
	};

	constexpr size_t numDefVals = std::size(defVals);
	constexpr size_t numDefValsDOUBLE = std::size(defValsDOUBLE);

	element.m_text = StringConversion::Convert_NativePtr_To_Ansi(szText);
	element.xpos = xPos;
	element.ypos = yPos;
	std::string sFontName = "Helvetica";
	if (!fontName)
		element.m_font.m_fontName = sFontName;
	else
		element.m_font.m_fontName = StringConversion::Convert_NativePtr_To_Ansi(fontName);
	for (int i = 0; i < numDefVals; ++i)
	{
		if (Flags & defVals[i].flagValue)
			*defVals[i].pDestination = defVals[i].DefaultValue;
		else
			*defVals[i].pDestination = defVals[i].pSource;
	}

	for (int i = 0; i < numDefValsDOUBLE; ++i)
	{
		if (Flags & defValsDOUBLE[i].flagValue)
			*defValsDOUBLE[i].pDestination = defValsDOUBLE[i].DefaultValue;
		else
			*defValsDOUBLE[i].pDestination = defValsDOUBLE[i].pSource;
	}

	/* Now get the position */
	if (Flags & DTWAIN_PDFTEXT_NOABSPOSITION)
		element.stockPosition = Flags & 0x000FFF00;

    if (!pTextElement)
    {
        PDFTextElementPtr pPtr = std::make_shared<PDFTextElement>();
        *pPtr = element;
        // Add to the global list
		auto& globalTextElementList = CTL_StaticData::GetPDFTextElementList();
        globalTextElementList.push_back(pPtr);

        if ( pSource )
            // Now point the source to this item
			pSource->SetPDFValue(PDFTEXTELEMENTKEY, pPtr);

		// Set the "has already been displayed" to false
        pPtr->hasBeenDisplayed = false;
        pPtr->displayFlags = Flags;
    }
    else
    {
        pTextElement->hasBeenDisplayed = false;
        if ( pSource )
            pSource->SetPDFValue(PDFTEXTELEMENTKEY, pTextElement);
        pTextElement->displayFlags = Flags;
    }
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddPDFTextElement(DTWAIN_SOURCE Source, DTWAIN_PDFTEXTELEMENT TextElement)
{
    LOG_FUNC_ENTRY_PARAMS((Source, TextElement))
    auto [pHandle, pSource] = VerifyHandles(Source);
    PDFTextElement* pElement = static_cast<PDFTextElement*>(TextElement);
    auto validElement = CheckGlobalPDFTextElement(TextElement);
	DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return !validElement.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    GenericAddPDFText(pSource,
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
                        pElement->displayFlags,
                        *validElement.second);
    pElement->vptrTwainSource.insert(pSource);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddPDFText(DTWAIN_SOURCE Source,
                                           LPCTSTR szText, LONG xPos, LONG yPos,
                                           LPCTSTR fontName, DTWAIN_FLOAT fontSize, LONG colorRGB,
                                           LONG renderMode, DTWAIN_FLOAT scaling,
                                           DTWAIN_FLOAT charSpacing, DTWAIN_FLOAT wordSpacing,
                                           DTWAIN_FLOAT strokeWidth, DWORD Flags)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szText, xPos, yPos, fontName, fontSize, colorRGB,
                           renderMode, scaling, charSpacing, wordSpacing, strokeWidth, Flags))
    auto [pHandle, pSource] = VerifyHandles(Source);
    GenericAddPDFText(pSource, szText, xPos, yPos, fontName, fontSize, colorRGB,
                      renderMode, scaling, charSpacing, wordSpacing, strokeWidth, Flags);
	LOG_FUNC_EXIT_NONAME_PARAMS(true)
	CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AddPDFTextString(DTWAIN_SOURCE Source,
	                                            LPCTSTR szText, LONG xPos, LONG yPos,
	                                            LPCTSTR fontName, LPCTSTR fontSize, LONG colorRGB,
	                                            LONG renderMode, LPCTSTR scaling,
                                                LPCTSTR charSpacing, LPCTSTR wordSpacing,
	                                            LPCTSTR strokeWidth, DWORD Flags)
{
	LOG_FUNC_ENTRY_PARAMS((Source, szText, xPos, yPos, fontName, fontSize, colorRGB,
                   		   renderMode, scaling, charSpacing, wordSpacing, strokeWidth, Flags))
    const DTWAIN_FLOAT val1 = StringWrapper::ToDouble(fontSize);
	const DTWAIN_FLOAT val2 = StringWrapper::ToDouble(scaling);
	const DTWAIN_FLOAT val3 = StringWrapper::ToDouble(charSpacing);
	const DTWAIN_FLOAT val4 = StringWrapper::ToDouble(wordSpacing);
	const DTWAIN_FLOAT val5 = StringWrapper::ToDouble(strokeWidth);
    auto retVal = DTWAIN_AddPDFText(Source, szText, xPos, yPos, fontName, val1,
                                    colorRGB, renderMode, val2, val3, val4, val5, Flags);
	LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ClearPDFTextElements(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->ClearPDFTextElements();
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RemovePDFTextElement(DTWAIN_SOURCE Source, DTWAIN_PDFTEXTELEMENT TextElement)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
	PDFTextElement* pElement = static_cast<PDFTextElement*>(TextElement);
	auto validElement = CheckGlobalPDFTextElement(TextElement);
	DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return !validElement.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    pSource->ClearOnePDFTextElement(pElement);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_PDFTEXTELEMENT DLLENTRY_DEF DTWAIN_CreatePDFTextElement()
{
	LOG_FUNC_ENTRY_PARAMS(())
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
	GenericAddPDFText(pSource, 
                      StringWrapper::traits_type::GetEmptyString(), 
                      0, 
                      0, 
                      _T("Helvetica"), 
                      12, 
                      DTWAIN_MakeRGB(0, 0, 0), 
                      0, 
                      100, 
                      0, 
                      1.0, 
                      0, 
                      DTWAIN_PDFTEXT_ALLPAGES, 
                      nullptr);

	auto& globalTextElementList = CTL_StaticData::GetPDFTextElementList();
    if (!globalTextElementList.empty())
    {
        auto pPtr = globalTextElementList.back();
        LOG_FUNC_EXIT_NONAME_PARAMS(pPtr.get())
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(nullptr)
    CATCH_BLOCK_LOG_PARAMS(nullptr)
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
    auto itGeneric = CheckGlobalPDFTextElement(TextElement);
    if (itGeneric.first)
    {
        // Remove the raw pointers from any sources that have this text element
        for (auto& pr : pHandle->m_mapPDFTextElement)
        {
            if (pr.first)
            {
                auto& theNode = pr.second;
                auto& theSet = theNode.first;
                auto& theList = theNode.second;
                theList.erase(std::find_if(theList.begin(), theList.end(),
                    [&](auto& ptr) { return ptr == TextElement; }), theList.end());
                theSet.erase(static_cast<PDFTextElement*>(TextElement));
            }
        }

        // Now erase the global instance
        auto& globalList = CTL_StaticData::GetPDFTextElementList();
        globalList.erase(itGeneric.second);
    }

    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTextElementFloat(DTWAIN_PDFTEXTELEMENT TextElement, DTWAIN_FLOAT val1, DTWAIN_FLOAT val2, LONG Flags)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement, val1, val2, Flags))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const auto it = CheckGlobalPDFTextElement(TextElement);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !it.first;}, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    auto itFound = generic_array_finder(aPDFFloatTypes, Flags);
	DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !itFound.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

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

		case DTWAIN_PDFTEXTELEMENT_STROKEWIDTH:
			pPtr->strokeWidth = val1;
    	break;

        default:
            LOG_FUNC_EXIT_NONAME_PARAMS(false)

    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTextElementFloatString(DTWAIN_PDFTEXTELEMENT TextElement, LPCTSTR val1, LPCTSTR val2, LONG Flags)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement, val1, val2, Flags))
	const DTWAIN_FLOAT value1 = StringWrapper::ToDouble(val1);
	const DTWAIN_FLOAT value2 = StringWrapper::ToDouble(val2);
    auto retVal = DTWAIN_SetPDFTextElementFloat(TextElement, value1, value2, Flags);
    LOG_FUNC_EXIT_PARAMS(retVal)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFTextElementLong(DTWAIN_PDFTEXTELEMENT TextElement, LONG val1, LONG val2, LONG Flags)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement, val1, val2, Flags))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const auto it = CheckGlobalPDFTextElement(TextElement);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&]{ return !it.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

	auto itFound = generic_array_finder(aPDFLongTypes, Flags);
	DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !itFound.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

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

        case DTWAIN_PDFTEXTELEMENT_DISPLAYFLAGS:
            pPtr->displayFlags = val1;
        break;

        case DTWAIN_PDFTEXTELEMENT_RENDERMODE:
            pPtr->renderMode = val1;
        break;

        case DTWAIN_PDFTEXTELEMENT_TRANSFORMORDER:
            pPtr->textTransform = (std::max<LONG>)(0, (std::min<LONG>)(val1, DTWAIN_PDFTEXTTRANFORM_LAST));
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
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return !val1; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    const auto it = CheckGlobalPDFTextElement(TextElement);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !it.first;}, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

	auto itFound = generic_array_finder(aPDFStringTypes, Flags);
	DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !itFound.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

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
    const auto it = CheckGlobalPDFTextElement(TextElement);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !it.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

	auto itFound = generic_array_finder(aPDFFloatTypes, Flags);
	DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !itFound.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    const PDFTextElement* pPtr = static_cast<PDFTextElement*>(TextElement);

    switch (Flags)
    {
        case DTWAIN_PDFTEXTELEMENT_SCALINGXY:
            if (val1)
                *val1 = pPtr->scalingX;
            if ( val2 )
                *val2 = pPtr->scalingY;
        break;

        case DTWAIN_PDFTEXTELEMENT_FONTHEIGHT:
            if ( val1 )
                *val1 = pPtr->fontSize;
        break;

        case DTWAIN_PDFTEXTELEMENT_ROTATIONANGLE:
            if ( val1 )
                *val1 = pPtr->rotationAngle;
        break;

        case DTWAIN_PDFTEXTELEMENT_SCALING:
            if ( val1 )
                *val1 = pPtr->scaling;
        break;

        case DTWAIN_PDFTEXTELEMENT_CHARSPACING:
            if ( val1 )
                *val1 = pPtr->charSpacing;
        break;

        case DTWAIN_PDFTEXTELEMENT_WORDSPACING:
            if ( val1 )
                *val1 = pPtr->wordSpacing;
        break;

        case DTWAIN_PDFTEXTELEMENT_SKEWANGLES:
            if ( val1 )
                *val1 = pPtr->skewAngleX;
            if ( val2 )
                *val2 = pPtr->skewAngleY;
        break;

		case DTWAIN_PDFTEXTELEMENT_STROKEWIDTH:
            if ( val1 )
			    *val1 = pPtr->strokeWidth;
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
    const auto it = CheckGlobalPDFTextElement(TextElement);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !it.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

	auto itFound = generic_array_finder(aPDFLongTypes, Flags);
	DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !itFound.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    const PDFTextElement* pPtr = static_cast<PDFTextElement*>(TextElement);

    switch (Flags)
    {
        case DTWAIN_PDFTEXTELEMENT_POSITION:
            if ( val1 )
                *val1 = static_cast<LONG>(pPtr->xpos);
            if ( val2 )
                *val2 = static_cast<LONG>(pPtr->ypos);
        break;

        case DTWAIN_PDFTEXTELEMENT_COLOR:
			if (val1)
				*val1 = pPtr->colorRGB;
        break;

        case DTWAIN_PDFTEXTELEMENT_DISPLAYFLAGS:
			if (val1)
				*val1 = pPtr->displayFlags;
        break;

        case DTWAIN_PDFTEXTELEMENT_RENDERMODE:
			if (val1)
				*val1 = pPtr->renderMode;
        break;

        case DTWAIN_PDFTEXTELEMENT_TEXTLENGTH:
			if (val1)
				*val1 = static_cast<long>(pPtr->m_text.size());
        break;

        default:
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((val1, val2, Flags))
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetPDFTextElementString(DTWAIN_PDFTEXTELEMENT TextElement, LPTSTR lpszStr, LONG maxLen, LONG Flags)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement, lpszStr, maxLen, Flags))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const auto it = CheckGlobalPDFTextElement(TextElement);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !it.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

	auto itFound = generic_array_finder(aPDFStringTypes, Flags);
	DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !itFound.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    const PDFTextElement* pPtr = static_cast<PDFTextElement*>(TextElement);

    int32_t retLength = 0;

    switch (Flags)
    {
        case DTWAIN_PDFTEXTELEMENT_FONTNAME:
            retLength = StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(pPtr->m_font.m_fontName), lpszStr, maxLen);
        break;

        case DTWAIN_PDFTEXTELEMENT_TEXT:
            retLength = StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(pPtr->m_text), lpszStr, maxLen);
        break;

        default:
            LOG_FUNC_EXIT_NONAME_PARAMS(0)
    }
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszStr))
    LOG_FUNC_EXIT_NONAME_PARAMS(retLength)
    CATCH_BLOCK(0)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ResetPDFTextElement(DTWAIN_PDFTEXTELEMENT TextElement)
{
    LOG_FUNC_ENTRY_PARAMS((TextElement))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const auto it = CheckGlobalPDFTextElement(TextElement);
    if ( !it.first )
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    const auto pPtr = static_cast<PDFTextElement*>(TextElement);
    *pPtr = {};
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_PDFTEXTELEMENT DLLENTRY_DEF DTWAIN_CreatePDFTextElementCopy(DTWAIN_PDFTEXTELEMENT TextElement)
{
	LOG_FUNC_ENTRY_PARAMS((TextElement))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    if ( !TextElement )
		DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
	PDFTextElementPtr pPtr = std::make_shared<PDFTextElement>();
    auto it = CheckGlobalPDFTextElement(TextElement);
	DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !it.first; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
	auto* pText = pPtr.get();
    auto* srcElement = static_cast<PDFTextElement*>(TextElement);
    *pText = *srcElement;
	auto& globalTextElementList = CTL_StaticData::GetPDFTextElementList();
	globalTextElementList.push_back(pPtr);
	LOG_FUNC_EXIT_NONAME_PARAMS(pText)
	CATCH_BLOCK(nullptr)
}

std::pair<bool, CTL_TEXTELEMENTPTRLIST::iterator> CheckGlobalPDFTextElement(DTWAIN_PDFTEXTELEMENT TextElement)
{
	auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_TEST_NOTHROW | DTWAIN_VERIFY_DLLHANDLE | DTWAIN_TEST_SETLASTERROR);

	// Get the list of created text elements from global list
	auto& textElementList = CTL_StaticData::GetPDFTextElementList();

	// First check if DLL handle is valid
    if (!pHandle)
        return { false, textElementList.end() };

    auto foundIt = 
        std::find_if(textElementList.begin(), textElementList.end(), [&](auto& spTheList) { return spTheList.get() == TextElement; });
    if (foundIt != textElementList.end())
        return { true, foundIt };
    return { false, textElementList.end() };
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPDFPolarity(DTWAIN_SOURCE Source, LONG Polarity)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Polarity))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPDFValue(PDFPOLARITYKEY, Polarity);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}
