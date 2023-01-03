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
void LoadFunction(Fn& apifn, HMODULE hModule, const char *fnName)
{
    DTWAINAPI_ASSERT(apifn = reinterpret_cast<Fn>(::GetProcAddress(hModule, fnName)));
}
#define LoadFunctionImpl(fn, module) LoadFunction(fn, module, #fn);
#else
#define LoadFunctionImpl(fn, module) { \
        DTWAINAPI_ASSERT(DTWAIN_INSTANCE fn = GetProcAddress(module, #fn)); }
#endif
#ifdef __cplusplus
    #define DTWAIN_INSTANCE DYNDTWAIN_API::
    int DYNDTWAIN_API::InitDTWAINInterface(HMODULE hModule)
    {
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
          LoadFunctionImpl(DTWAIN_GetVersion, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionEx, hModule)
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

          LoadFunctionImpl(DTWAIN_AcquireAudioFileA, hModule)
          LoadFunctionImpl(DTWAIN_AcquireAudioFile, hModule)
          LoadFunctionImpl(DTWAIN_AcquireAudioFileW, hModule)
          LoadFunctionImpl(DTWAIN_AcquireAudioNativeEx, hModule)
          LoadFunctionImpl(DTWAIN_AcquireAudioNative, hModule)
          LoadFunctionImpl(DTWAIN_AcquireBufferedEx, hModule)
          LoadFunctionImpl(DTWAIN_AcquireBuffered, hModule)
          LoadFunctionImpl(DTWAIN_AcquireFileA, hModule)
          LoadFunctionImpl(DTWAIN_AcquireFileEx, hModule)
          LoadFunctionImpl(DTWAIN_AcquireFile, hModule)
          LoadFunctionImpl(DTWAIN_AcquireFileW, hModule)
          LoadFunctionImpl(DTWAIN_AcquireNativeEx, hModule)
          LoadFunctionImpl(DTWAIN_AcquireNative, hModule)
          LoadFunctionImpl(DTWAIN_AcquireToClipboard, hModule)
          LoadFunctionImpl(DTWAIN_AddExtImageInfoQuery, hModule)
          LoadFunctionImpl(DTWAIN_AddPDFTextA, hModule)
          LoadFunctionImpl(DTWAIN_AddPDFTextEx, hModule)
          LoadFunctionImpl(DTWAIN_AddPDFText, hModule)
          LoadFunctionImpl(DTWAIN_AddPDFTextW, hModule)
          LoadFunctionImpl(DTWAIN_AllocateMemoryEx, hModule)
          LoadFunctionImpl(DTWAIN_AllocateMemory, hModule)
          LoadFunctionImpl(DTWAIN_AppHandlesExceptions, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddANSIString, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddANSIStringN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddFloat, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddFloatN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAdd, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddLong64, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddLong64N, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddLong, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddLongN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddStringA, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddString, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddStringNA, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddStringN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddStringNW, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddStringW, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddWideString, hModule)
          LoadFunctionImpl(DTWAIN_ArrayAddWideStringN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayConvertFix32ToFloat, hModule)
          LoadFunctionImpl(DTWAIN_ArrayConvertFloatToFix32, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCopy, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreateCopy, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreateFromCap, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreateFromLong64s, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreateFromLongs, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreateFromReals, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreateFromStrings, hModule)
          LoadFunctionImpl(DTWAIN_ArrayCreate, hModule)
          LoadFunctionImpl(DTWAIN_ArrayDestroyFrames, hModule)
          LoadFunctionImpl(DTWAIN_ArrayDestroy, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindANSIString, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindFloat, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFind, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindLong64, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindLong, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindStringA, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindString, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindStringW, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFindWideString, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFix32GetAt, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFix32SetAt, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFrameGetAt, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFrameGetFrameAt, hModule)
          LoadFunctionImpl(DTWAIN_ArrayFrameSetAt, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtANSIString, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtANSIStringPtr, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtFloat, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAt, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtLong64, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtLong, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtStringA, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtString, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtStringPtr, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtStringW, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtWideString, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetAtWideStringPtr, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetBuffer, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetCount, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetMaxStringLength, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetSourceAt, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetStringLength, hModule)
          LoadFunctionImpl(DTWAIN_ArrayGetType, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInit, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtANSIString, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtANSIStringN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtFloat, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtFloatN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAt, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtLong64, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtLong64N, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtLong, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtLongN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtStringA, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtString, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtStringNA, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtStringN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtStringNW, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtStringW, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtWideString, hModule)
          LoadFunctionImpl(DTWAIN_ArrayInsertAtWideStringN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayRemoveAll, hModule)
          LoadFunctionImpl(DTWAIN_ArrayRemoveAt, hModule)
          LoadFunctionImpl(DTWAIN_ArrayRemoveAtN, hModule)
          LoadFunctionImpl(DTWAIN_ArrayResize, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtANSIString, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtFloat, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAt, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtLong64, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtLong, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtStringA, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtString, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtStringW, hModule)
          LoadFunctionImpl(DTWAIN_ArraySetAtWideString, hModule)
          LoadFunctionImpl(DTWAIN_CallCallback64, hModule)
          LoadFunctionImpl(DTWAIN_CallCallback, hModule)
          LoadFunctionImpl(DTWAIN_CallDSMProc, hModule)
          LoadFunctionImpl(DTWAIN_CheckHandles, hModule)
          LoadFunctionImpl(DTWAIN_ClearBuffers, hModule)
          LoadFunctionImpl(DTWAIN_ClearErrorBuffer, hModule)
          LoadFunctionImpl(DTWAIN_ClearPage, hModule)
          LoadFunctionImpl(DTWAIN_ClearPDFText, hModule)
          LoadFunctionImpl(DTWAIN_CloseSource, hModule)
          LoadFunctionImpl(DTWAIN_CloseSourceUI, hModule)
          LoadFunctionImpl(DTWAIN_ConvertDIBToBitmap, hModule)
          LoadFunctionImpl(DTWAIN_CreateAcquisitionArray, hModule)
          LoadFunctionImpl(DTWAIN_CreatePDFTextElement, hModule)
          LoadFunctionImpl(DTWAIN_DestroyAcquisitionArray, hModule)
          LoadFunctionImpl(DTWAIN_DestroyPDFTextElement, hModule)
          LoadFunctionImpl(DTWAIN_DisableAppWindow, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutoBorderDetect, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutoBright, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutoDeskew, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutoFeed, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutomaticSenseMedium, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutoRotate, hModule)
          LoadFunctionImpl(DTWAIN_EnableAutoScan, hModule)
          LoadFunctionImpl(DTWAIN_EnableDuplex, hModule)
          LoadFunctionImpl(DTWAIN_EnableFeeder, hModule)
          LoadFunctionImpl(DTWAIN_EnableIndicator, hModule)
          LoadFunctionImpl(DTWAIN_EnableJobFileHandling, hModule)
          LoadFunctionImpl(DTWAIN_EnableLamp, hModule)
          LoadFunctionImpl(DTWAIN_EnableMsgNotify, hModule)
          LoadFunctionImpl(DTWAIN_EnablePatchDetect, hModule)
          LoadFunctionImpl(DTWAIN_EnablePrinter, hModule)
          LoadFunctionImpl(DTWAIN_EnableThumbnail, hModule)
          LoadFunctionImpl(DTWAIN_EndThread, hModule)
          LoadFunctionImpl(DTWAIN_EndTwainSession, hModule)
          LoadFunctionImpl(DTWAIN_EnumAlarmsEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumAlarms, hModule)
          LoadFunctionImpl(DTWAIN_EnumAlarmVolumesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumAlarmVolumes, hModule)
          LoadFunctionImpl(DTWAIN_EnumAudioXferMechsEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumAudioXferMechs, hModule)
          LoadFunctionImpl(DTWAIN_EnumAutoFeedValuesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumAutoFeedValues, hModule)
          LoadFunctionImpl(DTWAIN_EnumAutomaticCapturesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumAutomaticCaptures, hModule)
          LoadFunctionImpl(DTWAIN_EnumAutomaticSenseMediumEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumAutomaticSenseMedium, hModule)
          LoadFunctionImpl(DTWAIN_EnumBitDepthsEx2, hModule)
          LoadFunctionImpl(DTWAIN_EnumBitDepthsEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumBitDepths, hModule)
          LoadFunctionImpl(DTWAIN_EnumBottomCameras, hModule)
          LoadFunctionImpl(DTWAIN_EnumBrightnessValuesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumBrightnessValues, hModule)
          LoadFunctionImpl(DTWAIN_EnumCameras, hModule)
          LoadFunctionImpl(DTWAIN_EnumCompressionTypesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumCompressionTypes, hModule)
          LoadFunctionImpl(DTWAIN_EnumContrastValuesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumContrastValues, hModule)
          LoadFunctionImpl(DTWAIN_EnumCustomCapsEx2, hModule)
          LoadFunctionImpl(DTWAIN_EnumCustomCaps, hModule)
          LoadFunctionImpl(DTWAIN_EnumDoubleFeedDetectLengthsEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumDoubleFeedDetectLengths, hModule)
          LoadFunctionImpl(DTWAIN_EnumDoubleFeedDetectValuesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumDoubleFeedDetectValues, hModule)
          LoadFunctionImpl(DTWAIN_EnumExtendedCapsEx2, hModule)
          LoadFunctionImpl(DTWAIN_EnumExtendedCapsEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumExtendedCaps, hModule)
          LoadFunctionImpl(DTWAIN_EnumExtImageInfoTypes, hModule)
          LoadFunctionImpl(DTWAIN_EnumFileTypeBitsPerPixel, hModule)
          LoadFunctionImpl(DTWAIN_EnumFileXferFormatsEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumFileXferFormats, hModule)
          LoadFunctionImpl(DTWAIN_EnumHalftonesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumHalftones, hModule)
          LoadFunctionImpl(DTWAIN_EnumHighlightValuesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumHighlightValues, hModule)
          LoadFunctionImpl(DTWAIN_EnumJobControlsEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumJobControls, hModule)
          LoadFunctionImpl(DTWAIN_EnumLightPathsEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumLightPaths, hModule)
          LoadFunctionImpl(DTWAIN_EnumLightSourcesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumLightSources, hModule)
          LoadFunctionImpl(DTWAIN_EnumMaxBuffersEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumMaxBuffers, hModule)
          LoadFunctionImpl(DTWAIN_EnumNoiseFiltersEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumNoiseFilters, hModule)
          LoadFunctionImpl(DTWAIN_EnumOCRInterfaces, hModule)
          LoadFunctionImpl(DTWAIN_EnumOCRSupportedCaps, hModule)
          LoadFunctionImpl(DTWAIN_EnumOrientationsEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumOrientations, hModule)
          LoadFunctionImpl(DTWAIN_EnumOverscanValuesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumOverscanValues, hModule)
          LoadFunctionImpl(DTWAIN_EnumPaperSizesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumPaperSizes, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchCodesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchCodes, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchMaxPrioritiesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchMaxPriorities, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchMaxRetriesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchMaxRetries, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchPrioritiesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchPriorities, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchSearchModesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchSearchModes, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchTimeOutValuesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumPatchTimeOutValues, hModule)
          LoadFunctionImpl(DTWAIN_EnumPixelTypes, hModule)
          LoadFunctionImpl(DTWAIN_EnumPrinterStringModesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumPrinterStringModes, hModule)
          LoadFunctionImpl(DTWAIN_EnumResolutionValuesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumResolutionValues, hModule)
          LoadFunctionImpl(DTWAIN_EnumShadowValuesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumShadowValues, hModule)
          LoadFunctionImpl(DTWAIN_EnumSourcesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumSources, hModule)
          LoadFunctionImpl(DTWAIN_EnumSourceUnitsEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumSourceUnits, hModule)
          LoadFunctionImpl(DTWAIN_EnumSourceValuesA, hModule)
          LoadFunctionImpl(DTWAIN_EnumSourceValues, hModule)
          LoadFunctionImpl(DTWAIN_EnumSourceValuesW, hModule)
          LoadFunctionImpl(DTWAIN_EnumSupportedCapsEx2, hModule)
          LoadFunctionImpl(DTWAIN_EnumSupportedCapsEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumSupportedCaps, hModule)
          LoadFunctionImpl(DTWAIN_EnumSupportedSinglePageFileTypes, hModule)
          LoadFunctionImpl(DTWAIN_EnumSupportedMultiPageFileTypes, hModule)
          LoadFunctionImpl(DTWAIN_EnumThresholdValuesEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumThresholdValues, hModule)
          LoadFunctionImpl(DTWAIN_EnumTopCameras, hModule)
          LoadFunctionImpl(DTWAIN_EnumTwainPrintersArrayEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumTwainPrintersArray, hModule)
          LoadFunctionImpl(DTWAIN_EnumTwainPrintersEx, hModule)
          LoadFunctionImpl(DTWAIN_EnumTwainPrinters, hModule)
          LoadFunctionImpl(DTWAIN_ExecuteOCRA, hModule)
          LoadFunctionImpl(DTWAIN_ExecuteOCR, hModule)
          LoadFunctionImpl(DTWAIN_ExecuteOCRW, hModule)
          LoadFunctionImpl(DTWAIN_FeedPage, hModule)
          LoadFunctionImpl(DTWAIN_FlipBitmap, hModule)
          LoadFunctionImpl(DTWAIN_FlushAcquiredPages, hModule)
          LoadFunctionImpl(DTWAIN_ForceAcquireBitDepth, hModule)
          LoadFunctionImpl(DTWAIN_ForceScanOnNoUI, hModule)
          LoadFunctionImpl(DTWAIN_FrameCreate, hModule)
          LoadFunctionImpl(DTWAIN_FrameCreateStringA, hModule)
          LoadFunctionImpl(DTWAIN_FrameCreateString, hModule)
          LoadFunctionImpl(DTWAIN_FrameCreateStringW, hModule)
          LoadFunctionImpl(DTWAIN_FrameDestroy, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetAll, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetAllStringA, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetAllString, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetAllStringW, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetValue, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetValueStringA, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetValueString, hModule)
          LoadFunctionImpl(DTWAIN_FrameGetValueStringW, hModule)
          LoadFunctionImpl(DTWAIN_FrameIsValid, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetAll, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetAllStringA, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetAllString, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetAllStringW, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetValue, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetValueStringA, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetValueString, hModule)
          LoadFunctionImpl(DTWAIN_FrameSetValueStringW, hModule)
          LoadFunctionImpl(DTWAIN_FreeExtImageInfo, hModule)
          LoadFunctionImpl(DTWAIN_FreeMemoryEx, hModule)
          LoadFunctionImpl(DTWAIN_FreeMemory, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireArea2, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireArea2StringA, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireArea2String, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireArea2StringW, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireArea, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquiredImageArray, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquiredImage, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireMetrics, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireStripBuffer, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireStripData, hModule)
          LoadFunctionImpl(DTWAIN_GetAcquireStripSizes, hModule)
          LoadFunctionImpl(DTWAIN_GetAlarmVolume, hModule)
          LoadFunctionImpl(DTWAIN_GetAPIHandleStatus, hModule)
          LoadFunctionImpl(DTWAIN_GetAppInfoA, hModule)
          LoadFunctionImpl(DTWAIN_GetAppInfo, hModule)
          LoadFunctionImpl(DTWAIN_GetAppInfoW, hModule)
          LoadFunctionImpl(DTWAIN_GetAuthorA, hModule)
          LoadFunctionImpl(DTWAIN_GetAuthor, hModule)
          LoadFunctionImpl(DTWAIN_GetAuthorW, hModule)
          LoadFunctionImpl(DTWAIN_GetBatteryMinutes, hModule)
          LoadFunctionImpl(DTWAIN_GetBatteryPercent, hModule)
          LoadFunctionImpl(DTWAIN_GetBitDepth, hModule)
          LoadFunctionImpl(DTWAIN_GetBlankPageAutoDetection, hModule)
          LoadFunctionImpl(DTWAIN_GetBrightness, hModule)
          LoadFunctionImpl(DTWAIN_GetBrightnessStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetBrightnessString, hModule)
          LoadFunctionImpl(DTWAIN_GetBrightnessStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetCallback64, hModule)
          LoadFunctionImpl(DTWAIN_GetCallback, hModule)
          LoadFunctionImpl(DTWAIN_GetCapArrayType, hModule)
          LoadFunctionImpl(DTWAIN_GetCapContainerEx, hModule)
          LoadFunctionImpl(DTWAIN_GetCapContainer, hModule)
          LoadFunctionImpl(DTWAIN_GetCapDataType, hModule)
          LoadFunctionImpl(DTWAIN_GetCapFromNameA, hModule)
          LoadFunctionImpl(DTWAIN_GetCapFromName, hModule)
          LoadFunctionImpl(DTWAIN_GetCapFromNameW, hModule)
          LoadFunctionImpl(DTWAIN_GetCapOperations, hModule)
          LoadFunctionImpl(DTWAIN_GetCaptionA, hModule)
          LoadFunctionImpl(DTWAIN_GetCaption, hModule)
          LoadFunctionImpl(DTWAIN_GetCaptionW, hModule)
          LoadFunctionImpl(DTWAIN_GetCapValuesEx2, hModule)
          LoadFunctionImpl(DTWAIN_GetCapValuesEx, hModule)
          LoadFunctionImpl(DTWAIN_GetCapValues, hModule)
          LoadFunctionImpl(DTWAIN_GetCompressionSize, hModule)
          LoadFunctionImpl(DTWAIN_GetCompressionType, hModule)
          LoadFunctionImpl(DTWAIN_GetConditionCodeStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetConditionCodeString, hModule)
          LoadFunctionImpl(DTWAIN_GetConditionCodeStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetContrast, hModule)
          LoadFunctionImpl(DTWAIN_GetContrastStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetContrastString, hModule)
          LoadFunctionImpl(DTWAIN_GetContrastStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetCountry, hModule)
          LoadFunctionImpl(DTWAIN_GetCurrentAcquiredImage, hModule)
          LoadFunctionImpl(DTWAIN_GetCurrentFileNameA, hModule)
          LoadFunctionImpl(DTWAIN_GetCurrentFileName, hModule)
          LoadFunctionImpl(DTWAIN_GetCurrentFileNameW, hModule)
          LoadFunctionImpl(DTWAIN_GetCurrentPageNum, hModule)
          LoadFunctionImpl(DTWAIN_GetCurrentRetryCount, hModule)
          LoadFunctionImpl(DTWAIN_GetCustomDSData, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceEventEx, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceEvent, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceEventInfo, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceNotifications, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceTimeDateA, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceTimeDate, hModule)
          LoadFunctionImpl(DTWAIN_GetDeviceTimeDateW, hModule)
          LoadFunctionImpl(DTWAIN_GetDoubleFeedDetectLength, hModule)
          LoadFunctionImpl(DTWAIN_GetDoubleFeedDetectValues, hModule)
          LoadFunctionImpl(DTWAIN_GetDSMFullNameA, hModule)
          LoadFunctionImpl(DTWAIN_GetDSMFullName, hModule)
          LoadFunctionImpl(DTWAIN_GetDSMFullNameW, hModule)
          LoadFunctionImpl(DTWAIN_GetDSMSearchOrder, hModule)
          LoadFunctionImpl(DTWAIN_GetDTWAINHandle, hModule)
          LoadFunctionImpl(DTWAIN_GetDuplexType, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorBuffer, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorBufferThreshold, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorCallback64, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorCallback, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorString, hModule)
          LoadFunctionImpl(DTWAIN_GetErrorStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetExtCapFromNameA, hModule)
          LoadFunctionImpl(DTWAIN_GetExtCapFromName, hModule)
          LoadFunctionImpl(DTWAIN_GetExtCapFromNameW, hModule)
          LoadFunctionImpl(DTWAIN_GetExtImageInfoData, hModule)
          LoadFunctionImpl(DTWAIN_GetExtImageInfo, hModule)
          LoadFunctionImpl(DTWAIN_GetExtImageInfoItem, hModule)
          LoadFunctionImpl(DTWAIN_GetExtNameFromCapA, hModule)
          LoadFunctionImpl(DTWAIN_GetExtNameFromCap, hModule)
          LoadFunctionImpl(DTWAIN_GetExtNameFromCapW, hModule)
          LoadFunctionImpl(DTWAIN_GetFeederAlignment, hModule)
          LoadFunctionImpl(DTWAIN_GetFeederFuncs, hModule)
          LoadFunctionImpl(DTWAIN_GetFeederOrder, hModule)
          LoadFunctionImpl(DTWAIN_GetFileTypeName, hModule)
          LoadFunctionImpl(DTWAIN_GetFileTypeNameA, hModule)
          LoadFunctionImpl(DTWAIN_GetFileTypeNameW, hModule)
          LoadFunctionImpl(DTWAIN_GetFileTypeExtensions, hModule)
          LoadFunctionImpl(DTWAIN_GetFileTypeExtensionsA, hModule)
          LoadFunctionImpl(DTWAIN_GetFileTypeExtensionsW, hModule)
          LoadFunctionImpl(DTWAIN_GetHalftoneA, hModule)
          LoadFunctionImpl(DTWAIN_GetHalftone, hModule)
          LoadFunctionImpl(DTWAIN_GetHalftoneW, hModule)
          LoadFunctionImpl(DTWAIN_GetHighlight, hModule)
          LoadFunctionImpl(DTWAIN_GetHighlightStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetHighlightString, hModule)
          LoadFunctionImpl(DTWAIN_GetHighlightStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetImageInfo, hModule)
          LoadFunctionImpl(DTWAIN_GetImageInfoStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetImageInfoString, hModule)
          LoadFunctionImpl(DTWAIN_GetImageInfoStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetJobControl, hModule)
          LoadFunctionImpl(DTWAIN_GetJpegValues, hModule)
          LoadFunctionImpl(DTWAIN_GetLanguage, hModule)
          LoadFunctionImpl(DTWAIN_GetLastError, hModule)
          LoadFunctionImpl(DTWAIN_GetLibraryPathA, hModule)
          LoadFunctionImpl(DTWAIN_GetLibraryPath, hModule)
          LoadFunctionImpl(DTWAIN_GetLibraryPathW, hModule)
          LoadFunctionImpl(DTWAIN_GetLightPath, hModule)
          LoadFunctionImpl(DTWAIN_GetLightSource, hModule)
          LoadFunctionImpl(DTWAIN_GetLightSources, hModule)
          LoadFunctionImpl(DTWAIN_GetLoggerCallbackA, hModule)
          LoadFunctionImpl(DTWAIN_GetLoggerCallback, hModule)
          LoadFunctionImpl(DTWAIN_GetLoggerCallbackW, hModule)
          LoadFunctionImpl(DTWAIN_GetManualDuplexCount, hModule)
          LoadFunctionImpl(DTWAIN_GetMaxAcquisitions, hModule)
          LoadFunctionImpl(DTWAIN_GetMaxBuffers, hModule)
          LoadFunctionImpl(DTWAIN_GetMaxPagesToAcquire, hModule)
          LoadFunctionImpl(DTWAIN_GetMaxRetryAttempts, hModule)
          LoadFunctionImpl(DTWAIN_GetNameFromCapA, hModule)
          LoadFunctionImpl(DTWAIN_GetNameFromCap, hModule)
          LoadFunctionImpl(DTWAIN_GetNameFromCapW, hModule)
          LoadFunctionImpl(DTWAIN_GetNoiseFilter, hModule)
          LoadFunctionImpl(DTWAIN_GetNumAcquiredImages, hModule)
          LoadFunctionImpl(DTWAIN_GetNumAcquisitions, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRCapValues, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRErrorStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRErrorString, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRErrorStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRLastError, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRManufacturerA, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRManufacturer, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRManufacturerW, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRProductFamilyA, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRProductFamily, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRProductFamilyW, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRProductNameA, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRProductName, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRProductNameW, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextA, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRText, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextInfoFloatEx, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextInfoFloat, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextInfoHandle, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextInfoLongEx, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextInfoLong, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRTextW, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRVersionInfoA, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRVersionInfo, hModule)
          LoadFunctionImpl(DTWAIN_GetOCRVersionInfoW, hModule)
          LoadFunctionImpl(DTWAIN_GetOrientation, hModule)
          LoadFunctionImpl(DTWAIN_GetOverscan, hModule)
          LoadFunctionImpl(DTWAIN_GetPaperSize, hModule)
          LoadFunctionImpl(DTWAIN_GetPatchMaxPriorities, hModule)
          LoadFunctionImpl(DTWAIN_GetPatchMaxRetries, hModule)
          LoadFunctionImpl(DTWAIN_GetPatchPriorities, hModule)
          LoadFunctionImpl(DTWAIN_GetPatchSearchMode, hModule)
          LoadFunctionImpl(DTWAIN_GetPatchTimeOut, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFTextElementFloat, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFTextElementLong, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFTextElementStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFTextElementString, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFTextElementStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFType1FontNameA, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFType1FontName, hModule)
          LoadFunctionImpl(DTWAIN_GetPDFType1FontNameW, hModule)
          LoadFunctionImpl(DTWAIN_GetPixelFlavor, hModule)
          LoadFunctionImpl(DTWAIN_GetPixelType, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinter, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinterStartNumber, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinterStringMode, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinterStrings, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinterSuffixStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinterSuffixString, hModule)
          LoadFunctionImpl(DTWAIN_GetPrinterSuffixStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetRegisteredMsg, hModule)
          LoadFunctionImpl(DTWAIN_GetResolution, hModule)
          LoadFunctionImpl(DTWAIN_GetResolutionStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetResolutionString, hModule)
          LoadFunctionImpl(DTWAIN_GetResolutionStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetRotation, hModule)
          LoadFunctionImpl(DTWAIN_GetRotationStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetRotationString, hModule)
          LoadFunctionImpl(DTWAIN_GetRotationStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetSaveFileNameA, hModule)
          LoadFunctionImpl(DTWAIN_GetSaveFileName, hModule)
          LoadFunctionImpl(DTWAIN_GetSaveFileNameW, hModule)
          LoadFunctionImpl(DTWAIN_GetShadow, hModule)
          LoadFunctionImpl(DTWAIN_GetShadowStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetShadowString, hModule)
          LoadFunctionImpl(DTWAIN_GetShadowStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetShortVersionStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetShortVersionString, hModule)
          LoadFunctionImpl(DTWAIN_GetShortVersionStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceAcquisitions, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceIDEx, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceID, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceManufacturerA, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceManufacturer, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceManufacturerW, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceProductFamilyA, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceProductFamily, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceProductFamilyW, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceProductNameA, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceProductName, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceProductNameW, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceUnit, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceVersionInfoA, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceVersionInfo, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceVersionInfoW, hModule)
          LoadFunctionImpl(DTWAIN_GetSourceVersionNumber, hModule)
          LoadFunctionImpl(DTWAIN_GetStaticLibVersion, hModule)
          LoadFunctionImpl(DTWAIN_GetTempFileDirectoryA, hModule)
          LoadFunctionImpl(DTWAIN_GetTempFileDirectory, hModule)
          LoadFunctionImpl(DTWAIN_GetTempFileDirectoryW, hModule)
          LoadFunctionImpl(DTWAIN_GetThreshold, hModule)
          LoadFunctionImpl(DTWAIN_GetThresholdStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetThresholdString, hModule)
          LoadFunctionImpl(DTWAIN_GetThresholdStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetTimeDateA, hModule)
          LoadFunctionImpl(DTWAIN_GetTimeDate, hModule)
          LoadFunctionImpl(DTWAIN_GetTimeDateW, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainAppIDEx, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainAppID, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainAvailability, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainCountryNameA, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainCountryName, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainCountryNameW, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainCountryValueA, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainCountryValue, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainCountryValueW, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainHwnd, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainLanguageNameA, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainLanguageName, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainLanguageNameW, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainLanguageValueA, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainLanguageValue, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainLanguageValueW, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainMode, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainNameFromConstantA, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainNameFromConstantW, hModule)
          LoadFunctionImpl(DTWAIN_GetTwainTimeout, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionEx, hModule)
          LoadFunctionImpl(DTWAIN_GetVersion, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionInfoA, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionInfo, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionInfoW, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionString, hModule)
          LoadFunctionImpl(DTWAIN_GetVersionStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetXResolution, hModule)
          LoadFunctionImpl(DTWAIN_GetXResolutionStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetXResolutionString, hModule)
          LoadFunctionImpl(DTWAIN_GetXResolutionStringW, hModule)
          LoadFunctionImpl(DTWAIN_GetYResolution, hModule)
          LoadFunctionImpl(DTWAIN_GetYResolutionStringA, hModule)
          LoadFunctionImpl(DTWAIN_GetYResolutionString, hModule)
          LoadFunctionImpl(DTWAIN_GetYResolutionStringW, hModule)
          LoadFunctionImpl(DTWAIN_InitExtImageInfo, hModule)
          LoadFunctionImpl(DTWAIN_InitImageFileAppendA, hModule)
          LoadFunctionImpl(DTWAIN_InitImageFileAppend, hModule)
          LoadFunctionImpl(DTWAIN_InitImageFileAppendW, hModule)
          LoadFunctionImpl(DTWAIN_InitOCRInterface, hModule)
          LoadFunctionImpl(DTWAIN_IsAcquiring, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoBorderDetectEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoBorderDetectSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoBrightEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoBrightSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoDeskewEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoDeskewSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoFeedEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoFeedSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsAutomaticSenseMediumEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsAutomaticSenseMediumSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoRotateEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoRotateSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsAutoScanEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsBlankPageDetectionOn, hModule)
          LoadFunctionImpl(DTWAIN_IsCapSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsCompressionSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsCustomDSDataSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsDeviceEventSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsDeviceOnLine, hModule)
          LoadFunctionImpl(DTWAIN_IsDIBBlank, hModule)
          LoadFunctionImpl(DTWAIN_IsDIBBlankStringA, hModule)
          LoadFunctionImpl(DTWAIN_IsDIBBlankString, hModule)
          LoadFunctionImpl(DTWAIN_IsDIBBlankStringW, hModule)
          LoadFunctionImpl(DTWAIN_IsDoubleFeedDetectLengthSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsDoubleFeedDetectSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsDuplexEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsDuplexSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsExtImageInfoSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsFeederEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsFeederLoaded, hModule)
          LoadFunctionImpl(DTWAIN_IsFeederSensitive, hModule)
          LoadFunctionImpl(DTWAIN_IsFeederSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsFileSystemSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsFileXferSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsIndicatorEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsIndicatorSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsInitialized, hModule)
          LoadFunctionImpl(DTWAIN_IsJobControlSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsJPEGSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsLampEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsLampSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsLightPathSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsLightSourceSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsMaxBuffersSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsMsgNotifyEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsOCREngineActivated, hModule)
          LoadFunctionImpl(DTWAIN_IsOrientationSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsOverscanSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsPaperDetectable, hModule)
          LoadFunctionImpl(DTWAIN_IsPaperSizeSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsPatchCapsSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsPatchDetectEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsPatchSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsPDFSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsPixelTypeSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsPNGSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsPrinterEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsPrinterSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsRotationSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsSessionEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsSkipImageInfoError, hModule)
          LoadFunctionImpl(DTWAIN_IsSourceAcquiring, hModule)
          LoadFunctionImpl(DTWAIN_IsSourceOpen, hModule)
          LoadFunctionImpl(DTWAIN_IsSourceSelected, hModule)
          LoadFunctionImpl(DTWAIN_IsThumbnailEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsThumbnailSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsTIFFSupported, hModule)
          LoadFunctionImpl(DTWAIN_IsTwainAvailable, hModule)
          LoadFunctionImpl(DTWAIN_IsTwainMsg, hModule)
          LoadFunctionImpl(DTWAIN_IsUIControllable, hModule)
          LoadFunctionImpl(DTWAIN_IsUIEnabled, hModule)
          LoadFunctionImpl(DTWAIN_IsUIOnlySupported, hModule)
          LoadFunctionImpl(DTWAIN_LoadCustomStringResourcesA, hModule)
          LoadFunctionImpl(DTWAIN_LoadCustomStringResources, hModule)
          LoadFunctionImpl(DTWAIN_LoadCustomStringResourcesW, hModule)
          LoadFunctionImpl(DTWAIN_LoadLanguageResource, hModule)
          LoadFunctionImpl(DTWAIN_LockMemoryEx, hModule)
          LoadFunctionImpl(DTWAIN_LockMemory, hModule)
          LoadFunctionImpl(DTWAIN_LogMessageA, hModule)
          LoadFunctionImpl(DTWAIN_LogMessage, hModule)
          LoadFunctionImpl(DTWAIN_LogMessageW, hModule)
          LoadFunctionImpl(DTWAIN_MakeRGB, hModule)
          LoadFunctionImpl(DTWAIN_OpenSource, hModule)
          LoadFunctionImpl(DTWAIN_OpenSourcesOnSelect, hModule)
          LoadFunctionImpl(DTWAIN_RangeCreateFromCap, hModule)
          LoadFunctionImpl(DTWAIN_RangeCreate, hModule)
          LoadFunctionImpl(DTWAIN_RangeDestroy, hModule)
          LoadFunctionImpl(DTWAIN_RangeExpand, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetAllFloat, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetAllFloatStringA, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetAllFloatString, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetAllFloatStringW, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetAll, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetAllLong, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetCount, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetExpValueFloat, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetExpValueFloatStringA, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetExpValueFloatString, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetExpValueFloatStringW, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetExpValue, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetExpValueLong, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetNearestValue, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetPosFloat, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetPosFloatStringA, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetPosFloatString, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetPosFloatStringW, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetPos, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetPosLong, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetValueFloat, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetValueFloatStringA, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetValueFloatString, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetValueFloatStringW, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetValue, hModule)
          LoadFunctionImpl(DTWAIN_RangeGetValueLong, hModule)
          LoadFunctionImpl(DTWAIN_RangeIsValid, hModule)
          LoadFunctionImpl(DTWAIN_RangeNearestValueFloat, hModule)
          LoadFunctionImpl(DTWAIN_RangeNearestValueFloatStringA, hModule)
          LoadFunctionImpl(DTWAIN_RangeNearestValueFloatString, hModule)
          LoadFunctionImpl(DTWAIN_RangeNearestValueFloatStringW, hModule)
          LoadFunctionImpl(DTWAIN_RangeNearestValueLong, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetAllFloat, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetAllFloatStringA, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetAllFloatString, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetAllFloatStringW, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetAll, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetAllLong, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetValueFloat, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetValueFloatStringA, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetValueFloatString, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetValueFloatStringW, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetValue, hModule)
          LoadFunctionImpl(DTWAIN_RangeSetValueLong, hModule)
          LoadFunctionImpl(DTWAIN_ResetPDFTextElement, hModule)
          LoadFunctionImpl(DTWAIN_RewindPage, hModule)
          LoadFunctionImpl(DTWAIN_SelectDefaultOCREngine, hModule)
          LoadFunctionImpl(DTWAIN_SelectDefaultSource, hModule)
          LoadFunctionImpl(DTWAIN_SelectOCREngineByNameA, hModule)
          LoadFunctionImpl(DTWAIN_SelectOCREngineByName, hModule)
          LoadFunctionImpl(DTWAIN_SelectOCREngineByNameW, hModule)
          LoadFunctionImpl(DTWAIN_SelectOCREngine, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource2A, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource2ExA, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource2Ex, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource2ExW, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource2, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource2W, hModule)
          LoadFunctionImpl(DTWAIN_SelectSourceByNameA, hModule)
          LoadFunctionImpl(DTWAIN_SelectSourceByName, hModule)
          LoadFunctionImpl(DTWAIN_SelectSourceByNameW, hModule)
          LoadFunctionImpl(DTWAIN_SelectSource, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireArea2, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireArea2StringA, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireArea2String, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireArea2StringW, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireArea, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireImageNegative, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireImageScale, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireImageScaleStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireImageScaleString, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireImageScaleStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetAcquireStripBuffer, hModule)
          LoadFunctionImpl(DTWAIN_SetAlarms, hModule)
          LoadFunctionImpl(DTWAIN_SetAlarmVolume, hModule)
          LoadFunctionImpl(DTWAIN_SetAllCapsToDefault, hModule)
          LoadFunctionImpl(DTWAIN_SetAppInfoA, hModule)
          LoadFunctionImpl(DTWAIN_SetAppInfo, hModule)
          LoadFunctionImpl(DTWAIN_SetAppInfoW, hModule)
          LoadFunctionImpl(DTWAIN_SetAuthorA, hModule)
          LoadFunctionImpl(DTWAIN_SetAuthor, hModule)
          LoadFunctionImpl(DTWAIN_SetAuthorW, hModule)
          LoadFunctionImpl(DTWAIN_SetAvailablePrintersArray, hModule)
          LoadFunctionImpl(DTWAIN_SetAvailablePrinters, hModule)
          LoadFunctionImpl(DTWAIN_SetBitDepth, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetection, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionString, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionEx, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionExString, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionExStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetBlankPageDetectionExStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetBrightness, hModule)
          LoadFunctionImpl(DTWAIN_SetBrightnessStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetBrightnessString, hModule)
          LoadFunctionImpl(DTWAIN_SetBrightnessStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetCallback64, hModule)
          LoadFunctionImpl(DTWAIN_SetCallback, hModule)
          LoadFunctionImpl(DTWAIN_SetCameraA, hModule)
          LoadFunctionImpl(DTWAIN_SetCamera, hModule)
          LoadFunctionImpl(DTWAIN_SetCameraW, hModule)
          LoadFunctionImpl(DTWAIN_SetCaptionA, hModule)
          LoadFunctionImpl(DTWAIN_SetCaption, hModule)
          LoadFunctionImpl(DTWAIN_SetCaptionW, hModule)
          LoadFunctionImpl(DTWAIN_SetCapValuesEx2, hModule)
          LoadFunctionImpl(DTWAIN_SetCapValuesEx, hModule)
          LoadFunctionImpl(DTWAIN_SetCapValues, hModule)
          LoadFunctionImpl(DTWAIN_SetCompressionType, hModule)
          LoadFunctionImpl(DTWAIN_SetContrast, hModule)
          LoadFunctionImpl(DTWAIN_SetContrastStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetContrastString, hModule)
          LoadFunctionImpl(DTWAIN_SetContrastStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetCountry, hModule)
          LoadFunctionImpl(DTWAIN_SetCurrentRetryCount, hModule)
          LoadFunctionImpl(DTWAIN_SetCustomDSData, hModule)
          LoadFunctionImpl(DTWAIN_SetCustomFileSave, hModule)
          LoadFunctionImpl(DTWAIN_SetDefaultSource, hModule)
          LoadFunctionImpl(DTWAIN_SetDeviceNotifications, hModule)
          LoadFunctionImpl(DTWAIN_SetDeviceTimeDateA, hModule)
          LoadFunctionImpl(DTWAIN_SetDeviceTimeDate, hModule)
          LoadFunctionImpl(DTWAIN_SetDeviceTimeDateW, hModule)
          LoadFunctionImpl(DTWAIN_SetDoubleFeedDetectLength, hModule)
          LoadFunctionImpl(DTWAIN_SetDoubleFeedDetectLengthStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetDoubleFeedDetectLengthString, hModule)
          LoadFunctionImpl(DTWAIN_SetDoubleFeedDetectLengthStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetDoubleFeedDetectValues, hModule)
          LoadFunctionImpl(DTWAIN_SetDSMSearchOrderExA, hModule)
          LoadFunctionImpl(DTWAIN_SetDSMSearchOrderEx, hModule)
          LoadFunctionImpl(DTWAIN_SetDSMSearchOrderExW, hModule)
          LoadFunctionImpl(DTWAIN_SetDSMSearchOrder, hModule)
          LoadFunctionImpl(DTWAIN_SetEOJDetectValue, hModule)
          LoadFunctionImpl(DTWAIN_SetErrorBufferThreshold, hModule)
          LoadFunctionImpl(DTWAIN_SetErrorCallback64, hModule)
          LoadFunctionImpl(DTWAIN_SetErrorCallback, hModule)
          LoadFunctionImpl(DTWAIN_SetFeederAlignment, hModule)
          LoadFunctionImpl(DTWAIN_SetFeederOrder, hModule)
          LoadFunctionImpl(DTWAIN_SetFileAutoIncrement, hModule)
          LoadFunctionImpl(DTWAIN_SetFileSavePosA, hModule)
          LoadFunctionImpl(DTWAIN_SetFileSavePos, hModule)
          LoadFunctionImpl(DTWAIN_SetFileSavePosW, hModule)
          LoadFunctionImpl(DTWAIN_SetFileXferFormat, hModule)
          LoadFunctionImpl(DTWAIN_SetHalftoneA, hModule)
          LoadFunctionImpl(DTWAIN_SetHalftone, hModule)
          LoadFunctionImpl(DTWAIN_SetHalftoneW, hModule)
          LoadFunctionImpl(DTWAIN_SetHighlight, hModule)
          LoadFunctionImpl(DTWAIN_SetHighlightStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetHighlightString, hModule)
          LoadFunctionImpl(DTWAIN_SetHighlightStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetJobControl, hModule)
          LoadFunctionImpl(DTWAIN_SetJpegValues, hModule)
          LoadFunctionImpl(DTWAIN_SetLanguage, hModule)
          LoadFunctionImpl(DTWAIN_SetLastError, hModule)
          LoadFunctionImpl(DTWAIN_SetLightPathEx, hModule)
          LoadFunctionImpl(DTWAIN_SetLightPath, hModule)
          LoadFunctionImpl(DTWAIN_SetLightSource, hModule)
          LoadFunctionImpl(DTWAIN_SetLightSources, hModule)
          LoadFunctionImpl(DTWAIN_SetLoggerCallbackA, hModule)
          LoadFunctionImpl(DTWAIN_SetLoggerCallback, hModule)
          LoadFunctionImpl(DTWAIN_SetLoggerCallbackW, hModule)
          LoadFunctionImpl(DTWAIN_SetManualDuplexMode, hModule)
          LoadFunctionImpl(DTWAIN_SetMaxAcquisitions, hModule)
          LoadFunctionImpl(DTWAIN_SetMaxBuffers, hModule)
          LoadFunctionImpl(DTWAIN_SetMaxRetryAttempts, hModule)
          LoadFunctionImpl(DTWAIN_SetMultipageScanMode, hModule)
          LoadFunctionImpl(DTWAIN_SetNoiseFilter, hModule)
          LoadFunctionImpl(DTWAIN_SetOCRCapValues, hModule)
          LoadFunctionImpl(DTWAIN_SetOrientation, hModule)
          LoadFunctionImpl(DTWAIN_SetOverscan, hModule)
          LoadFunctionImpl(DTWAIN_SetPaperSize, hModule)
          LoadFunctionImpl(DTWAIN_SetPatchMaxPriorities, hModule)
          LoadFunctionImpl(DTWAIN_SetPatchMaxRetries, hModule)
          LoadFunctionImpl(DTWAIN_SetPatchPriorities, hModule)
          LoadFunctionImpl(DTWAIN_SetPatchSearchMode, hModule)
          LoadFunctionImpl(DTWAIN_SetPatchTimeOut, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFAESEncryption, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFASCIICompression, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFAuthorA, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFAuthor, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFAuthorW, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFCompression, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFCreatorA, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFCreator, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFCreatorW, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFEncryptionA, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFEncryption, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFEncryptionW, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFJpegQuality, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFKeywordsA, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFKeywords, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFKeywordsW, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFOCRConversion, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFOCRMode, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFOrientation, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageScale, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageScaleStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageScaleString, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageScaleStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageSize, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageSizeStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageSizeString, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPageSizeStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFPolarity, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFProducerA, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFProducer, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFProducerW, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFSubjectA, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFSubject, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFSubjectW, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTextElementFloat, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTextElementLong, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTextElementStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTextElementString, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTextElementStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTitleA, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTitle, hModule)
          LoadFunctionImpl(DTWAIN_SetPDFTitleW, hModule)
          LoadFunctionImpl(DTWAIN_SetPixelFlavor, hModule)
          LoadFunctionImpl(DTWAIN_SetPixelType, hModule)
          LoadFunctionImpl(DTWAIN_SetPostScriptTitleA, hModule)
          LoadFunctionImpl(DTWAIN_SetPostScriptTitle, hModule)
          LoadFunctionImpl(DTWAIN_SetPostScriptTitleW, hModule)
          LoadFunctionImpl(DTWAIN_SetPostScriptType, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinter, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinterStartNumber, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinterStringMode, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinterStrings, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinterSuffixStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinterSuffixString, hModule)
          LoadFunctionImpl(DTWAIN_SetPrinterSuffixStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetQueryCapSupport, hModule)
          LoadFunctionImpl(DTWAIN_SetResolution, hModule)
          LoadFunctionImpl(DTWAIN_SetResolutionStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetResolutionString, hModule)
          LoadFunctionImpl(DTWAIN_SetResolutionStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetResourcePathA, hModule)
          LoadFunctionImpl(DTWAIN_SetResourcePath, hModule)
          LoadFunctionImpl(DTWAIN_SetResourcePathW, hModule)
          LoadFunctionImpl(DTWAIN_SetRotation, hModule)
          LoadFunctionImpl(DTWAIN_SetRotationStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetRotationString, hModule)
          LoadFunctionImpl(DTWAIN_SetRotationStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetSaveFileNameA, hModule)
          LoadFunctionImpl(DTWAIN_SetSaveFileName, hModule)
          LoadFunctionImpl(DTWAIN_SetSaveFileNameW, hModule)
          LoadFunctionImpl(DTWAIN_SetShadow, hModule)
          LoadFunctionImpl(DTWAIN_SetShadowStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetShadowString, hModule)
          LoadFunctionImpl(DTWAIN_SetShadowStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetSourceUnit, hModule)
          LoadFunctionImpl(DTWAIN_SetTempFileDirectoryA, hModule)
          LoadFunctionImpl(DTWAIN_SetTempFileDirectory, hModule)
          LoadFunctionImpl(DTWAIN_SetTempFileDirectoryW, hModule)
          LoadFunctionImpl(DTWAIN_SetThreshold, hModule)
          LoadFunctionImpl(DTWAIN_SetThresholdStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetThresholdString, hModule)
          LoadFunctionImpl(DTWAIN_SetThresholdStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetTIFFCompressType, hModule)
          LoadFunctionImpl(DTWAIN_SetTIFFInvert, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainDialogFont, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainDSM, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainLogA, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainLog, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainLogW, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainMode, hModule)
          LoadFunctionImpl(DTWAIN_SetTwainTimeout, hModule)
          LoadFunctionImpl(DTWAIN_SetUpdateDibProc, hModule)
          LoadFunctionImpl(DTWAIN_SetXResolution, hModule)
          LoadFunctionImpl(DTWAIN_SetXResolutionStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetXResolutionString, hModule)
          LoadFunctionImpl(DTWAIN_SetXResolutionStringW, hModule)
          LoadFunctionImpl(DTWAIN_SetYResolution, hModule)
          LoadFunctionImpl(DTWAIN_SetYResolutionStringA, hModule)
          LoadFunctionImpl(DTWAIN_SetYResolutionString, hModule)
          LoadFunctionImpl(DTWAIN_SetYResolutionStringW, hModule)
          LoadFunctionImpl(DTWAIN_ShowUIOnly, hModule)
          LoadFunctionImpl(DTWAIN_ShutdownOCREngine, hModule)
          LoadFunctionImpl(DTWAIN_SkipImageInfoError, hModule)
          LoadFunctionImpl(DTWAIN_StartThread, hModule)
          LoadFunctionImpl(DTWAIN_StartTwainSessionA, hModule)
          LoadFunctionImpl(DTWAIN_StartTwainSession, hModule)
          LoadFunctionImpl(DTWAIN_StartTwainSessionW, hModule)
          LoadFunctionImpl(DTWAIN_SysDestroy, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeEx2A, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeEx2, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeEx2W, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeExA, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeEx, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeExW, hModule)
          LoadFunctionImpl(DTWAIN_SysInitialize, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLibEx2A, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLibEx2, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLibEx2W, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLibExA, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLibEx, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLibExW, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeLib, hModule)
          LoadFunctionImpl(DTWAIN_SysInitializeNoBlocking, hModule)
          LoadFunctionImpl(DTWAIN_UnlockMemoryEx, hModule)
          LoadFunctionImpl(DTWAIN_UnlockMemory, hModule)
          LoadFunctionImpl(DTWAIN_UseMultipleThreads, hModule)
    }
    return 1;
}
#pragma warning (pop)

