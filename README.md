# twain_library_source
Dynarithmic TWAIN library source and development repository.

This repositiory contains the source code and development versions of the Dynarithmic TWAIN Library DLL and PDB files.  

----

### Rebuilding the Dynarithmic TWAIN Library from source ###

If you want to rebuild the libraries, you will need the following tools and computer resources:

One of the following compilers:

      * Visual Studio 2015 with Update 3
      * Visual Studio 2017 (may need to install XP tools and Windows 10 SDK from the VS 2017 Installation Manager).
      * Visual Studio 2019
      
In addition, you will need 

1) At least 20 GB of free disk space.
2) An installation of the [Boost](http://www.boost.org/) library (version 1.68 or above)
3) An installation of the [7-Zip](https://www.7-zip.org/) archiving utility.
4) The following environment variables must be set before compilation (if using the Visual Studio IDE, these variables must be set before starting the IDE):

*    BOOST_INCLUDE_DIR
*    ZIP7_UTIL_CMD

The **BOOST_INCLUDE_DIR** should point to your boost installation folder.  This is usually **C:\boost_install_directory**, where this folder contains a subsequent **boost** folder, containing the **boost** header files, and directories where the prebuilt boost libraries are found.

The boost library directories must be prefixed with **lib32-msvc-xx.y** or **lib64-msvc-xx.y**, where the **xx.y** is the compiler verion (for example, **14.0** for Visual C++ 2015, **14.1** for Visual C++ 2017, etc.).  In the directory, you should see libraries named similar to **libboost_xxxxxxxx-vcyyy-zzzz-<bits>.lib**.

For example:

```plaintext
SET BOOST_INCLUDE_DIR=C:\boost_installation\boost
```


If you are using Visual C++, I recommend getting the pre-built boost libraries found at [SourceForge](https://sourceforge.net/projects/boost/files/boost-binaries/).  When installed, you will get the boost **include** files, plus the library files.  Please download the version of boost that fits your compiler.  

```plaintext
For Visual C++ 2015 -- Download files with "14.0" in the file name.
For Visual C++ 2017 -- Download files with "14.1" in the file name.
For Visual C++ 2019 -- Download files with "14.2" in the file name.
```


If you are **not** using Visual C++, you will need to build the following boost libraries:  
* chrono
* date_time
* filesystem
* log
* system
* thread

and change the make file or whatever build utility you're using to point to the correct version of the boost library.

----
The **ZIP7_UTIL_CMD** is the full path name of the executable file **7z.exe** of the **7-Zip** archiving utility.  For example:

```plaintext
SET ZIP7_UTIL_CMD=C:\7-Zip\7z.exe
```

----

Note that the C++ source code should be able to be built with any C++11 C++14 compliant compiler that recognizes the Windows API headers (MingW using g++ 5.0 or above is an example).  However we have not tested builds of the DTWAIN library that have been built with any other compiler other than the Visual Studio family.   

* Start Visual Studio, and open one of the DTWAIN solution.  The DTWAIN solution files are found in the [source](https://github.com/dynarithmic/twain_library_source/tree/master/source) directory.  Open **dtwain_5_vs2015.sln**, **dtwain_5_vs2017.sln**, or **dtwain_5_vs2019.sln**, depending on whether you are using Visual Studio 2015, 2017, or 2019, respectively. 

* A full rebuild of all the configurations available is recommended.  Use the "Build -> Batch Build..." option in the Visual Studio IDE and check all of the configurations to build everything (take a coffee break -- this could take a while).  This will create a **source/binaries** directory that will contain the following DLLs:

        32bit/dtwain32.dll -32-bit ANSI (MBCS) DLL
        32bit/dtwain32u.dll -32-bit Unicode DLL
        32bit/dtwain32d.dll -32-bit Debug ANSI (MBCS) DLL
        32bit/dtwain32ud.dll -32-bit Debug Unicode DLL
        64bit/dtwain64.dll -64-bit ANSI (MBCS) DLL
        64bit/dtwain64u.dll -64-bit Unicode DLL
        64bit/dtwain64d.dll -64-bit Debug ANSI (MBCS) DLL
        64bit/dtwain64ud.dll -64-bit Debug Unicode DLL

* Note -- the resulting "*.lib* files that reside in these directories are import libraries compatible with the Visual Studio toolset.  Other compilers will require converting these .lib files to your compiler's import library format, or you can use the LoadLibrary / GetProcAddress approach (we have a wrapper for this -- see below in the "Getting DTWAIN to work with other programming languages" section).

* When all the configurations are built, there should be multiple DTWDEMO*.exe programs residing in the **binaries** subdirectory, where the suffix used in the program name matches the DTWAIN DLL that will be loaded.  For example, DTWDEMO32U.exe will load the dtwain32u.dll library when run. The easiest way to get started is to debug DTWDEMO.EXE and single step through the program using the debugger to get a feel of the flow of the program.  You should get a good idea of how DTWAIN works if you step into one or more of the DTWAIN functions (such as DTWAIN_SysInitialize or DTWAIN_SelectSource).

* In addition to the above, after a rebuild there will be zip files created that will contain all of the release and debug DLL's, import libraries, and PDB (debug symbols) files.  In the **source/binaries/32-bit** and **source/binaries/64-bit** folders will be two files with a .zip extension, namely **debug_libraries.zip** and **release_libraries.zip**, containing all of the binaries in a convenient zip file that can be used immediately.

Please note that the .zip files will only be created if you have installed **7-Zip** and have set the **ZIP7_UTIL_CMD** environment variable, as explained in the previous section of this document.

----

### Contributing your updates to this repository
If you wish to add your own changes to this repository, it is highly suggested that you "git clone" the **development** branch, and then make a pull request to have your changes merged into the development branch (not the **main** branch).  Once the pull request passes review, the updated changes will be merged into the development branch.  

When deemed appropriate by the maintainer of this repository, the development branch will be merged into the main branch.  Then the main branch will be used to build the libraries found in the dynarithmic/twain_library repository.  The main branch will always reflect the current build being distributed to the public in the dynarithmic/twain_library repository. 

