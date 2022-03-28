# twain_library_source
Dynarithmic TWAIN library source and development repository.

This repositiory contains the source code and development versions of the Dynarithmic TWAIN Library DLL and PDB files.  

----

# Current release builds of the Dynarithmic TWAIN Library
Currently, all the binaries that are released to the general public (the libraries in the twain_library repository) are compiled using **Visual Studio 2015, Update 3**.  

This may change over time, as Visual Studio 2019 and higher are now becoming the mainstream compiler used for Windows.  

----

### Rebuilding the Dynarithmic TWAIN Library from source ###

If you want to rebuild the libraries, you will need the following tools and computer resources:

One of the following compilers:

      * Visual Studio 2015 with Update 3
      * Visual Studio 2017 (may need to install XP tools and Windows 10 SDK from the VS 2017 Installation Manager).
      * Visual Studio 2019
      * Visual Studio 2022
      
In addition, you will need 

1) At least 20 GB of free disk space.
2) An installation of the [Boost](http://www.boost.org/) library (version 1.68 or above)
3) An installation of the [7-Zip](https://www.7-zip.org/) archiving utility.
4) An installation of the [DirHash](https://idrassi.github.io/DirHash/) utility to compute the hash value of the resulting output files.
5) The following environment variables must be set before compilation (if using the Visual Studio IDE, these variables must be set before starting the IDE):

*    BOOST_INCLUDE_DIR
*    ZIP7_UTIL_CMD
*    DIRHASH_UTIL_CMD

The **BOOST_INCLUDE_DIR** should point to your boost installation folder.  This is usually **C:\boost_install_directory**, where this folder contains a subsequent **boost** folder, containing the **boost** header files.  For example:

```plaintext
SET BOOST_INCLUDE_DIR=C:\boost_installation\boost
```
    
Since the DTWAIN build also requires to link to the prebuilt boost libraries, you must also set the appropriate environment variable to link to the boost library:

**If using Visual Studio 2015:**
*    BOOST_LIBRARY_DIR_2015_32
*    BOOST_LIBRARY_DIR_2015_64    

**If using Visual Studio 2017:**
*    BOOST_LIBRARY_DIR_2017_32
*    BOOST_LIBRARY_DIR_2017_64    

**If using Visual Studio 2019:**
*    BOOST_LIBRARY_DIR_2019_32
*    BOOST_LIBRARY_DIR_2019_64    

**If using Visual Studio 2022:**
*    BOOST_LIBRARY_DIR_2022_32
*    BOOST_LIBRARY_DIR_2022_64    

The **BOOST_LIBRARY_DIR_xxxx_32**, where **xxxx** is the compiler you are using to build with, is the directory where the 32-bit boost libraries are installed.  These libraries would be named similar to **libboost_xxxxxxxx-vcyyy-zzzz-x32.lib**.  For example:

```plaintext
SET BOOST_LIBRARY_DIR_2015_32=C:\boost_installation\lib32-msvc-14.0
```

The **BOOST_LIBRARY_DIR_xxxx_64**, where **xxxx** is the compiler you are using to build with, is the directory where the 64-bit boost libraries are installed.  These libraries would be named similar to **libboost_xxxxxxxx-vcyyy-zzzz-x64.lib**.  For example:

```plaintext
SET BOOST_LIBRARY_DIR_2015_64=C:\boost_installation\lib64-msvc-14.0
```

If you are using Visual C++, I recommend getting the pre-built boost libraries found at [SourceForge](https://sourceforge.net/projects/boost/files/boost-binaries/).  When installed, you will get the boost **include** files, plus the library files.  Please download the version of boost that fits your compiler.  

```plaintext
For Visual C++ 2015 -- Download files with "14.0" in the file name.
For Visual C++ 2017 -- Download files with "14.1" in the file name.
For Visual C++ 2019 -- Download files with "14.2" in the file name.
For Visual C++ 2022 -- Download files with "14.3" in the file name.
```


If you are not using Visual C++, you will need to build the following boost libraries:  
* chrono
* date_time
* filesystem
* log
* system
* thread

----
The **ZIP7_UTIL_CMD** is the full path name of the executable file **7z.exe** of the **7-Zip** archiving utility.  For example:

```plaintext
SET ZIP7_UTIL_CMD=C:\7-Zip\7z.exe
```
----
The **DIRHASH_UTIL_CMD** is the full path name of the executable file **DirHash.exe** of the **DirHash** hash value computing utility.  For example:

```plaintext
SET DIRHASH_UTIL_CMD=C:\DirHash\DirHash.exe
```

If the environment variable is not set, the library files will be built, but the hash code computation of the output files will fail to execute.  

The final hashes of the files are stored as text in the **binaries\32bit** and **binaries\64bit** folders (the names of the files will look something like **xxxxxxhash.txt**).

----------
Note that the C++ source code should be able to be built with any C++11 or C++14 compliant compiler that recognizes the Windows API headers (MingW using g++ 5.0 or above is an example).  However we have not tested builds of the DTWAIN library that have been built with any other compiler other than the Visual Studio family.   

* Start Visual Studio, and open one of the DTWAIN solution.  The DTWAIN solution files are found in the [source](https://github.com/dynarithmic/twain_library_source/tree/master/source) directory.  Open **dtwain_5_vs2015.sln**, **dtwain_5_vs2017.sln**, **dtwain_5_vs2019.sln**, or **dtwain_5_vs2022.sln**, depending on whether you are using Visual Studio 2015, 2017, 2019, or 2022, respectively. 

* A full rebuild of all the configurations available is recommended.  Use the "Build -> Batch Build..." option in the Visual Studio IDE and check all of the configurations to build everything (take a coffee break -- this could take a while).  This will create a "binaries" directory that will contain the following DLLs:

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


----

### Contributing your updates to this repository
If you wish to add your own changes to this repository, it is highly suggested that you "git clone" the **development** branch, and then make a pull request to have your changes merged into the development branch (not the **main** branch).  Once the pull request passes review, the updated changes will be merged into the development branch.  

When deemed appropriate by the maintainer of this repository, the development branch will be merged into the main branch.  Then the main branch will be used to build the libraries found in the dynarithmic/twain_library repository.  The main branch will always reflect the current build being distributed to the public in the dynarithmic/twain_library repository. 

