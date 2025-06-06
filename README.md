# twain_library_source
Dynarithmic TWAIN library source and development repository.

This repository contains the source code and development versions of the Dynarithmic TWAIN Library DLL.  

The Microsoft PDB files for the debug versions of the DTWAIN library can be found [here](https://github.com/dynarithmic/dtwain-pdb).

----

##### [Rebuilding DTWAIN from source](#rebuild-source)
##### [Starting Visual Studio](#visual-studio-details)
##### [Building the Demo Programs](#build-demo)  
##### [Contributing updates](#contribute-updates)
##### [Source Code Analysis Tools used](#tools-we-use)

----

### <a name="rebuild-source"></a> Rebuilding the Dynarithmic TWAIN Library from source ###

**Please note -- these build steps only supports Visual Studio 2019 and above.**

The DTWAIN libraries are written in C++, and the minimum C++ standard is **C++17**.

If you want to rebuild the libraries, you will need the following tools and computer resources:

* One of the following development environments:

      Visual Studio 2019 (Community or Professional) with Platform Toolset v142 installed.
      Visual Studio 2022 (Community or Professional) with Platform Toolset v142 or v143 installed.
      
By default Visual Studio 2019 has Platform Toolset v142 installed.  
By default Visual Studio 2022 has Platform Toolset v143 installed. 

In addition, you will need 

1) At least 20 GB of free disk space.
2) An installation of the [Boost](http://www.boost.org/) library (version 1.80 or above).  I recommend getting the pre-built Boost libraries found at [SourceForge](https://sourceforge.net/projects/boost/files/boost-binaries/).  When installed, you will get the boost **include** files, plus the library files.  Please download the version of boost that fits your compiler.  

```plaintext
For Visual Studio 2019 or 2022 (using platform toolset v142) -- Download files with "14.2" in the file name.
For Visual Studio 2022 (using platform toolset v143) -- Download files with "14.3" in the file name.
```

3) An installation of the [7-Zip](https://www.7-zip.org/) archiving utility.
4) An installation of the [DirHash](https://idrassi.github.io/DirHash/) utility to compute the hash value of the resulting output files.
5) The following environment variables must be set before compilation (these variables must be set before starting the Visual Studio IDE):

*    BOOST_INCLUDE_DIR
*    ZIP7_UTIL_CMD
*    DIRHASH_UTIL_CMD

The following environment variables should be set, depending on the platform toolset being used when building DTWAIN using Visual Studio:

*    BOOST_LIBRARY_DIR_V142_32 -- (required: Visual Studio 2019 and Visual Studio 2022 using Platform Toolset v142)
*    BOOST_LIBRARY_DIR_V142_64 -- (required: Visual Studio 2019 and Visual Studio 2022 using Platform Toolset v142)   
*    BOOST_LIBRARY_DIR_V143_32 -- (required: Visual Studio 2022, Platform Toolset v143)
*    BOOST_LIBRARY_DIR_V143_64 -- (required: Visual Studio 2022, Platform Toolset v143)   

#### Definition of environment variables:

* The **BOOST_INCLUDE_DIR** should point to your boost installation folder that points to the base of the boost C++ header files.  This is usually **C:\boost_install_directory**, where this folder contains a subsequent **Boost** folder, containing the **Boost** header files.  For example:

```plaintext
SET BOOST_INCLUDE_DIR=C:\boost_installation\boost
```

* The environment variables named **BOOST_LIBRARY_DIR_xxxx_32**, where **xxxx** is the platform toolset you are using to build DTWAIN with, is the directory where the 32-bit boost libraries are installed.  These libraries would be named similar to **libboost_xxxxxxxx-vcyyy-zzzz-x32.lib**.  

For example, the following will set the boost environment variable to allow the usage of the 32-bit version of Boost for the v142 platform toolset:

```plaintext
SET BOOST_LIBRARY_DIR_V142_32=C:\boost_installation\lib32-msvc-14.2
```

The environment variables named **BOOST_LIBRARY_DIR_xxxx_64**, where **xxxx** is the compiler you are using to build with, is the directory where the 64-bit boost libraries are installed.  These libraries would be named similar to **libboost_xxxxxxxx-vcyyy-zzzz-x64.lib**.  For example, the following will set the boost environment variable to allow the usage of the 64-bit version of Boost for the v142 platform toolset:

```plaintext
SET BOOST_LIBRARY_DIR_V142_64=C:\boost_installation\lib64-msvc-14.2
```

If the **BOOST_LIBRARY_DIR_xxxx_32** or **BOOST_LIBRARY_DIR_xxxx_64** environment variables are not set or are set incorrectly, the final build of DTWAIN will not link, with the error message stating that the linker could not find the **libboost...** libraries.

----
* The **ZIP7_UTIL_CMD** is the full path name of the executable file **7z.exe** of the **7-Zip** archiving utility.  For example:

```plaintext
SET ZIP7_UTIL_CMD=C:\7-Zip\7z.exe
```
----
* The **DIRHASH_UTIL_CMD** is the full path name of the executable file **DirHash.exe** of the **DirHash** hash value computing utility.  For example:

```plaintext
SET DIRHASH_UTIL_CMD=C:\DirHash\DirHash.exe
```

If the environment variable is not set, the library files will be built, but the hash code computation of the output files will fail to execute.  

The final hashes of the files are stored as text in the **binaries\32bit** and **binaries\64bit** folders (the names of the files will look something like **xxxxxxhash.txt**).

---- 
----------
### <a name="visual-studio-details"></a>Starting Visual Studio ###

* Start the Microsoft Visual Studio, and open the DTWAIN solution file found in the [source](https://github.com/dynarithmic/twain_library_source/tree/master/source) directory named **dtwain_5_x_vs2019.sln**.

When you load the solution and project files in the Visual Studio IDE, you may be prompted to upgrade the platform toolset from v142 to the latest version that is supported by the particular Visual Studio compiler you are using.  You can choose to leave the platform toolset as-is, or decide to upgrade to the latest one.  However the platform toolset must be v142 or v143, and your current installation must have this toolset installed for your compiler.

Please note that the version of the Boost library that will be used at link time must match the platform toolset that DTWAIN was built with.  

* The next step is to check the **dtwain_config.h**.  This header file has the following entries:

```cpp
#define DTWAIN_BUILD_LOGCALLSTACK 1 
#define DTWAIN_BUILD_LOGPOINTERS 1
```
The **DTWAIN_BUILD_LOGCALLSTACK** denotes whether the libraries will be built with full call stack logging enabled.  If this value is **1**, then the libraries will be built with call stack logging.  Otherwise if the value is **0**, no call stack logging will be built.  

The **DTWAIN_BUILD_LOGPOINTERS** denotes whether the libraries will be built with the logging of any pointer values on return of a DTWAIN function that have output pointers as parameters.  As with **DTWAIN_BUILD_LOGCALLSTACK**, the values of **0** or **1** denote whether this option will be built into the DTWAIN DLL's.  Note that **DTWAIN_BUILD_CALLSTACK** must be **1** for **DTWAIN_BUILD_LOGPOINTERS** to take effect.

* A full rebuild of all the configurations available is recommended.  Use the "Build -> Batch Build..." option in the Visual Studio IDE and check all of the configurations to build everything (take a coffee break -- this could take a while).  This will create a "binaries" directory that will contain the following DLLs:

        32bit/dtwain32.dll   - 32-bit ANSI (MBCS) DLL
        32bit/dtwain32u.dll  - 32-bit Unicode DLL
        32bit/dtwain32d.dll  - 32-bit Debug ANSI (MBCS) DLL
        32bit/dtwain32ud.dll - 32-bit Debug Unicode DLL
        64bit/dtwain64.dll   - 64-bit ANSI (MBCS) DLL
        64bit/dtwain64u.dll  - 64-bit Unicode DLL
        64bit/dtwain64d.dll  - 64-bit Debug ANSI (MBCS) DLL
        64bit/dtwain64ud.dll - 64-bit Debug Unicode DLL

* Note -- the resulting "*.lib* files that reside in these directories are import libraries compatible with the Visual Studio toolset.  Other compilers will require converting these .lib files to your compiler's import library format, or you can use the [LoadLibrary / GetProcAddress approach](https://github.com/dynarithmic/twain_library?tab=readme-ov-file#what-if-i-dont-have-visual-c-as-the-compiler-to-use-when-building-an-application--the-visual-c-import-libraries-will-not-work-for-me--i-use-embarcaderogclangmingw-fill-in-with-your-favorite-compiler-or-ide--so-how-do-i-use-the-library) to remove the need for import libraries.

* If the IMPLIB.EXE program from Embarcadero is available on the path, it will be called to create Embarcadero C++ compatible import libraries for the 32-bit DLL's.  The names of the Embarcadero import libraries will have a **_embarcadero** appended to the library name.

* When all the configurations are built, there should be multiple DTWDEMO*.exe programs residing in the **binaries** subdirectory, where the suffix used in the program name matches the DTWAIN DLL that will be loaded.  For example, DTWDEMO32U.exe will load the dtwain32u.dll library when run. The easiest way to get started is to debug DTWDEMO.EXE and single step through the program using the debugger to get a feel of the flow of the program.  You should get a good idea of how DTWAIN works if you step into one or more of the DTWAIN functions (such as DTWAIN_SysInitialize or DTWAIN_SelectSource).

----
----
### <a name="build-demo"></a> Building the demo applications
##### C++

If you wish to build the C and C++ demo applications, the **demos\AllDemos.sln** file can be loaded into Visual Studio 2019 or 2022.  Please note that you must build the base libraries first (by building using the **dtwain_5_x_vs2019.sln** project, mentioned above) before building the demos.  The demos consist of C and C++ language demos, plus C++ demos based on an experimental C++ wrapper library that is currently being developed.

###### C#

The C# demo is **demos\csharp\Examples\FullDemo\CSharp_FullDemo.csproj**
This project is by default, setup for 32-bit Unicode (the dtwain32u.cs is part of the projects).  If you want to try 64-bit builds, please replace the dtwain32u.cs with one of the 64-bit .cs files (for example dtwain64u.cs).

###### Visual Basic

The Visual Basic demo is **demos\VisualBasic\Examples\FullDemo\VB_FullDemo.vbproj**
This project is by default, setup for 32-bit Unicode (the dtwain32u.vb is part of the projects).  If you want to try a 64-bit builds, please replace the dtwain32u.vb with one of the 64-bit .cs files (for example dtwain64u.vb).

-----
##### Using the debug DTWAIN DLL's in the C/C++ demo programs
The demo programs, by default, will use the release versions of the DTWAIN DLL's.  To use the debug versions of the DTWAIN library, one of the two options is available:

1) Use the following `#define` at the beginning of the main source file that include's **dtwain.h**:
```cpp
#define USE_DTWAIN_DEBUG_LIB
```
or

2) Define `USE_DTWAIN_DEBUG_LIB` as a **-D** or **/D** preprocessor setting on the compiler-command line (example would be **/DUSE_DTWAIN_DEBUG_LIB** on the compiler's command-line).

![](demos/preprocess.jpg)

If the debug libraries are being used, during the compilation phase in Visual Studio, you should receive the following message in the compiler Output Window:

```plaintext
Using import library xxxxxx.lib in link phase..
```
where "xxxxxx.lib" will be one of the following:

```plaintext
dtwain32d.lib    (this will use dtwain32d.dll at runtime)
dtwain32ud.lib   (this will use dtwain32ud.dll at runtime)
dtwain64d.lib    (this will use dtwain64d.dll at runtime)
dtwain64ud.lib   (this will use dtwain64ud.dll at runtime)
```

-----
##### Using the debug DTWAIN DLL's in a C# or Visual Basic program
The demo programs, by default, will use the release versions of the DTWAIN DLL's.  To use the debug versions of the DTWAIN library, build your application with one of the following interface files:

C#:
```plaintext
dtwain32d.cs     (this will use dtwain32d.dll at runtime)
dtwain32ud.cs    (this will use dtwain32ud.dll at runtime)
dtwain64d.cs     (this will use dtwain64d.dll at runtime)
dtwain64ud.cs    (this will use dtwain64ud.dll at runtime)
```


Visual Basic:
```plaintext
dtwain32d.vb     (this will use dtwain32d.dll at runtime)
dtwain32ud.vb    (this will use dtwain32ud.dll at runtime)
dtwain64d.vb     (this will use dtwain64d.dll at runtime)
dtwain64ud.vb    (this will use dtwain64ud.dll at runtime)
```



----

### <a name="contribute-updates"></a> Contributing your updates to this repository
If you wish to add your own changes to this repository, it is highly suggested that you "git clone" the **development** branch, and then make a pull request to have your changes merged into the development branch (not the **main** branch).  Once the pull request passes review, the updated changes will be merged into the development branch.  

When deemed appropriate by the maintainer of this repository, the development branch will be merged into the main branch.  Then the main branch will be used to build the libraries found in the dynarithmic/twain_library repository.  The main branch will always reflect the current build being distributed to the public in the dynarithmic/twain_library repository. 

----
### <a name="tools-we-use"></a> Source Code Analysis Tools used
We use the following tools for source code analysis of the underlying C++ code base for DTWAIN:

[PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.


