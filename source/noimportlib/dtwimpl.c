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

/* This file is compatible with DTWAIN versions 5.0 and above.
 * If you are running a previous version, you will need to comment out
 * the "assert" lines that fail when a function cannot be found, or compile
 * your application using the IGNORE_FUNC_ERRORS preprocessor constant
 *
 *  i.e.  #define IGNORE_FUNC_ERRORS                     */

/* Uncomment the line below if you're using Visual Studio and are using precompiled headers */
/* #include "stdafx.h" */

#include "dtwainx2.h"
#include <assert.h>
#include <commdlg.h>
#pragma warning (push)
#pragma warning (disable:4113)
#pragma warning (disable:4047)

/* declare function pointers */
#ifdef __cplusplus

    D_ACQUIREAUDIOFILEAFUNC                                           DYNDTWAIN_API::DTWAIN_AcquireAudioFileA = nullptr;
    D_ACQUIREAUDIOFILEFUNC                                            DYNDTWAIN_API::DTWAIN_AcquireAudioFile = nullptr;
    D_ACQUIREAUDIOFILEWFUNC                                           DYNDTWAIN_API::DTWAIN_AcquireAudioFileW = nullptr;
    D_ACQUIREAUDIONATIVEEXFUNC                                        DYNDTWAIN_API::DTWAIN_AcquireAudioNativeEx = nullptr;
    D_ACQUIREAUDIONATIVEFUNC                                          DYNDTWAIN_API::DTWAIN_AcquireAudioNative = nullptr;
    D_ACQUIREBUFFEREDEXFUNC                                           DYNDTWAIN_API::DTWAIN_AcquireBufferedEx = nullptr;
    D_ACQUIREBUFFEREDFUNC                                             DYNDTWAIN_API::DTWAIN_AcquireBuffered = nullptr;
    D_ACQUIREFILEAFUNC                                                DYNDTWAIN_API::DTWAIN_AcquireFileA = nullptr;
    D_ACQUIREFILEEXFUNC                                               DYNDTWAIN_API::DTWAIN_AcquireFileEx = nullptr;
    D_ACQUIREFILEFUNC                                                 DYNDTWAIN_API::DTWAIN_AcquireFile = nullptr;
    D_ACQUIREFILEWFUNC                                                DYNDTWAIN_API::DTWAIN_AcquireFileW = nullptr;
    D_ACQUIRENATIVEEXFUNC                                             DYNDTWAIN_API::DTWAIN_AcquireNativeEx = nullptr;
    D_ACQUIRENATIVEFUNC                                               DYNDTWAIN_API::DTWAIN_AcquireNative = nullptr;
    D_ACQUIRETOCLIPBOARDFUNC                                          DYNDTWAIN_API::DTWAIN_AcquireToClipboard = nullptr;
    D_ADDEXTIMAGEINFOQUERYFUNC                                        DYNDTWAIN_API::DTWAIN_AddExtImageInfoQuery = nullptr;
    D_ADDPDFTEXTAFUNC                                                 DYNDTWAIN_API::DTWAIN_AddPDFTextA = nullptr;
    D_ADDPDFTEXTEXFUNC                                                DYNDTWAIN_API::DTWAIN_AddPDFTextEx = nullptr;
    D_ADDPDFTEXTFUNC                                                  DYNDTWAIN_API::DTWAIN_AddPDFText = nullptr;
    D_ADDPDFTEXTWFUNC                                                 DYNDTWAIN_API::DTWAIN_AddPDFTextW = nullptr;
    D_ALLOCATEMEMORYEXFUNC                                            DYNDTWAIN_API::DTWAIN_AllocateMemoryEx = nullptr;
    D_ALLOCATEMEMORYFUNC                                              DYNDTWAIN_API::DTWAIN_AllocateMemory = nullptr;
    D_APPHANDLESEXCEPTIONSFUNC                                        DYNDTWAIN_API::DTWAIN_AppHandlesExceptions = nullptr;
    D_ARRAYADDANSISTRINGFUNC                                          DYNDTWAIN_API::DTWAIN_ArrayAddANSIString = nullptr;
    D_ARRAYADDANSISTRINGNFUNC                                         DYNDTWAIN_API::DTWAIN_ArrayAddANSIStringN = nullptr;
    D_ARRAYADDFLOATFUNC                                               DYNDTWAIN_API::DTWAIN_ArrayAddFloat = nullptr;
    D_ARRAYADDFLOATNFUNC                                              DYNDTWAIN_API::DTWAIN_ArrayAddFloatN = nullptr;
    D_ARRAYADDFUNC                                                    DYNDTWAIN_API::DTWAIN_ArrayAdd = nullptr;
    D_ARRAYADDLONG64FUNC                                              DYNDTWAIN_API::DTWAIN_ArrayAddLong64 = nullptr;
    D_ARRAYADDLONG64NFUNC                                             DYNDTWAIN_API::DTWAIN_ArrayAddLong64N = nullptr;
    D_ARRAYADDLONGFUNC                                                DYNDTWAIN_API::DTWAIN_ArrayAddLong = nullptr;
    D_ARRAYADDLONGNFUNC                                               DYNDTWAIN_API::DTWAIN_ArrayAddLongN = nullptr;
    D_ARRAYADDNFUNC                                                   DYNDTWAIN_API::DTWAIN_ArrayAddN = nullptr;
    D_ARRAYADDSTRINGAFUNC                                             DYNDTWAIN_API::DTWAIN_ArrayAddStringA = nullptr;
    D_ARRAYADDSTRINGFUNC                                              DYNDTWAIN_API::DTWAIN_ArrayAddString = nullptr;
    D_ARRAYADDSTRINGNAFUNC                                            DYNDTWAIN_API::DTWAIN_ArrayAddStringNA = nullptr;
    D_ARRAYADDSTRINGNFUNC                                             DYNDTWAIN_API::DTWAIN_ArrayAddStringN = nullptr;
    D_ARRAYADDSTRINGNWFUNC                                            DYNDTWAIN_API::DTWAIN_ArrayAddStringNW = nullptr;
    D_ARRAYADDSTRINGWFUNC                                             DYNDTWAIN_API::DTWAIN_ArrayAddStringW = nullptr;
    D_ARRAYADDWIDESTRINGFUNC                                          DYNDTWAIN_API::DTWAIN_ArrayAddWideString = nullptr;
    D_ARRAYADDWIDESTRINGNFUNC                                         DYNDTWAIN_API::DTWAIN_ArrayAddWideStringN = nullptr;
    D_ARRAYCONVERTFIX32TOFLOATFUNC                                    DYNDTWAIN_API::DTWAIN_ArrayConvertFix32ToFloat = nullptr;
    D_ARRAYCONVERTFLOATTOFIX32FUNC                                    DYNDTWAIN_API::DTWAIN_ArrayConvertFloatToFix32 = nullptr;
    D_ARRAYCOPYFUNC                                                   DYNDTWAIN_API::DTWAIN_ArrayCopy = nullptr;
    D_ARRAYCREATECOPYFUNC                                             DYNDTWAIN_API::DTWAIN_ArrayCreateCopy = nullptr;
    D_ARRAYCREATEFROMCAPFUNC                                          DYNDTWAIN_API::DTWAIN_ArrayCreateFromCap = nullptr;
    D_ARRAYCREATEFROMLONG64SFUNC                                      DYNDTWAIN_API::DTWAIN_ArrayCreateFromLong64s = nullptr;
    D_ARRAYCREATEFROMLONGSFUNC                                        DYNDTWAIN_API::DTWAIN_ArrayCreateFromLongs = nullptr;
    D_ARRAYCREATEFROMREALSFUNC                                        DYNDTWAIN_API::DTWAIN_ArrayCreateFromReals = nullptr;
    D_ARRAYCREATEFROMSTRINGSFUNC                                      DYNDTWAIN_API::DTWAIN_ArrayCreateFromStrings = nullptr;
    D_ARRAYCREATEFUNC                                                 DYNDTWAIN_API::DTWAIN_ArrayCreate = nullptr;
    D_ARRAYDESTROYFRAMESFUNC                                          DYNDTWAIN_API::DTWAIN_ArrayDestroyFrames = nullptr;
    D_ARRAYDESTROYFUNC                                                DYNDTWAIN_API::DTWAIN_ArrayDestroy = nullptr;
    D_ARRAYFINDANSISTRINGFUNC                                         DYNDTWAIN_API::DTWAIN_ArrayFindANSIString = nullptr;
    D_ARRAYFINDFLOATFUNC                                              DYNDTWAIN_API::DTWAIN_ArrayFindFloat = nullptr;
    D_ARRAYFINDFUNC                                                   DYNDTWAIN_API::DTWAIN_ArrayFind = nullptr;
    D_ARRAYFINDLONG64FUNC                                             DYNDTWAIN_API::DTWAIN_ArrayFindLong64 = nullptr;
    D_ARRAYFINDLONGFUNC                                               DYNDTWAIN_API::DTWAIN_ArrayFindLong = nullptr;
    D_ARRAYFINDSTRINGAFUNC                                            DYNDTWAIN_API::DTWAIN_ArrayFindStringA = nullptr;
    D_ARRAYFINDSTRINGFUNC                                             DYNDTWAIN_API::DTWAIN_ArrayFindString = nullptr;
    D_ARRAYFINDSTRINGWFUNC                                            DYNDTWAIN_API::DTWAIN_ArrayFindStringW = nullptr;
    D_ARRAYFINDWIDESTRINGFUNC                                         DYNDTWAIN_API::DTWAIN_ArrayFindWideString = nullptr;
    D_ARRAYFIX32GETATFUNC                                             DYNDTWAIN_API::DTWAIN_ArrayFix32GetAt = nullptr;
    D_ARRAYFIX32SETATFUNC                                             DYNDTWAIN_API::DTWAIN_ArrayFix32SetAt = nullptr;
    D_ARRAYFRAMEGETATFUNC                                             DYNDTWAIN_API::DTWAIN_ArrayFrameGetAt = nullptr;
    D_ARRAYFRAMEGETFRAMEATFUNC                                        DYNDTWAIN_API::DTWAIN_ArrayFrameGetFrameAt = nullptr;
    D_ARRAYFRAMESETATFUNC                                             DYNDTWAIN_API::DTWAIN_ArrayFrameSetAt = nullptr;
    D_ARRAYGETATANSISTRINGFUNC                                        DYNDTWAIN_API::DTWAIN_ArrayGetAtANSIString = nullptr;
    D_ARRAYGETATANSISTRINGPTRFUNC                                     DYNDTWAIN_API::DTWAIN_ArrayGetAtANSIStringPtr = nullptr;
    D_ARRAYGETATFLOATFUNC                                             DYNDTWAIN_API::DTWAIN_ArrayGetAtFloat = nullptr;
    D_ARRAYGETATFUNC                                                  DYNDTWAIN_API::DTWAIN_ArrayGetAt = nullptr;
    D_ARRAYGETATLONG64FUNC                                            DYNDTWAIN_API::DTWAIN_ArrayGetAtLong64 = nullptr;
    D_ARRAYGETATLONGFUNC                                              DYNDTWAIN_API::DTWAIN_ArrayGetAtLong = nullptr;
    D_ARRAYGETATSTRINGAFUNC                                           DYNDTWAIN_API::DTWAIN_ArrayGetAtStringA = nullptr;
    D_ARRAYGETATSTRINGFUNC                                            DYNDTWAIN_API::DTWAIN_ArrayGetAtString = nullptr;
    D_ARRAYGETATSTRINGPTRFUNC                                         DYNDTWAIN_API::DTWAIN_ArrayGetAtStringPtr = nullptr;
    D_ARRAYGETATSTRINGWFUNC                                           DYNDTWAIN_API::DTWAIN_ArrayGetAtStringW = nullptr;
    D_ARRAYGETATWIDESTRINGFUNC                                        DYNDTWAIN_API::DTWAIN_ArrayGetAtWideString = nullptr;
    D_ARRAYGETATWIDESTRINGPTRFUNC                                     DYNDTWAIN_API::DTWAIN_ArrayGetAtWideStringPtr = nullptr;
    D_ARRAYGETBUFFERFUNC                                              DYNDTWAIN_API::DTWAIN_ArrayGetBuffer = nullptr;
    D_ARRAYGETCOUNTFUNC                                               DYNDTWAIN_API::DTWAIN_ArrayGetCount = nullptr;
    D_ARRAYGETMAXSTRINGLENGTHFUNC                                     DYNDTWAIN_API::DTWAIN_ArrayGetMaxStringLength = nullptr;
    D_ARRAYGETSOURCEATFUNC                                            DYNDTWAIN_API::DTWAIN_ArrayGetSourceAt = nullptr;
    D_ARRAYGETSTRINGLENGTHFUNC                                        DYNDTWAIN_API::DTWAIN_ArrayGetStringLength = nullptr;
    D_ARRAYGETTYPEFUNC                                                DYNDTWAIN_API::DTWAIN_ArrayGetType = nullptr;
    D_ARRAYINITFUNC                                                   DYNDTWAIN_API::DTWAIN_ArrayInit = nullptr;
    D_ARRAYINSERTATANSISTRINGFUNC                                     DYNDTWAIN_API::DTWAIN_ArrayInsertAtANSIString = nullptr;
    D_ARRAYINSERTATANSISTRINGNFUNC                                    DYNDTWAIN_API::DTWAIN_ArrayInsertAtANSIStringN = nullptr;
    D_ARRAYINSERTATFLOATFUNC                                          DYNDTWAIN_API::DTWAIN_ArrayInsertAtFloat = nullptr;
    D_ARRAYINSERTATFLOATNFUNC                                         DYNDTWAIN_API::DTWAIN_ArrayInsertAtFloatN = nullptr;
    D_ARRAYINSERTATFUNC                                               DYNDTWAIN_API::DTWAIN_ArrayInsertAt = nullptr;
    D_ARRAYINSERTATLONG64FUNC                                         DYNDTWAIN_API::DTWAIN_ArrayInsertAtLong64 = nullptr;
    D_ARRAYINSERTATLONG64NFUNC                                        DYNDTWAIN_API::DTWAIN_ArrayInsertAtLong64N = nullptr;
    D_ARRAYINSERTATLONGFUNC                                           DYNDTWAIN_API::DTWAIN_ArrayInsertAtLong = nullptr;
    D_ARRAYINSERTATLONGNFUNC                                          DYNDTWAIN_API::DTWAIN_ArrayInsertAtLongN = nullptr;
    D_ARRAYINSERTATNFUNC                                              DYNDTWAIN_API::DTWAIN_ArrayInsertAtN = nullptr;
    D_ARRAYINSERTATSTRINGAFUNC                                        DYNDTWAIN_API::DTWAIN_ArrayInsertAtStringA = nullptr;
    D_ARRAYINSERTATSTRINGFUNC                                         DYNDTWAIN_API::DTWAIN_ArrayInsertAtString = nullptr;
    D_ARRAYINSERTATSTRINGNAFUNC                                       DYNDTWAIN_API::DTWAIN_ArrayInsertAtStringNA = nullptr;
    D_ARRAYINSERTATSTRINGNFUNC                                        DYNDTWAIN_API::DTWAIN_ArrayInsertAtStringN = nullptr;
    D_ARRAYINSERTATSTRINGNWFUNC                                       DYNDTWAIN_API::DTWAIN_ArrayInsertAtStringNW = nullptr;
    D_ARRAYINSERTATSTRINGWFUNC                                        DYNDTWAIN_API::DTWAIN_ArrayInsertAtStringW = nullptr;
    D_ARRAYINSERTATWIDESTRINGFUNC                                     DYNDTWAIN_API::DTWAIN_ArrayInsertAtWideString = nullptr;
    D_ARRAYINSERTATWIDESTRINGNFUNC                                    DYNDTWAIN_API::DTWAIN_ArrayInsertAtWideStringN = nullptr;
    D_ARRAYREMOVEALLFUNC                                              DYNDTWAIN_API::DTWAIN_ArrayRemoveAll = nullptr;
    D_ARRAYREMOVEATFUNC                                               DYNDTWAIN_API::DTWAIN_ArrayRemoveAt = nullptr;
    D_ARRAYREMOVEATNFUNC                                              DYNDTWAIN_API::DTWAIN_ArrayRemoveAtN = nullptr;
    D_ARRAYRESIZEFUNC                                                 DYNDTWAIN_API::DTWAIN_ArrayResize = nullptr;
    D_ARRAYSETATANSISTRINGFUNC                                        DYNDTWAIN_API::DTWAIN_ArraySetAtANSIString = nullptr;
    D_ARRAYSETATFLOATFUNC                                             DYNDTWAIN_API::DTWAIN_ArraySetAtFloat = nullptr;
    D_ARRAYSETATFUNC                                                  DYNDTWAIN_API::DTWAIN_ArraySetAt = nullptr;
    D_ARRAYSETATLONG64FUNC                                            DYNDTWAIN_API::DTWAIN_ArraySetAtLong64 = nullptr;
    D_ARRAYSETATLONGFUNC                                              DYNDTWAIN_API::DTWAIN_ArraySetAtLong = nullptr;
    D_ARRAYSETATSTRINGAFUNC                                           DYNDTWAIN_API::DTWAIN_ArraySetAtStringA = nullptr;
    D_ARRAYSETATSTRINGFUNC                                            DYNDTWAIN_API::DTWAIN_ArraySetAtString = nullptr;
    D_ARRAYSETATSTRINGWFUNC                                           DYNDTWAIN_API::DTWAIN_ArraySetAtStringW = nullptr;
    D_ARRAYSETATWIDESTRINGFUNC                                        DYNDTWAIN_API::DTWAIN_ArraySetAtWideString = nullptr;
    D_CALLCALLBACK64FUNC                                              DYNDTWAIN_API::DTWAIN_CallCallback64 = nullptr;
    D_CALLCALLBACKFUNC                                                DYNDTWAIN_API::DTWAIN_CallCallback = nullptr;
    D_CALLDSMPROC                                                     DYNDTWAIN_API::DTWAIN_CallDSMProc = nullptr;
    D_CHECKHANDLESFUNC                                                DYNDTWAIN_API::DTWAIN_CheckHandles = nullptr;
    D_CLEARBUFFERSFUNC                                                DYNDTWAIN_API::DTWAIN_ClearBuffers = nullptr;
    D_CLEARERRORBUFFERFUNC                                            DYNDTWAIN_API::DTWAIN_ClearErrorBuffer = nullptr;
    D_CLEARPAGEFUNC                                                   DYNDTWAIN_API::DTWAIN_ClearPage = nullptr;
    D_CLEARPDFTEXTFUNC                                                DYNDTWAIN_API::DTWAIN_ClearPDFText = nullptr;
    D_CLOSESOURCEFUNC                                                 DYNDTWAIN_API::DTWAIN_CloseSource = nullptr;
    D_CLOSESOURCEUIFUNC                                               DYNDTWAIN_API::DTWAIN_CloseSourceUI = nullptr;
    D_CONVERTDIBTOBITMAPFUNC                                          DYNDTWAIN_API::DTWAIN_ConvertDIBToBitmap = nullptr;
    D_CREATEACQUISITIONARRAYFUNC                                      DYNDTWAIN_API::DTWAIN_CreateAcquisitionArray = nullptr;
    D_CREATEPDFTEXTELEMENTFUNC                                        DYNDTWAIN_API::DTWAIN_CreatePDFTextElement = nullptr;
    D_DESTROYACQUISITIONARRAYFUNC                                     DYNDTWAIN_API::DTWAIN_DestroyAcquisitionArray = nullptr;
    D_DESTROYPDFTEXTELEMENTFUNC                                       DYNDTWAIN_API::DTWAIN_DestroyPDFTextElement = nullptr;
    D_DISABLEAPPWINDOWFUNC                                            DYNDTWAIN_API::DTWAIN_DisableAppWindow = nullptr;
    D_ENABLEAUTOBORDERDETECTFUNC                                      DYNDTWAIN_API::DTWAIN_EnableAutoBorderDetect = nullptr;
    D_ENABLEAUTOBRIGHTFUNC                                            DYNDTWAIN_API::DTWAIN_EnableAutoBright = nullptr;
    D_ENABLEAUTODESKEWFUNC                                            DYNDTWAIN_API::DTWAIN_EnableAutoDeskew = nullptr;
    D_ENABLEAUTOFEEDFUNC                                              DYNDTWAIN_API::DTWAIN_EnableAutoFeed = nullptr;
    D_ENABLEAUTOMATICSENSEMEDIUMFUNC                                  DYNDTWAIN_API::DTWAIN_EnableAutomaticSenseMedium = nullptr;
    D_ENABLEAUTOROTATEFUNC                                            DYNDTWAIN_API::DTWAIN_EnableAutoRotate = nullptr;
    D_ENABLEAUTOSCANFUNC                                              DYNDTWAIN_API::DTWAIN_EnableAutoScan = nullptr;
    D_ENABLEDUPLEXFUNC                                                DYNDTWAIN_API::DTWAIN_EnableDuplex = nullptr;
    D_ENABLEFEEDERFUNC                                                DYNDTWAIN_API::DTWAIN_EnableFeeder = nullptr;
    D_ENABLEINDICATORFUNC                                             DYNDTWAIN_API::DTWAIN_EnableIndicator = nullptr;
    D_ENABLEJOBFILEHANDLINGFUNC                                       DYNDTWAIN_API::DTWAIN_EnableJobFileHandling = nullptr;
    D_ENABLELAMPFUNC                                                  DYNDTWAIN_API::DTWAIN_EnableLamp = nullptr;
    D_ENABLEMSGNOTIFYFUNC                                             DYNDTWAIN_API::DTWAIN_EnableMsgNotify = nullptr;
    D_ENABLEPATCHDETECTFUNC                                           DYNDTWAIN_API::DTWAIN_EnablePatchDetect = nullptr;
    D_ENABLEPRINTERFUNC                                               DYNDTWAIN_API::DTWAIN_EnablePrinter = nullptr;
    D_ENABLETHUMBNAILFUNC                                             DYNDTWAIN_API::DTWAIN_EnableThumbnail = nullptr;
    D_ENDTHREADFUNC                                                   DYNDTWAIN_API::DTWAIN_EndThread = nullptr;
    D_ENDTWAINSESSIONFUNC                                             DYNDTWAIN_API::DTWAIN_EndTwainSession = nullptr;
    D_ENUMALARMSEXFUNC                                                DYNDTWAIN_API::DTWAIN_EnumAlarmsEx = nullptr;
    D_ENUMALARMSFUNC                                                  DYNDTWAIN_API::DTWAIN_EnumAlarms = nullptr;
    D_ENUMALARMVOLUMESEXFUNC                                          DYNDTWAIN_API::DTWAIN_EnumAlarmVolumesEx = nullptr;
    D_ENUMALARMVOLUMESFUNC                                            DYNDTWAIN_API::DTWAIN_EnumAlarmVolumes = nullptr;
    D_ENUMAUDIOXFERMECHSEXFUNC                                        DYNDTWAIN_API::DTWAIN_EnumAudioXferMechsEx = nullptr;
    D_ENUMAUDIOXFERMECHSFUNC                                          DYNDTWAIN_API::DTWAIN_EnumAudioXferMechs = nullptr;
    D_ENUMAUTOFEEDVALUESEXFUNC                                        DYNDTWAIN_API::DTWAIN_EnumAutoFeedValuesEx = nullptr;
    D_ENUMAUTOFEEDVALUESFUNC                                          DYNDTWAIN_API::DTWAIN_EnumAutoFeedValues = nullptr;
    D_ENUMAUTOMATICCAPTURESEXFUNC                                     DYNDTWAIN_API::DTWAIN_EnumAutomaticCapturesEx = nullptr;
    D_ENUMAUTOMATICCAPTURESFUNC                                       DYNDTWAIN_API::DTWAIN_EnumAutomaticCaptures = nullptr;
    D_ENUMAUTOMATICSENSEMEDIUMEXFUNC                                  DYNDTWAIN_API::DTWAIN_EnumAutomaticSenseMediumEx = nullptr;
    D_ENUMAUTOMATICSENSEMEDIUMFUNC                                    DYNDTWAIN_API::DTWAIN_EnumAutomaticSenseMedium = nullptr;
    D_ENUMBITDEPTHSEX2FUNC                                            DYNDTWAIN_API::DTWAIN_EnumBitDepthsEx2 = nullptr;
    D_ENUMBITDEPTHSEXFUNC                                             DYNDTWAIN_API::DTWAIN_EnumBitDepthsEx = nullptr;
    D_ENUMBITDEPTHSFUNC                                               DYNDTWAIN_API::DTWAIN_EnumBitDepths = nullptr;
    D_ENUMBOTTOMCAMERASFUNC                                           DYNDTWAIN_API::DTWAIN_EnumBottomCameras = nullptr;
    D_ENUMBRIGHTNESSVALUESEXFUNC                                      DYNDTWAIN_API::DTWAIN_EnumBrightnessValuesEx = nullptr;
    D_ENUMBRIGHTNESSVALUESFUNC                                        DYNDTWAIN_API::DTWAIN_EnumBrightnessValues = nullptr;
    D_ENUMCAMERASFUNC                                                 DYNDTWAIN_API::DTWAIN_EnumCameras = nullptr;
    D_ENUMCOMPRESSIONTYPESEXFUNC                                      DYNDTWAIN_API::DTWAIN_EnumCompressionTypesEx = nullptr;
    D_ENUMCOMPRESSIONTYPESFUNC                                        DYNDTWAIN_API::DTWAIN_EnumCompressionTypes = nullptr;
    D_ENUMCONTRASTVALUESEXFUNC                                        DYNDTWAIN_API::DTWAIN_EnumContrastValuesEx = nullptr;
    D_ENUMCONTRASTVALUESFUNC                                          DYNDTWAIN_API::DTWAIN_EnumContrastValues = nullptr;
    D_ENUMCUSTOMCAPSEX2FUNC                                           DYNDTWAIN_API::DTWAIN_EnumCustomCapsEx2 = nullptr;
    D_ENUMCUSTOMCAPSFUNC                                              DYNDTWAIN_API::DTWAIN_EnumCustomCaps = nullptr;
    D_ENUMDOUBLEFEEDDETECTLENGTHSEXFUNC                               DYNDTWAIN_API::DTWAIN_EnumDoubleFeedDetectLengthsEx = nullptr;
    D_ENUMDOUBLEFEEDDETECTLENGTHSFUNC                                 DYNDTWAIN_API::DTWAIN_EnumDoubleFeedDetectLengths = nullptr;
    D_ENUMDOUBLEFEEDDETECTVALUESEXFUNC                                DYNDTWAIN_API::DTWAIN_EnumDoubleFeedDetectValuesEx = nullptr;
    D_ENUMDOUBLEFEEDDETECTVALUESFUNC                                  DYNDTWAIN_API::DTWAIN_EnumDoubleFeedDetectValues = nullptr;
    D_ENUMEXTENDEDCAPSEX2FUNC                                         DYNDTWAIN_API::DTWAIN_EnumExtendedCapsEx2 = nullptr;
    D_ENUMEXTENDEDCAPSEXFUNC                                          DYNDTWAIN_API::DTWAIN_EnumExtendedCapsEx = nullptr;
    D_ENUMEXTENDEDCAPSFUNC                                            DYNDTWAIN_API::DTWAIN_EnumExtendedCaps = nullptr;
    D_ENUMEXTIMAGEINFOTYPESFUNC                                       DYNDTWAIN_API::DTWAIN_EnumExtImageInfoTypes = nullptr;
    D_ENUMFILETYPEBITSPERPIXELFUNC                                    DYNDTWAIN_API::DTWAIN_EnumFileTypeBitsPerPixel = nullptr;
    D_ENUMFILEXFERFORMATSEXFUNC                                       DYNDTWAIN_API::DTWAIN_EnumFileXferFormatsEx = nullptr;
    D_ENUMFILEXFERFORMATSFUNC                                         DYNDTWAIN_API::DTWAIN_EnumFileXferFormats = nullptr;
    D_ENUMHALFTONESEXFUNC                                             DYNDTWAIN_API::DTWAIN_EnumHalftonesEx = nullptr;
    D_ENUMHALFTONESFUNC                                               DYNDTWAIN_API::DTWAIN_EnumHalftones = nullptr;
    D_ENUMHIGHLIGHTVALUESEXFUNC                                       DYNDTWAIN_API::DTWAIN_EnumHighlightValuesEx = nullptr;
    D_ENUMHIGHLIGHTVALUESFUNC                                         DYNDTWAIN_API::DTWAIN_EnumHighlightValues = nullptr;
    D_ENUMJOBCONTROLSEXFUNC                                           DYNDTWAIN_API::DTWAIN_EnumJobControlsEx = nullptr;
    D_ENUMJOBCONTROLSFUNC                                             DYNDTWAIN_API::DTWAIN_EnumJobControls = nullptr;
    D_ENUMLIGHTPATHSEXFUNC                                            DYNDTWAIN_API::DTWAIN_EnumLightPathsEx = nullptr;
    D_ENUMLIGHTPATHSFUNC                                              DYNDTWAIN_API::DTWAIN_EnumLightPaths = nullptr;
    D_ENUMLIGHTSOURCESEXFUNC                                          DYNDTWAIN_API::DTWAIN_EnumLightSourcesEx = nullptr;
    D_ENUMLIGHTSOURCESFUNC                                            DYNDTWAIN_API::DTWAIN_EnumLightSources = nullptr;
    D_ENUMMAXBUFFERSEXFUNC                                            DYNDTWAIN_API::DTWAIN_EnumMaxBuffersEx = nullptr;
    D_ENUMMAXBUFFERSFUNC                                              DYNDTWAIN_API::DTWAIN_EnumMaxBuffers = nullptr;
    D_ENUMNOISEFILTERSEXFUNC                                          DYNDTWAIN_API::DTWAIN_EnumNoiseFiltersEx = nullptr;
    D_ENUMNOISEFILTERSFUNC                                            DYNDTWAIN_API::DTWAIN_EnumNoiseFilters = nullptr;
    D_ENUMOCRINTERFACESFUNC                                           DYNDTWAIN_API::DTWAIN_EnumOCRInterfaces = nullptr;
    D_ENUMOCRSUPPORTEDCAPSFUNC                                        DYNDTWAIN_API::DTWAIN_EnumOCRSupportedCaps = nullptr;
    D_ENUMORIENTATIONSEXFUNC                                          DYNDTWAIN_API::DTWAIN_EnumOrientationsEx = nullptr;
    D_ENUMORIENTATIONSFUNC                                            DYNDTWAIN_API::DTWAIN_EnumOrientations = nullptr;
    D_ENUMOVERSCANVALUESEXFUNC                                        DYNDTWAIN_API::DTWAIN_EnumOverscanValuesEx = nullptr;
    D_ENUMOVERSCANVALUESFUNC                                          DYNDTWAIN_API::DTWAIN_EnumOverscanValues = nullptr;
    D_ENUMPAPERSIZESEXFUNC                                            DYNDTWAIN_API::DTWAIN_EnumPaperSizesEx = nullptr;
    D_ENUMPAPERSIZESFUNC                                              DYNDTWAIN_API::DTWAIN_EnumPaperSizes = nullptr;
    D_ENUMPATCHCODESEXFUNC                                            DYNDTWAIN_API::DTWAIN_EnumPatchCodesEx = nullptr;
    D_ENUMPATCHCODESFUNC                                              DYNDTWAIN_API::DTWAIN_EnumPatchCodes = nullptr;
    D_ENUMPATCHMAXPRIORITIESEXFUNC                                    DYNDTWAIN_API::DTWAIN_EnumPatchMaxPrioritiesEx = nullptr;
    D_ENUMPATCHMAXPRIORITIESFUNC                                      DYNDTWAIN_API::DTWAIN_EnumPatchMaxPriorities = nullptr;
    D_ENUMPATCHMAXRETRIESEXFUNC                                       DYNDTWAIN_API::DTWAIN_EnumPatchMaxRetriesEx = nullptr;
    D_ENUMPATCHMAXRETRIESFUNC                                         DYNDTWAIN_API::DTWAIN_EnumPatchMaxRetries = nullptr;
    D_ENUMPATCHPRIORITIESEXFUNC                                       DYNDTWAIN_API::DTWAIN_EnumPatchPrioritiesEx = nullptr;
    D_ENUMPATCHPRIORITIESFUNC                                         DYNDTWAIN_API::DTWAIN_EnumPatchPriorities = nullptr;
    D_ENUMPATCHSEARCHMODESEXFUNC                                      DYNDTWAIN_API::DTWAIN_EnumPatchSearchModesEx = nullptr;
    D_ENUMPATCHSEARCHMODESFUNC                                        DYNDTWAIN_API::DTWAIN_EnumPatchSearchModes = nullptr;
    D_ENUMPATCHTIMEOUTVALUESEXFUNC                                    DYNDTWAIN_API::DTWAIN_EnumPatchTimeOutValuesEx = nullptr;
    D_ENUMPATCHTIMEOUTVALUESFUNC                                      DYNDTWAIN_API::DTWAIN_EnumPatchTimeOutValues = nullptr;
    D_ENUMPIXELTYPESFUNC                                              DYNDTWAIN_API::DTWAIN_EnumPixelTypes = nullptr;
    D_ENUMPRINTERSTRINGMODESEXFUNC                                    DYNDTWAIN_API::DTWAIN_EnumPrinterStringModesEx = nullptr;
    D_ENUMPRINTERSTRINGMODESFUNC                                      DYNDTWAIN_API::DTWAIN_EnumPrinterStringModes = nullptr;
    D_ENUMRESOLUTIONVALUESEXFUNC                                      DYNDTWAIN_API::DTWAIN_EnumResolutionValuesEx = nullptr;
    D_ENUMRESOLUTIONVALUESFUNC                                        DYNDTWAIN_API::DTWAIN_EnumResolutionValues = nullptr;
    D_ENUMSHADOWVALUESEXFUNC                                          DYNDTWAIN_API::DTWAIN_EnumShadowValuesEx = nullptr;
    D_ENUMSHADOWVALUESFUNC                                            DYNDTWAIN_API::DTWAIN_EnumShadowValues = nullptr;
    D_ENUMSOURCESEXFUNC                                               DYNDTWAIN_API::DTWAIN_EnumSourcesEx = nullptr;
    D_ENUMSOURCESFUNC                                                 DYNDTWAIN_API::DTWAIN_EnumSources = nullptr;
    D_ENUMSOURCEUNITSEXFUNC                                           DYNDTWAIN_API::DTWAIN_EnumSourceUnitsEx = nullptr;
    D_ENUMSOURCEUNITSFUNC                                             DYNDTWAIN_API::DTWAIN_EnumSourceUnits = nullptr;
    D_ENUMSOURCEVALUESAFUNC                                           DYNDTWAIN_API::DTWAIN_EnumSourceValuesA = nullptr;
    D_ENUMSOURCEVALUESFUNC                                            DYNDTWAIN_API::DTWAIN_EnumSourceValues = nullptr;
    D_ENUMSOURCEVALUESWFUNC                                           DYNDTWAIN_API::DTWAIN_EnumSourceValuesW = nullptr;
    D_ENUMSUPPORTEDCAPSEX2FUNC                                        DYNDTWAIN_API::DTWAIN_EnumSupportedCapsEx2 = nullptr;
    D_ENUMSUPPORTEDCAPSEXFUNC                                         DYNDTWAIN_API::DTWAIN_EnumSupportedCapsEx = nullptr;
    D_ENUMSUPPORTEDCAPSFUNC                                           DYNDTWAIN_API::DTWAIN_EnumSupportedCaps = nullptr;
    D_ENUMSUPPORTEDSINGLEPAGEFILETYPES                                DYNDTWAIN_API::DTWAIN_EnumSupportedSinglePageFileTypes = nullptr;
    D_ENUMSUPPORTEDMULTIPAGEFILETYPES                                 DYNDTWAIN_API::DTWAIN_EnumSupportedMultiPageFileTypes = nullptr;
    D_ENUMTHRESHOLDVALUESEXFUNC                                       DYNDTWAIN_API::DTWAIN_EnumThresholdValuesEx = nullptr;
    D_ENUMTHRESHOLDVALUESFUNC                                         DYNDTWAIN_API::DTWAIN_EnumThresholdValues = nullptr;
    D_ENUMTOPCAMERASFUNC                                              DYNDTWAIN_API::DTWAIN_EnumTopCameras = nullptr;
    D_ENUMTWAINPRINTERSARRAYEXFUNC                                    DYNDTWAIN_API::DTWAIN_EnumTwainPrintersArrayEx = nullptr;
    D_ENUMTWAINPRINTERSARRAYFUNC                                      DYNDTWAIN_API::DTWAIN_EnumTwainPrintersArray = nullptr;
    D_ENUMTWAINPRINTERSEXFUNC                                         DYNDTWAIN_API::DTWAIN_EnumTwainPrintersEx = nullptr;
    D_ENUMTWAINPRINTERSFUNC                                           DYNDTWAIN_API::DTWAIN_EnumTwainPrinters = nullptr;
    D_EXECUTEOCRAFUNC                                                 DYNDTWAIN_API::DTWAIN_ExecuteOCRA = nullptr;
    D_EXECUTEOCRFUNC                                                  DYNDTWAIN_API::DTWAIN_ExecuteOCR = nullptr;
    D_EXECUTEOCRWFUNC                                                 DYNDTWAIN_API::DTWAIN_ExecuteOCRW = nullptr;
    D_FEEDPAGEFUNC                                                    DYNDTWAIN_API::DTWAIN_FeedPage = nullptr;
    D_FLIPBITMAPFUNC                                                  DYNDTWAIN_API::DTWAIN_FlipBitmap = nullptr;
    D_FLUSHACQUIREDPAGESFUNC                                          DYNDTWAIN_API::DTWAIN_FlushAcquiredPages = nullptr;
    D_FORCEACQUIREBITDEPTHFUNC                                        DYNDTWAIN_API::DTWAIN_ForceAcquireBitDepth = nullptr;
    D_FORCESCANONNOUIFUNC                                             DYNDTWAIN_API::DTWAIN_ForceScanOnNoUI = nullptr;
    D_FRAMECREATEFUNC                                                 DYNDTWAIN_API::DTWAIN_FrameCreate = nullptr;
    D_FRAMECREATESTRINGAFUNC                                          DYNDTWAIN_API::DTWAIN_FrameCreateStringA = nullptr;
    D_FRAMECREATESTRINGFUNC                                           DYNDTWAIN_API::DTWAIN_FrameCreateString = nullptr;
    D_FRAMECREATESTRINGWFUNC                                          DYNDTWAIN_API::DTWAIN_FrameCreateStringW = nullptr;
    D_FRAMEDESTROYFUNC                                                DYNDTWAIN_API::DTWAIN_FrameDestroy = nullptr;
    D_FRAMEGETALLFUNC                                                 DYNDTWAIN_API::DTWAIN_FrameGetAll = nullptr;
    D_FRAMEGETALLSTRINGAFUNC                                          DYNDTWAIN_API::DTWAIN_FrameGetAllStringA = nullptr;
    D_FRAMEGETALLSTRINGFUNC                                           DYNDTWAIN_API::DTWAIN_FrameGetAllString = nullptr;
    D_FRAMEGETALLSTRINGWFUNC                                          DYNDTWAIN_API::DTWAIN_FrameGetAllStringW = nullptr;
    D_FRAMEGETVALUEFUNC                                               DYNDTWAIN_API::DTWAIN_FrameGetValue = nullptr;
    D_FRAMEGETVALUESTRINGAFUNC                                        DYNDTWAIN_API::DTWAIN_FrameGetValueStringA = nullptr;
    D_FRAMEGETVALUESTRINGFUNC                                         DYNDTWAIN_API::DTWAIN_FrameGetValueString = nullptr;
    D_FRAMEGETVALUESTRINGWFUNC                                        DYNDTWAIN_API::DTWAIN_FrameGetValueStringW = nullptr;
    D_FRAMEISVALIDFUNC                                                DYNDTWAIN_API::DTWAIN_FrameIsValid = nullptr;
    D_FRAMESETALLFUNC                                                 DYNDTWAIN_API::DTWAIN_FrameSetAll = nullptr;
    D_FRAMESETALLSTRINGAFUNC                                          DYNDTWAIN_API::DTWAIN_FrameSetAllStringA = nullptr;
    D_FRAMESETALLSTRINGFUNC                                           DYNDTWAIN_API::DTWAIN_FrameSetAllString = nullptr;
    D_FRAMESETALLSTRINGWFUNC                                          DYNDTWAIN_API::DTWAIN_FrameSetAllStringW = nullptr;
    D_FRAMESETVALUEFUNC                                               DYNDTWAIN_API::DTWAIN_FrameSetValue = nullptr;
    D_FRAMESETVALUESTRINGAFUNC                                        DYNDTWAIN_API::DTWAIN_FrameSetValueStringA = nullptr;
    D_FRAMESETVALUESTRINGFUNC                                         DYNDTWAIN_API::DTWAIN_FrameSetValueString = nullptr;
    D_FRAMESETVALUESTRINGWFUNC                                        DYNDTWAIN_API::DTWAIN_FrameSetValueStringW = nullptr;
    D_FREEEXTIMAGEINFOFUNC                                            DYNDTWAIN_API::DTWAIN_FreeExtImageInfo = nullptr;
    D_FREEMEMORYEXFUNC                                                DYNDTWAIN_API::DTWAIN_FreeMemoryEx = nullptr;
    D_FREEMEMORYFUNC                                                  DYNDTWAIN_API::DTWAIN_FreeMemory = nullptr;
    D_GETACQUIREAREA2FUNC                                             DYNDTWAIN_API::DTWAIN_GetAcquireArea2 = nullptr;
    D_GETACQUIREAREA2STRINGAFUNC                                      DYNDTWAIN_API::DTWAIN_GetAcquireArea2StringA = nullptr;
    D_GETACQUIREAREA2STRINGFUNC                                       DYNDTWAIN_API::DTWAIN_GetAcquireArea2String = nullptr;
    D_GETACQUIREAREA2STRINGWFUNC                                      DYNDTWAIN_API::DTWAIN_GetAcquireArea2StringW = nullptr;
    D_GETACQUIREAREAFUNC                                              DYNDTWAIN_API::DTWAIN_GetAcquireArea = nullptr;
    D_GETACQUIREDIMAGEARRAYFUNC                                       DYNDTWAIN_API::DTWAIN_GetAcquiredImageArray = nullptr;
    D_GETACQUIREDIMAGEFUNC                                            DYNDTWAIN_API::DTWAIN_GetAcquiredImage = nullptr;
    D_GETACQUIREMETRICSFUNC                                           DYNDTWAIN_API::DTWAIN_GetAcquireMetrics = nullptr;
    D_GETACQUIRESTRIPBUFFERFUNC                                       DYNDTWAIN_API::DTWAIN_GetAcquireStripBuffer = nullptr;
    D_GETACQUIRESTRIPDATAFUNC                                         DYNDTWAIN_API::DTWAIN_GetAcquireStripData = nullptr;
    D_GETACQUIRESTRIPSIZESFUNC                                        DYNDTWAIN_API::DTWAIN_GetAcquireStripSizes = nullptr;
    D_GETALARMVOLUMEFUNC                                              DYNDTWAIN_API::DTWAIN_GetAlarmVolume = nullptr;
    D_GETAPIHANDLESTATUS                                              DYNDTWAIN_API::DTWAIN_GetAPIHandleStatus = nullptr;
    D_GETAPPINFOAFUNC                                                 DYNDTWAIN_API::DTWAIN_GetAppInfoA = nullptr;
    D_GETAPPINFOFUNC                                                  DYNDTWAIN_API::DTWAIN_GetAppInfo = nullptr;
    D_GETAPPINFOWFUNC                                                 DYNDTWAIN_API::DTWAIN_GetAppInfoW = nullptr;
    D_GETAUTHORAFUNC                                                  DYNDTWAIN_API::DTWAIN_GetAuthorA = nullptr;
    D_GETAUTHORFUNC                                                   DYNDTWAIN_API::DTWAIN_GetAuthor = nullptr;
    D_GETAUTHORWFUNC                                                  DYNDTWAIN_API::DTWAIN_GetAuthorW = nullptr;
    D_GETBATTERYMINUTESFUNC                                           DYNDTWAIN_API::DTWAIN_GetBatteryMinutes = nullptr;
    D_GETBATTERYPERCENTFUNC                                           DYNDTWAIN_API::DTWAIN_GetBatteryPercent = nullptr;
    D_GETBITDEPTHFUNC                                                 DYNDTWAIN_API::DTWAIN_GetBitDepth = nullptr;
    D_GETBLANKPAGEAUTODETECTIONFUNC                                   DYNDTWAIN_API::DTWAIN_GetBlankPageAutoDetection = nullptr;
    D_GETBRIGHTNESSFUNC                                               DYNDTWAIN_API::DTWAIN_GetBrightness = nullptr;
    D_GETBRIGHTNESSSTRINGAFUNC                                        DYNDTWAIN_API::DTWAIN_GetBrightnessStringA = nullptr;
    D_GETBRIGHTNESSSTRINGFUNC                                         DYNDTWAIN_API::DTWAIN_GetBrightnessString = nullptr;
    D_GETBRIGHTNESSSTRINGWFUNC                                        DYNDTWAIN_API::DTWAIN_GetBrightnessStringW = nullptr;
    D_GETCALLBACK64FUNC                                               DYNDTWAIN_API::DTWAIN_GetCallback64 = nullptr;
    D_GETCALLBACKFUNC                                                 DYNDTWAIN_API::DTWAIN_GetCallback = nullptr;
    D_GETCAPARRAYTYPEFUNC                                             DYNDTWAIN_API::DTWAIN_GetCapArrayType = nullptr;
    D_GETCAPCONTAINEREXFUNC                                           DYNDTWAIN_API::DTWAIN_GetCapContainerEx = nullptr;
    D_GETCAPCONTAINERFUNC                                             DYNDTWAIN_API::DTWAIN_GetCapContainer = nullptr;
    D_GETCAPDATATYPEFUNC                                              DYNDTWAIN_API::DTWAIN_GetCapDataType = nullptr;
    D_GETCAPFROMNAMEAFUNC                                             DYNDTWAIN_API::DTWAIN_GetCapFromNameA = nullptr;
    D_GETCAPFROMNAMEFUNC                                              DYNDTWAIN_API::DTWAIN_GetCapFromName = nullptr;
    D_GETCAPFROMNAMEWFUNC                                             DYNDTWAIN_API::DTWAIN_GetCapFromNameW = nullptr;
    D_GETCAPOPERATIONSFUNC                                            DYNDTWAIN_API::DTWAIN_GetCapOperations = nullptr;
    D_GETCAPTIONAFUNC                                                 DYNDTWAIN_API::DTWAIN_GetCaptionA = nullptr;
    D_GETCAPTIONFUNC                                                  DYNDTWAIN_API::DTWAIN_GetCaption = nullptr;
    D_GETCAPTIONWFUNC                                                 DYNDTWAIN_API::DTWAIN_GetCaptionW = nullptr;
    D_GETCAPVALUESEX2FUNC                                             DYNDTWAIN_API::DTWAIN_GetCapValuesEx2 = nullptr;
    D_GETCAPVALUESEXFUNC                                              DYNDTWAIN_API::DTWAIN_GetCapValuesEx = nullptr;
    D_GETCAPVALUESFUNC                                                DYNDTWAIN_API::DTWAIN_GetCapValues = nullptr;
    D_GETCOMPRESSIONSIZEFUNC                                          DYNDTWAIN_API::DTWAIN_GetCompressionSize = nullptr;
    D_GETCOMPRESSIONTYPEFUNC                                          DYNDTWAIN_API::DTWAIN_GetCompressionType = nullptr;
    D_GETCONDITIONCODESTRINGAFUNC                                     DYNDTWAIN_API::DTWAIN_GetConditionCodeStringA = nullptr;
    D_GETCONDITIONCODESTRINGFUNC                                      DYNDTWAIN_API::DTWAIN_GetConditionCodeString = nullptr;
    D_GETCONDITIONCODESTRINGWFUNC                                     DYNDTWAIN_API::DTWAIN_GetConditionCodeStringW = nullptr;
    D_GETCONTRASTFUNC                                                 DYNDTWAIN_API::DTWAIN_GetContrast = nullptr;
    D_GETCONTRASTSTRINGAFUNC                                          DYNDTWAIN_API::DTWAIN_GetContrastStringA = nullptr;
    D_GETCONTRASTSTRINGFUNC                                           DYNDTWAIN_API::DTWAIN_GetContrastString = nullptr;
    D_GETCONTRASTSTRINGWFUNC                                          DYNDTWAIN_API::DTWAIN_GetContrastStringW = nullptr;
    D_GETCOUNTRYFUNC                                                  DYNDTWAIN_API::DTWAIN_GetCountry = nullptr;
    D_GETCURRENTACQUIREDIMAGEFUNC                                     DYNDTWAIN_API::DTWAIN_GetCurrentAcquiredImage = nullptr;
    D_GETCURRENTFILENAMEAFUNC                                         DYNDTWAIN_API::DTWAIN_GetCurrentFileNameA = nullptr;
    D_GETCURRENTFILENAMEFUNC                                          DYNDTWAIN_API::DTWAIN_GetCurrentFileName = nullptr;
    D_GETCURRENTFILENAMEWFUNC                                         DYNDTWAIN_API::DTWAIN_GetCurrentFileNameW = nullptr;
    D_GETCURRENTPAGENUMFUNC                                           DYNDTWAIN_API::DTWAIN_GetCurrentPageNum = nullptr;
    D_GETCURRENTRETRYCOUNTFUNC                                        DYNDTWAIN_API::DTWAIN_GetCurrentRetryCount = nullptr;
    D_GETCUSTOMDSDATAFUNC                                             DYNDTWAIN_API::DTWAIN_GetCustomDSData = nullptr;
    D_GETDEVICEEVENTEXFUNC                                            DYNDTWAIN_API::DTWAIN_GetDeviceEventEx = nullptr;
    D_GETDEVICEEVENTFUNC                                              DYNDTWAIN_API::DTWAIN_GetDeviceEvent = nullptr;
    D_GETDEVICEEVENTINFOFUNC                                          DYNDTWAIN_API::DTWAIN_GetDeviceEventInfo = nullptr;
    D_GETDEVICENOTIFICATIONSFUNC                                      DYNDTWAIN_API::DTWAIN_GetDeviceNotifications = nullptr;
    D_GETDEVICETIMEDATEAFUNC                                          DYNDTWAIN_API::DTWAIN_GetDeviceTimeDateA = nullptr;
    D_GETDEVICETIMEDATEFUNC                                           DYNDTWAIN_API::DTWAIN_GetDeviceTimeDate = nullptr;
    D_GETDEVICETIMEDATEWFUNC                                          DYNDTWAIN_API::DTWAIN_GetDeviceTimeDateW = nullptr;
    D_GETDOUBLEFEEDDETECTLENGTHFUNC                                   DYNDTWAIN_API::DTWAIN_GetDoubleFeedDetectLength = nullptr;
    D_GETDOUBLEFEEDDETECTVALUESFUNC                                   DYNDTWAIN_API::DTWAIN_GetDoubleFeedDetectValues = nullptr;
    D_GETDSMFULLNAMEAFUNC                                             DYNDTWAIN_API::DTWAIN_GetDSMFullNameA = nullptr;
    D_GETDSMFULLNAMEFUNC                                              DYNDTWAIN_API::DTWAIN_GetDSMFullName = nullptr;
    D_GETDSMFULLNAMEWFUNC                                             DYNDTWAIN_API::DTWAIN_GetDSMFullNameW = nullptr;
    D_GETDSMSEARCHORDERFUNC                                           DYNDTWAIN_API::DTWAIN_GetDSMSearchOrder = nullptr;
    D_GETDTWAINHANDLEFUNC                                             DYNDTWAIN_API::DTWAIN_GetDTWAINHandle = nullptr;
    D_GETDUPLEXTYPEFUNC                                               DYNDTWAIN_API::DTWAIN_GetDuplexType = nullptr;
    D_GETERRORBUFFERFUNC                                              DYNDTWAIN_API::DTWAIN_GetErrorBuffer = nullptr;
    D_GETERRORBUFFERTHRESHOLDFUNC                                     DYNDTWAIN_API::DTWAIN_GetErrorBufferThreshold = nullptr;
    D_GETERRORCALLBACK64FUNC                                          DYNDTWAIN_API::DTWAIN_GetErrorCallback64 = nullptr;
    D_GETERRORCALLBACKFUNC                                            DYNDTWAIN_API::DTWAIN_GetErrorCallback = nullptr;
    D_GETERRORSTRINGAFUNC                                             DYNDTWAIN_API::DTWAIN_GetErrorStringA = nullptr;
    D_GETERRORSTRINGFUNC                                              DYNDTWAIN_API::DTWAIN_GetErrorString = nullptr;
    D_GETERRORSTRINGWFUNC                                             DYNDTWAIN_API::DTWAIN_GetErrorStringW = nullptr;
    D_GETEXTCAPFROMNAMEAFUNC                                          DYNDTWAIN_API::DTWAIN_GetExtCapFromNameA = nullptr;
    D_GETEXTCAPFROMNAMEFUNC                                           DYNDTWAIN_API::DTWAIN_GetExtCapFromName = nullptr;
    D_GETEXTCAPFROMNAMEWFUNC                                          DYNDTWAIN_API::DTWAIN_GetExtCapFromNameW = nullptr;
    D_GETEXTIMAGEINFODATAFUNC                                         DYNDTWAIN_API::DTWAIN_GetExtImageInfoData = nullptr;
    D_GETEXTIMAGEINFOFUNC                                             DYNDTWAIN_API::DTWAIN_GetExtImageInfo = nullptr;
    D_GETEXTIMAGEINFOITEMFUNC                                         DYNDTWAIN_API::DTWAIN_GetExtImageInfoItem = nullptr;
    D_GETEXTNAMEFROMCAPAFUNC                                          DYNDTWAIN_API::DTWAIN_GetExtNameFromCapA = nullptr;
    D_GETEXTNAMEFROMCAPFUNC                                           DYNDTWAIN_API::DTWAIN_GetExtNameFromCap = nullptr;
    D_GETEXTNAMEFROMCAPWFUNC                                          DYNDTWAIN_API::DTWAIN_GetExtNameFromCapW = nullptr;
    D_GETFEEDERALIGNMENTFUNC                                          DYNDTWAIN_API::DTWAIN_GetFeederAlignment = nullptr;
    D_GETFEEDERFUNCSFUNC                                              DYNDTWAIN_API::DTWAIN_GetFeederFuncs = nullptr;
    D_GETFEEDERORDERFUNC                                              DYNDTWAIN_API::DTWAIN_GetFeederOrder = nullptr;
    D_GETFILETYPENAME                                                 DYNDTWAIN_API::DTWAIN_GetFileTypeName = nullptr;
    D_GETFILETYPENAMEA                                                DYNDTWAIN_API::DTWAIN_GetFileTypeNameA = nullptr;
    D_GETFILETYPENAMEW                                                DYNDTWAIN_API::DTWAIN_GetFileTypeNameW = nullptr;
    D_GETFILETYPEEXTENSIONS                                           DYNDTWAIN_API::DTWAIN_GetFileTypeExtensions = nullptr;
    D_GETFILETYPEEXTENSIONSA                                          DYNDTWAIN_API::DTWAIN_GetFileTypeExtensionsA = nullptr;
    D_GETFILETYPEEXTENSIONSW                                          DYNDTWAIN_API::DTWAIN_GetFileTypeExtensionsW = nullptr;
    D_GETHALFTONEAFUNC                                                DYNDTWAIN_API::DTWAIN_GetHalftoneA = nullptr;
    D_GETHALFTONEFUNC                                                 DYNDTWAIN_API::DTWAIN_GetHalftone = nullptr;
    D_GETHALFTONEWFUNC                                                DYNDTWAIN_API::DTWAIN_GetHalftoneW = nullptr;
    D_GETHIGHLIGHTFUNC                                                DYNDTWAIN_API::DTWAIN_GetHighlight = nullptr;
    D_GETHIGHLIGHTSTRINGAFUNC                                         DYNDTWAIN_API::DTWAIN_GetHighlightStringA = nullptr;
    D_GETHIGHLIGHTSTRINGFUNC                                          DYNDTWAIN_API::DTWAIN_GetHighlightString = nullptr;
    D_GETHIGHLIGHTSTRINGWFUNC                                         DYNDTWAIN_API::DTWAIN_GetHighlightStringW = nullptr;
    D_GETIMAGEINFOFUNC                                                DYNDTWAIN_API::DTWAIN_GetImageInfo = nullptr;
    D_GETIMAGEINFOSTRINGAFUNC                                         DYNDTWAIN_API::DTWAIN_GetImageInfoStringA = nullptr;
    D_GETIMAGEINFOSTRINGFUNC                                          DYNDTWAIN_API::DTWAIN_GetImageInfoString = nullptr;
    D_GETIMAGEINFOSTRINGWFUNC                                         DYNDTWAIN_API::DTWAIN_GetImageInfoStringW = nullptr;
    D_GETJOBCONTROLFUNC                                               DYNDTWAIN_API::DTWAIN_GetJobControl = nullptr;
    D_GETJPEGVALUESFUNC                                               DYNDTWAIN_API::DTWAIN_GetJpegValues = nullptr;
    D_GETLANGUAGEFUNC                                                 DYNDTWAIN_API::DTWAIN_GetLanguage = nullptr;
    D_GETLASTERRORFUNC                                                DYNDTWAIN_API::DTWAIN_GetLastError = nullptr;
    D_GETLIBRARYPATHAFUNC                                             DYNDTWAIN_API::DTWAIN_GetLibraryPathA = nullptr;
    D_GETLIBRARYPATHFUNC                                              DYNDTWAIN_API::DTWAIN_GetLibraryPath = nullptr;
    D_GETLIBRARYPATHWFUNC                                             DYNDTWAIN_API::DTWAIN_GetLibraryPathW = nullptr;
    D_GETLIGHTPATHFUNC                                                DYNDTWAIN_API::DTWAIN_GetLightPath = nullptr;
    D_GETLIGHTSOURCEFUNC                                              DYNDTWAIN_API::DTWAIN_GetLightSource = nullptr;
    D_GETLIGHTSOURCESFUNC                                             DYNDTWAIN_API::DTWAIN_GetLightSources = nullptr;
    D_GETLOGGERCALLBACKAFUNC                                          DYNDTWAIN_API::DTWAIN_GetLoggerCallbackA = nullptr;
    D_GETLOGGERCALLBACKFUNC                                           DYNDTWAIN_API::DTWAIN_GetLoggerCallback = nullptr;
    D_GETLOGGERCALLBACKWFUNC                                          DYNDTWAIN_API::DTWAIN_GetLoggerCallbackW = nullptr;
    D_GETMANUALDUPLEXCOUNTFUNC                                        DYNDTWAIN_API::DTWAIN_GetManualDuplexCount = nullptr;
    D_GETMAXACQUISITIONSFUNC                                          DYNDTWAIN_API::DTWAIN_GetMaxAcquisitions = nullptr;
    D_GETMAXBUFFERSFUNC                                               DYNDTWAIN_API::DTWAIN_GetMaxBuffers = nullptr;
    D_GETMAXPAGESTOACQUIREFUNC                                        DYNDTWAIN_API::DTWAIN_GetMaxPagesToAcquire = nullptr;
    D_GETMAXRETRYATTEMPTSFUNC                                         DYNDTWAIN_API::DTWAIN_GetMaxRetryAttempts = nullptr;
    D_GETNAMEFROMCAPAFUNC                                             DYNDTWAIN_API::DTWAIN_GetNameFromCapA = nullptr;
    D_GETNAMEFROMCAPFUNC                                              DYNDTWAIN_API::DTWAIN_GetNameFromCap = nullptr;
    D_GETNAMEFROMCAPWFUNC                                             DYNDTWAIN_API::DTWAIN_GetNameFromCapW = nullptr;
    D_GETNOISEFILTERFUNC                                              DYNDTWAIN_API::DTWAIN_GetNoiseFilter = nullptr;
    D_GETNUMACQUIREDIMAGESFUNC                                        DYNDTWAIN_API::DTWAIN_GetNumAcquiredImages = nullptr;
    D_GETNUMACQUISITIONSFUNC                                          DYNDTWAIN_API::DTWAIN_GetNumAcquisitions = nullptr;
    D_GETOCRCAPVALUESFUNC                                             DYNDTWAIN_API::DTWAIN_GetOCRCapValues = nullptr;
    D_GETOCRERRORSTRINGAFUNC                                          DYNDTWAIN_API::DTWAIN_GetOCRErrorStringA = nullptr;
    D_GETOCRERRORSTRINGFUNC                                           DYNDTWAIN_API::DTWAIN_GetOCRErrorString = nullptr;
    D_GETOCRERRORSTRINGWFUNC                                          DYNDTWAIN_API::DTWAIN_GetOCRErrorStringW = nullptr;
    D_GETOCRLASTERRORFUNC                                             DYNDTWAIN_API::DTWAIN_GetOCRLastError = nullptr;
    D_GETOCRMANUFACTURERAFUNC                                         DYNDTWAIN_API::DTWAIN_GetOCRManufacturerA = nullptr;
    D_GETOCRMANUFACTURERFUNC                                          DYNDTWAIN_API::DTWAIN_GetOCRManufacturer = nullptr;
    D_GETOCRMANUFACTURERWFUNC                                         DYNDTWAIN_API::DTWAIN_GetOCRManufacturerW = nullptr;
    D_GETOCRPRODUCTFAMILYAFUNC                                        DYNDTWAIN_API::DTWAIN_GetOCRProductFamilyA = nullptr;
    D_GETOCRPRODUCTFAMILYFUNC                                         DYNDTWAIN_API::DTWAIN_GetOCRProductFamily = nullptr;
    D_GETOCRPRODUCTFAMILYWFUNC                                        DYNDTWAIN_API::DTWAIN_GetOCRProductFamilyW = nullptr;
    D_GETOCRPRODUCTNAMEAFUNC                                          DYNDTWAIN_API::DTWAIN_GetOCRProductNameA = nullptr;
    D_GETOCRPRODUCTNAMEFUNC                                           DYNDTWAIN_API::DTWAIN_GetOCRProductName = nullptr;
    D_GETOCRPRODUCTNAMEWFUNC                                          DYNDTWAIN_API::DTWAIN_GetOCRProductNameW = nullptr;
    D_GETOCRTEXTAFUNC                                                 DYNDTWAIN_API::DTWAIN_GetOCRTextA = nullptr;
    D_GETOCRTEXTFUNC                                                  DYNDTWAIN_API::DTWAIN_GetOCRText = nullptr;
    D_GETOCRTEXTINFOFLOATEXFUNC                                       DYNDTWAIN_API::DTWAIN_GetOCRTextInfoFloatEx = nullptr;
    D_GETOCRTEXTINFOFLOATFUNC                                         DYNDTWAIN_API::DTWAIN_GetOCRTextInfoFloat = nullptr;
    D_GETOCRTEXTINFOHANDLEFUNC                                        DYNDTWAIN_API::DTWAIN_GetOCRTextInfoHandle = nullptr;
    D_GETOCRTEXTINFOLONGEXFUNC                                        DYNDTWAIN_API::DTWAIN_GetOCRTextInfoLongEx = nullptr;
    D_GETOCRTEXTINFOLONGFUNC                                          DYNDTWAIN_API::DTWAIN_GetOCRTextInfoLong = nullptr;
    D_GETOCRTEXTWFUNC                                                 DYNDTWAIN_API::DTWAIN_GetOCRTextW = nullptr;
    D_GETOCRVERSIONINFOAFUNC                                          DYNDTWAIN_API::DTWAIN_GetOCRVersionInfoA = nullptr;
    D_GETOCRVERSIONINFOFUNC                                           DYNDTWAIN_API::DTWAIN_GetOCRVersionInfo = nullptr;
    D_GETOCRVERSIONINFOWFUNC                                          DYNDTWAIN_API::DTWAIN_GetOCRVersionInfoW = nullptr;
    D_GETORIENTATIONFUNC                                              DYNDTWAIN_API::DTWAIN_GetOrientation = nullptr;
    D_GETOVERSCANFUNC                                                 DYNDTWAIN_API::DTWAIN_GetOverscan = nullptr;
    D_GETPAPERSIZEFUNC                                                DYNDTWAIN_API::DTWAIN_GetPaperSize = nullptr;
    D_GETPATCHMAXPRIORITIESFUNC                                       DYNDTWAIN_API::DTWAIN_GetPatchMaxPriorities = nullptr;
    D_GETPATCHMAXRETRIESFUNC                                          DYNDTWAIN_API::DTWAIN_GetPatchMaxRetries = nullptr;
    D_GETPATCHPRIORITIESFUNC                                          DYNDTWAIN_API::DTWAIN_GetPatchPriorities = nullptr;
    D_GETPATCHSEARCHMODEFUNC                                          DYNDTWAIN_API::DTWAIN_GetPatchSearchMode = nullptr;
    D_GETPATCHTIMEOUTFUNC                                             DYNDTWAIN_API::DTWAIN_GetPatchTimeOut = nullptr;
    D_GETPDFTEXTELEMENTFLOATFUNC                                      DYNDTWAIN_API::DTWAIN_GetPDFTextElementFloat = nullptr;
    D_GETPDFTEXTELEMENTLONGFUNC                                       DYNDTWAIN_API::DTWAIN_GetPDFTextElementLong = nullptr;
    D_GETPDFTEXTELEMENTSTRINGAFUNC                                    DYNDTWAIN_API::DTWAIN_GetPDFTextElementStringA = nullptr;
    D_GETPDFTEXTELEMENTSTRINGFUNC                                     DYNDTWAIN_API::DTWAIN_GetPDFTextElementString = nullptr;
    D_GETPDFTEXTELEMENTSTRINGWFUNC                                    DYNDTWAIN_API::DTWAIN_GetPDFTextElementStringW = nullptr;
    D_GETPDFTYPE1FONTNAMEAFUNC                                        DYNDTWAIN_API::DTWAIN_GetPDFType1FontNameA = nullptr;
    D_GETPDFTYPE1FONTNAMEFUNC                                         DYNDTWAIN_API::DTWAIN_GetPDFType1FontName = nullptr;
    D_GETPDFTYPE1FONTNAMEWFUNC                                        DYNDTWAIN_API::DTWAIN_GetPDFType1FontNameW = nullptr;
    D_GETPIXELFLAVORFUNC                                              DYNDTWAIN_API::DTWAIN_GetPixelFlavor = nullptr;
    D_GETPIXELTYPEFUNC                                                DYNDTWAIN_API::DTWAIN_GetPixelType = nullptr;
    D_GETPRINTERFUNC                                                  DYNDTWAIN_API::DTWAIN_GetPrinter = nullptr;
    D_GETPRINTERSTARTNUMBERFUNC                                       DYNDTWAIN_API::DTWAIN_GetPrinterStartNumber = nullptr;
    D_GETPRINTERSTRINGMODEFUNC                                        DYNDTWAIN_API::DTWAIN_GetPrinterStringMode = nullptr;
    D_GETPRINTERSTRINGSFUNC                                           DYNDTWAIN_API::DTWAIN_GetPrinterStrings = nullptr;
    D_GETPRINTERSUFFIXSTRINGAFUNC                                     DYNDTWAIN_API::DTWAIN_GetPrinterSuffixStringA = nullptr;
    D_GETPRINTERSUFFIXSTRINGFUNC                                      DYNDTWAIN_API::DTWAIN_GetPrinterSuffixString = nullptr;
    D_GETPRINTERSUFFIXSTRINGWFUNC                                     DYNDTWAIN_API::DTWAIN_GetPrinterSuffixStringW = nullptr;
    D_GETREGISTEREDMSGFUNC                                            DYNDTWAIN_API::DTWAIN_GetRegisteredMsg = nullptr;
    D_GETRESOLUTIONFUNC                                               DYNDTWAIN_API::DTWAIN_GetResolution = nullptr;
    D_GETRESOLUTIONSTRINGAFUNC                                        DYNDTWAIN_API::DTWAIN_GetResolutionStringA = nullptr;
    D_GETRESOLUTIONSTRINGFUNC                                         DYNDTWAIN_API::DTWAIN_GetResolutionString = nullptr;
    D_GETRESOLUTIONSTRINGWFUNC                                        DYNDTWAIN_API::DTWAIN_GetResolutionStringW = nullptr;
    D_GETROTATIONFUNC                                                 DYNDTWAIN_API::DTWAIN_GetRotation = nullptr;
    D_GETROTATIONSTRINGAFUNC                                          DYNDTWAIN_API::DTWAIN_GetRotationStringA = nullptr;
    D_GETROTATIONSTRINGFUNC                                           DYNDTWAIN_API::DTWAIN_GetRotationString = nullptr;
    D_GETROTATIONSTRINGWFUNC                                          DYNDTWAIN_API::DTWAIN_GetRotationStringW = nullptr;
    D_GETSAVEFILENAMEAFUNC                                            DYNDTWAIN_API::DTWAIN_GetSaveFileNameA = nullptr;
    D_GETSAVEFILENAMEFUNC                                             DYNDTWAIN_API::DTWAIN_GetSaveFileName = nullptr;
    D_GETSAVEFILENAMEWFUNC                                            DYNDTWAIN_API::DTWAIN_GetSaveFileNameW = nullptr;
    D_GETSHADOWFUNC                                                   DYNDTWAIN_API::DTWAIN_GetShadow = nullptr;
    D_GETSHADOWSTRINGAFUNC                                            DYNDTWAIN_API::DTWAIN_GetShadowStringA = nullptr;
    D_GETSHADOWSTRINGFUNC                                             DYNDTWAIN_API::DTWAIN_GetShadowString = nullptr;
    D_GETSHADOWSTRINGWFUNC                                            DYNDTWAIN_API::DTWAIN_GetShadowStringW = nullptr;
    D_GETSHORTVERSIONSTRINGAFUNC                                      DYNDTWAIN_API::DTWAIN_GetShortVersionStringA = nullptr;
    D_GETSHORTVERSIONSTRINGFUNC                                       DYNDTWAIN_API::DTWAIN_GetShortVersionString = nullptr;
    D_GETSHORTVERSIONSTRINGWFUNC                                      DYNDTWAIN_API::DTWAIN_GetShortVersionStringW = nullptr;
    D_GETSOURCEACQUISITIONSFUNC                                       DYNDTWAIN_API::DTWAIN_GetSourceAcquisitions = nullptr;
    D_GETSOURCEIDEXFUNC                                               DYNDTWAIN_API::DTWAIN_GetSourceIDEx = nullptr;
    D_GETSOURCEIDFUNC                                                 DYNDTWAIN_API::DTWAIN_GetSourceID = nullptr;
    D_GETSOURCEMANUFACTURERAFUNC                                      DYNDTWAIN_API::DTWAIN_GetSourceManufacturerA = nullptr;
    D_GETSOURCEMANUFACTURERFUNC                                       DYNDTWAIN_API::DTWAIN_GetSourceManufacturer = nullptr;
    D_GETSOURCEMANUFACTURERWFUNC                                      DYNDTWAIN_API::DTWAIN_GetSourceManufacturerW = nullptr;
    D_GETSOURCEPRODUCTFAMILYAFUNC                                     DYNDTWAIN_API::DTWAIN_GetSourceProductFamilyA = nullptr;
    D_GETSOURCEPRODUCTFAMILYFUNC                                      DYNDTWAIN_API::DTWAIN_GetSourceProductFamily = nullptr;
    D_GETSOURCEPRODUCTFAMILYWFUNC                                     DYNDTWAIN_API::DTWAIN_GetSourceProductFamilyW = nullptr;
    D_GETSOURCEPRODUCTNAMEAFUNC                                       DYNDTWAIN_API::DTWAIN_GetSourceProductNameA = nullptr;
    D_GETSOURCEPRODUCTNAMEFUNC                                        DYNDTWAIN_API::DTWAIN_GetSourceProductName = nullptr;
    D_GETSOURCEPRODUCTNAMEWFUNC                                       DYNDTWAIN_API::DTWAIN_GetSourceProductNameW = nullptr;
    D_GETSOURCEUNITFUNC                                               DYNDTWAIN_API::DTWAIN_GetSourceUnit = nullptr;
    D_GETSOURCEVERSIONINFOAFUNC                                       DYNDTWAIN_API::DTWAIN_GetSourceVersionInfoA = nullptr;
    D_GETSOURCEVERSIONINFOFUNC                                        DYNDTWAIN_API::DTWAIN_GetSourceVersionInfo = nullptr;
    D_GETSOURCEVERSIONINFOWFUNC                                       DYNDTWAIN_API::DTWAIN_GetSourceVersionInfoW = nullptr;
    D_GETSOURCEVERSIONNUMBERFUNC                                      DYNDTWAIN_API::DTWAIN_GetSourceVersionNumber = nullptr;
    D_GETSTATICLIBVERSIONFUNC                                         DYNDTWAIN_API::DTWAIN_GetStaticLibVersion = nullptr;
    D_GETTEMPFILEDIRECTORYAFUNC                                       DYNDTWAIN_API::DTWAIN_GetTempFileDirectoryA = nullptr;
    D_GETTEMPFILEDIRECTORYFUNC                                        DYNDTWAIN_API::DTWAIN_GetTempFileDirectory = nullptr;
    D_GETTEMPFILEDIRECTORYWFUNC                                       DYNDTWAIN_API::DTWAIN_GetTempFileDirectoryW = nullptr;
    D_GETTHRESHOLDFUNC                                                DYNDTWAIN_API::DTWAIN_GetThreshold = nullptr;
    D_GETTHRESHOLDSTRINGAFUNC                                         DYNDTWAIN_API::DTWAIN_GetThresholdStringA = nullptr;
    D_GETTHRESHOLDSTRINGFUNC                                          DYNDTWAIN_API::DTWAIN_GetThresholdString = nullptr;
    D_GETTHRESHOLDSTRINGWFUNC                                         DYNDTWAIN_API::DTWAIN_GetThresholdStringW = nullptr;
    D_GETTIMEDATEAFUNC                                                DYNDTWAIN_API::DTWAIN_GetTimeDateA = nullptr;
    D_GETTIMEDATEFUNC                                                 DYNDTWAIN_API::DTWAIN_GetTimeDate = nullptr;
    D_GETTIMEDATEWFUNC                                                DYNDTWAIN_API::DTWAIN_GetTimeDateW = nullptr;
    D_GETTWAINAPPIDEXFUNC                                             DYNDTWAIN_API::DTWAIN_GetTwainAppIDEx = nullptr;
    D_GETTWAINAPPIDFUNC                                               DYNDTWAIN_API::DTWAIN_GetTwainAppID = nullptr;
    D_GETTWAINAVAILABILITYFUNC                                        DYNDTWAIN_API::DTWAIN_GetTwainAvailability = nullptr;
    D_GETTWAINCOUNTRYNAMEAFUNC                                        DYNDTWAIN_API::DTWAIN_GetTwainCountryNameA = nullptr;
    D_GETTWAINCOUNTRYNAMEFUNC                                         DYNDTWAIN_API::DTWAIN_GetTwainCountryName = nullptr;
    D_GETTWAINCOUNTRYNAMEWFUNC                                        DYNDTWAIN_API::DTWAIN_GetTwainCountryNameW = nullptr;
    D_GETTWAINCOUNTRYVALUEAFUNC                                       DYNDTWAIN_API::DTWAIN_GetTwainCountryValueA = nullptr;
    D_GETTWAINCOUNTRYVALUEFUNC                                        DYNDTWAIN_API::DTWAIN_GetTwainCountryValue = nullptr;
    D_GETTWAINCOUNTRYVALUEWFUNC                                       DYNDTWAIN_API::DTWAIN_GetTwainCountryValueW = nullptr;
    D_GETTWAINHWNDFUNC                                                DYNDTWAIN_API::DTWAIN_GetTwainHwnd = nullptr;
    D_GETTWAINLANGUAGENAMEAFUNC                                       DYNDTWAIN_API::DTWAIN_GetTwainLanguageNameA = nullptr;
    D_GETTWAINLANGUAGENAMEFUNC                                        DYNDTWAIN_API::DTWAIN_GetTwainLanguageName = nullptr;
    D_GETTWAINLANGUAGENAMEWFUNC                                       DYNDTWAIN_API::DTWAIN_GetTwainLanguageNameW = nullptr;
    D_GETTWAINLANGUAGEVALUEAFUNC                                      DYNDTWAIN_API::DTWAIN_GetTwainLanguageValueA = nullptr;
    D_GETTWAINLANGUAGEVALUEFUNC                                       DYNDTWAIN_API::DTWAIN_GetTwainLanguageValue = nullptr;
    D_GETTWAINLANGUAGEVALUEWFUNC                                      DYNDTWAIN_API::DTWAIN_GetTwainLanguageValueW = nullptr;
    D_GETTWAINMODEFUNC                                                DYNDTWAIN_API::DTWAIN_GetTwainMode = nullptr;
    D_GETTWAINNAMEFROMCONSTANTA                                       DYNDTWAIN_API::DTWAIN_GetTwainNameFromConstantA = nullptr;
    D_GETTWAINNAMEFROMCONSTANTW                                       DYNDTWAIN_API::DTWAIN_GetTwainNameFromConstantW = nullptr;
    D_GETTWAINTIMEOUTFUNC                                             DYNDTWAIN_API::DTWAIN_GetTwainTimeout = nullptr;
    D_GETVERSIONEXFUNC                                                DYNDTWAIN_API::DTWAIN_GetVersionEx = nullptr;
    D_GETVERSIONFUNC                                                  DYNDTWAIN_API::DTWAIN_GetVersion = nullptr;
    D_GETVERSIONINFOAFUNC                                             DYNDTWAIN_API::DTWAIN_GetVersionInfoA = nullptr;
    D_GETVERSIONINFOFUNC                                              DYNDTWAIN_API::DTWAIN_GetVersionInfo = nullptr;
    D_GETVERSIONINFOWFUNC                                             DYNDTWAIN_API::DTWAIN_GetVersionInfoW = nullptr;
    D_GETVERSIONSTRINGAFUNC                                           DYNDTWAIN_API::DTWAIN_GetVersionStringA = nullptr;
    D_GETVERSIONSTRINGFUNC                                            DYNDTWAIN_API::DTWAIN_GetVersionString = nullptr;
    D_GETVERSIONSTRINGWFUNC                                           DYNDTWAIN_API::DTWAIN_GetVersionStringW = nullptr;
    D_GETXRESOLUTIONFUNC                                              DYNDTWAIN_API::DTWAIN_GetXResolution = nullptr;
    D_GETXRESOLUTIONSTRINGAFUNC                                       DYNDTWAIN_API::DTWAIN_GetXResolutionStringA = nullptr;
    D_GETXRESOLUTIONSTRINGFUNC                                        DYNDTWAIN_API::DTWAIN_GetXResolutionString = nullptr;
    D_GETXRESOLUTIONSTRINGWFUNC                                       DYNDTWAIN_API::DTWAIN_GetXResolutionStringW = nullptr;
    D_GETYRESOLUTIONFUNC                                              DYNDTWAIN_API::DTWAIN_GetYResolution = nullptr;
    D_GETYRESOLUTIONSTRINGAFUNC                                       DYNDTWAIN_API::DTWAIN_GetYResolutionStringA = nullptr;
    D_GETYRESOLUTIONSTRINGFUNC                                        DYNDTWAIN_API::DTWAIN_GetYResolutionString = nullptr;
    D_GETYRESOLUTIONSTRINGWFUNC                                       DYNDTWAIN_API::DTWAIN_GetYResolutionStringW = nullptr;
    D_INITEXTIMAGEINFOFUNC                                            DYNDTWAIN_API::DTWAIN_InitExtImageInfo = nullptr;
    D_INITIMAGEFILEAPPENDAFUNC                                        DYNDTWAIN_API::DTWAIN_InitImageFileAppendA = nullptr;
    D_INITIMAGEFILEAPPENDFUNC                                         DYNDTWAIN_API::DTWAIN_InitImageFileAppend = nullptr;
    D_INITIMAGEFILEAPPENDWFUNC                                        DYNDTWAIN_API::DTWAIN_InitImageFileAppendW = nullptr;
    D_INITOCRINTERFACEFUNC                                            DYNDTWAIN_API::DTWAIN_InitOCRInterface = nullptr;
    D_ISACQUIRINGFUNC                                                 DYNDTWAIN_API::DTWAIN_IsAcquiring = nullptr;
    D_ISAUTOBORDERDETECTENABLEDFUNC                                   DYNDTWAIN_API::DTWAIN_IsAutoBorderDetectEnabled = nullptr;
    D_ISAUTOBORDERDETECTSUPPORTEDFUNC                                 DYNDTWAIN_API::DTWAIN_IsAutoBorderDetectSupported = nullptr;
    D_ISAUTOBRIGHTENABLEDFUNC                                         DYNDTWAIN_API::DTWAIN_IsAutoBrightEnabled = nullptr;
    D_ISAUTOBRIGHTSUPPORTEDFUNC                                       DYNDTWAIN_API::DTWAIN_IsAutoBrightSupported = nullptr;
    D_ISAUTODESKEWENABLEDFUNC                                         DYNDTWAIN_API::DTWAIN_IsAutoDeskewEnabled = nullptr;
    D_ISAUTODESKEWSUPPORTEDFUNC                                       DYNDTWAIN_API::DTWAIN_IsAutoDeskewSupported = nullptr;
    D_ISAUTOFEEDENABLEDFUNC                                           DYNDTWAIN_API::DTWAIN_IsAutoFeedEnabled = nullptr;
    D_ISAUTOFEEDSUPPORTEDFUNC                                         DYNDTWAIN_API::DTWAIN_IsAutoFeedSupported = nullptr;
    D_ISAUTOMATICSENSEMEDIUMENABLEDFUNC                               DYNDTWAIN_API::DTWAIN_IsAutomaticSenseMediumEnabled = nullptr;
    D_ISAUTOMATICSENSEMEDIUMSUPPORTEDFUNC                             DYNDTWAIN_API::DTWAIN_IsAutomaticSenseMediumSupported = nullptr;
    D_ISAUTOROTATEENABLEDFUNC                                         DYNDTWAIN_API::DTWAIN_IsAutoRotateEnabled = nullptr;
    D_ISAUTOROTATESUPPORTEDFUNC                                       DYNDTWAIN_API::DTWAIN_IsAutoRotateSupported = nullptr;
    D_ISAUTOSCANENABLEDFUNC                                           DYNDTWAIN_API::DTWAIN_IsAutoScanEnabled = nullptr;
    D_ISBLANKPAGEDETECTIONONFUNC                                      DYNDTWAIN_API::DTWAIN_IsBlankPageDetectionOn = nullptr;
    D_ISCAPSUPPORTEDFUNC                                              DYNDTWAIN_API::DTWAIN_IsCapSupported = nullptr;
    D_ISCOMPRESSIONSUPPORTEDFUNC                                      DYNDTWAIN_API::DTWAIN_IsCompressionSupported = nullptr;
    D_ISCUSTOMDSDATASUPPORTEDFUNC                                     DYNDTWAIN_API::DTWAIN_IsCustomDSDataSupported = nullptr;
    D_ISDEVICEEVENTSUPPORTEDFUNC                                      DYNDTWAIN_API::DTWAIN_IsDeviceEventSupported = nullptr;
    D_ISDEVICEONLINEFUNC                                              DYNDTWAIN_API::DTWAIN_IsDeviceOnLine = nullptr;
    D_ISDIBBLANKFUNC                                                  DYNDTWAIN_API::DTWAIN_IsDIBBlank = nullptr;
    D_ISDIBBLANKSTRINGAFUNC                                           DYNDTWAIN_API::DTWAIN_IsDIBBlankStringA = nullptr;
    D_ISDIBBLANKSTRINGFUNC                                            DYNDTWAIN_API::DTWAIN_IsDIBBlankString = nullptr;
    D_ISDIBBLANKSTRINGWFUNC                                           DYNDTWAIN_API::DTWAIN_IsDIBBlankStringW = nullptr;
    D_ISDOUBLEFEEDDETECTLENGTHSUPPORTEDFUNC                           DYNDTWAIN_API::DTWAIN_IsDoubleFeedDetectLengthSupported = nullptr;
    D_ISDOUBLEFEEDDETECTSUPPORTEDFUNC                                 DYNDTWAIN_API::DTWAIN_IsDoubleFeedDetectSupported = nullptr;
    D_ISDUPLEXENABLEDFUNC                                             DYNDTWAIN_API::DTWAIN_IsDuplexEnabled = nullptr;
    D_ISDUPLEXSUPPORTEDFUNC                                           DYNDTWAIN_API::DTWAIN_IsDuplexSupported = nullptr;
    D_ISEXTIMAGEINFOSUPPORTEDFUNC                                     DYNDTWAIN_API::DTWAIN_IsExtImageInfoSupported = nullptr;
    D_ISFEEDERENABLEDFUNC                                             DYNDTWAIN_API::DTWAIN_IsFeederEnabled = nullptr;
    D_ISFEEDERLOADEDFUNC                                              DYNDTWAIN_API::DTWAIN_IsFeederLoaded = nullptr;
    D_ISFEEDERSENSITIVEFUNC                                           DYNDTWAIN_API::DTWAIN_IsFeederSensitive = nullptr;
    D_ISFEEDERSUPPORTEDFUNC                                           DYNDTWAIN_API::DTWAIN_IsFeederSupported = nullptr;
    D_ISFILESYSTEMSUPPORTEDFUNC                                       DYNDTWAIN_API::DTWAIN_IsFileSystemSupported = nullptr;
    D_ISFILEXFERSUPPORTEDFUNC                                         DYNDTWAIN_API::DTWAIN_IsFileXferSupported = nullptr;
    D_ISINDICATORENABLEDFUNC                                          DYNDTWAIN_API::DTWAIN_IsIndicatorEnabled = nullptr;
    D_ISINDICATORSUPPORTEDFUNC                                        DYNDTWAIN_API::DTWAIN_IsIndicatorSupported = nullptr;
    D_ISINITIALIZEDFUNC                                               DYNDTWAIN_API::DTWAIN_IsInitialized = nullptr;
    D_ISJOBCONTROLSUPPORTEDFUNC                                       DYNDTWAIN_API::DTWAIN_IsJobControlSupported = nullptr;
    D_ISJPEGSUPPORTEDFUNC                                             DYNDTWAIN_API::DTWAIN_IsJPEGSupported = nullptr;
    D_ISLAMPENABLEDFUNC                                               DYNDTWAIN_API::DTWAIN_IsLampEnabled = nullptr;
    D_ISLAMPSUPPORTEDFUNC                                             DYNDTWAIN_API::DTWAIN_IsLampSupported = nullptr;
    D_ISLIGHTPATHSUPPORTEDFUNC                                        DYNDTWAIN_API::DTWAIN_IsLightPathSupported = nullptr;
    D_ISLIGHTSOURCESUPPORTEDFUNC                                      DYNDTWAIN_API::DTWAIN_IsLightSourceSupported = nullptr;
    D_ISMAXBUFFERSSUPPORTEDFUNC                                       DYNDTWAIN_API::DTWAIN_IsMaxBuffersSupported = nullptr;
    D_ISMSGNOTIFYENABLEDFUNC                                          DYNDTWAIN_API::DTWAIN_IsMsgNotifyEnabled = nullptr;
    D_ISOCRENGINEACTIVATEDFUNC                                        DYNDTWAIN_API::DTWAIN_IsOCREngineActivated = nullptr;
    D_ISORIENTATIONSUPPORTEDFUNC                                      DYNDTWAIN_API::DTWAIN_IsOrientationSupported = nullptr;
    D_ISOVERSCANSUPPORTEDFUNC                                         DYNDTWAIN_API::DTWAIN_IsOverscanSupported = nullptr;
    D_ISPAPERDETECTABLEFUNC                                           DYNDTWAIN_API::DTWAIN_IsPaperDetectable = nullptr;
    D_ISPAPERSIZESUPPORTEDFUNC                                        DYNDTWAIN_API::DTWAIN_IsPaperSizeSupported = nullptr;
    D_ISPATCHCAPSSUPPORTEDFUNC                                        DYNDTWAIN_API::DTWAIN_IsPatchCapsSupported = nullptr;
    D_ISPATCHDETECTENABLEDFUNC                                        DYNDTWAIN_API::DTWAIN_IsPatchDetectEnabled = nullptr;
    D_ISPATCHSUPPORTEDFUNC                                            DYNDTWAIN_API::DTWAIN_IsPatchSupported = nullptr;
    D_ISPDFSUPPORTEDFUNC                                              DYNDTWAIN_API::DTWAIN_IsPDFSupported = nullptr;
    D_ISPIXELTYPESUPPORTEDFUNC                                        DYNDTWAIN_API::DTWAIN_IsPixelTypeSupported = nullptr;
    D_ISPNGSUPPORTEDFUNC                                              DYNDTWAIN_API::DTWAIN_IsPNGSupported = nullptr;
    D_ISPRINTERENABLEDFUNC                                            DYNDTWAIN_API::DTWAIN_IsPrinterEnabled = nullptr;
    D_ISPRINTERSUPPORTEDFUNC                                          DYNDTWAIN_API::DTWAIN_IsPrinterSupported = nullptr;
    D_ISROTATIONSUPPORTEDFUNC                                         DYNDTWAIN_API::DTWAIN_IsRotationSupported = nullptr;
    D_ISSESSIONENABLEDFUNC                                            DYNDTWAIN_API::DTWAIN_IsSessionEnabled = nullptr;
    D_ISSKIPIMAGEINFOERRORFUNC                                        DYNDTWAIN_API::DTWAIN_IsSkipImageInfoError = nullptr;
    D_ISSOURCEACQUIRINGFUNC                                           DYNDTWAIN_API::DTWAIN_IsSourceAcquiring = nullptr;
    D_ISSOURCEOPENFUNC                                                DYNDTWAIN_API::DTWAIN_IsSourceOpen = nullptr;
    D_ISSOURCESELECTEDFUNC                                            DYNDTWAIN_API::DTWAIN_IsSourceSelected = nullptr;
    D_ISTHUMBNAILENABLEDFUNC                                          DYNDTWAIN_API::DTWAIN_IsThumbnailEnabled = nullptr;
    D_ISTHUMBNAILSUPPORTEDFUNC                                        DYNDTWAIN_API::DTWAIN_IsThumbnailSupported = nullptr;
    D_ISTIFFSUPPORTEDFUNC                                             DYNDTWAIN_API::DTWAIN_IsTIFFSupported = nullptr;
    D_ISTWAINAVAILABLEFUNC                                            DYNDTWAIN_API::DTWAIN_IsTwainAvailable = nullptr;
    D_ISTWAINMSGFUNC                                                  DYNDTWAIN_API::DTWAIN_IsTwainMsg = nullptr;
    D_ISUICONTROLLABLEFUNC                                            DYNDTWAIN_API::DTWAIN_IsUIControllable = nullptr;
    D_ISUIENABLEDFUNC                                                 DYNDTWAIN_API::DTWAIN_IsUIEnabled = nullptr;
    D_ISUIONLYSUPPORTEDFUNC                                           DYNDTWAIN_API::DTWAIN_IsUIOnlySupported = nullptr;
    D_LOADCUSTOMSTRINGRESOURCESAFUNC                                  DYNDTWAIN_API::DTWAIN_LoadCustomStringResourcesA = nullptr;
    D_LOADCUSTOMSTRINGRESOURCESFUNC                                   DYNDTWAIN_API::DTWAIN_LoadCustomStringResources = nullptr;
    D_LOADCUSTOMSTRINGRESOURCESWFUNC                                  DYNDTWAIN_API::DTWAIN_LoadCustomStringResourcesW = nullptr;
    D_LOADLANGUAGERESOURCEFUNC                                        DYNDTWAIN_API::DTWAIN_LoadLanguageResource = nullptr;
    D_LOCKMEMORYEXFUNC                                                DYNDTWAIN_API::DTWAIN_LockMemoryEx = nullptr;
    D_LOCKMEMORYFUNC                                                  DYNDTWAIN_API::DTWAIN_LockMemory = nullptr;
    D_LOGMESSAGEAFUNC                                                 DYNDTWAIN_API::DTWAIN_LogMessageA = nullptr;
    D_LOGMESSAGEFUNC                                                  DYNDTWAIN_API::DTWAIN_LogMessage = nullptr;
    D_LOGMESSAGEWFUNC                                                 DYNDTWAIN_API::DTWAIN_LogMessageW = nullptr;
    D_MAKERGBFUNC                                                     DYNDTWAIN_API::DTWAIN_MakeRGB = nullptr;
    D_OPENSOURCEFUNC                                                  DYNDTWAIN_API::DTWAIN_OpenSource = nullptr;
    D_OPENSOURCESONSELECTFUNC                                         DYNDTWAIN_API::DTWAIN_OpenSourcesOnSelect = nullptr;
    D_RANGECREATEFROMCAPFUNC                                          DYNDTWAIN_API::DTWAIN_RangeCreateFromCap = nullptr;
    D_RANGECREATEFUNC                                                 DYNDTWAIN_API::DTWAIN_RangeCreate = nullptr;
    D_RANGEDESTROYFUNC                                                DYNDTWAIN_API::DTWAIN_RangeDestroy = nullptr;
    D_RANGEEXPANDFUNC                                                 DYNDTWAIN_API::DTWAIN_RangeExpand = nullptr;
    D_RANGEGETALLFLOATFUNC                                            DYNDTWAIN_API::DTWAIN_RangeGetAllFloat = nullptr;
    D_RANGEGETALLFLOATSTRINGAFUNC                                     DYNDTWAIN_API::DTWAIN_RangeGetAllFloatStringA = nullptr;
    D_RANGEGETALLFLOATSTRINGFUNC                                      DYNDTWAIN_API::DTWAIN_RangeGetAllFloatString = nullptr;
    D_RANGEGETALLFLOATSTRINGWFUNC                                     DYNDTWAIN_API::DTWAIN_RangeGetAllFloatStringW = nullptr;
    D_RANGEGETALLFUNC                                                 DYNDTWAIN_API::DTWAIN_RangeGetAll = nullptr;
    D_RANGEGETALLLONGFUNC                                             DYNDTWAIN_API::DTWAIN_RangeGetAllLong = nullptr;
    D_RANGEGETCOUNTFUNC                                               DYNDTWAIN_API::DTWAIN_RangeGetCount = nullptr;
    D_RANGEGETEXPVALUEFLOATFUNC                                       DYNDTWAIN_API::DTWAIN_RangeGetExpValueFloat = nullptr;
    D_RANGEGETEXPVALUEFLOATSTRINGAFUNC                                DYNDTWAIN_API::DTWAIN_RangeGetExpValueFloatStringA = nullptr;
    D_RANGEGETEXPVALUEFLOATSTRINGFUNC                                 DYNDTWAIN_API::DTWAIN_RangeGetExpValueFloatString = nullptr;
    D_RANGEGETEXPVALUEFLOATSTRINGWFUNC                                DYNDTWAIN_API::DTWAIN_RangeGetExpValueFloatStringW = nullptr;
    D_RANGEGETEXPVALUEFUNC                                            DYNDTWAIN_API::DTWAIN_RangeGetExpValue = nullptr;
    D_RANGEGETEXPVALUELONGFUNC                                        DYNDTWAIN_API::DTWAIN_RangeGetExpValueLong = nullptr;
    D_RANGEGETNEARESTVALUEFUNC                                        DYNDTWAIN_API::DTWAIN_RangeGetNearestValue = nullptr;
    D_RANGEGETPOSFLOATFUNC                                            DYNDTWAIN_API::DTWAIN_RangeGetPosFloat = nullptr;
    D_RANGEGETPOSFLOATSTRINGAFUNC                                     DYNDTWAIN_API::DTWAIN_RangeGetPosFloatStringA = nullptr;
    D_RANGEGETPOSFLOATSTRINGFUNC                                      DYNDTWAIN_API::DTWAIN_RangeGetPosFloatString = nullptr;
    D_RANGEGETPOSFLOATSTRINGWFUNC                                     DYNDTWAIN_API::DTWAIN_RangeGetPosFloatStringW = nullptr;
    D_RANGEGETPOSFUNC                                                 DYNDTWAIN_API::DTWAIN_RangeGetPos = nullptr;
    D_RANGEGETPOSLONGFUNC                                             DYNDTWAIN_API::DTWAIN_RangeGetPosLong = nullptr;
    D_RANGEGETVALUEFLOATFUNC                                          DYNDTWAIN_API::DTWAIN_RangeGetValueFloat = nullptr;
    D_RANGEGETVALUEFLOATSTRINGAFUNC                                   DYNDTWAIN_API::DTWAIN_RangeGetValueFloatStringA = nullptr;
    D_RANGEGETVALUEFLOATSTRINGFUNC                                    DYNDTWAIN_API::DTWAIN_RangeGetValueFloatString = nullptr;
    D_RANGEGETVALUEFLOATSTRINGWFUNC                                   DYNDTWAIN_API::DTWAIN_RangeGetValueFloatStringW = nullptr;
    D_RANGEGETVALUEFUNC                                               DYNDTWAIN_API::DTWAIN_RangeGetValue = nullptr;
    D_RANGEGETVALUELONGFUNC                                           DYNDTWAIN_API::DTWAIN_RangeGetValueLong = nullptr;
    D_RANGEISVALIDFUNC                                                DYNDTWAIN_API::DTWAIN_RangeIsValid = nullptr;
    D_RANGENEARESTVALUEFLOATFUNC                                      DYNDTWAIN_API::DTWAIN_RangeNearestValueFloat = nullptr;
    D_RANGENEARESTVALUEFLOATSTRINGAFUNC                               DYNDTWAIN_API::DTWAIN_RangeNearestValueFloatStringA = nullptr;
    D_RANGENEARESTVALUEFLOATSTRINGFUNC                                DYNDTWAIN_API::DTWAIN_RangeNearestValueFloatString = nullptr;
    D_RANGENEARESTVALUEFLOATSTRINGWFUNC                               DYNDTWAIN_API::DTWAIN_RangeNearestValueFloatStringW = nullptr;
    D_RANGENEARESTVALUELONGFUNC                                       DYNDTWAIN_API::DTWAIN_RangeNearestValueLong = nullptr;
    D_RANGESETALLFLOATFUNC                                            DYNDTWAIN_API::DTWAIN_RangeSetAllFloat = nullptr;
    D_RANGESETALLFLOATSTRINGAFUNC                                     DYNDTWAIN_API::DTWAIN_RangeSetAllFloatStringA = nullptr;
    D_RANGESETALLFLOATSTRINGFUNC                                      DYNDTWAIN_API::DTWAIN_RangeSetAllFloatString = nullptr;
    D_RANGESETALLFLOATSTRINGWFUNC                                     DYNDTWAIN_API::DTWAIN_RangeSetAllFloatStringW = nullptr;
    D_RANGESETALLFUNC                                                 DYNDTWAIN_API::DTWAIN_RangeSetAll = nullptr;
    D_RANGESETALLLONGFUNC                                             DYNDTWAIN_API::DTWAIN_RangeSetAllLong = nullptr;
    D_RANGESETVALUEFLOATFUNC                                          DYNDTWAIN_API::DTWAIN_RangeSetValueFloat = nullptr;
    D_RANGESETVALUEFLOATSTRINGAFUNC                                   DYNDTWAIN_API::DTWAIN_RangeSetValueFloatStringA = nullptr;
    D_RANGESETVALUEFLOATSTRINGFUNC                                    DYNDTWAIN_API::DTWAIN_RangeSetValueFloatString = nullptr;
    D_RANGESETVALUEFLOATSTRINGWFUNC                                   DYNDTWAIN_API::DTWAIN_RangeSetValueFloatStringW = nullptr;
    D_RANGESETVALUEFUNC                                               DYNDTWAIN_API::DTWAIN_RangeSetValue = nullptr;
    D_RANGESETVALUELONGFUNC                                           DYNDTWAIN_API::DTWAIN_RangeSetValueLong = nullptr;
    D_RESETPDFTEXTELEMENTFUNC                                         DYNDTWAIN_API::DTWAIN_ResetPDFTextElement = nullptr;
    D_REWINDPAGEFUNC                                                  DYNDTWAIN_API::DTWAIN_RewindPage = nullptr;
    D_SELECTDEFAULTOCRENGINEFUNC                                      DYNDTWAIN_API::DTWAIN_SelectDefaultOCREngine = nullptr;
    D_SELECTDEFAULTSOURCEFUNC                                         DYNDTWAIN_API::DTWAIN_SelectDefaultSource = nullptr;
    D_SELECTOCRENGINEBYNAMEAFUNC                                      DYNDTWAIN_API::DTWAIN_SelectOCREngineByNameA = nullptr;
    D_SELECTOCRENGINEBYNAMEFUNC                                       DYNDTWAIN_API::DTWAIN_SelectOCREngineByName = nullptr;
    D_SELECTOCRENGINEBYNAMEWFUNC                                      DYNDTWAIN_API::DTWAIN_SelectOCREngineByNameW = nullptr;
    D_SELECTOCRENGINEFUNC                                             DYNDTWAIN_API::DTWAIN_SelectOCREngine = nullptr;
    D_SELECTSOURCE2AFUNC                                              DYNDTWAIN_API::DTWAIN_SelectSource2A = nullptr;
    D_SELECTSOURCE2EXAFUNC                                            DYNDTWAIN_API::DTWAIN_SelectSource2ExA = nullptr;
    D_SELECTSOURCE2EXFUNC                                             DYNDTWAIN_API::DTWAIN_SelectSource2Ex = nullptr;
    D_SELECTSOURCE2EXWFUNC                                            DYNDTWAIN_API::DTWAIN_SelectSource2ExW = nullptr;
    D_SELECTSOURCE2FUNC                                               DYNDTWAIN_API::DTWAIN_SelectSource2 = nullptr;
    D_SELECTSOURCE2WFUNC                                              DYNDTWAIN_API::DTWAIN_SelectSource2W = nullptr;
    D_SELECTSOURCEBYNAMEAFUNC                                         DYNDTWAIN_API::DTWAIN_SelectSourceByNameA = nullptr;
    D_SELECTSOURCEBYNAMEFUNC                                          DYNDTWAIN_API::DTWAIN_SelectSourceByName = nullptr;
    D_SELECTSOURCEBYNAMEWFUNC                                         DYNDTWAIN_API::DTWAIN_SelectSourceByNameW = nullptr;
    D_SELECTSOURCEFUNC                                                DYNDTWAIN_API::DTWAIN_SelectSource = nullptr;
    D_SETACQUIREAREA2FUNC                                             DYNDTWAIN_API::DTWAIN_SetAcquireArea2 = nullptr;
    D_SETACQUIREAREA2STRINGAFUNC                                      DYNDTWAIN_API::DTWAIN_SetAcquireArea2StringA = nullptr;
    D_SETACQUIREAREA2STRINGFUNC                                       DYNDTWAIN_API::DTWAIN_SetAcquireArea2String = nullptr;
    D_SETACQUIREAREA2STRINGWFUNC                                      DYNDTWAIN_API::DTWAIN_SetAcquireArea2StringW = nullptr;
    D_SETACQUIREAREAFUNC                                              DYNDTWAIN_API::DTWAIN_SetAcquireArea = nullptr;
    D_SETACQUIREIMAGENEGATIVEFUNC                                     DYNDTWAIN_API::DTWAIN_SetAcquireImageNegative = nullptr;
    D_SETACQUIREIMAGESCALEFUNC                                        DYNDTWAIN_API::DTWAIN_SetAcquireImageScale = nullptr;
    D_SETACQUIREIMAGESCALESTRINGAFUNC                                 DYNDTWAIN_API::DTWAIN_SetAcquireImageScaleStringA = nullptr;
    D_SETACQUIREIMAGESCALESTRINGFUNC                                  DYNDTWAIN_API::DTWAIN_SetAcquireImageScaleString = nullptr;
    D_SETACQUIREIMAGESCALESTRINGWFUNC                                 DYNDTWAIN_API::DTWAIN_SetAcquireImageScaleStringW = nullptr;
    D_SETACQUIRESTRIPBUFFERFUNC                                       DYNDTWAIN_API::DTWAIN_SetAcquireStripBuffer = nullptr;
    D_SETALARMSFUNC                                                   DYNDTWAIN_API::DTWAIN_SetAlarms = nullptr;
    D_SETALARMVOLUMEFUNC                                              DYNDTWAIN_API::DTWAIN_SetAlarmVolume = nullptr;
    D_SETALLCAPSTODEFAULTFUNC                                         DYNDTWAIN_API::DTWAIN_SetAllCapsToDefault = nullptr;
    D_SETAPPINFOAFUNC                                                 DYNDTWAIN_API::DTWAIN_SetAppInfoA = nullptr;
    D_SETAPPINFOFUNC                                                  DYNDTWAIN_API::DTWAIN_SetAppInfo = nullptr;
    D_SETAPPINFOWFUNC                                                 DYNDTWAIN_API::DTWAIN_SetAppInfoW = nullptr;
    D_SETAUTHORAFUNC                                                  DYNDTWAIN_API::DTWAIN_SetAuthorA = nullptr;
    D_SETAUTHORFUNC                                                   DYNDTWAIN_API::DTWAIN_SetAuthor = nullptr;
    D_SETAUTHORWFUNC                                                  DYNDTWAIN_API::DTWAIN_SetAuthorW = nullptr;
    D_SETAVAILABLEPRINTERSARRAYFUNC                                   DYNDTWAIN_API::DTWAIN_SetAvailablePrintersArray = nullptr;
    D_SETAVAILABLEPRINTERSFUNC                                        DYNDTWAIN_API::DTWAIN_SetAvailablePrinters = nullptr;
    D_SETBITDEPTHFUNC                                                 DYNDTWAIN_API::DTWAIN_SetBitDepth = nullptr;
    D_SETBLANKPAGEDETECTIONFUNC                                       DYNDTWAIN_API::DTWAIN_SetBlankPageDetection = nullptr;
    D_SETBLANKPAGEDETECTIONSTRINGAFUNC                                DYNDTWAIN_API::DTWAIN_SetBlankPageDetectionStringA = nullptr;
    D_SETBLANKPAGEDETECTIONSTRINGFUNC                                 DYNDTWAIN_API::DTWAIN_SetBlankPageDetectionString = nullptr;
    D_SETBLANKPAGEDETECTIONSTRINGWFUNC                                DYNDTWAIN_API::DTWAIN_SetBlankPageDetectionStringW = nullptr;
    D_SETBLANKPAGEDETECTIONEX                                         DYNDTWAIN_API::DTWAIN_SetBlankPageDetectionEx = nullptr;
    D_SETBLANKPAGEDETECTIONEXSTRING                                   DYNDTWAIN_API::DTWAIN_SetBlankPageDetectionExString = nullptr;
    D_SETBLANKPAGEDETECTIONEXSTRINGA                                  DYNDTWAIN_API::DTWAIN_SetBlankPageDetectionExStringA = nullptr;
    D_SETBLANKPAGEDETECTIONEXSTRINGW                                  DYNDTWAIN_API::DTWAIN_SetBlankPageDetectionExStringW = nullptr;
    D_SETBRIGHTNESSFUNC                                               DYNDTWAIN_API::DTWAIN_SetBrightness = nullptr;
    D_SETBRIGHTNESSSTRINGAFUNC                                        DYNDTWAIN_API::DTWAIN_SetBrightnessStringA = nullptr;
    D_SETBRIGHTNESSSTRINGFUNC                                         DYNDTWAIN_API::DTWAIN_SetBrightnessString = nullptr;
    D_SETBRIGHTNESSSTRINGWFUNC                                        DYNDTWAIN_API::DTWAIN_SetBrightnessStringW = nullptr;
    D_SETCALLBACK64FUNC                                               DYNDTWAIN_API::DTWAIN_SetCallback64 = nullptr;
    D_SETCALLBACKFUNC                                                 DYNDTWAIN_API::DTWAIN_SetCallback = nullptr;
    D_SETCAMERAAFUNC                                                  DYNDTWAIN_API::DTWAIN_SetCameraA = nullptr;
    D_SETCAMERAFUNC                                                   DYNDTWAIN_API::DTWAIN_SetCamera = nullptr;
    D_SETCAMERAWFUNC                                                  DYNDTWAIN_API::DTWAIN_SetCameraW = nullptr;
    D_SETCAPTIONAFUNC                                                 DYNDTWAIN_API::DTWAIN_SetCaptionA = nullptr;
    D_SETCAPTIONFUNC                                                  DYNDTWAIN_API::DTWAIN_SetCaption = nullptr;
    D_SETCAPTIONWFUNC                                                 DYNDTWAIN_API::DTWAIN_SetCaptionW = nullptr;
    D_SETCAPVALUESEX2FUNC                                             DYNDTWAIN_API::DTWAIN_SetCapValuesEx2 = nullptr;
    D_SETCAPVALUESEXFUNC                                              DYNDTWAIN_API::DTWAIN_SetCapValuesEx = nullptr;
    D_SETCAPVALUESFUNC                                                DYNDTWAIN_API::DTWAIN_SetCapValues = nullptr;
    D_SETCOMPRESSIONTYPEFUNC                                          DYNDTWAIN_API::DTWAIN_SetCompressionType = nullptr;
    D_SETCONTRASTFUNC                                                 DYNDTWAIN_API::DTWAIN_SetContrast = nullptr;
    D_SETCONTRASTSTRINGAFUNC                                          DYNDTWAIN_API::DTWAIN_SetContrastStringA = nullptr;
    D_SETCONTRASTSTRINGFUNC                                           DYNDTWAIN_API::DTWAIN_SetContrastString = nullptr;
    D_SETCONTRASTSTRINGWFUNC                                          DYNDTWAIN_API::DTWAIN_SetContrastStringW = nullptr;
    D_SETCOUNTRYFUNC                                                  DYNDTWAIN_API::DTWAIN_SetCountry = nullptr;
    D_SETCURRENTRETRYCOUNTFUNC                                        DYNDTWAIN_API::DTWAIN_SetCurrentRetryCount = nullptr;
    D_SETCUSTOMDSDATAFUNC                                             DYNDTWAIN_API::DTWAIN_SetCustomDSData = nullptr;
    D_SETCUSTOMFILESAVEFUNC                                           DYNDTWAIN_API::DTWAIN_SetCustomFileSave = nullptr;
    D_SETDEFAULTSOURCEFUNC                                            DYNDTWAIN_API::DTWAIN_SetDefaultSource = nullptr;
    D_SETDEVICENOTIFICATIONSFUNC                                      DYNDTWAIN_API::DTWAIN_SetDeviceNotifications = nullptr;
    D_SETDEVICETIMEDATEAFUNC                                          DYNDTWAIN_API::DTWAIN_SetDeviceTimeDateA = nullptr;
    D_SETDEVICETIMEDATEFUNC                                           DYNDTWAIN_API::DTWAIN_SetDeviceTimeDate = nullptr;
    D_SETDEVICETIMEDATEWFUNC                                          DYNDTWAIN_API::DTWAIN_SetDeviceTimeDateW = nullptr;
    D_SETDOUBLEFEEDDETECTLENGTHFUNC                                   DYNDTWAIN_API::DTWAIN_SetDoubleFeedDetectLength = nullptr;
    D_SETDOUBLEFEEDDETECTLENGTHSTRINGAFUNC                            DYNDTWAIN_API::DTWAIN_SetDoubleFeedDetectLengthStringA = nullptr;
    D_SETDOUBLEFEEDDETECTLENGTHSTRINGFUNC                             DYNDTWAIN_API::DTWAIN_SetDoubleFeedDetectLengthString = nullptr;
    D_SETDOUBLEFEEDDETECTLENGTHSTRINGWFUNC                            DYNDTWAIN_API::DTWAIN_SetDoubleFeedDetectLengthStringW = nullptr;
    D_SETDOUBLEFEEDDETECTVALUESFUNC                                   DYNDTWAIN_API::DTWAIN_SetDoubleFeedDetectValues = nullptr;
    D_SETDSMSEARCHORDEREXAFUNC                                        DYNDTWAIN_API::DTWAIN_SetDSMSearchOrderExA = nullptr;
    D_SETDSMSEARCHORDEREXFUNC                                         DYNDTWAIN_API::DTWAIN_SetDSMSearchOrderEx = nullptr;
    D_SETDSMSEARCHORDEREXWFUNC                                        DYNDTWAIN_API::DTWAIN_SetDSMSearchOrderExW = nullptr;
    D_SETDSMSEARCHORDERFUNC                                           DYNDTWAIN_API::DTWAIN_SetDSMSearchOrder = nullptr;
    D_SETEOJDETECTVALUEFUNC                                           DYNDTWAIN_API::DTWAIN_SetEOJDetectValue = nullptr;
    D_SETERRORBUFFERTHRESHOLDFUNC                                     DYNDTWAIN_API::DTWAIN_SetErrorBufferThreshold = nullptr;
    D_SETERRORCALLBACK64FUNC                                          DYNDTWAIN_API::DTWAIN_SetErrorCallback64 = nullptr;
    D_SETERRORCALLBACKFUNC                                            DYNDTWAIN_API::DTWAIN_SetErrorCallback = nullptr;
    D_SETFEEDERALIGNMENTFUNC                                          DYNDTWAIN_API::DTWAIN_SetFeederAlignment = nullptr;
    D_SETFEEDERORDERFUNC                                              DYNDTWAIN_API::DTWAIN_SetFeederOrder = nullptr;
    D_SETFILEAUTOINCREMENTFUNC                                        DYNDTWAIN_API::DTWAIN_SetFileAutoIncrement = nullptr;
    D_SETFILESAVEPOSAFUNC                                             DYNDTWAIN_API::DTWAIN_SetFileSavePosA = nullptr;
    D_SETFILESAVEPOSFUNC                                              DYNDTWAIN_API::DTWAIN_SetFileSavePos = nullptr;
    D_SETFILESAVEPOSWFUNC                                             DYNDTWAIN_API::DTWAIN_SetFileSavePosW = nullptr;
    D_SETFILEXFERFORMATFUNC                                           DYNDTWAIN_API::DTWAIN_SetFileXferFormat = nullptr;
    D_SETHALFTONEAFUNC                                                DYNDTWAIN_API::DTWAIN_SetHalftoneA = nullptr;
    D_SETHALFTONEFUNC                                                 DYNDTWAIN_API::DTWAIN_SetHalftone = nullptr;
    D_SETHALFTONEWFUNC                                                DYNDTWAIN_API::DTWAIN_SetHalftoneW = nullptr;
    D_SETHIGHLIGHTFUNC                                                DYNDTWAIN_API::DTWAIN_SetHighlight = nullptr;
    D_SETHIGHLIGHTSTRINGAFUNC                                         DYNDTWAIN_API::DTWAIN_SetHighlightStringA = nullptr;
    D_SETHIGHLIGHTSTRINGFUNC                                          DYNDTWAIN_API::DTWAIN_SetHighlightString = nullptr;
    D_SETHIGHLIGHTSTRINGWFUNC                                         DYNDTWAIN_API::DTWAIN_SetHighlightStringW = nullptr;
    D_SETJOBCONTROLFUNC                                               DYNDTWAIN_API::DTWAIN_SetJobControl = nullptr;
    D_SETJPEGVALUESFUNC                                               DYNDTWAIN_API::DTWAIN_SetJpegValues = nullptr;
    D_SETLANGUAGEFUNC                                                 DYNDTWAIN_API::DTWAIN_SetLanguage = nullptr;
    D_SETLASTERROR                                                    DYNDTWAIN_API::DTWAIN_SetLastError = nullptr;
    D_SETLIGHTPATHEXFUNC                                              DYNDTWAIN_API::DTWAIN_SetLightPathEx = nullptr;
    D_SETLIGHTPATHFUNC                                                DYNDTWAIN_API::DTWAIN_SetLightPath = nullptr;
    D_SETLIGHTSOURCEFUNC                                              DYNDTWAIN_API::DTWAIN_SetLightSource = nullptr;
    D_SETLIGHTSOURCESFUNC                                             DYNDTWAIN_API::DTWAIN_SetLightSources = nullptr;
    D_SETLOGGERCALLBACKAFUNC                                          DYNDTWAIN_API::DTWAIN_SetLoggerCallbackA = nullptr;
    D_SETLOGGERCALLBACKFUNC                                           DYNDTWAIN_API::DTWAIN_SetLoggerCallback = nullptr;
    D_SETLOGGERCALLBACKWFUNC                                          DYNDTWAIN_API::DTWAIN_SetLoggerCallbackW = nullptr;
    D_SETMANUALDUPLEXMODEFUNC                                         DYNDTWAIN_API::DTWAIN_SetManualDuplexMode = nullptr;
    D_SETMAXACQUISITIONSFUNC                                          DYNDTWAIN_API::DTWAIN_SetMaxAcquisitions = nullptr;
    D_SETMAXBUFFERSFUNC                                               DYNDTWAIN_API::DTWAIN_SetMaxBuffers = nullptr;
    D_SETMAXRETRYATTEMPTSFUNC                                         DYNDTWAIN_API::DTWAIN_SetMaxRetryAttempts = nullptr;
    D_SETMULTIPAGESCANMODEFUNC                                        DYNDTWAIN_API::DTWAIN_SetMultipageScanMode = nullptr;
    D_SETNOISEFILTERFUNC                                              DYNDTWAIN_API::DTWAIN_SetNoiseFilter = nullptr;
    D_SETOCRCAPVALUESFUNC                                             DYNDTWAIN_API::DTWAIN_SetOCRCapValues = nullptr;
    D_SETORIENTATIONFUNC                                              DYNDTWAIN_API::DTWAIN_SetOrientation = nullptr;
    D_SETOVERSCANFUNC                                                 DYNDTWAIN_API::DTWAIN_SetOverscan = nullptr;
    D_SETPAPERSIZEFUNC                                                DYNDTWAIN_API::DTWAIN_SetPaperSize = nullptr;
    D_SETPATCHMAXPRIORITIESFUNC                                       DYNDTWAIN_API::DTWAIN_SetPatchMaxPriorities = nullptr;
    D_SETPATCHMAXRETRIESFUNC                                          DYNDTWAIN_API::DTWAIN_SetPatchMaxRetries = nullptr;
    D_SETPATCHPRIORITIESFUNC                                          DYNDTWAIN_API::DTWAIN_SetPatchPriorities = nullptr;
    D_SETPATCHSEARCHMODEFUNC                                          DYNDTWAIN_API::DTWAIN_SetPatchSearchMode = nullptr;
    D_SETPATCHTIMEOUTFUNC                                             DYNDTWAIN_API::DTWAIN_SetPatchTimeOut = nullptr;
    D_SETPDFAESENCRYPTIONFUNC                                         DYNDTWAIN_API::DTWAIN_SetPDFAESEncryption = nullptr;
    D_SETPDFASCIICOMPRESSIONFUNC                                      DYNDTWAIN_API::DTWAIN_SetPDFASCIICompression = nullptr;
    D_SETPDFAUTHORAFUNC                                               DYNDTWAIN_API::DTWAIN_SetPDFAuthorA = nullptr;
    D_SETPDFAUTHORFUNC                                                DYNDTWAIN_API::DTWAIN_SetPDFAuthor = nullptr;
    D_SETPDFAUTHORWFUNC                                               DYNDTWAIN_API::DTWAIN_SetPDFAuthorW = nullptr;
    D_SETPDFCOMPRESSIONFUNC                                           DYNDTWAIN_API::DTWAIN_SetPDFCompression = nullptr;
    D_SETPDFCREATORAFUNC                                              DYNDTWAIN_API::DTWAIN_SetPDFCreatorA = nullptr;
    D_SETPDFCREATORFUNC                                               DYNDTWAIN_API::DTWAIN_SetPDFCreator = nullptr;
    D_SETPDFCREATORWFUNC                                              DYNDTWAIN_API::DTWAIN_SetPDFCreatorW = nullptr;
    D_SETPDFENCRYPTIONAFUNC                                           DYNDTWAIN_API::DTWAIN_SetPDFEncryptionA = nullptr;
    D_SETPDFENCRYPTIONFUNC                                            DYNDTWAIN_API::DTWAIN_SetPDFEncryption = nullptr;
    D_SETPDFENCRYPTIONWFUNC                                           DYNDTWAIN_API::DTWAIN_SetPDFEncryptionW = nullptr;
    D_SETPDFJPEGQUALITYFUNC                                           DYNDTWAIN_API::DTWAIN_SetPDFJpegQuality = nullptr;
    D_SETPDFKEYWORDSAFUNC                                             DYNDTWAIN_API::DTWAIN_SetPDFKeywordsA = nullptr;
    D_SETPDFKEYWORDSFUNC                                              DYNDTWAIN_API::DTWAIN_SetPDFKeywords = nullptr;
    D_SETPDFKEYWORDSWFUNC                                             DYNDTWAIN_API::DTWAIN_SetPDFKeywordsW = nullptr;
    D_SETPDFOCRCONVERSIONFUNC                                         DYNDTWAIN_API::DTWAIN_SetPDFOCRConversion = nullptr;
    D_SETPDFOCRMODEFUNC                                               DYNDTWAIN_API::DTWAIN_SetPDFOCRMode = nullptr;
    D_SETPDFORIENTATIONFUNC                                           DYNDTWAIN_API::DTWAIN_SetPDFOrientation = nullptr;
    D_SETPDFPAGESCALEFUNC                                             DYNDTWAIN_API::DTWAIN_SetPDFPageScale = nullptr;
    D_SETPDFPAGESCALESTRINGAFUNC                                      DYNDTWAIN_API::DTWAIN_SetPDFPageScaleStringA = nullptr;
    D_SETPDFPAGESCALESTRINGFUNC                                       DYNDTWAIN_API::DTWAIN_SetPDFPageScaleString = nullptr;
    D_SETPDFPAGESCALESTRINGWFUNC                                      DYNDTWAIN_API::DTWAIN_SetPDFPageScaleStringW = nullptr;
    D_SETPDFPAGESIZEFUNC                                              DYNDTWAIN_API::DTWAIN_SetPDFPageSize = nullptr;
    D_SETPDFPAGESIZESTRINGAFUNC                                       DYNDTWAIN_API::DTWAIN_SetPDFPageSizeStringA = nullptr;
    D_SETPDFPAGESIZESTRINGFUNC                                        DYNDTWAIN_API::DTWAIN_SetPDFPageSizeString = nullptr;
    D_SETPDFPAGESIZESTRINGWFUNC                                       DYNDTWAIN_API::DTWAIN_SetPDFPageSizeStringW = nullptr;
    D_SETPDFPOLARITYFUNC                                              DYNDTWAIN_API::DTWAIN_SetPDFPolarity = nullptr;
    D_SETPDFPRODUCERAFUNC                                             DYNDTWAIN_API::DTWAIN_SetPDFProducerA = nullptr;
    D_SETPDFPRODUCERFUNC                                              DYNDTWAIN_API::DTWAIN_SetPDFProducer = nullptr;
    D_SETPDFPRODUCERWFUNC                                             DYNDTWAIN_API::DTWAIN_SetPDFProducerW = nullptr;
    D_SETPDFSUBJECTAFUNC                                              DYNDTWAIN_API::DTWAIN_SetPDFSubjectA = nullptr;
    D_SETPDFSUBJECTFUNC                                               DYNDTWAIN_API::DTWAIN_SetPDFSubject = nullptr;
    D_SETPDFSUBJECTWFUNC                                              DYNDTWAIN_API::DTWAIN_SetPDFSubjectW = nullptr;
    D_SETPDFTEXTELEMENTFLOATFUNC                                      DYNDTWAIN_API::DTWAIN_SetPDFTextElementFloat = nullptr;
    D_SETPDFTEXTELEMENTLONGFUNC                                       DYNDTWAIN_API::DTWAIN_SetPDFTextElementLong = nullptr;
    D_SETPDFTEXTELEMENTSTRINGAFUNC                                    DYNDTWAIN_API::DTWAIN_SetPDFTextElementStringA = nullptr;
    D_SETPDFTEXTELEMENTSTRINGFUNC                                     DYNDTWAIN_API::DTWAIN_SetPDFTextElementString = nullptr;
    D_SETPDFTEXTELEMENTSTRINGWFUNC                                    DYNDTWAIN_API::DTWAIN_SetPDFTextElementStringW = nullptr;
    D_SETPDFTITLEAFUNC                                                DYNDTWAIN_API::DTWAIN_SetPDFTitleA = nullptr;
    D_SETPDFTITLEFUNC                                                 DYNDTWAIN_API::DTWAIN_SetPDFTitle = nullptr;
    D_SETPDFTITLEWFUNC                                                DYNDTWAIN_API::DTWAIN_SetPDFTitleW = nullptr;
    D_SETPIXELFLAVORFUNC                                              DYNDTWAIN_API::DTWAIN_SetPixelFlavor = nullptr;
    D_SETPIXELTYPEFUNC                                                DYNDTWAIN_API::DTWAIN_SetPixelType = nullptr;
    D_SETPOSTSCRIPTTITLEAFUNC                                         DYNDTWAIN_API::DTWAIN_SetPostScriptTitleA = nullptr;
    D_SETPOSTSCRIPTTITLEFUNC                                          DYNDTWAIN_API::DTWAIN_SetPostScriptTitle = nullptr;
    D_SETPOSTSCRIPTTITLEWFUNC                                         DYNDTWAIN_API::DTWAIN_SetPostScriptTitleW = nullptr;
    D_SETPOSTSCRIPTTYPEFUNC                                           DYNDTWAIN_API::DTWAIN_SetPostScriptType = nullptr;
    D_SETPRINTERFUNC                                                  DYNDTWAIN_API::DTWAIN_SetPrinter = nullptr;
    D_SETPRINTERSTARTNUMBERFUNC                                       DYNDTWAIN_API::DTWAIN_SetPrinterStartNumber = nullptr;
    D_SETPRINTERSTRINGMODEFUNC                                        DYNDTWAIN_API::DTWAIN_SetPrinterStringMode = nullptr;
    D_SETPRINTERSTRINGSFUNC                                           DYNDTWAIN_API::DTWAIN_SetPrinterStrings = nullptr;
    D_SETPRINTERSUFFIXSTRINGAFUNC                                     DYNDTWAIN_API::DTWAIN_SetPrinterSuffixStringA = nullptr;
    D_SETPRINTERSUFFIXSTRINGFUNC                                      DYNDTWAIN_API::DTWAIN_SetPrinterSuffixString = nullptr;
    D_SETPRINTERSUFFIXSTRINGWFUNC                                     DYNDTWAIN_API::DTWAIN_SetPrinterSuffixStringW = nullptr;
    D_SETQUERYCAPSUPPORTFUNC                                          DYNDTWAIN_API::DTWAIN_SetQueryCapSupport = nullptr;
    D_SETRESOLUTIONFUNC                                               DYNDTWAIN_API::DTWAIN_SetResolution = nullptr;
    D_SETRESOLUTIONSTRINGAFUNC                                        DYNDTWAIN_API::DTWAIN_SetResolutionStringA = nullptr;
    D_SETRESOLUTIONSTRINGFUNC                                         DYNDTWAIN_API::DTWAIN_SetResolutionString = nullptr;
    D_SETRESOLUTIONSTRINGWFUNC                                        DYNDTWAIN_API::DTWAIN_SetResolutionStringW = nullptr;
    D_SETRESOURCEPATHAFUNC                                            DYNDTWAIN_API::DTWAIN_SetResourcePathA = nullptr;
    D_SETRESOURCEPATHFUNC                                             DYNDTWAIN_API::DTWAIN_SetResourcePath = nullptr;
    D_SETRESOURCEPATHWFUNC                                            DYNDTWAIN_API::DTWAIN_SetResourcePathW = nullptr;
    D_SETROTATIONFUNC                                                 DYNDTWAIN_API::DTWAIN_SetRotation = nullptr;
    D_SETROTATIONSTRINGAFUNC                                          DYNDTWAIN_API::DTWAIN_SetRotationStringA = nullptr;
    D_SETROTATIONSTRINGFUNC                                           DYNDTWAIN_API::DTWAIN_SetRotationString = nullptr;
    D_SETROTATIONSTRINGWFUNC                                          DYNDTWAIN_API::DTWAIN_SetRotationStringW = nullptr;
    D_SETSAVEFILENAMEAFUNC                                            DYNDTWAIN_API::DTWAIN_SetSaveFileNameA = nullptr;
    D_SETSAVEFILENAMEFUNC                                             DYNDTWAIN_API::DTWAIN_SetSaveFileName = nullptr;
    D_SETSAVEFILENAMEWFUNC                                            DYNDTWAIN_API::DTWAIN_SetSaveFileNameW = nullptr;
    D_SETSHADOWFUNC                                                   DYNDTWAIN_API::DTWAIN_SetShadow = nullptr;
    D_SETSHADOWSTRINGAFUNC                                            DYNDTWAIN_API::DTWAIN_SetShadowStringA = nullptr;
    D_SETSHADOWSTRINGFUNC                                             DYNDTWAIN_API::DTWAIN_SetShadowString = nullptr;
    D_SETSHADOWSTRINGWFUNC                                            DYNDTWAIN_API::DTWAIN_SetShadowStringW = nullptr;
    D_SETSOURCEUNITFUNC                                               DYNDTWAIN_API::DTWAIN_SetSourceUnit = nullptr;
    D_SETTEMPFILEDIRECTORYAFUNC                                       DYNDTWAIN_API::DTWAIN_SetTempFileDirectoryA = nullptr;
    D_SETTEMPFILEDIRECTORYFUNC                                        DYNDTWAIN_API::DTWAIN_SetTempFileDirectory = nullptr;
    D_SETTEMPFILEDIRECTORYWFUNC                                       DYNDTWAIN_API::DTWAIN_SetTempFileDirectoryW = nullptr;
    D_SETTHRESHOLDFUNC                                                DYNDTWAIN_API::DTWAIN_SetThreshold = nullptr;
    D_SETTHRESHOLDSTRINGAFUNC                                         DYNDTWAIN_API::DTWAIN_SetThresholdStringA = nullptr;
    D_SETTHRESHOLDSTRINGFUNC                                          DYNDTWAIN_API::DTWAIN_SetThresholdString = nullptr;
    D_SETTHRESHOLDSTRINGWFUNC                                         DYNDTWAIN_API::DTWAIN_SetThresholdStringW = nullptr;
    D_SETTIFFCOMPRESSTYPEFUNC                                         DYNDTWAIN_API::DTWAIN_SetTIFFCompressType = nullptr;
    D_SETTIFFINVERTFUNC                                               DYNDTWAIN_API::DTWAIN_SetTIFFInvert = nullptr;
    D_SETTWAINDIALOGFONTFUNC                                          DYNDTWAIN_API::DTWAIN_SetTwainDialogFont = nullptr;
    D_SETTWAINDSMFUNC                                                 DYNDTWAIN_API::DTWAIN_SetTwainDSM = nullptr;
    D_SETTWAINLOGAFUNC                                                DYNDTWAIN_API::DTWAIN_SetTwainLogA = nullptr;
    D_SETTWAINLOGFUNC                                                 DYNDTWAIN_API::DTWAIN_SetTwainLog = nullptr;
    D_SETTWAINLOGWFUNC                                                DYNDTWAIN_API::DTWAIN_SetTwainLogW = nullptr;
    D_SETTWAINMODEFUNC                                                DYNDTWAIN_API::DTWAIN_SetTwainMode = nullptr;
    D_SETTWAINTIMEOUTFUNC                                             DYNDTWAIN_API::DTWAIN_SetTwainTimeout = nullptr;
    D_SETUPDATEDIBPROCFUNC                                            DYNDTWAIN_API::DTWAIN_SetUpdateDibProc = nullptr;
    D_SETXRESOLUTIONFUNC                                              DYNDTWAIN_API::DTWAIN_SetXResolution = nullptr;
    D_SETXRESOLUTIONSTRINGAFUNC                                       DYNDTWAIN_API::DTWAIN_SetXResolutionStringA = nullptr;
    D_SETXRESOLUTIONSTRINGFUNC                                        DYNDTWAIN_API::DTWAIN_SetXResolutionString = nullptr;
    D_SETXRESOLUTIONSTRINGWFUNC                                       DYNDTWAIN_API::DTWAIN_SetXResolutionStringW = nullptr;
    D_SETYRESOLUTIONFUNC                                              DYNDTWAIN_API::DTWAIN_SetYResolution = nullptr;
    D_SETYRESOLUTIONSTRINGAFUNC                                       DYNDTWAIN_API::DTWAIN_SetYResolutionStringA = nullptr;
    D_SETYRESOLUTIONSTRINGFUNC                                        DYNDTWAIN_API::DTWAIN_SetYResolutionString = nullptr;
    D_SETYRESOLUTIONSTRINGWFUNC                                       DYNDTWAIN_API::DTWAIN_SetYResolutionStringW = nullptr;
    D_SHOWUIONLYFUNC                                                  DYNDTWAIN_API::DTWAIN_ShowUIOnly = nullptr;
    D_SHUTDOWNOCRENGINEFUNC                                           DYNDTWAIN_API::DTWAIN_ShutdownOCREngine = nullptr;
    D_SKIPIMAGEINFOERRORFUNC                                          DYNDTWAIN_API::DTWAIN_SkipImageInfoError = nullptr;
    D_STARTTHREADFUNC                                                 DYNDTWAIN_API::DTWAIN_StartThread = nullptr;
    D_STARTTWAINSESSIONAFUNC                                          DYNDTWAIN_API::DTWAIN_StartTwainSessionA = nullptr;
    D_STARTTWAINSESSIONFUNC                                           DYNDTWAIN_API::DTWAIN_StartTwainSession = nullptr;
    D_STARTTWAINSESSIONWFUNC                                          DYNDTWAIN_API::DTWAIN_StartTwainSessionW = nullptr;
    D_SYSDESTROYFUNC                                                  DYNDTWAIN_API::DTWAIN_SysDestroy = nullptr;
    D_SYSINITIALIZEEX2AFUNC                                           DYNDTWAIN_API::DTWAIN_SysInitializeEx2A = nullptr;
    D_SYSINITIALIZEEX2FUNC                                            DYNDTWAIN_API::DTWAIN_SysInitializeEx2 = nullptr;
    D_SYSINITIALIZEEX2WFUNC                                           DYNDTWAIN_API::DTWAIN_SysInitializeEx2W = nullptr;
    D_SYSINITIALIZEEXAFUNC                                            DYNDTWAIN_API::DTWAIN_SysInitializeExA = nullptr;
    D_SYSINITIALIZEEXFUNC                                             DYNDTWAIN_API::DTWAIN_SysInitializeEx = nullptr;
    D_SYSINITIALIZEEXWFUNC                                            DYNDTWAIN_API::DTWAIN_SysInitializeExW = nullptr;
    D_SYSINITIALIZEFUNC                                               DYNDTWAIN_API::DTWAIN_SysInitialize = nullptr;
    D_SYSINITIALIZELIBEX2AFUNC                                        DYNDTWAIN_API::DTWAIN_SysInitializeLibEx2A = nullptr;
    D_SYSINITIALIZELIBEX2FUNC                                         DYNDTWAIN_API::DTWAIN_SysInitializeLibEx2 = nullptr;
    D_SYSINITIALIZELIBEX2WFUNC                                        DYNDTWAIN_API::DTWAIN_SysInitializeLibEx2W = nullptr;
    D_SYSINITIALIZELIBEXAFUNC                                         DYNDTWAIN_API::DTWAIN_SysInitializeLibExA = nullptr;
    D_SYSINITIALIZELIBEXFUNC                                          DYNDTWAIN_API::DTWAIN_SysInitializeLibEx = nullptr;
    D_SYSINITIALIZELIBEXWFUNC                                         DYNDTWAIN_API::DTWAIN_SysInitializeLibExW = nullptr;
    D_SYSINITIALIZELIBFUNC                                            DYNDTWAIN_API::DTWAIN_SysInitializeLib = nullptr;
    D_SYSINITIALIZENOBLOCKINGFUNC                                     DYNDTWAIN_API::DTWAIN_SysInitializeNoBlocking = nullptr;
    D_UNLOCKMEMORYEXFUNC                                              DYNDTWAIN_API::DTWAIN_UnlockMemoryEx = nullptr;
    D_UNLOCKMEMORYFUNC                                                DYNDTWAIN_API::DTWAIN_UnlockMemory = nullptr;
    D_USEMULTIPLETHREADSFUNC                                          DYNDTWAIN_API::DTWAIN_UseMultipleThreads = nullptr;
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation
#ifdef __cplusplus
template <typename Fn>
void LoadFunction(DYNDTWAIN_API* pApi, Fn& apifn, HMODULE hModule, const char *fnName)
{
    DTWAINAPI_ASSERT(apifn = reinterpret_cast<Fn>(::GetProcAddress(hModule, fnName)));
}
#define LoadFunctionImpl(fn, apiptr, module) LoadFunction(nullptr, fn, module, #fn);
#else
#define LoadFunctionImpl(fn, apiptr, module) { \
        DTWAINAPI_ASSERT(DTWAIN_INSTANCE fn = GetProcAddress(module, #fn)); }
#endif
#ifdef __cplusplus
    #define DTWAIN_INSTANCE DYNDTWAIN_API::
    int DYNDTWAIN_API::InitDTWAINInterface(HMODULE hModule)
    {
        DYNDTWAIN_API* pApi = nullptr;
#else
    #define DTWAIN_INSTANCE pApi->
    int InitDTWAINInterface(DYNDTWAIN_API* pApi, HMODULE hModule)
    {
#endif
#ifndef __cplusplus
        memset(pApi, 0, sizeof(DYNDTWAIN_API));
#endif
    /* hModule must be the return value of LoadLibraryA(LibraryVersion);
       where LibraryVersion is one of the following, depending on the DTWAIN DLL that is being used:

       "dtwain32"
       "dtwain32d"
       "dtwain32u"
       "dtwain32ud"
       "dtwain64"
       "dtwain64d"
       "dtwain64u"
       "dtwain64ud"
       */
    if ( hModule )
    {
          DTWAINAPI_ASSERT(DTWAIN_INSTANCE DTWAIN_GetVersion  =  (D_GETVERSIONFUNC)  GetProcAddress(hModule,"DTWAIN_GetVersion"));
          DTWAINAPI_ASSERT(DTWAIN_INSTANCE DTWAIN_GetVersionEx = (D_GETVERSIONEXFUNC)GetProcAddress(hModule, "DTWAIN_GetVersionEx"));
          if ( DTWAIN_INSTANCE DTWAIN_GetVersionEx )
          {
              LONG Major, Minor, VerType, Patch;
              DTWAIN_INSTANCE DTWAIN_GetVersionEx(&Major, &Minor, &VerType, &Patch);
              if (Major >= DTWAIN_MAJOR_VERSION)
              {
                  if (Minor >= DTWAIN_MINOR_VERSION)
                      DTWAINAPI_ASSERT(Patch >= DTWAIN_SUBMINOR_VERSION)
                  else
                      DTWAINAPI_ASSERT(Minor >= DTWAIN_MINOR_VERSION)
              }
              else
                  DTWAINAPI_ASSERT(Major >= DTWAIN_MAJOR_VERSION);
          }

          LoadFunctionImpl(DTWAIN_AcquireAudioFileA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireAudioFile, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireAudioFileW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireAudioNativeEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireAudioNative, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireBufferedEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireBuffered, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireFileA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireFileEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireFile, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireFileW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireNativeEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireNative, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AcquireToClipboard, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AddExtImageInfoQuery, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AddPDFTextA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AddPDFTextEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AddPDFText, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AddPDFTextW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AllocateMemoryEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AllocateMemory, pApi, hModule)
          LoadFunctionImpl(DTWAIN_AppHandlesExceptions, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddANSIString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddANSIStringN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddFloatN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAdd, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddLong64, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddLong64N, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddLongN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddStringNA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddStringN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddStringNW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddWideString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddWideStringN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayConvertFix32ToFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayConvertFloatToFix32, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCopy, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreateCopy, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreateFromCap, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreateFromLong64s, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreateFromLongs, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreateFromReals, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreateFromStrings, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreate, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayDestroyFrames, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayDestroy, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindANSIString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFind, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindLong64, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindWideString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFix32GetAt, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFix32SetAt, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFrameGetAt, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFrameGetFrameAt, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFrameSetAt, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtANSIString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtANSIStringPtr, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAt, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtLong64, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtStringPtr, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtWideString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtWideStringPtr, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetBuffer, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetCount, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetMaxStringLength, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetSourceAt, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetStringLength, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetType, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInit, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtANSIString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtANSIStringN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtFloatN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAt, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtLong64, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtLong64N, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtLongN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtStringNA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtStringN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtStringNW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtWideString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtWideStringN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayRemoveAll, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayRemoveAt, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayRemoveAtN, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArrayResize, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtANSIString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAt, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtLong64, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtWideString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_CallCallback64, pApi, hModule)
          LoadFunctionImpl(DTWAIN_CallCallback, pApi, hModule)
          LoadFunctionImpl(DTWAIN_CallDSMProc, pApi, hModule)
          LoadFunctionImpl(DTWAIN_CheckHandles, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ClearBuffers, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ClearErrorBuffer, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ClearPage, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ClearPDFText, pApi, hModule)
          LoadFunctionImpl(DTWAIN_CloseSource, pApi, hModule)
          LoadFunctionImpl(DTWAIN_CloseSourceUI, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ConvertDIBToBitmap, pApi, hModule)
          LoadFunctionImpl(DTWAIN_CreateAcquisitionArray, pApi, hModule)
          LoadFunctionImpl(DTWAIN_CreatePDFTextElement, pApi, hModule)
          LoadFunctionImpl(DTWAIN_DestroyAcquisitionArray, pApi, hModule)
          LoadFunctionImpl(DTWAIN_DestroyPDFTextElement, pApi, hModule)
          LoadFunctionImpl(DTWAIN_DisableAppWindow, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutoBorderDetect, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutoBright, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutoDeskew, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutoFeed, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutomaticSenseMedium, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutoRotate, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutoScan, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableDuplex, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableFeeder, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableIndicator, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableJobFileHandling, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableLamp, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableMsgNotify, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnablePatchDetect, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnablePrinter, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnableThumbnail, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EndThread, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EndTwainSession, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumAlarmsEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumAlarms, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumAlarmVolumesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumAlarmVolumes, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumAudioXferMechsEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumAudioXferMechs, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumAutoFeedValuesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumAutoFeedValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumAutomaticCapturesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumAutomaticCaptures, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumAutomaticSenseMediumEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumAutomaticSenseMedium, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumBitDepthsEx2, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumBitDepthsEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumBitDepths, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumBottomCameras, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumBrightnessValuesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumBrightnessValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumCameras, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumCompressionTypesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumCompressionTypes, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumContrastValuesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumContrastValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumCustomCapsEx2, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumCustomCaps, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumDoubleFeedDetectLengthsEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumDoubleFeedDetectLengths, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumDoubleFeedDetectValuesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumDoubleFeedDetectValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumExtendedCapsEx2, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumExtendedCapsEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumExtendedCaps, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumExtImageInfoTypes, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumFileTypeBitsPerPixel, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumFileXferFormatsEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumFileXferFormats, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumHalftonesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumHalftones, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumHighlightValuesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumHighlightValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumJobControlsEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumJobControls, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumLightPathsEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumLightPaths, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumLightSourcesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumLightSources, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumMaxBuffersEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumMaxBuffers, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumNoiseFiltersEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumNoiseFilters, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumOCRInterfaces, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumOCRSupportedCaps, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumOrientationsEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumOrientations, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumOverscanValuesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumOverscanValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPaperSizesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPaperSizes, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchCodesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchCodes, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchMaxPrioritiesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchMaxPriorities, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchMaxRetriesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchMaxRetries, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchPrioritiesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchPriorities, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchSearchModesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchSearchModes, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchTimeOutValuesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchTimeOutValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPixelTypes, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPrinterStringModesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumPrinterStringModes, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumResolutionValuesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumResolutionValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumShadowValuesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumShadowValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumSourcesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumSources, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumSourceUnitsEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumSourceUnits, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumSourceValuesA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumSourceValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumSourceValuesW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumSupportedCapsEx2, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumSupportedCapsEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumSupportedCaps, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumSupportedSinglePageFileTypes, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumSupportedMultiPageFileTypes, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumThresholdValuesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumThresholdValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumTopCameras, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumTwainPrintersArrayEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumTwainPrintersArray, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumTwainPrintersEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_EnumTwainPrinters, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ExecuteOCRA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ExecuteOCR, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ExecuteOCRW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FeedPage, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FlipBitmap, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FlushAcquiredPages, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ForceAcquireBitDepth, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ForceScanOnNoUI, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameCreate, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameCreateStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameCreateString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameCreateStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameDestroy, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetAll, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetAllStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetAllString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetAllStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetValue, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetValueStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetValueString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetValueStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameIsValid, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetAll, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetAllStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetAllString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetAllStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetValue, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetValueStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetValueString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetValueStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FreeExtImageInfo, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FreeMemoryEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_FreeMemory, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireArea2, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireArea2StringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireArea2String, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireArea2StringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireArea, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquiredImageArray, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquiredImage, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireMetrics, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireStripBuffer, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireStripData, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireStripSizes, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAlarmVolume, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAPIHandleStatus, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAppInfoA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAppInfo, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAppInfoW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAuthorA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAuthor, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetAuthorW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetBatteryMinutes, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetBatteryPercent, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetBitDepth, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetBlankPageAutoDetection, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetBrightness, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetBrightnessStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetBrightnessString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetBrightnessStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCallback64, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCallback, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCapArrayType, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCapContainerEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCapContainer, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCapDataType, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCapFromNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCapFromName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCapFromNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCapOperations, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCaptionA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCaption, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCaptionW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCapValuesEx2, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCapValuesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCapValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCompressionSize, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCompressionType, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetConditionCodeStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetConditionCodeString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetConditionCodeStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetContrast, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetContrastStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetContrastString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetContrastStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCountry, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCurrentAcquiredImage, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCurrentFileNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCurrentFileName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCurrentFileNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCurrentPageNum, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCurrentRetryCount, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetCustomDSData, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceEventEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceEvent, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceEventInfo, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceNotifications, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceTimeDateA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceTimeDate, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceTimeDateW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDoubleFeedDetectLength, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDoubleFeedDetectValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDSMFullNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDSMFullName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDSMFullNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDSMSearchOrder, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDTWAINHandle, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetDuplexType, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorBuffer, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorBufferThreshold, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorCallback64, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorCallback, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetExtCapFromNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetExtCapFromName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetExtCapFromNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetExtImageInfoData, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetExtImageInfo, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetExtImageInfoItem, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetExtNameFromCapA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetExtNameFromCap, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetExtNameFromCapW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetFeederAlignment, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetFeederFuncs, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetFeederOrder, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetFileTypeName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetFileTypeNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetFileTypeNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetFileTypeExtensions, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetFileTypeExtensionsA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetFileTypeExtensionsW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetHalftoneA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetHalftone, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetHalftoneW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetHighlight, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetHighlightStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetHighlightString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetHighlightStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetImageInfo, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetImageInfoStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetImageInfoString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetImageInfoStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetJobControl, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetJpegValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetLanguage, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetLastError, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetLibraryPathA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetLibraryPath, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetLibraryPathW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetLightPath, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetLightSource, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetLightSources, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetLoggerCallbackA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetLoggerCallback, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetLoggerCallbackW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetManualDuplexCount, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetMaxAcquisitions, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetMaxBuffers, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetMaxPagesToAcquire, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetMaxRetryAttempts, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetNameFromCapA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetNameFromCap, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetNameFromCapW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetNoiseFilter, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetNumAcquiredImages, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetNumAcquisitions, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRCapValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRErrorStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRErrorString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRErrorStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRLastError, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRManufacturerA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRManufacturer, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRManufacturerW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRProductFamilyA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRProductFamily, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRProductFamilyW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRProductNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRProductName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRProductNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRText, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextInfoFloatEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextInfoFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextInfoHandle, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextInfoLongEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextInfoLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRVersionInfoA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRVersionInfo, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRVersionInfoW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOrientation, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetOverscan, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPaperSize, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPatchMaxPriorities, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPatchMaxRetries, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPatchPriorities, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPatchSearchMode, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPatchTimeOut, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFTextElementFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFTextElementLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFTextElementStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFTextElementString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFTextElementStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFType1FontNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFType1FontName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFType1FontNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPixelFlavor, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPixelType, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinter, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinterStartNumber, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinterStringMode, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinterStrings, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinterSuffixStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinterSuffixString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinterSuffixStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetRegisteredMsg, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetResolution, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetResolutionStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetResolutionString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetResolutionStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetRotation, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetRotationStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetRotationString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetRotationStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSaveFileNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSaveFileName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSaveFileNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetShadow, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetShadowStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetShadowString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetShadowStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetShortVersionStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetShortVersionString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetShortVersionStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceAcquisitions, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceIDEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceID, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceManufacturerA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceManufacturer, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceManufacturerW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceProductFamilyA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceProductFamily, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceProductFamilyW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceProductNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceProductName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceProductNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceUnit, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceVersionInfoA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceVersionInfo, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceVersionInfoW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceVersionNumber, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetStaticLibVersion, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTempFileDirectoryA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTempFileDirectory, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTempFileDirectoryW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetThreshold, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetThresholdStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetThresholdString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetThresholdStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTimeDateA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTimeDate, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTimeDateW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainAppIDEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainAppID, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainAvailability, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainCountryNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainCountryName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainCountryNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainCountryValueA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainCountryValue, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainCountryValueW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainHwnd, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainLanguageNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainLanguageName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainLanguageNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainLanguageValueA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainLanguageValue, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainLanguageValueW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainMode, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainNameFromConstantA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainNameFromConstantW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainTimeout, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetVersion, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionInfoA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionInfo, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionInfoW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetXResolution, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetXResolutionStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetXResolutionString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetXResolutionStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetYResolution, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetYResolutionStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetYResolutionString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_GetYResolutionStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_InitExtImageInfo, pApi, hModule)
          LoadFunctionImpl(DTWAIN_InitImageFileAppendA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_InitImageFileAppend, pApi, hModule)
          LoadFunctionImpl(DTWAIN_InitImageFileAppendW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_InitOCRInterface, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAcquiring, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoBorderDetectEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoBorderDetectSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoBrightEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoBrightSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoDeskewEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoDeskewSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoFeedEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoFeedSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutomaticSenseMediumEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutomaticSenseMediumSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoRotateEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoRotateSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoScanEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsBlankPageDetectionOn, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsCapSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsCompressionSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsCustomDSDataSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsDeviceEventSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsDeviceOnLine, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsDIBBlank, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsDIBBlankStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsDIBBlankString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsDIBBlankStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsDoubleFeedDetectLengthSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsDoubleFeedDetectSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsDuplexEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsDuplexSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsExtImageInfoSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsFeederEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsFeederLoaded, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsFeederSensitive, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsFeederSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsFileSystemSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsFileXferSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsIndicatorEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsIndicatorSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsInitialized, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsJobControlSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsJPEGSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsLampEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsLampSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsLightPathSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsLightSourceSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsMaxBuffersSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsMsgNotifyEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsOCREngineActivated, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsOrientationSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsOverscanSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsPaperDetectable, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsPaperSizeSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsPatchCapsSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsPatchDetectEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsPatchSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsPDFSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsPixelTypeSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsPNGSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsPrinterEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsPrinterSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsRotationSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsSessionEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsSkipImageInfoError, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsSourceAcquiring, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsSourceOpen, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsSourceSelected, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsThumbnailEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsThumbnailSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsTIFFSupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsTwainAvailable, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsTwainMsg, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsUIControllable, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsUIEnabled, pApi, hModule)
          LoadFunctionImpl(DTWAIN_IsUIOnlySupported, pApi, hModule)
          LoadFunctionImpl(DTWAIN_LoadCustomStringResourcesA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_LoadCustomStringResources, pApi, hModule)
          LoadFunctionImpl(DTWAIN_LoadCustomStringResourcesW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_LoadLanguageResource, pApi, hModule)
          LoadFunctionImpl(DTWAIN_LockMemoryEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_LockMemory, pApi, hModule)
          LoadFunctionImpl(DTWAIN_LogMessageA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_LogMessage, pApi, hModule)
          LoadFunctionImpl(DTWAIN_LogMessageW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_MakeRGB, pApi, hModule)
          LoadFunctionImpl(DTWAIN_OpenSource, pApi, hModule)
          LoadFunctionImpl(DTWAIN_OpenSourcesOnSelect, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeCreateFromCap, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeCreate, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeDestroy, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeExpand, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetAllFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetAllFloatStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetAllFloatString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetAllFloatStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetAll, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetAllLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetCount, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetExpValueFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetExpValueFloatStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetExpValueFloatString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetExpValueFloatStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetExpValue, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetExpValueLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetNearestValue, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetPosFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetPosFloatStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetPosFloatString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetPosFloatStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetPos, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetPosLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetValueFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetValueFloatStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetValueFloatString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetValueFloatStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetValue, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetValueLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeIsValid, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeNearestValueFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeNearestValueFloatStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeNearestValueFloatString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeNearestValueFloatStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeNearestValueLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetAllFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetAllFloatStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetAllFloatString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetAllFloatStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetAll, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetAllLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetValueFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetValueFloatStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetValueFloatString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetValueFloatStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetValue, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetValueLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ResetPDFTextElement, pApi, hModule)
          LoadFunctionImpl(DTWAIN_RewindPage, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectDefaultOCREngine, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectDefaultSource, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectOCREngineByNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectOCREngineByName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectOCREngineByNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectOCREngine, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource2A, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource2ExA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource2Ex, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource2ExW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource2, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource2W, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectSourceByNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectSourceByName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectSourceByNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireArea2, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireArea2StringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireArea2String, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireArea2StringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireArea, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireImageNegative, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireImageScale, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireImageScaleStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireImageScaleString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireImageScaleStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireStripBuffer, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAlarms, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAlarmVolume, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAllCapsToDefault, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAppInfoA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAppInfo, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAppInfoW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAuthorA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAuthor, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAuthorW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAvailablePrintersArray, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetAvailablePrinters, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBitDepth, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetection, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionExString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionExStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionExStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBrightness, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBrightnessStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBrightnessString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetBrightnessStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCallback64, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCallback, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCameraA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCamera, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCameraW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCaptionA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCaption, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCaptionW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCapValuesEx2, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCapValuesEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCapValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCompressionType, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetContrast, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetContrastStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetContrastString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetContrastStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCountry, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCurrentRetryCount, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCustomDSData, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetCustomFileSave, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDefaultSource, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDeviceNotifications, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDeviceTimeDateA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDeviceTimeDate, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDeviceTimeDateW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDoubleFeedDetectLength, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDoubleFeedDetectLengthStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDoubleFeedDetectLengthString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDoubleFeedDetectLengthStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDoubleFeedDetectValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDSMSearchOrderExA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDSMSearchOrderEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDSMSearchOrderExW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetDSMSearchOrder, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetEOJDetectValue, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetErrorBufferThreshold, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetErrorCallback64, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetErrorCallback, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetFeederAlignment, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetFeederOrder, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetFileAutoIncrement, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetFileSavePosA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetFileSavePos, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetFileSavePosW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetFileXferFormat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetHalftoneA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetHalftone, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetHalftoneW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetHighlight, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetHighlightStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetHighlightString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetHighlightStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetJobControl, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetJpegValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetLanguage, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetLastError, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetLightPathEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetLightPath, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetLightSource, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetLightSources, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetLoggerCallbackA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetLoggerCallback, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetLoggerCallbackW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetManualDuplexMode, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetMaxAcquisitions, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetMaxBuffers, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetMaxRetryAttempts, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetMultipageScanMode, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetNoiseFilter, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetOCRCapValues, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetOrientation, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetOverscan, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPaperSize, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPatchMaxPriorities, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPatchMaxRetries, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPatchPriorities, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPatchSearchMode, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPatchTimeOut, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFAESEncryption, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFASCIICompression, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFAuthorA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFAuthor, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFAuthorW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFCompression, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFCreatorA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFCreator, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFCreatorW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFEncryptionA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFEncryption, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFEncryptionW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFJpegQuality, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFKeywordsA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFKeywords, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFKeywordsW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFOCRConversion, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFOCRMode, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFOrientation, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageScale, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageScaleStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageScaleString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageScaleStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageSize, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageSizeStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageSizeString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageSizeStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPolarity, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFProducerA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFProducer, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFProducerW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFSubjectA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFSubject, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFSubjectW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTextElementFloat, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTextElementLong, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTextElementStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTextElementString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTextElementStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTitleA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTitle, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTitleW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPixelFlavor, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPixelType, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPostScriptTitleA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPostScriptTitle, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPostScriptTitleW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPostScriptType, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinter, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinterStartNumber, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinterStringMode, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinterStrings, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinterSuffixStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinterSuffixString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinterSuffixStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetQueryCapSupport, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetResolution, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetResolutionStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetResolutionString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetResolutionStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetResourcePathA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetResourcePath, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetResourcePathW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetRotation, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetRotationStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetRotationString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetRotationStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetSaveFileNameA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetSaveFileName, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetSaveFileNameW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetShadow, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetShadowStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetShadowString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetShadowStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetSourceUnit, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetTempFileDirectoryA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetTempFileDirectory, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetTempFileDirectoryW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetThreshold, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetThresholdStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetThresholdString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetThresholdStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetTIFFCompressType, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetTIFFInvert, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainDialogFont, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainDSM, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainLogA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainLog, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainLogW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainMode, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainTimeout, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetUpdateDibProc, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetXResolution, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetXResolutionStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetXResolutionString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetXResolutionStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetYResolution, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetYResolutionStringA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetYResolutionString, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SetYResolutionStringW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ShowUIOnly, pApi, hModule)
          LoadFunctionImpl(DTWAIN_ShutdownOCREngine, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SkipImageInfoError, pApi, hModule)
          LoadFunctionImpl(DTWAIN_StartThread, pApi, hModule)
          LoadFunctionImpl(DTWAIN_StartTwainSessionA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_StartTwainSession, pApi, hModule)
          LoadFunctionImpl(DTWAIN_StartTwainSessionW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysDestroy, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeEx2A, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeEx2, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeEx2W, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeExA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeExW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitialize, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLibEx2A, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLibEx2, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLibEx2W, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLibExA, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLibEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLibExW, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLib, pApi, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeNoBlocking, pApi, hModule)
          LoadFunctionImpl(DTWAIN_UnlockMemoryEx, pApi, hModule)
          LoadFunctionImpl(DTWAIN_UnlockMemory, pApi, hModule)
          LoadFunctionImpl(DTWAIN_UseMultipleThreads, pApi, hModule)
    }
    return 1;
}
#pragma warning (pop)

