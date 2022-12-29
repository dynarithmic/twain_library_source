// Example1.cpp : Acquires an image from a TWAIN Source
//
#include <dtwain.h>
#include <stdio.h>

int main()
{
    DTWAIN_SOURCE Source = NULL;
    DTWAIN_HANDLE isInitialized = DTWAIN_SysInitialize();
    char dll_version[20];

    if (!isInitialized)
    {
        printf("Could not initialize DTWAIN library");
        return -1;
    }

    /* This returns the compiled version */
    printf("Hello -- we compiled DTWAIN Version %s\n", DTWAIN_VERINFO_FILEVERSION);

	/* This gets the actual version of DTWAIN that is running */
	DTWAIN_GetShortVersionStringA(dll_version, 20);
	printf("Hello -- we are running DTWAIN Version %s\n", dll_version);

    Source = DTWAIN_SelectSource();
    if (Source)
        DTWAIN_AcquireFile(Source, NULL, DTWAIN_BMP, DTWAIN_USEPROMPT, DTWAIN_PT_BW, DTWAIN_MAXACQUIRE, TRUE, TRUE, NULL);
    DTWAIN_SysDestroy();
}