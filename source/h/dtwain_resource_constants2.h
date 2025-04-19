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
#ifndef DTWAIN_RESOURCE_CONSTANTS2_H
#define DTWAIN_RESOURCE_CONSTANTS2_H

#define IDS_ErrNullWindow           1
#define IDS_ErrAllocationFailure    2
#define IDS_ErrInvalidDLLHandle     3
#define IDS_ErrInvalidSourceHandle  4
#define IDS_ErrTwainDLLNotFound     5
#define IDS_ErrTwainDLLInvalid      6
#define IDS_ErrInvalidSessionHandle 7
#define IDS_ErrTwainMgrInvalid      8
#define IDS_ErrTwainDLLLoad         9
#define IDS_ErrTwainSourceOpen      10
#define IDS_ErrTwainSourceClose     11
#define IDS_ErrTwainSourceNotOpened 12
#define IDS_ErrTwainResolutionMatchError    27
#define IDS_ErrInvalidFileName      28
#define IDS_ErrInvalidFileFormat    29
#define IDS_ErrSourceMgrOpen        30
#define IDS_ErrSourceMgrClose       31
#define IDS_ErrTripletNotExecuted   32

 // Select Source dialog constants
#define IDS_SELECT_SOURCE_TEXT              3000
#define IDS_SELECT_TEXT                     3001
#define IDS_CANCEL_TEXT                     3002
#define IDS_SOURCES_TEXT                    3003
#define IDS_SELECT_OCRENGINE_TEXT           3004

#define IDS_LOGMSG_START                    3005
#define IDS_LOGMSG_ENTERTEXT                (IDS_LOGMSG_START + 0)
#define IDS_LOGMSG_EXITTEXT                 (IDS_LOGMSG_START + 1)
#define IDS_LOGMSG_RETURNTEXT               (IDS_LOGMSG_START + 2)
#define IDS_LOGMSG_EXCEPTERRORTEXT          (IDS_LOGMSG_START + 3)
#define IDS_LOGMSG_MODULETEXT               (IDS_LOGMSG_START + 4)
#define IDS_LOGMSG_NOPARAMINFOTEXT          (IDS_LOGMSG_START + 5)
#define IDS_LOGMSG_NOINFOERRORTEXT          (IDS_LOGMSG_START + 6)
#define IDS_LOGMSG_INPUTTEXT                (IDS_LOGMSG_START + 7)
#define IDS_LOGMSG_OUTPUTDSMTEXT            (IDS_LOGMSG_START + 8)
#define IDS_LOGMSG_NOINFOTEXT               (IDS_LOGMSG_START + 9)
#define IDS_LOGMSG_RETURNEDERRORTEXT        (IDS_LOGMSG_START + 10)
#define IDS_LOGMSG_ERRORTEXT                (IDS_LOGMSG_START + 11)
#define IDS_LOGMSG_TEMPFILENOTEXISTTEXT     (IDS_LOGMSG_START + 12)
#define IDS_LOGMSG_CALLEDTEXT               (IDS_LOGMSG_START + 13)
#define IDS_LOGMSG_TEMPIMAGEFILETEXT        (IDS_LOGMSG_START + 14)
#define IDS_LOGMSG_TEMPFILECREATEERRORTEXT  (IDS_LOGMSG_START + 15)
#define IDS_LOGMSG_IMAGEFILESUCCESSTEXT     (IDS_LOGMSG_START + 16)
#define IDS_LOGMSG_RETURNSETVALUES          (IDS_LOGMSG_START + 17)
#define IDS_LOGMSG_MORETEXT                 (IDS_LOGMSG_START + 18)
#define IDS_LOGMSG_CAPABILITYLISTING        (IDS_LOGMSG_START + 19)
#define IDS_LOGMSG_DEFAULTSOURCE            (IDS_LOGMSG_START + 20)
#define IDS_LOGMSG_END                      (4999)

// PDF Font text
#define DTWAIN_FONT_START_                 5000
#define DTWAIN_FONT_COURIER_              (DTWAIN_FONT_START_ + 0)
#define DTWAIN_FONT_COURIERBOLD_          (DTWAIN_FONT_START_ + 1)
#define DTWAIN_FONT_COURIERBOLDOBLIQUE_   (DTWAIN_FONT_START_ + 2)
#define DTWAIN_FONT_COURIEROBLIQUE_       (DTWAIN_FONT_START_ + 3)
#define DTWAIN_FONT_HELVETICA_            (DTWAIN_FONT_START_ + 4)
#define DTWAIN_FONT_HELVETICABOLD_        (DTWAIN_FONT_START_ + 5)
#define DTWAIN_FONT_HELVETICABOLDOBLIQUE_ (DTWAIN_FONT_START_ + 6)
#define DTWAIN_FONT_HELVETICAOBLIQUE_     (DTWAIN_FONT_START_ + 7)
#define DTWAIN_FONT_TIMESBOLD_            (DTWAIN_FONT_START_ + 8)
#define DTWAIN_FONT_TIMESBOLDITALIC_      (DTWAIN_FONT_START_ + 9)
#define DTWAIN_FONT_TIMESROMAN_           (DTWAIN_FONT_START_ + 10)
#define DTWAIN_FONT_TIMESITALIC_          (DTWAIN_FONT_START_ + 11)
#define DTWAIN_FONT_SYMBOL_               (DTWAIN_FONT_START_ + 12)
#define DTWAIN_FONT_ZAPFDINGBATS_         (DTWAIN_FONT_START_ + 13)

#define IDS_ErrCCLowMemory          TWAIN_ERR_LOW_MEMORY
#define IDS_ErrCCFalseAlarm         TWAIN_ERR_FALSE_ALARM
#define IDS_ErrCCBummer             TWAIN_ERR_BUMMER
#define IDS_ErrCCNoDataSource       TWAIN_ERR_NODATASOURCE
#define IDS_ErrCCMaxConnections     TWAIN_ERR_MAXCONNECTIONS
#define IDS_ErrCCOperationError     TWAIN_ERR_OPERATIONERROR
#define IDS_ErrCCBadCapability      TWAIN_ERR_BADCAPABILITY
#define IDS_ErrCCBadValue           TWAIN_ERR_BADVALUE
#define IDS_ErrCCBadProtocol        TWAIN_ERR_BADPROTOCOL
#define IDS_ErrCCSequenceError      TWAIN_ERR_SEQUENCEERROR
#define IDS_ErrCCBadDestination     TWAIN_ERR_BADDESTINATION
#define IDS_ErrCCCapNotSupported    TWAIN_ERR_CAPNOTSUPPORTED
#define IDS_ErrCCCapBadOperation    TWAIN_ERR_CAPBADOPERATION
#define IDS_ErrCCCapSequenceError   TWAIN_ERR_CAPSEQUENCEERROR

#define IDS_ErrCCFileProtected      TWAIN_ERR_FILEPROTECTEDERROR
#define IDS_ErrCCFileExists         TWAIN_ERR_FILEEXISTERROR
#define IDS_ErrCCFileNotFound       TWAIN_ERR_FILENOTFOUND
#define IDS_ErrCCDirectoryNotEmpty  TWAIN_ERR_DIRNOTEMPTY
#define IDS_ErrCCFeederJammed       TWAIN_ERR_FEEDERJAMMED
#define IDS_ErrCCFeederMultPages    TWAIN_ERR_FEEDERMULTPAGES
#define IDS_ErrCCFileWriteError     TWAIN_ERR_FEEDERWRITEERROR
#define IDS_ErrCCDeviceOffline      TWAIN_ERR_DEVICEOFFLINE
#define IDS_ErrCCInterlock          TWAIN_ERR_INTERLOCK
#define IDS_ErrCCDamagedCorner      TWAIN_ERR_DAMAGEDCORNER
#define IDS_ErrCCFocusError         TWAIN_ERR_FOCUSERROR
#define IDS_ErrCCDoctooLight        TWAIN_ERR_DOCTOOLIGHT
#define IDS_ErrCCDoctooDark         TWAIN_ERR_DOCTOODARK
#define IDS_ErrCCNoMedia            TWAIN_ERR_NOMEDIA

#define IDC_TWAINDATA           8888
#define IDC_TWAINDEBUGDATA      8889
#define IDC_TWAINDGDATA         8890
#define IDC_TWAINDATDATA        8891
#define IDC_TWAINMSGDATA        8892
#define IDC_TWAINFILEDATA       8893
#define IDS_LIMITEDFUNCMSG1     8894
#define IDS_LIMITEDFUNCMSG2     8895
#define IDS_LIMITEDFUNCMSG3     8896
#define IDS_TWCCBASE            9500
#define IDS_TWRCBASE            9600
#define IDS_TWCC_EXCEPTION      9999
#define IDS_DTWAINFUNCSTART     9001

#define IDS_TWCC_ERRORSTART        9500
#define IDS_TWCC_SUCCESS           9500 /* OkIt worked!                                */
#define IDS_TWCC_BUMMER            9501 /* Failure due to unknown causes             */
#define IDS_TWCC_LOWMEMORY         9502 /* Not enough memory to perform operation    */
#define IDS_TWCC_NODS              9503 /* No Data Source                            */
#define IDS_TWCC_MAXCONNECTIONS    9504 /* DS is connected to max possible applications      */
#define IDS_TWCC_OPERATIONERROR    9505 /* DS or DSM reported error, application shouldn't   */
#define IDS_TWCC_BADCAP            9506 /* Unknown capability                        */
#define IDS_TWCC_BADPROTOCOL       9509 /* Unrecognized MSG DG DAT combination       */
#define IDS_TWCC_BADVALUE          9510 /* Data parameter out of range              */
#define IDS_TWCC_SEQERROR          9511 /* DG DAT MSG out of expected sequence      */
#define IDS_TWCC_BADDEST           9512 /* Unknown destination Application/Source in DSM_Entry */
#define IDS_TWCC_CAPUNSUPPORTED    9513 /* Capability not supported by source            */
#define IDS_TWCC_CAPBADOPERATION   9514 /* Operation not supported by capability         */
#define IDS_TWCC_CAPSEQERROR       9515 /* Capability has dependancy on other capability */
#define IDS_TWCC_DENIED            9516 /* File System operation is denied (file is protected) */
#define IDS_TWCC_FILEEXISTS        9517 /* Operation failed because file already exists. */
#define IDS_TWCC_FILENOTFOUND      9518 /* File not found */
#define IDS_TWCC_NOTEMPTY          9519 /* Operation failed because directory is not empty */
#define IDS_TWCC_PAPERJAM          9520  /* The feeder is jammed */
#define IDS_TWCC_PAPERDOUBLEFEED   9521  /* The feeder detected multiple pages */
#define IDS_TWCC_FILEWRITEERROR    9522  /* Error writing the file (meant for things like disk full conditions) */
#define IDS_TWCC_CHECKDEVICEONLINE 9523  /* The device went offline prior to or during this operation */

#define IDS_TWRC_ERRORSTART       9600
#define IDS_TWRC_SUCCESS          9600
#define IDS_TWRC_FAILURE          9601
#define IDS_TWRC_CHECKSTATUS      9602
#define IDS_TWRC_CANCEL           9603
#define IDS_TWRC_DSEVENT          9604
#define IDS_TWRC_NOTDSEVENT       9605
#define IDS_TWRC_XFERDONE         9606
#define IDS_TWRC_ENDOFLIST        9607
#define IDS_TWRC_INFONOTSUPPORTED 9608
#define IDS_TWRC_DATANOTAVAILABLE 9609
#define IDS_DTWAIN_APPTITLE       9700

#define IDS_TWCC_EXCEPTION      9999
#define IDS_DTWAIN_USERMSG_INDICATOR            10000
#define IDS_DTWAIN_ERROR_CLOSING_DSM            10001 
#define IDS_DTWAIN_ERROR_CLOSING_TWAIN_SESSION  10002
#define IDS_DTWAIN_ERROR_CLOSING_DTWAIN_MANAGER 10003
#define IDS_CLOSING_DTWAIN                      10004
#define IDS_DTWAIN_ERROR_REMOVE_WINDOW          10005
#define IDS_DTWAIN_ERROR_REPORTED_TYPE_MISMATCH 10006
#define IDS_DTWAIN_FILE_COMPRESS_TYPE_MISMATCH  10007

//#define VS_VERSION_INFO     9000
#define IDS_DTWAINFUNCSTART     9001


#define DTW_CONTARRAY           8
#define DTW_CONTENUMERATION     16
#define DTW_CONTONEVALUE        32
#define DTW_CONTRANGE           64

#define DTW_FF_TIFF        0
#define DTW_FF_PICT        1
#define DTW_FF_BMP         2
#define DTW_FF_XBM         3
#define DTW_FF_JFIF        4
#define DTW_FF_FPX         5
#define DTW_FF_TIFFMULTI   6
#define DTW_FF_PNG         7
#define DTW_FF_SPIFF       8
#define DTW_FF_EXIF        9
#define DTW_BMP          DTW_FF_BMP
#define DTW_JPEG         DTW_FF_JFIF
#define DTW_PCX          10
#define DTW_TGA          11
#define DTW_TIFFLZW      12
#define DTW_TIFFNONE     DTW_FF_TIFF
#define DTW_TIFFG3       13
#define DTW_TIFFG4       14
#define DTW_GIF          15
#define DTW_PNG          DTW_FF_PNG

#define LTWRC_SUCCESS          0L
#define LTWRC_FAILURE          1L
#define LTWRC_CHECKSTATUS      2L
#define LTWRC_CANCEL           3L
#define LTWRC_DSEVENT          4L
#define LTWRC_NOTDSEVENT       5L
#define LTWRC_XFERDONE         6L
#define LTWRC_ENDOFLIST        7L
#define LTWRC_INFONOTSUPPORTED 8L
#define LTWRC_DATANOTAVAILABLE 9L

#define LTWCC_SUCCESS            0L /* It worked!                                */
#define LTWCC_BUMMER             1L /* Failure due to unknown causes             */
#define LTWCC_LOWMEMORY          2L /* Not enough memory to perform operation    */
#define LTWCC_NODS               3L /* No Data Source                            */
#define LTWCC_MAXCONNECTIONS     4L /* DS is connected to max possible applications      */
#define LTWCC_OPERATIONERROR     5L /* DS or DSM reported error, application shouldn't   */
#define LTWCC_BADCAP             6L /* Unknown capability                        */
#define LTWCC_BADPROTOCOL        9L /* Unrecognized MSG DG DAT combination       */
#define LTWCC_BADVALUE           10L /* Data parameter out of range              */
#define LTWCC_SEQERROR           11L /* DG DAT MSG out of expected sequence      */
#define LTWCC_BADDEST            12L /* Unknown destination Application/Source in DSM_Entry */
#define LTWCC_CAPUNSUPPORTED     13L /* Capability not supported by source            */
#define LTWCC_CAPBADOPERATION    14L /* Operation not supported by capability         */
#define LTWCC_CAPSEQERROR        15L /* Capability has dependancy on other capability */
#define LTWCC_DENIED             16L /* File System operation is denied (file is protected) */
#define LTWCC_FILEEXISTS         17L /* Operation failed because file already exists. */
#define LTWCC_FILENOTFOUND       18L /* File not found */
#define LTWCC_NOTEMPTY           19L /* Operation failed because directory is not empty */
#define LTWCC_PAPERJAM           20L  /* The feeder is jammed */
#define LTWCC_PAPERDOUBLEFEED    21L  /* The feeder detected multiple pages */
#define LTWCC_FILEWRITEERROR     22L  /* Error writing the file (meant for things like disk full conditions) */
#define LTWCC_CHECKDEVICEONLINE  23L  /* The device went offline prior to or during this operation */

#define IDC_DLGSELECTSOURCE      10000
#define IDC_LSTSOURCES           10001
#define IDC_SOURCETEXT           10002
#define IDC_EDIT1                10003
#define IDC_STATIC               -1

// version string
#define IDS_DTWAIN_VERSIONSTRING_MAJOR      11000
#define IDS_DTWAIN_VERSIONSTRING_MINOR      11001
#define IDS_DTWAIN_VERSIONSTRING_SUBBUILD1  11002
#define IDS_DTWAIN_VERSIONSTRING_SUBBUILD2  11003

#endif
