#ifndef _TOCRLANGSEL_
#define _TOCRLANGSEL_

typedef unsigned short		CSET;
#define	TOCRcsetdim		608
#define	TOCRnumlangrecs		46

typedef struct LEXLANGDATA
{
	char	langext[4];
	char	langname[16];
	unsigned short	alphabet;			// 0 latin, 1 latin extended, 2 Cyrillic, 3 Greek
	unsigned char	lang_group;
	unsigned char	lang_file;			// 1 if has one
	unsigned short	numshare;			// Number of languages relying on file, calculated in init$pd
	// pointer to structure SX
} LEXLANGDATA;

typedef unsigned char	CharStatusItem;
typedef CharStatusItem	CharStatusMap[TOCRcsetdim];
typedef unsigned char	LangStatusMap[TOCRnumlangrecs];

#endif // _TOCRLANGSEL_