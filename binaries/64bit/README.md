# Dynarithmic TWAIN library binaries

This directory consists of two sub-directories, **full_logging** and **partial_logging**.

The **full_logging** directory contains the DLL's that are built with full logging capabilities when the API function **DTWAIN_SetTwainLog** is invoked.  Full logging consists of logging the call stack and return values when calling DTWAIN functions, plus the lower level calls that DTWAIN makes to the TWAIN Data Source Manager.  This is valuable in detecting issues that may occur when issuing calls to DTWAIN.

The **partial_logging** directory contains the DLL's that are built without call stack and return values being logged.  These DLL's are around 500K smaller in size than the DLL's in **full_logging**.  Direct calls to the lower level TWAIN DSM are included, but the call stack and return value logging is not available.

If you are not concerned with sizes of the DLL's, the **full_logging** should be used.  If you desire DLL's that are a bit smaller and can "sacrifice" call stack / return value logging, the **partial_logging** DLL's should be used.

----

### How to distinguish between full and partial logging DLLs
Since the names of the DLL's are the same, there are a few ways to distinguish which one is being used:

1) Inspecting the properties of the DLL will show **[Logging+]** in the **Product Name** section if the DLL has full logging enabled.  You can use Window's File Explorer or other utility to inspect the properties of a DLL file.

2) Calling **DTWAIN_SetTwainLog** will show **[Logging+]** in the header information when the log is started.

3) Calling **DTWAIN_GetVersionString** will show **[Logging+]** in the returned string.
