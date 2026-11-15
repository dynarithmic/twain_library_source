// User constants, Version 5.1.0.0

#define TOCRJOBMSGLENGTH 512		// max length of a job status message
#define TOCRFONTNAMELENGTH 65		// max length of a returned font name

#define TOCRMAXPPM 78741			// max pixels per metre - OBSOLETE
#define TOCRMINPPM 984				// min pixels per metre - OBSOLETE

// Setting for JobNo for TOCRSetErrorMode and TOCRGetErrorMode
#define TOCRDEFERRORMODE -1			// set/get the default API error mode (applies
									// when there are no jobs and is applied to new jobs)

// Settings for ErrorMode for TOCRSetErrorMode and TOCRGetErrorMode
#define TOCRERRORMODE_NONE 0		// errors unseen (use return status of API calls)
#define TOCRERRORMODE_MSGBOX 1		// errors will bring up a message box
#define TOCRERRORMODE_LOG 2			// errors are sent to a log file

// Setting for TOCRShutdown
#define TOCRSHUTDOWNALL -1			// stop and shutdown processing for all jobs

// Values returned by TOCRGetJobStatus JobStatus
#define TOCRJOBSTATUS_ERROR -1		// an error ocurred
#define TOCRJOBSTATUS_BUSY 0		// the job is still processing
#define TOCRJOBSTATUS_DONE 1		// the job completed successfully
#define TOCRJOBSTATUS_IDLE 2		// no job has been specified yet

// Settings for TOCRJOBINFO.JobType
#define TOCRJOBTYPE_TIFFFILE 0		// TOCRJOBINFO.InputFile specifies a tiff file
#define TOCRJOBTYPE_DIBFILE 1		// TOCRJOBINFO.InputFile specifies a dib (bmp) file
#define TOCRJOBTYPE_DIBCLIPBOARD 2	// clipboard contains a dib (clipboard format CF_DIB)
#define TOCRJOBTYPE_MMFILEHANDLE  3 // TOCRJOBINFO.PageNo specifies a handle to a memory mapped DIB file
#define TOCRJOBTYPE_PDFFILE 4		// TOCRJOBINFO.InputFile specifies a PDF file

// Settings for TOCRJOBINFO.Orientation
// Settings for TOCRJOBINFO_EG.PROCESSOPTIONS_EG.Orientation
#define TOCRJOBORIENT_AUTO 0		// detect orientation and rotate automatically
#define TOCRJOBORIENT_OFF -1		// don't rotate
#define TOCRJOBORIENT_90 1			// 90 degrees clockwise rotation
#define TOCRJOBORIENT_180 2			// 180 degrees clockwise rotation
#define TOCRJOBORIENT_270 3			// 270 degrees clockwise rotation

// Settings for TOCRJOBINFO_EG.PROCESSOPTIONS_EG.ResultsReference
#define TOCRRESULTSREFERENCE_SELFREL 0	// relative to the first top left character recognised
#define TOCRRESULTSREFERENCE_BEFORE 1	// page position before rotation and deskewing
#define TOCRRESULTSREFERENCE_BETWEEN 2	// page position after rotation but before deskewing
#define TOCRRESULTSREFERENCE_AFTER 3	// page position after rotation and deskewing

// Settings for TOCRJOBINFO_EG.PROCESSOPTIONS_EG.LexMode
#define TOCRJOBLEXMODE_AUTO  0      // decide whether to apply lex
#define TOCRJOBLEXMODE_ON  1		// lex always on
#define TOCRJOBLEXMODE_OFF  2		// lex always off

// Settings for TOCRJOBINFO_EG.PROCESSOPTIONS_EG.Speed
#define TOCRJOBSPEED_SLOW  0
#define TOCRJOBSPEED_MEDIUM 1
#define TOCRJOBSPEED_FAST 2
#define TOCRJOBSPEED_EXPRESS 3

// Settings for TOCRJOBINFO_EG.PROCESSOPTIONS_EG.CCAlgorithm (thresholded Conversions from Colour)
#define TOCRJOBCC_AVERAGE  0        // (R+G+B)/3
#define TOCRJOBCC_LUMA_BT601  1     // 0.299*R + 0.587*G + 0.114*B
#define TOCRJOBCC_LUMA_BT709  2     // 0.2126*R + 0.7152*G + 0.0722*B
#define TOCRJOBCC_DESATURATION  3   // (max(R,G,B) + min(R,G,B))/2
#define TOCRJOBCC_DECOMPOSITION_MAX  4// max(R,G,B)
#define TOCRJOBCC_DECOMPOSITION_MIN  5// min(R,G,B)
#define TOCRJOBCC_RED  6            // R
#define TOCRJOBCC_GREEN  7          // G
#define TOCRJOBCC_BLUE  8			// B

// Settings for TOCRJOBINFO_EG.PROCESSOPTIONS_EG.CGAlgorithm (Conversions from Grey)
#define TOCRJOBCG_HISTOGRAM  9		
#define TOCRJOBCG_REGIONS  10		

// Flags for TOCRJOBINFO_EG.PROCESSOPTIONS_EG.ExtraInfFlags
#define TOCREXTRAINF_RETURNBITMAP1 1
#define TOCREXTRAINF_RETURNBITMAPONLY 2

	// Values returned by TOCRGetJobDBInfo
#define TOCRJOBSLOT_FREE 0			// job slot is free for use
#define TOCRJOBSLOT_OWNEDBYYOU 1	// job slot is in use by your process
#define TOCRJOBSLOT_BLOCKEDBYYOU 2	// blocked by own process (re-initialise)
#define TOCRJOBSLOT_OWNEDBYOTHER -1	// job slot is in use by another process (can't use)
#define TOCRJOBSLOT_BLOCKEDBYOTHER -2// blocked by another process (can't use)

// Values returned in WaitAnyStatus by TOCRWaitForAnyJob
#define TOCRWAIT_OK 0				// JobNo is the job that finished (get and check it's JobStatus)
#define TOCRWAIT_SERVICEABORT 1		// JobNo is the job that failed (re-initialise)
#define TOCRWAIT_CONNECTIONBROKEN 2	// JobNo is the job that failed (re-initialise)
#define TOCRWAIT_FAILED -1			// JobNo not set - check manually
#define TOCRWAIT_NOJOBSFOUND -2		// JobNo not set - no running jobs found

// Settings for Mode for TOCRGetJobResultsEx
#define TOCRGETRESULTS_NORMAL 0		// return results for TOCRRESULTS
#define TOCRGETRESULTS_EXTENDED 1	// return results for TOCRRESULTSEX

// Settings for Mode for TOCRGetJobResultsEx_EG
#define TOCRGETRESULTS_NORMAL_EG 2		// return results for TOCRRESULTS_EG
#define TOCRGETRESULTS_EXTENDED_EG 3	// return results for TOCRRESULTSEX_EG

// Values returned in ResultsInf by TOCRGetJobResults and TOCRGetJobResultsEx
#define TOCRGETRESULTS_NORESULTS -1	// no results are available

// Flags returned by TOCRResults_EG.Item().FontStyleInfo
// Flags returned by TOCRResultsEx_EG.Item().FontStyleInfo
#define TOCRRESULTSFONT_NOTSET 0		// character style is not specified
#define TOCRRESULTSFONT_NORMAL 1		// character is Normal
#define TOCRRESULTSFONT_ITALIC 2		// character is Italic
#define TOCRRESULTSFONT_UNDERLINE 4		// character is Underlined
//#define TOCRRESULTSFONT_BOLD 8		// character is Bold - removed not yet done

// Values for TOCRConvertFormat InputFormat
#define TOCRCONVERTFORMAT_TIFFFILE TOCRJOBTYPE_TIFFFILE
#define TOCRCONVERTFORMAT_PDFFILE TOCRJOBTYPE_PDFFILE

// Values for TOCRConvertFormat OutputFormat
#define TOCRCONVERTFORMAT_DIBFILE TOCRJOBTYPE_DIBFILE
#define TOCRCONVERTFORMAT_MMFILEHANDLE TOCRJOBTYPE_MMFILEHANDLE

// Values for licence features (returned by TOCRGetLicenceInfoEx)
#define TOCRLICENCE_STANDARD 1		// V1 standard licence (no higher characters)
#define TOCRLICENCE_EURO 2			// V2 (higher characters)
#define TOCRLICENCE_EUROUPGRADE 3	// standard licence upgraded to euro (V1.4->V2)
#define TOCRLICENCE_V3SE 4			// V3SE version 3 standard edition licence (no API)
#define TOCRLICENCE_V3SEUPGRADE 5	// versions 1/2 upgraded to V3 standard edition (no API)
// Note V4 licences are the same as V3 Pro licences
#define TOCRLICENCE_V3PRO 6			// V3PRO version 3 pro licence
#define TOCRLICENCE_V3PROUPGRADE 7	// versions 1/2 upgraded to version 3 pro
#define TOCRLICENCE_V3SEPROUPGRADE 8// version 3 standard edition upgraded to version 3 pro
#define TOCRLICENCE_V5 9			// version 5
#define TOCRLICENCE_V5UPGRADE3 10	// version 5 upgraded from version 3
#define TOCRLICENCE_V5UPGRADE12 11	// version 5 upgraded from version 1/2

// Values for TOCRSetConfig and TOCRGetConfig
#define TOCRCONFIG_DEFAULTJOB -1	// default job number (all new jobs)
#define TOCRCONFIG_DLL_ERRORMODE 0	// set the dll ErrorMode
#define TOCRCONFIG_SRV_ERRORMODE 1	// set the service ErrorMode
#define TOCRCONFIG_SRV_THREADPRIORITY 2 // set the service thread priority
#define TOCRCONFIG_DLL_MUTEXWAIT 3	// set the dll mutex wait timeout (ms)
#define TOCRCONFIG_DLL_EVENTWAIT 4	// set the dll event wait timeout (ms)
#define TOCRCONFIG_SRV_MUTEXWAIT 5	// set the service mutex wait timeout (ms)
#define TOCRCONFIG_LOGFILE 6		// set the log file name
