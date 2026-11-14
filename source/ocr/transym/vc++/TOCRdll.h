// TOCR declares Version 5.0.0.0


#ifndef _TOCRDLL_
#define _TOCRDLL_

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

typedef unsigned char       BYTE;

#include "TOCRlangSel.h"

#define SUPERSEDED

#pragma region Structures
typedef struct tagTOCRPPROCESSPDFOPTIONS_EG
{
	BYTE		ResultsOn;
	BYTE		OriginalImageOn;
	BYTE		ProcessedImageOn;
	BYTE		PDFSpare;
} TOCRPROCESSPDFOPTIONS_EG;

typedef struct tagTOCRCHAROPTIONS_EG
{
	CharStatusMap		DisableCharW;
} TOCRCHAROPTIONS_EG;

typedef struct tagTOCRLANGUAGEOPTIONS_EG
{
	//CharStatusMap	DisableCharW;
	//BYTE			filler1[2];
	LangStatusMap	DisableLangs;		
} TOCRLANGUAGEOPTIONS_EG;

typedef struct tagTOCRPROCESSOPTIONS_EG
{
	long		StructId;
	BYTE		InvertWholePage;
	BYTE		DeskewOff;
	BYTE		Orientation;
	BYTE		NoiseRemoveOff;
	BYTE		ReturnNoiseOn;
	BYTE		LineRemoveOff;
	BYTE		DeshadeOff;
	BYTE		InvertOff;
	BYTE		SectioningOn;
	BYTE		MergeBreakOff;
	BYTE		LineRejectOff;
	BYTE		CharacterRejectOff;
	BYTE		ResultsReference;
	BYTE		LexMode;
	BYTE		OCRBOnly;
	BYTE		Speed;
	BYTE		FonStyleInfoOff;
	BYTE		filler1[3];
	long		CCAlgorithm;
	float		CCThreshold;
	long		CGAlgorithm;
	long		ExtraInfFlags;
	LangStatusMap		DisableLangs;
	BYTE		filler2[2];
	CharStatusMap		DisableCharW;
} TOCRPROCESSOPTIONS_EG;

typedef struct tagTOCRJOBINFO_EG
{
	HANDLE		hMMF;
	char		*InputFile;
	long		StructId;
	long		JobType;
	long		PageNo;
	TOCRPROCESSOPTIONS_EG	ProcessOptions;
} TOCRJOBINFO_EG;

typedef struct tagTOCRRESULTSHEADER_EG
{
    long		StructId;
	long		XPixelsPerInch;
	long		YPixelsPerInch;
	long		NumItems;
	float		MeanConfidence;
	long		DominantLanguage;
} TOCRRESULTSHEADER_EG;

typedef struct tagTOCRRESULTSITEM_EG
{
	float			Confidence;
	unsigned short	StructId;
	unsigned short	OCRCharWUnicode;
	unsigned short	OCRCharWInternal;
	unsigned short	FontID;
	unsigned short	FontStyleInfo;
	unsigned short	XPos;
	unsigned short	YPos;
	unsigned short	XDim;
	unsigned short	YDim;
	unsigned short	YDimRef;
	unsigned short	Noise;
} TOCRRESULTSITEM_EG;

typedef struct tagTOCRRESULTS_EG
{
	TOCRRESULTSHEADER_EG	Hdr;
	TOCRRESULTSITEM_EG		Item[1];
} TOCRRESULTS_EG;

typedef struct tagTOCRRESULTSITEMEXALT_EG
{
	float			Factor;
	unsigned short	Valid;
	unsigned short	OCRCharWUnicode;
	unsigned short	OCRCharWInternal;
} TOCRRESULTSITEMEXALT_EG;

typedef struct tagTOCRRESULTSITEMEX_EG
{
	float			Confidence;
	unsigned short	StructId;
	unsigned short	OCRCharWUnicode;
	unsigned short	OCRCharWInternal;
	unsigned short	FontID;
	unsigned short	FontStyleInfo;
	unsigned short	XPos;
	unsigned short	YPos;
	unsigned short	XDim;
	unsigned short	YDim;
	unsigned short	YDimRef;
	unsigned short	Noise;
	TOCRRESULTSITEMEXALT_EG	Alt[5];
} TOCRRESULTSITEMEX_EG;

typedef struct tagTOCRRESULTSEX_EG
{
	TOCRRESULTSHEADER_EG	Hdr;
	TOCRRESULTSITEMEX_EG	Item[1];
} TOCRRESULTSEX_EG;
#pragma endregion

#pragma region SUPERSEDED Structures

typedef signed short		VBBOOL;	// in VB Boolean is a signed short True is -1

typedef struct tagTOCRProcessOptions
{
	long		StructId;
	VBBOOL		InvertWholePage;
	VBBOOL		DeskewOff;
	BYTE		Orientation;
	VBBOOL		NoiseRemoveOff;
	VBBOOL		LineRemoveOff;
	VBBOOL		DeshadeOff;
	VBBOOL		InvertOff;
	VBBOOL		SectioningOn;
	VBBOOL		MergeBreakOff;
	VBBOOL		LineRejectOff;
	VBBOOL		CharacterRejectOff;
	VBBOOL		LexOff;
	VBBOOL		DisableCharacter[256];
} TOCRPROCESSOPTIONS;

typedef struct tagTOCRJobInfo2
{
	long		StructId;
	long		JobType;
	char		*InputFile;
	HANDLE		hMMF;
	long		PageNo;
	TOCRPROCESSOPTIONS	ProcessOptions;
} TOCRJOBINFO2;

typedef struct tagTOCRJobInfo
{
	long		StructId;
	long		JobType;
	char		*InputFile;
	long		PageNo;
	TOCRPROCESSOPTIONS	ProcessOptions;
} TOCRJOBINFO;

typedef struct tagTOCRRESULTSHEADER
{
    long		StructId;
	long		XPixelsPerInch;
	long		YPixelsPerInch;
	long		NumItems;
	float		MeanConfidence;
} TOCRRESULTSHEADER;

typedef struct tagTOCRRESULTSITEM
{
	unsigned short	StructId;
	unsigned short	OCRCha;
	float			Confidence;
	unsigned short	XPos;
	unsigned short	YPos;
	unsigned short	XDim;
	unsigned short	YDim;
} TOCRRESULTSITEM;


typedef struct tagTOCRRESULTS
{
	TOCRRESULTSHEADER	Hdr;
	TOCRRESULTSITEM		Item[1];
} TOCRRESULTS;


typedef struct tagTOCRRESULTSITEMEXALT
{
	unsigned short	Valid;
	unsigned short	OCRCha;
	float			Factor;
} TOCRRESULTSITEMEXALT;

typedef struct tagTOCRRESULTSITEMEX
{
	unsigned short			StructId;
	unsigned short			OCRCha;
	float					Confidence;
	unsigned short			XPos;
	unsigned short			YPos;
	unsigned short			XDim;
	unsigned short			YDim;
	TOCRRESULTSITEMEXALT	Alt[5];
} TOCRRESULTSITEMEX;

typedef struct tagTOCRRESULTSEX
{
	TOCRRESULTSHEADER	Hdr;
	TOCRRESULTSITEMEX	Item[1];
} TOCRRESULTSEX;

#pragma endregion

#pragma region Declares
#ifdef RESELLER1
EXTERN_C long WINAPI TOCRInitialise(long *JobNo, unsigned long *RegNo);
#else
EXTERN_C long WINAPI TOCRInitialise(long *JobNo);
#endif

EXTERN_C long WINAPI TOCRShutdown(long JobNo);

EXTERN_C long WINAPI TOCRDoJob_EG(long JobNo, TOCRJOBINFO_EG *JobInfo_EG);
EXTERN_C long WINAPI TOCRDoJobPDF_EG(long JobNo, TOCRJOBINFO_EG *JobInfo_EG, char *OutputFilename, TOCRPROCESSPDFOPTIONS_EG* pPdfOpts);

EXTERN_C long WINAPI TOCRWaitForJob(long JobNo, long *JobStatus);

EXTERN_C long WINAPI TOCRWaitForAnyJob(long *WaitAnyStatus, long *JobNo);

EXTERN_C long WINAPI TOCRGetJobDBInfo(long *JobSlotInf);

EXTERN_C long WINAPI TOCRGetJobStatus(long JobNo, long *JobStatus);

EXTERN_C long WINAPI TOCRGetJobStatusEx2(long JobNo, long *JobStatus, float *Progress, long *AutoOrientation, long *ExtraInfFlags);

EXTERN_C long WINAPI TOCRGetJobStatusMsg(long JobNo, char *Msg);

EXTERN_C long WINAPI TOCRGetNumPages(long JobNo, char *Filename, long JobType, long *NumPages);

EXTERN_C long WINAPI TOCRGetJobResultsEx_EG(long JobNo, long Mode, long *ResultsInf, void *Results);

EXTERN_C long WINAPI TOCRConvertFormat(long JobNo, void *InputAddr, long InputFormat, void *OutputAddr, long OutputFormat, long PageNo);

EXTERN_C long WINAPI TOCRGetLicenceInfoEx(long JobNo, char *Licence, long *Volume, long *Time, long *Remaining, long *Features);

EXTERN_C long WINAPI TOCRPopulateCharStatusMap(TOCRLANGUAGEOPTIONS_EG *p_lang_opts, TOCRCHAROPTIONS_EG *p_usercharvalid);

// These functions cannot be used to get/set the log file name in x64
EXTERN_C long WINAPI TOCRSetConfig(long JobNo, long Parameter, long Value);
EXTERN_C long WINAPI TOCRGetConfig(long JobNo, long Parameter, long *Value);

// These functions can be used to get/set the log file name in x64
EXTERN_C long WINAPI TOCRSetConfigStr(long JobNo, long Parameter, char *Value);
EXTERN_C long WINAPI TOCRGetConfigStr(long JobNo, long Parameter, char *Value);

EXTERN_C long WINAPI TOCRGetFontName(long JobNo, long FontID, char *FontName);

EXTERN_C long WINAPI TOCRExtraInfGetMMF(long JobNo, long ExtraInfFlag, HANDLE *MMF);

EXTERN_C long WINAPI TOCRTWAINAcquire(long *NumberOfImages);

// was EXTERN_C long WINAPI TOCRTWAINGetImages(long *GlobalMemoryDIBs);
EXTERN_C long WINAPI TOCRTWAINGetImages(HGLOBAL *GlobalMemoryDIBs);

EXTERN_C long WINAPI TOCRTWAINSelectDS(void);

EXTERN_C long WINAPI TOCRTWAINShowUI(signed short Show);

#pragma endregion

#pragma region SUPERSEDED Declares
#ifdef SUPERSEDED
// Superseded by TOCRGetConfig
EXTERN_C long WINAPI TOCRGetErrorMode(long JobNo, long *ErrorMode);

// Superseded by TOCRSetConfig
EXTERN_C long WINAPI TOCRSetErrorMode(long JobNo, long ErrorMode);

//Superseded by TOCRDoJob_EG
EXTERN_C long WINAPI TOCRDoJob2(long JobNo, TOCRJOBINFO2 *JobInfo2);

//Superseded by TOCRDoJob2
EXTERN_C long WINAPI TOCRDoJob(long JobNo, TOCRJOBINFO *JobInfo);

//Superseded by TOCRGetJobStatusEx2
EXTERN_C long WINAPI TOCRGetJobStatusEx(long JobNo, long *JobStatus, float *Progress, long *AutoOrientation);

//Superseded by TOCRGetJobResultsEx_EG
EXTERN_C long WINAPI TOCRGetJobResults(long JobNo, long *ResultsInf, TOCRRESULTS *Results);

//Superseded by TOCRGetJobResultsEx_EG
EXTERN_C long WINAPI TOCRGetJobResultsEx(long JobNo, long Mode, long *ResultsInf, void *ResultsEx);

// was EXTERN_C long WINAPI TOCRRotateMonoBitmap(long *hBmp, long Width, long Height, long Orientation);
EXTERN_C long WINAPI TOCRRotateMonoBitmap(HBITMAP *hBmp, long Width, long Height, long Orientation);

// Superseded by TOCRConvertFormat
EXTERN_C long WINAPI TOCRConvertTIFFtoDIB(long JobNo, char *InputFilename, char *OutputFilename, long PageNo);

// UNTESTED superseded by TOCRGetLicenceInfoEx
EXTERN_C long WINAPI TOCRGetLicenceInfo(long *NumberOfJobSlots, long *Volume, long *Time, long *Remaining);
#endif
#pragma endregion

#endif // _TOCRDLL_
