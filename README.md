# Dynarithmic TWAIN Library Source Code

This repository contains the source code and development versions of the Dynarithmic TWAIN Library DLL (DTWAIN).  

The DTWAIN library is written in C++ with a few modules written in C (mostly the third-party imaging libraries are written in C).

----
##### [Obtaining the latest development binaries](#obtain-dev-libraries)
##### [Rebuilding DTWAIN from source](#rebuild-source)
##### [Building the Demo Programs](#build-demo)  
##### [Contributing updates](#contribute-updates)
##### [Source Code Analysis Tools used](#tools-we-use)

----

## <a name="obtain-dev-libraries"></a> Obtaining the latest development binaries ###

To obtain the latest development binaries (DLL's, PDB files, import libraries, etc.):<br>
For 32-bit: 
[DTWAIN-Binaries-x86.zip](https://github.com/dynarithmic/twain_library_source/releases/download/development-latest/DTWAIN-Binaries-x86.zip)


For 64-bit:
[DTWAIN-Binaries-x64.zip](https://github.com/dynarithmic/twain_library_source/releases/download/development-latest/DTWAIN-Binaries-x64.zip)

The development binaries are built from the latest source code found in the [development branch](https://github.com/dynarithmic/twain_library_source/tree/development).  

1) To run DTWAIN using the development libraries will also require you to use the [text resources](https://github.com/dynarithmic/twain_library_source/tree/development/source/txt) found in the development branch, and not the text resources found in the main branch.  

2) The code found in the main branch reflects the current release version of DTWAIN.<br> If you want to debug the current release version of DTWAIN, the code in the main branch is the code you should retrieve.  

3) Please note that whenever new code is pushed to the development branch, new development binaries will also be created.  

----

## <a name="rebuild-source"></a> Rebuilding the Dynarithmic TWAIN Library from Source

Beginning with **DTWAIN 5.9.1**, rebuilding DTWAIN from source has transitioned from pre-generated Visual Studio solution files to a **CMake-based build system**.

Previous DTWAIN versions distributed Visual Studio project and solution files directly. These have been replaced with generated solutions created using CMake.

This change allows:

* Support for multiple Visual Studio versions from a single build system
* Automated [Boost C++ library](http://www.boost.org) setup (optional)
* Separate build directories for 32-bit and 64-bit builds
* Easier customization of Unicode / ANSI builds
* Configurable runtime (CRT / No CRT)
* Cleaner long-term maintenance of the build system

Users who only consume the prebuilt DTWAIN binaries are unaffected.

---

### Requirements

Before rebuilding DTWAIN from source:

#### Required

* Visual Studio 2019 or later (see below for the exact Visual Studio requirements)
* [CMake 3.25 or later (including CMake 4.x)](https://cmake.org/download/)

#### Optional

* Existing Boost library installation

or

* Enable automatic Boost library download during configuration

----

### Visual Studio Requirements

Building DTWAIN from source requires a Visual Studio installation with C/C++ development tools.

#### Installing Visual Studio

If Visual Studio is not already installed:

Visual Studio Community (free):
https://visualstudio.microsoft.com/vs/community/

Install Visual Studio 2019 or later.

During installation, ensure that the following workload is selected:

Required workload:
- Desktop development with C++

Required components:
- MSVC C++ compiler
- MSVC v142 (Visual Studio 2019) or MSVC v143 (Visual Studio 2022)
- Windows 10 SDK or Windows 11 SDK
- C++ CMake tools for Windows

The default Visual Studio installation may not include C/C++ support.

Optional:

- Git for Windows
- C++ AddressSanitizer (developer use)

If CMake configuration reports that no suitable compiler is found, re-run the Visual Studio Installer and add the C++ workload.

---

### Build Configurations

DTWAIN supports the following release build variants:

| Architecture | Character Set | Runtime |
| ------------ | ------------- | ------- |
| x86          | ANSI          | CRT     |
| x86          | ANSI          | No CRT  |
| x86          | Unicode       | CRT     |
| x86          | Unicode       | No CRT  |
| x64          | ANSI          | CRT     |
| x64          | ANSI          | No CRT  |
| x64          | Unicode       | CRT     |
| x64          | Unicode       | No CRT  |

Debug builds are generated separately and are intended primarily for developers debugging DTWAIN internals.

---

### Building Using Batch Files (Recommended)

The repository contains build scripts which generate the Visual Studio solution and build DTWAIN automatically.

Example:

```text
build_vs2019_x64_crt_unicode.bat
```

Running the script performs:

1. Configure CMake
2. Generate Visual Studio solution files
3. Locate or install Boost (if enabled)
4. Build DTWAIN
5. Build DTWDEMO

Example:

```bat
build_vs2019_x64_crt_unicode.bat
```

Generated output appears under:

```text
build-vs2019-x64-crt_unicode\
```

---

### Using an Existing Boost Installation

If Boost is already installed:

Edit the batch file:

```bat
set "BOOST_CACHE_ROOT="
set "EXISTING_BOOST_ROOT=D:/boost_1_90_0"
```

and disable automatic download:

```bat
-DTWAIN_AUTO_DOWNLOAD_BOOST=OFF
```

The existing Boost installation must follow the directory layout expected by DTWAIN.

Example:

```text
boost_1_xx_x/
    boost/
    lib32-msvc-14.x/
    lib64-msvc-14.x/
```

---

### Automatic Boost Download

If automatic Boost download is enabled:

```text
-DTWAIN_AUTO_DOWNLOAD_BOOST=ON
```

DTWAIN will:

* Download Boost binaries (if needed)
* Install Boost locally
* Configure include and library paths automatically

Installed Boost binaries may be reused by later builds.

---

### Manual CMake Usage

Advanced users may invoke CMake directly.

Example:

```bat
cmake -S . ^
      -B build-vs2019-x64-crt_unicode ^
      -G "Visual Studio 16 2019" ^
      -A x64 ^
      -DDTWAIN_BUILD_UNICODE=ON ^
      -DDTWAIN_USE_DYNAMIC_CRT=ON ^
      -DTWAIN_AUTO_DOWNLOAD_BOOST=ON

cmake --build build-vs2019-x64-crt_unicode --config MinSizeRel
cmake --build build-vs2019-x64-crt_unicode --config Debug
```

---

### Notes for Existing Users

If you previously rebuilt DTWAIN by opening `.sln` files directly:

```text
Old:
Open solution → Build

New:
Run batch file → Open generated solution → Build
```

Generated Visual Studio solutions continue to support debugging, natvis visualizers, and standard Visual Studio workflows.


----

### Generated files generated on successful build

A build will create two directories, **MinSizeRel** and **Debug**, within the output folder of the build (the name of the output folder will match the name of the batch file that was used to compile the source code.)

The **MinSizeRel** contains the release, non-debug versions of the DLL's, PDB files and import libraries that have been built.  The **Debug** directory contains the debug version of the DLL's, PDB files and import libraries.

----
----
## <a name="build-demo"></a> Building the demo applications
##### C++

If you wish to build the C and C++ demo applications, the **demos\AllDemos.sln** file can be loaded into Visual Studio 2019 or 2022.  Please note that you must build the base libraries first.  The demos consist of C and C++ language demos, plus C++ demos based on an experimental C++ wrapper library that is currently being developed.

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
----

## <a name="contribute-updates"></a> Contributing your updates to this repository
If you wish to add your own changes to this repository, it is highly suggested that you "git clone" the **development** branch, and then make a pull request to have your changes merged into the development branch (not the **main** branch).  Once the pull request passes review, the updated changes will be merged into the development branch.  

When deemed appropriate by the maintainer of this repository, the development branch will be merged into the main branch.  Then the main branch will be used to build the libraries found in the dynarithmic/twain_library repository.  The main branch will always reflect the current build being distributed to the public in the dynarithmic/twain_library repository. 

----
----
## <a name="tools-we-use"></a> Source Code Analysis Tools used
We use the following tools for source code analysis of the underlying C++ code base for DTWAIN:

[PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.


