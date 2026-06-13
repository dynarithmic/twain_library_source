cmake_minimum_required(VERSION 3.25)

if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

project (dtwain VERSION 5.9.1 LANGUAGES C CXX)

option(DTWAIN_BUILD_UNICODE
       "Build DTWAIN as Unicode instead of ANSI"
       ON)

option(DTWAIN_ENABLE_LOGCALLSTACK
       "Build DTWAIN with call-stack logging enabled"
       ON)

if(DTWAIN_ENABLE_LOGCALLSTACK)
    set(DTWAIN_BUILD_LOGCALLSTACK 1)
else()
    set(DTWAIN_BUILD_LOGCALLSTACK 0)
endif()

configure_file(
    ${PROJECT_SOURCE_DIR}/dtwain_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/dtwain_config.h
    @ONLY
)

option(DTWAIN_USE_DYNAMIC_CRT
       "Use dynamic MSVC runtime DLL (/MD, /MDd)"
       ON)

option(DTWAIN_ENABLE_SEH_EXCEPTIONS
       "Advanced: ON=/EHa, OFF=/EHsc (recommended)"
       OFF)

mark_as_advanced(DTWAIN_ENABLE_SEH_EXCEPTIONS)

# -----------------------------------------------------------------------------
# Boost setup
# -----------------------------------------------------------------------------
# This project can download/install the matching prebuilt Boost binary package
# for the current Visual Studio toolset and platform.
#
# CMake-Gui workflow:
#   1. Press Configure once.
#   2. Set DTWAIN_BOOST_CACHE_ROOT if you do not want the default C:/BoostDeps.
#   3. Set DTWAIN_AUTO_DOWNLOAD_BOOST=ON.
#   4. Press Configure again, then Generate.
#
# The Boost install root is shared by Visual Studio toolset and bitness, for example:
#   C:/BoostDeps/boost_1_91_0/
#       boost/
#       lib32-msvc-14.2/
#       lib64-msvc-14.2/
#       lib32-msvc-14.3/
#       lib64-msvc-14.3/
#
# This avoids installing duplicate Boost headers/docs for Win32/x64 and VS2019/VS2022.
# Do not run multiple Boost installers into the same root concurrently.

set(DTWAIN_BOOST_VERSION "1.91.0" CACHE STRING
    "Boost version to download/use. DTWAIN requires Boost 1.89.0 or later.")

if(DTWAIN_BOOST_VERSION VERSION_LESS "1.89.0")
    message(FATAL_ERROR
        "DTWAIN requires Boost 1.89.0 or later. "
        "Requested Boost version: ${DTWAIN_BOOST_VERSION}")
endif()

string(REPLACE "." "_" DTWAIN_BOOST_VERSION_UNDERSCORE "${DTWAIN_BOOST_VERSION}")

set(DTWAIN_BOOST_CACHE_ROOT
    "C:/BoostDeps"
    CACHE PATH
    "Short directory where Boost installers and binaries are stored")

option(DTWAIN_AUTO_DOWNLOAD_BOOST
       "Automatically download/install the required Boost binary package if missing"
       OFF)

option(DTWAIN_DELETE_BOOST_INSTALLER_AFTER_INSTALL
       "Delete the downloaded Boost installer after a successful automatic install"
       OFF)

option(DTWAIN_DELETE_BOOST_INSTALL_LOG_AFTER_INSTALL
       "Delete the Boost installer log after a successful automatic install"
       OFF)

option(DTWAIN_ALLOW_INSECURE_BOOST_DOWNLOAD
       "Allow Boost download without TLS certificate verification"
       OFF)

set(DTWAIN_TLS_VERIFY ON)

if(DTWAIN_ALLOW_INSECURE_BOOST_DOWNLOAD)
    set(DTWAIN_TLS_VERIFY OFF)
endif()

set(DTWAIN_EXISTING_BOOST_ROOT
    ""
    CACHE PATH
    "Optional existing Boost root folder. If set, no Boost download/install is performed.")

if(NOT MSVC)
    message(FATAL_ERROR
        "This CMakeLists.txt currently expects MSVC because it downloads "
        "Boost Windows binaries for Visual Studio.")
endif()

# Visual Studio / MSVC Boost binary mapping:
#   VS 2019 v142 => msvc-14.2
#   VS 2022 v143 => msvc-14.3
#   Experimental -- For boost version < 1.91 -- VS 2026 v143 => msvc-14.3
set(DTWAIN_BOOST_MSVC_OVERRIDE "" CACHE STRING
"Override Boost binary MSVC tag, e.g. 14.3, 14.5")

if(DTWAIN_BOOST_MSVC_OVERRIDE)
    set(DTWAIN_BOOST_MSVC "${DTWAIN_BOOST_MSVC_OVERRIDE}")
else()
    if(MSVC_TOOLSET_VERSION MATCHES "^145")
        if(DTWAIN_BOOST_VERSION VERSION_LESS "1.91.0")
            set(DTWAIN_BOOST_MSVC "14.3")
        else()
            set(DTWAIN_BOOST_MSVC "14.5")
        endif()
    elseif(MSVC_TOOLSET_VERSION MATCHES "^143")
        set(DTWAIN_BOOST_MSVC "14.3")
    elseif(MSVC_TOOLSET_VERSION MATCHES "^142")
        set(DTWAIN_BOOST_MSVC "14.2")
    else()
        message(FATAL_ERROR
            "Unsupported MSVC toolset: ${MSVC_TOOLSET_VERSION}")
    endif()
endif()

# Platform mapping. CMAKE_SIZEOF_VOID_P is valid after project() enables C/CXX.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DTWAIN_BOOST_BITS "64")
else()
    set(DTWAIN_BOOST_BITS "32")
endif()

if(DTWAIN_EXISTING_BOOST_ROOT)
    file(TO_CMAKE_PATH
         "${DTWAIN_EXISTING_BOOST_ROOT}"
         DTWAIN_EFFECTIVE_BOOST_ROOT)
else()
    set(DTWAIN_EFFECTIVE_BOOST_ROOT
        "${DTWAIN_BOOST_CACHE_ROOT}/boost_${DTWAIN_BOOST_VERSION_UNDERSCORE}")
endif()

set(DTWAIN_REQUIRED_BOOST_LIBRARY_DIR
    "${DTWAIN_EFFECTIVE_BOOST_ROOT}/lib${DTWAIN_BOOST_BITS}-msvc-${DTWAIN_BOOST_MSVC}")

if(DTWAIN_EXISTING_BOOST_ROOT)
    if(NOT EXISTS "${DTWAIN_EFFECTIVE_BOOST_ROOT}/boost")
        message(FATAL_ERROR
            "Existing Boost root does not contain the Boost header directory:\n"
            "  ${DTWAIN_EFFECTIVE_BOOST_ROOT}/boost")
    endif()

    if(NOT EXISTS "${DTWAIN_REQUIRED_BOOST_LIBRARY_DIR}")
        message(FATAL_ERROR
            "Existing Boost root does not contain the required library directory for this build:\n"
            "  ${DTWAIN_REQUIRED_BOOST_LIBRARY_DIR}\n\n"
            "Install the matching Boost binary package or choose automatic Boost download.")
    endif()
endif()

if(DTWAIN_EXISTING_BOOST_ROOT)
    # User-supplied Boost installation. Expected layout:
    #   <root>/boost/version.hpp
    #   <root>/lib32-msvc-14.x  or  <root>/lib64-msvc-14.x
    set(DTWAIN_EFFECTIVE_BOOST_ROOT "${DTWAIN_EXISTING_BOOST_ROOT}")
else()
    file(MAKE_DIRECTORY "${DTWAIN_BOOST_CACHE_ROOT}")

    # Shared Boost root for this Boost version.
    # VS toolset and bitness are represented only by the library directory name:
    #   lib32-msvc-14.2, lib64-msvc-14.2, lib32-msvc-14.3, lib64-msvc-14.3, etc.
    set(DTWAIN_EFFECTIVE_BOOST_ROOT
        "${DTWAIN_BOOST_CACHE_ROOT}/boost_${DTWAIN_BOOST_VERSION_UNDERSCORE}")
endif()

set(DTWAIN_BOOST_INCLUDE_DIR "${DTWAIN_EFFECTIVE_BOOST_ROOT}")
set(DTWAIN_BOOST_LIBRARY_DIR
    "${DTWAIN_EFFECTIVE_BOOST_ROOT}/lib${DTWAIN_BOOST_BITS}-msvc-${DTWAIN_BOOST_MSVC}")

set(DTWAIN_BOOST_INSTALLER
    "boost_${DTWAIN_BOOST_VERSION_UNDERSCORE}-msvc-${DTWAIN_BOOST_MSVC}-${DTWAIN_BOOST_BITS}.exe")

set(DTWAIN_BOOST_URL
    "https://sourceforge.net/projects/boost/files/boost-binaries/${DTWAIN_BOOST_VERSION}/${DTWAIN_BOOST_INSTALLER}/download")

set(DTWAIN_BOOST_DOWNLOADED_EXE
    "${DTWAIN_BOOST_CACHE_ROOT}/${DTWAIN_BOOST_INSTALLER}")

set(DTWAIN_BOOST_INSTALL_LOG
    "${DTWAIN_BOOST_CACHE_ROOT}/${DTWAIN_BOOST_INSTALLER}.install.log")

set(DTWAIN_BOOST_HEADERS_FOUND FALSE)
if(EXISTS "${DTWAIN_BOOST_INCLUDE_DIR}/boost/version.hpp")
    set(DTWAIN_BOOST_HEADERS_FOUND TRUE)
endif()

set(DTWAIN_BOOST_LIBDIR_FOUND FALSE)
if(EXISTS "${DTWAIN_BOOST_LIBRARY_DIR}")
    set(DTWAIN_BOOST_LIBDIR_FOUND TRUE)
endif()

if(NOT DTWAIN_EXISTING_BOOST_ROOT AND
   (NOT DTWAIN_BOOST_HEADERS_FOUND OR NOT DTWAIN_BOOST_LIBDIR_FOUND))

    if(NOT DTWAIN_AUTO_DOWNLOAD_BOOST)
        message(STATUS "")
        message(STATUS "Boost is not installed for this configuration yet.")
        message(STATUS "")
        message(STATUS "Expected Boost root:")
        message(STATUS "  ${DTWAIN_EFFECTIVE_BOOST_ROOT}")
        message(STATUS "Expected Boost library directory:")
        message(STATUS "  ${DTWAIN_BOOST_LIBRARY_DIR}")
        message(STATUS "")
        message(STATUS "For CMake-Gui:")
        message(STATUS "  Option A: set DTWAIN_EXISTING_BOOST_ROOT to an existing Boost root")
        message(STATUS "  Option B:")
        message(STATUS "    1. Set DTWAIN_BOOST_CACHE_ROOT if desired, for example D:/BoostDeps")
        message(STATUS "    2. Set DTWAIN_AUTO_DOWNLOAD_BOOST=ON")
        message(STATUS "    3. Optionally set DTWAIN_DELETE_BOOST_INSTALLER_AFTER_INSTALL=ON")
        message(STATUS "    4. Optionally set DTWAIN_DELETE_BOOST_INSTALL_LOG_AFTER_INSTALL=ON")
        message(STATUS "    5. Press Configure again")
        message(STATUS "")
        message(STATUS "For command-line CMake, add:")
        message(STATUS "  Existing Boost: -DTWAIN_EXISTING_BOOST_ROOT=D:/boost_1_90_0")
        message(STATUS "  Auto download:  -DTWAIN_BOOST_CACHE_ROOT=D:/BoostDeps -DTWAIN_AUTO_DOWNLOAD_BOOST=ON")
        message(STATUS "  Delete installer/log after install:")
        message(STATUS "    -DTWAIN_DELETE_BOOST_INSTALLER_AFTER_INSTALL=ON")
        message(STATUS "    -DTWAIN_DELETE_BOOST_INSTALL_LOG_AFTER_INSTALL=ON")
        message(STATUS "")
        return()
    endif()

    if(NOT EXISTS "${DTWAIN_BOOST_DOWNLOADED_EXE}")
        message(STATUS "Downloading ${DTWAIN_BOOST_INSTALLER}")
        file(DOWNLOAD
            "${DTWAIN_BOOST_URL}"
            "${DTWAIN_BOOST_DOWNLOADED_EXE}"
            SHOW_PROGRESS
            STATUS DTWAIN_BOOST_DOWNLOAD_STATUS
            TLS_VERIFY ${DTWAIN_TLS_VERIFY})

        list(GET DTWAIN_BOOST_DOWNLOAD_STATUS 0 DTWAIN_BOOST_DOWNLOAD_RESULT)
        list(GET DTWAIN_BOOST_DOWNLOAD_STATUS 1 DTWAIN_BOOST_DOWNLOAD_MESSAGE)
        if(NOT DTWAIN_BOOST_DOWNLOAD_RESULT EQUAL 0)
          message(FATAL_ERROR
              "Boost download failed: \"${DTWAIN_BOOST_DOWNLOAD_MESSAGE}\"\n\n"
              "URL:\n"
              "  ${BOOST_DOWNLOAD_URL}\n\n"
              "Possible causes:\n"
              "  * Temporary SourceForge outage\n"
              "  * TLS/SSL certificate issue\n"
              "  * Corporate proxy/firewall\n"
              "  * Antivirus HTTPS inspection\n\n"
              "Try running CMake again.\n\n"
              "If the problem persists:\n"
              "  1) Download the installer manually from the URL above.\n"
              "  2) Save it to:\n"
              "     ${BOOST_INSTALLER}\n"
              "  3) Re-run CMake.\n\n"
              "Advanced users may also configure:\n"
              "  DTWAIN_ALLOW_INSECURE_BOOST_DOWNLOAD=ON"
          )
        endif()
    else()
        message(STATUS "Using existing downloaded Boost installer: ${DTWAIN_BOOST_DOWNLOADED_EXE}")
    endif()


    message(STATUS "Installing Boost to ${DTWAIN_EFFECTIVE_BOOST_ROOT}")
    execute_process(
        COMMAND "${DTWAIN_BOOST_DOWNLOADED_EXE}"
                /VERYSILENT
                /SUPPRESSMSGBOXES
                /NORESTART
                /NOCANCEL
                /SP-
                "/DIR=${DTWAIN_EFFECTIVE_BOOST_ROOT}"
                "/LOG=${DTWAIN_BOOST_INSTALL_LOG}"
        RESULT_VARIABLE DTWAIN_BOOST_INSTALL_RESULT)

    if(NOT DTWAIN_BOOST_INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR
            "Boost installer failed with exit code ${DTWAIN_BOOST_INSTALL_RESULT}.\n"
            "Installer: ${DTWAIN_BOOST_DOWNLOADED_EXE}\n"
            "Log: ${DTWAIN_BOOST_INSTALL_LOG}")
    endif()

    # Optional cleanup after a successful install.  These are OFF by default so
    # the installer and log remain available for troubleshooting unless the user
    # explicitly chooses to remove them.
    if(DTWAIN_DELETE_BOOST_INSTALLER_AFTER_INSTALL AND
       EXISTS "${DTWAIN_BOOST_DOWNLOADED_EXE}")
        file(REMOVE "${DTWAIN_BOOST_DOWNLOADED_EXE}")
        message(STATUS "Deleted Boost installer: ${DTWAIN_BOOST_DOWNLOADED_EXE}")
    endif()

    if(DTWAIN_DELETE_BOOST_INSTALL_LOG_AFTER_INSTALL AND
       EXISTS "${DTWAIN_BOOST_INSTALL_LOG}")
        file(REMOVE "${DTWAIN_BOOST_INSTALL_LOG}")
        message(STATUS "Deleted Boost installer log: ${DTWAIN_BOOST_INSTALL_LOG}")
    endif()

    # Make a later CMake-Gui Configure less likely to re-run installers accidentally.
    set(DTWAIN_AUTO_DOWNLOAD_BOOST OFF CACHE BOOL
        "Automatically download/install the required Boost binary package if missing"
        FORCE)
endif()

if(NOT EXISTS "${DTWAIN_BOOST_INCLUDE_DIR}/boost/version.hpp")
    message(FATAL_ERROR "Boost headers were not found at: ${DTWAIN_BOOST_INCLUDE_DIR}")
endif()

if(NOT EXISTS "${DTWAIN_BOOST_LIBRARY_DIR}")
    message(FATAL_ERROR "Boost library directory was not found at: ${DTWAIN_BOOST_LIBRARY_DIR}")
endif()

# These are CMake variables, not machine-wide environment variables. CMake writes
# the resulting include/library paths into the generated Visual Studio project.
set(BOOST_ROOT "${DTWAIN_EFFECTIVE_BOOST_ROOT}")
set(BOOST_INCLUDEDIR "${DTWAIN_BOOST_INCLUDE_DIR}")
set(BOOST_LIBRARYDIR "${DTWAIN_BOOST_LIBRARY_DIR}")

message(STATUS "Boost root: ${DTWAIN_EFFECTIVE_BOOST_ROOT}")
message(STATUS "Boost include dir: ${DTWAIN_BOOST_INCLUDE_DIR}")
message(STATUS "Boost library dir: ${DTWAIN_BOOST_LIBRARY_DIR}")
if(DTWAIN_EXISTING_BOOST_ROOT)
    message(STATUS "Boost mode: existing user-supplied installation")
else()
    message(STATUS "Boost mode: DTWAIN cache / auto-download")
    if(EXISTS "${DTWAIN_BOOST_DOWNLOADED_EXE}")
        message(STATUS "Boost installer: ${DTWAIN_BOOST_DOWNLOADED_EXE}")
    endif()
    if(EXISTS "${DTWAIN_BOOST_INSTALL_LOG}")
        message(STATUS "Boost installer log: ${DTWAIN_BOOST_INSTALL_LOG}")
    endif()
endif()

set(DTWAIN_RESOURCE_FILES
    ${PROJECT_SOURCE_DIR}/rc/twil32.rc
    ${PROJECT_SOURCE_DIR}/rc/twainresourcestrings_english.rc2
    ${PROJECT_SOURCE_DIR}/rc/resource_verinfo.rc2
)

set(MAIN_HEADERS 
	${PROJECT_SOURCE_DIR}/h/ansiwideconverter_generic.h
	${PROJECT_SOURCE_DIR}/h/ansiwideconverter_win32.h
	${PROJECT_SOURCE_DIR}/h/arrayfactory.h
	${PROJECT_SOURCE_DIR}/h/blankpage.h
	${PROJECT_SOURCE_DIR}/h/capconst.h
	${PROJECT_SOURCE_DIR}/h/capinfomap.h
	${PROJECT_SOURCE_DIR}/h/capstruc.h
	${PROJECT_SOURCE_DIR}/h/cppfunc.h
	${PROJECT_SOURCE_DIR}/h/ctlarray.h
	${PROJECT_SOURCE_DIR}/h/ctlbimap.h
	${PROJECT_SOURCE_DIR}/h/ctlclosesource.h
	${PROJECT_SOURCE_DIR}/h/ctlcntry.h
	${PROJECT_SOURCE_DIR}/h/ctlconstexprfind.h
	${PROJECT_SOURCE_DIR}/h/ctlconstexprutils.h
	${PROJECT_SOURCE_DIR}/h/ctldefsource.h
	${PROJECT_SOURCE_DIR}/h/ctldevnt.h
	${PROJECT_SOURCE_DIR}/h/ctldib.h
	${PROJECT_SOURCE_DIR}/h/ctldib32.h
	${PROJECT_SOURCE_DIR}/h/ctldib32ex.h
	${PROJECT_SOURCE_DIR}/h/ctlenum.h
	${PROJECT_SOURCE_DIR}/h/ctlfilesave.h
	${PROJECT_SOURCE_DIR}/h/ctlfileutils.h
	${PROJECT_SOURCE_DIR}/h/ctlguiddef.h
	${PROJECT_SOURCE_DIR}/h/ctliface.h
	${PROJECT_SOURCE_DIR}/h/ctlloadresources.h
	${PROJECT_SOURCE_DIR}/h/ctllogcalls.h
	${PROJECT_SOURCE_DIR}/h/ctllogfunctioncall.h
	${PROJECT_SOURCE_DIR}/h/ctllogsourcecaps.h
	${PROJECT_SOURCE_DIR}/h/ctlobstr.h
	${PROJECT_SOURCE_DIR}/h/ctlobtyp.h
	${PROJECT_SOURCE_DIR}/h/ctlrandnumutils.h
	${PROJECT_SOURCE_DIR}/h/ctlsetgetcaps.h
	${PROJECT_SOURCE_DIR}/h/ctlsourcedibs.h
	${PROJECT_SOURCE_DIR}/h/ctlstringconversion.h
	${PROJECT_SOURCE_DIR}/h/ctlstringutils.h
	${PROJECT_SOURCE_DIR}/h/ctlsupport.h
	${PROJECT_SOURCE_DIR}/h/ctlthreadutils.h
	${PROJECT_SOURCE_DIR}/h/ctltimer.h
	${PROJECT_SOURCE_DIR}/h/ctltmpl3.h
	${PROJECT_SOURCE_DIR}/h/ctltmpl4.h
	${PROJECT_SOURCE_DIR}/h/ctltmpl5.h
	${PROJECT_SOURCE_DIR}/h/ctltr000.h
	${PROJECT_SOURCE_DIR}/h/ctltr001.h
	${PROJECT_SOURCE_DIR}/h/ctltr002.h
	${PROJECT_SOURCE_DIR}/h/ctltr003.h
	${PROJECT_SOURCE_DIR}/h/ctltr004.h
	${PROJECT_SOURCE_DIR}/h/ctltr005.h
	${PROJECT_SOURCE_DIR}/h/ctltr006.h
	${PROJECT_SOURCE_DIR}/h/ctltr007.h
	${PROJECT_SOURCE_DIR}/h/ctltr008.h
	${PROJECT_SOURCE_DIR}/h/ctltr009.h
	${PROJECT_SOURCE_DIR}/h/ctltr010.h
	${PROJECT_SOURCE_DIR}/h/ctltr011.h
	${PROJECT_SOURCE_DIR}/h/ctltr012.h
	${PROJECT_SOURCE_DIR}/h/ctltr013.h
	${PROJECT_SOURCE_DIR}/h/ctltr014.h
	${PROJECT_SOURCE_DIR}/h/ctltr015.h
	${PROJECT_SOURCE_DIR}/h/ctltr016.h
	${PROJECT_SOURCE_DIR}/h/ctltr017.h
	${PROJECT_SOURCE_DIR}/h/ctltr018.h
	${PROJECT_SOURCE_DIR}/h/ctltr019.h
	${PROJECT_SOURCE_DIR}/h/ctltr020.h
	${PROJECT_SOURCE_DIR}/h/ctltr021.h
	${PROJECT_SOURCE_DIR}/h/ctltr022.h
	${PROJECT_SOURCE_DIR}/h/ctltr023.h
	${PROJECT_SOURCE_DIR}/h/ctltr024.h
	${PROJECT_SOURCE_DIR}/h/ctltr025.h
	${PROJECT_SOURCE_DIR}/h/ctltr026.h
	${PROJECT_SOURCE_DIR}/h/ctltr027.h
	${PROJECT_SOURCE_DIR}/h/ctltr028.h
	${PROJECT_SOURCE_DIR}/h/ctltr029.h
	${PROJECT_SOURCE_DIR}/h/ctltr030.h
	${PROJECT_SOURCE_DIR}/h/ctltr031.h
	${PROJECT_SOURCE_DIR}/h/ctltr032.h
	${PROJECT_SOURCE_DIR}/h/ctltr033.h
	${PROJECT_SOURCE_DIR}/h/ctltr034.h
	${PROJECT_SOURCE_DIR}/h/ctltr035.h
	${PROJECT_SOURCE_DIR}/h/ctltr036.h
	${PROJECT_SOURCE_DIR}/h/ctltr037.h
	${PROJECT_SOURCE_DIR}/h/ctltr038.h
	${PROJECT_SOURCE_DIR}/h/ctltr039.h
	${PROJECT_SOURCE_DIR}/h/ctltr040.h
	${PROJECT_SOURCE_DIR}/h/ctltr041.h
	${PROJECT_SOURCE_DIR}/h/ctltr042.h
	${PROJECT_SOURCE_DIR}/h/ctltr043.h
	${PROJECT_SOURCE_DIR}/h/ctltr044.h
	${PROJECT_SOURCE_DIR}/h/ctltr045.h
	${PROJECT_SOURCE_DIR}/h/ctltrall.h
	${PROJECT_SOURCE_DIR}/h/ctltripletbase.h
	${PROJECT_SOURCE_DIR}/h/ctltwaincompliancy.h
	${PROJECT_SOURCE_DIR}/h/ctltwaindecoder.h
	${PROJECT_SOURCE_DIR}/h/ctltwainidentity.h
	${PROJECT_SOURCE_DIR}/h/ctltwainmanager.h
	${PROJECT_SOURCE_DIR}/h/ctltwainmsgloop.h
	${PROJECT_SOURCE_DIR}/h/ctltwainsession.h
	${PROJECT_SOURCE_DIR}/h/ctltwainsource.h
	${PROJECT_SOURCE_DIR}/h/ctlutils.h
	${PROJECT_SOURCE_DIR}/h/dibinfox.h
	${PROJECT_SOURCE_DIR}/h/dibmulti.h
	${PROJECT_SOURCE_DIR}/h/dtwain.h
	${PROJECT_SOURCE_DIR}/h/dtwain_anyutils.h
	${PROJECT_SOURCE_DIR}/h/dtwain_exception.h
	${PROJECT_SOURCE_DIR}/h/dtwain_filesystem.h
	${PROJECT_SOURCE_DIR}/h/dtwain_filetypes.h
	${PROJECT_SOURCE_DIR}/h/dtwain_float_utils.h
	${PROJECT_SOURCE_DIR}/h/dtwain_library_selector.h
	${PROJECT_SOURCE_DIR}/h/dtwain_raii.h
	${PROJECT_SOURCE_DIR}/h/dtwain_resource_constants.h
	${PROJECT_SOURCE_DIR}/h/dtwain_resource_constants2.h
	${PROJECT_SOURCE_DIR}/h/dtwain_standard_defs.h
	${PROJECT_SOURCE_DIR}/h/dtwain_verinfo.h
	${PROJECT_SOURCE_DIR}/h/dtwain_version.h
	${PROJECT_SOURCE_DIR}/h/dtwainc.h
	${PROJECT_SOURCE_DIR}/h/dtwaindefs.h
	${PROJECT_SOURCE_DIR}/h/dtwainx.h
	${PROJECT_SOURCE_DIR}/h/dtwpdft.h
	${PROJECT_SOURCE_DIR}/h/dtwstrfn.h
	${PROJECT_SOURCE_DIR}/h/dtwtype.h
	${PROJECT_SOURCE_DIR}/h/errorcheck.h
	${PROJECT_SOURCE_DIR}/h/extendedimageinfo.h
	${PROJECT_SOURCE_DIR}/h/funcmac.h
	${PROJECT_SOURCE_DIR}/h/imagefun/bmprlewriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/dibutil.h
	${PROJECT_SOURCE_DIR}/h/imagefun/dtwpdf/crc32_aux.h
	${PROJECT_SOURCE_DIR}/h/imagefun/dtwpdf/dtwainpdf.h
	${PROJECT_SOURCE_DIR}/h/imagefun/gifwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/icowriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/imagefilewriterbase.h
	${PROJECT_SOURCE_DIR}/h/imagefun/jpeg2kwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/jpegwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/jpegxrwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/metafilewriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/pcxwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/pdffun32.h
	${PROJECT_SOURCE_DIR}/h/imagefun/pngwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/pnmwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/postscriptwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/psdwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/resample24to8.h
	${PROJECT_SOURCE_DIR}/h/imagefun/resamplefactory.h
	${PROJECT_SOURCE_DIR}/h/imagefun/svgwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/tgawriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/tiffwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/txtfun32.h
	${PROJECT_SOURCE_DIR}/h/imagefun/wbmpwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/webpwriter.h
	${PROJECT_SOURCE_DIR}/h/imagefun/winbit32.h
	${PROJECT_SOURCE_DIR}/h/imagexferfilewriter.h
	${PROJECT_SOURCE_DIR}/h/iohandler_bmp.h
	${PROJECT_SOURCE_DIR}/h/iohandler_gif.h
	${PROJECT_SOURCE_DIR}/h/iohandler_ico.h
	${PROJECT_SOURCE_DIR}/h/iohandler_jpeg.h
	${PROJECT_SOURCE_DIR}/h/iohandler_jpeg2k.h
	${PROJECT_SOURCE_DIR}/h/iohandler_jpegxr.h
	${PROJECT_SOURCE_DIR}/h/iohandler_pbm.h
	${PROJECT_SOURCE_DIR}/h/iohandler_pcx.h
	${PROJECT_SOURCE_DIR}/h/iohandler_pdf.h
	${PROJECT_SOURCE_DIR}/h/iohandler_png.h
	${PROJECT_SOURCE_DIR}/h/iohandler_ps.h
	${PROJECT_SOURCE_DIR}/h/iohandler_psd.h
	${PROJECT_SOURCE_DIR}/h/iohandler_svg.h
	${PROJECT_SOURCE_DIR}/h/iohandler_text.h
	${PROJECT_SOURCE_DIR}/h/iohandler_tga.h
	${PROJECT_SOURCE_DIR}/h/iohandler_tiff.h
	${PROJECT_SOURCE_DIR}/h/iohandler_wbmp.h
	${PROJECT_SOURCE_DIR}/h/iohandler_webp.h
	${PROJECT_SOURCE_DIR}/h/iohandler_wmf.h
	${PROJECT_SOURCE_DIR}/h/logmsg.h
	${PROJECT_SOURCE_DIR}/h/logwriterutils.h
	${PROJECT_SOURCE_DIR}/h/notimpl.h
	${PROJECT_SOURCE_DIR}/h/ocrengine/ocrinterface.h
	${PROJECT_SOURCE_DIR}/h/ocrengine/transym_ocrinterface.h
	${PROJECT_SOURCE_DIR}/h/sourceacquireopts.h
	${PROJECT_SOURCE_DIR}/h/sourceselectopts.h
	${PROJECT_SOURCE_DIR}/h/twain.h
	${PROJECT_SOURCE_DIR}/h/twainfix32.h
	${PROJECT_SOURCE_DIR}/h/twainframe.h
	${PROJECT_SOURCE_DIR}/h/versioninfo.h
	${PROJECT_SOURCE_DIR}/h/winconst.h
  ${PROJECT_SOURCE_DIR}/h/aeslib/AES.h
  ${PROJECT_SOURCE_DIR}/h/pdflib32/a85encode.h
  ${PROJECT_SOURCE_DIR}/h/pdflib32/ahexencode.h
  ${PROJECT_SOURCE_DIR}/h/pdflib32/base64encode.h
  ${PROJECT_SOURCE_DIR}/h/pdflib32/flateencode.h
  ${PROJECT_SOURCE_DIR}/h/pdflib32/pdfencrypt.h
  ${PROJECT_SOURCE_DIR}/h/pdflib32/pdflib32.h
  ${PROJECT_SOURCE_DIR}/h/ImageFun/dtwpdf/pdffont_basic.h
)

set(MAIN_INL
  ${PROJECT_SOURCE_DIR}/inl/ctlstrimpl.inl
  ${PROJECT_SOURCE_DIR}/inl/ctltr016.inl
  ${PROJECT_SOURCE_DIR}/inl/ctltr017.inl
  ${PROJECT_SOURCE_DIR}/inl/ctltr018.inl
  ${PROJECT_SOURCE_DIR}/inl/ctltr019.inl
  ${PROJECT_SOURCE_DIR}/inl/ctltr020.inl
  ${PROJECT_SOURCE_DIR}/inl/windowsinit_impl.inl
  ${PROJECT_SOURCE_DIR}/inl/windows_info.inl
  ${PROJECT_SOURCE_DIR}/inl/winget_twain.inl
  ${PROJECT_SOURCE_DIR}/inl/winlibraryloader_impl.inl
)

set(MAIN_SOURCES
  ${PROJECT_SOURCE_DIR}/cpp/aeslib/AES.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlacquirearea.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlacquirebuffered.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlacquireclipboard.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlacquirefile.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlacquirenative.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlarray.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlblankpage.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlcalldsmproc.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlcapcollect.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlcapcontainerfuncs.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlcapenumfuncs.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlcapsupport.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlcompressionsize.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctldeviceevents.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctldtwaininit.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctldtwainmem2.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlduplex.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlflipbmp.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlgetcapinfo.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlimageerrorskip.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlimageinfo.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlloggercallback.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctllogmessage.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlmanualduplex.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlmaxacquires.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlmultipagescan.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlpatchsupport.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlpixbitdepth.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlprinter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsavefilename.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsetgetcaps.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlshowuionly.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsourceacquire.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsourceclose.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsourcedibs.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsourceenum.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsourceinfo.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsourcenames.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsourceopen.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsourceretries.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsourcescale.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsourceselect.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsrceoj.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr040.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltwaindllhandle.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltwainmsgloop.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctluicontrollable.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctluienabled.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr008.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr010.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr011.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr015.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr016.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr017.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr018.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr019.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr020.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr024.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr025.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr026.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr027.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr031.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr035.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr037.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr038.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlobtyp.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlunicodeimpl.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlhigh.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctljob.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlfeed.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctldevnt.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsupport.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlverifysourcehandle.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagexferfilewriter.cpp
  ${PROJECT_SOURCE_DIR}/dtwinver/Dtwinver.cpp
  ${PROJECT_SOURCE_DIR}/dtwinver/Getwinver.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlflog.cpp
  ${PROJECT_SOURCE_DIR}/cpp/logmsg.cpp
  ${PROJECT_SOURCE_DIR}/cpp/a85decoder.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltempdir.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ocrengine/ocrinterface.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ocrengine/ocrinterface_internal.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ocrengine/transym_ocrinterface.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctljpeg.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltiff.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctldib32.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_base.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_bmp.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_gif.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_ico.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_jpeg.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_jpeg2k.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_pcx.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_pdf.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_png.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_ps.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_psd.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_text.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_tga.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_tiff.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_wbmp.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_wmf.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/winbit32.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlpdf.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/pdffun32.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/dtwpdf/dtwainpdf.cpp
  ${PROJECT_SOURCE_DIR}/cpp/arrayfactory.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlacquireaudio.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlarraydumper.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlbufferstrip.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlcustomdsdata.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlerrorbuffer.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlerrorcallback.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlextimageinfo.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlfileappend.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlfilesave.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlfileutils.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlgetmetrics.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlhashutils.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlloadresources.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlpapersize.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlselectdialog.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsourcecompression.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlsourcedetails.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlstringutils.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlthreadutils.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltimeout.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr041.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr042.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltr043.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltripletbase.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltwaincompliancy.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltwaindecoder.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltwainfilesystem.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltwainidentity.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltwainmanager.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltwainsession.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctltwainsource.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlupdatedibproc.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ctlwindowproc.cpp
  ${PROJECT_SOURCE_DIR}/cpp/extendedimageinfo.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/bmprlewriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/ctldibblank.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/dibutil.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/gifwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/icowriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/imagefilewriterbase.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/jpeg2kwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/jpegwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/jpegxrwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/metafilewriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/pcxwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/pngwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/pnmwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/postscriptwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/psdwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/resamplefactory.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/svgwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/tgawriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/tiffwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/txtfun32.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/wbmpwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/imagefun/webpwriter.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_jpegxr.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_pbm.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_svg.cpp
  ${PROJECT_SOURCE_DIR}/cpp/iohandler_webp.cpp
  ${PROJECT_SOURCE_DIR}/cpp/linux_versioninfo.cpp
  ${PROJECT_SOURCE_DIR}/cpp/logwriterutils.cpp
  ${PROJECT_SOURCE_DIR}/cpp/ocrengine/ocrselect.cpp
  ${PROJECT_SOURCE_DIR}/cpp/pdflib32/a85encode.cpp
  ${PROJECT_SOURCE_DIR}/cpp/pdflib32/ahexencode.cpp
  ${PROJECT_SOURCE_DIR}/cpp/pdflib32/base64encode.cpp
  ${PROJECT_SOURCE_DIR}/cpp/pdflib32/flateencode.cpp
  ${PROJECT_SOURCE_DIR}/cpp/pdflib32/pdfencrypt.cpp
  ${PROJECT_SOURCE_DIR}/cpp/pdflib32/pdflib32.cpp
  ${PROJECT_SOURCE_DIR}/cpp/stdafx.cpp
  ${PROJECT_SOURCE_DIR}/cpp/windows_versioninfo.cpp
  ${PROJECT_SOURCE_DIR}/c/imagefun/dtwpdf/crc32_aux.cpp
)

set(LIBGIF_SOURCES
  ${PROJECT_SOURCE_DIR}/LibGIF/dgif_lib.c
  ${PROJECT_SOURCE_DIR}/LibGIF/egif_lib.c
  ${PROJECT_SOURCE_DIR}/LibGIF/getarg.c
  ${PROJECT_SOURCE_DIR}/LibGIF/gifalloc.c
  ${PROJECT_SOURCE_DIR}/LibGIF/gif_err.c
  ${PROJECT_SOURCE_DIR}/LibGIF/gif_font.c
  ${PROJECT_SOURCE_DIR}/LibGIF/gif_hash.c
  ${PROJECT_SOURCE_DIR}/LibGIF/openbsd-reallocarray.c
  ${PROJECT_SOURCE_DIR}/LibGIF/qprintf.c
  ${PROJECT_SOURCE_DIR}/LibGIF/quantize.c
  ${PROJECT_SOURCE_DIR}/LibGIF/getarg.h
  ${PROJECT_SOURCE_DIR}/LibGIF/gif_hash.h
  ${PROJECT_SOURCE_DIR}/LibGIF/gif_lib.h
  ${PROJECT_SOURCE_DIR}/LibGIF/gif_lib_private.h
)

set(LIBJPEG_SOURCES
  ${PROJECT_SOURCE_DIR}/LibJPEG/jaricom.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jcapimin.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jcapistd.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jcarith.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jccoefct.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jccolor.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jcdctmgr.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jchuff.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jcinit.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jcmainct.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jcmarker.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jcmaster.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jcomapi.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jcparam.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jcprepct.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jcsample.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jctrans.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdapimin.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdapistd.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdarith.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdatadst.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdatasrc.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdcoefct.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdcolor.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jddctmgr.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdhuff.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdinput.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdmainct.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdmarker.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdmaster.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdmerge.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdpostct.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdsample.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jdtrans.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jerror.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jfdctflt.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jfdctfst.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jfdctint.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jidctflt.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jidctfst.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jidctint.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jmemmgr.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jmemnobs.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jquant1.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jquant2.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/jutils.c
  ${PROJECT_SOURCE_DIR}/LibJPEG/transupp.c
)

set(LIBJXR_SOURCES
  ${PROJECT_SOURCE_DIR}/LibJXR/image/decode/decode.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/decode/JXRTranscode.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/decode/postprocess.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/decode/segdec.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/decode/strdec.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/decode/strdec_x86.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/decode/strInvTransform.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/decode/strPredQuantDec.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/encode/encode.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/encode/segenc.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/encode/strenc.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/encode/strenc_x86.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/encode/strFwdTransform.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/encode/strPredQuantEnc.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/adapthuff.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/image_jxr.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/perfTimerANSI.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/strcodec.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/strPredQuant.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/strTransform.c
  ${PROJECT_SOURCE_DIR}/LibJXR/jxrgluelib/JXRGlue.c
  ${PROJECT_SOURCE_DIR}/LibJXR/jxrgluelib/JXRGlueJxr.c
  ${PROJECT_SOURCE_DIR}/LibJXR/jxrgluelib/JXRGluePFC.c
  ${PROJECT_SOURCE_DIR}/LibJXR/jxrgluelib/JXRMeta.c
  ${PROJECT_SOURCE_DIR}/LibJXR/image/decode/decode.h
  ${PROJECT_SOURCE_DIR}/LibJXR/image/encode/encode.h
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/ansi.h
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/common.h
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/perfTimer.h
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/strcodec.h
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/strTransform.h
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/windowsmediaphoto.h
  ${PROJECT_SOURCE_DIR}/LibJXR/image/sys/xplatform_image.h
  ${PROJECT_SOURCE_DIR}/LibJXR/jxrgluelib/JXRGlue.h
  ${PROJECT_SOURCE_DIR}/LibJXR/jxrgluelib/JXRMeta.h
)

set(LIBOPENJPEG_SOURCES
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/bio.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/cio.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/dwt.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/event.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/function_list.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/ht_dec.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/image.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/invert.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/j2k.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/jp2.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/mct.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/mqc.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/openjpeg.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/opj_clock.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/opj_malloc.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/pi.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/raw.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/sparse_array.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/t1.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/t2.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/tcd.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/tgt.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/thread.c
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/bio.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/cio.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/dwt.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/event.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/function_list.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/image.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/indexbox_manager.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/invert.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/j2k.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/jp2.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/mct.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/mqc.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/mqc_inl.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/openjpeg.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/opj_clock.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/opj_codec.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/opj_common.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/opj_config.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/opj_config_private.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/opj_includes.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/opj_intmath.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/opj_inttypes.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/opj_malloc.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/opj_stdint.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/pi.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/raw.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/sparse_array.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/t1.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/t1_luts.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/t2.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/tcd.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/tgt.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/thread.h
  ${PROJECT_SOURCE_DIR}/LibOpenJPEG/tls_keys.h
)

set(LIBPNG_SOURCES
  ${PROJECT_SOURCE_DIR}/LibPNG/png.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngerror.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngget.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngmem.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngpread.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngread.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngrio.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngrtran.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngrutil.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngset.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngtest.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngtrans.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngwio.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngwrite.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngwtran.c
  ${PROJECT_SOURCE_DIR}/LibPNG/pngwutil.c
  ${PROJECT_SOURCE_DIR}/LibPNG/png.h
  ${PROJECT_SOURCE_DIR}/LibPNG/pngconf.h
  ${PROJECT_SOURCE_DIR}/LibPNG/pngdebug.h
  ${PROJECT_SOURCE_DIR}/LibPNG/pnginfo.h
  ${PROJECT_SOURCE_DIR}/LibPNG/pnglibconf.h
  ${PROJECT_SOURCE_DIR}/LibPNG/pngpriv.h
  ${PROJECT_SOURCE_DIR}/LibPNG/pngstruct.h
)

set(LIBTIFF_SOURCES
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_aux.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_close.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_codec.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_color.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_compress.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_dir.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_dirinfo.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_dirread.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_dirwrite.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_dumpmode.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_error.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_extension.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_fax3.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_fax3sm.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_flush.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_getimage.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_hash_set.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_jbig.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_jpeg.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_lerc.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_luv.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_lzma.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_lzw.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_next.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_ojpeg.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_open.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_packbits.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_pixarlog.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_predict.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_print.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_read.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_strip.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_swab.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_thunder.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_tile.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_version.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_warning.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_webp.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_win32.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_write.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_zip.c
  ${PROJECT_SOURCE_DIR}/LibTIFF4/t4.h
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tiff.h
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tiffconf.h
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tiffio.h
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tiffiop.h
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tiffvers.h
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_config.h
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_dir.h
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_fax3.h
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_hash_set.h
  ${PROJECT_SOURCE_DIR}/LibTIFF4/tif_predict.h
)

set(LIBWEBP_SOURCES
  ${PROJECT_SOURCE_DIR}/LibWebP/sharpyuv/sharpyuv.c
  ${PROJECT_SOURCE_DIR}/LibWebP/sharpyuv/sharpyuv_cpu.c
  ${PROJECT_SOURCE_DIR}/LibWebP/sharpyuv/sharpyuv_csp.c
  ${PROJECT_SOURCE_DIR}/LibWebP/sharpyuv/sharpyuv_dsp.c
  ${PROJECT_SOURCE_DIR}/LibWebP/sharpyuv/sharpyuv_gamma.c
  ${PROJECT_SOURCE_DIR}/LibWebP/sharpyuv/sharpyuv_neon.c
  ${PROJECT_SOURCE_DIR}/LibWebP/sharpyuv/sharpyuv_sse2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/alpha_dec.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/buffer_dec.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/frame_dec.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/idec_dec.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/io_dec.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/quant_dec.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/tree_dec.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/vp8l_dec.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/vp8_dec.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/webp_dec.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/demux/anim_decode.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/demux/demux.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/alpha_processing.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/alpha_processing_mips_dsp_r2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/alpha_processing_neon.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/alpha_processing_sse2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/alpha_processing_sse41.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/cost.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/cost_mips32.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/cost_mips_dsp_r2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/cost_neon.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/cost_sse2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/cpu.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/dec.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/dec_clip_tables.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/dec_mips32.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/dec_mips_dsp_r2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/dec_msa.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/dec_neon.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/dec_sse2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/dec_sse41.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/enc_mips32.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/enc_mips_dsp_r2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/enc_msa.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/enc_neon.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/enc_sse2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/enc_sse41.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/filters.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/filters_mips_dsp_r2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/filters_msa.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/filters_neon.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/filters_sse2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_avx2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_enc_avx2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_enc_mips32.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_enc_mips_dsp_r2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_enc_msa.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_enc_neon.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_enc_sse2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_enc_sse41.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_mips_dsp_r2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_msa.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_neon.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_sse2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_sse41.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/rescaler.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/rescaler_mips32.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/rescaler_mips_dsp_r2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/rescaler_msa.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/rescaler_neon.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/rescaler_sse2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/ssim.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/ssim_sse2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/upsampling.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/upsampling_mips_dsp_r2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/upsampling_msa.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/upsampling_neon.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/upsampling_sse2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/upsampling_sse41.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/yuv.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/yuv_mips32.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/yuv_mips_dsp_r2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/yuv_neon.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/yuv_sse2.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/yuv_sse41.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/alpha_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/analysis_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/backward_references_cost_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/backward_references_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/config_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/cost_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/filter_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/frame_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/histogram_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/iterator_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/near_lossless_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/picture_csp_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/picture_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/picture_psnr_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/picture_rescale_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/picture_tools_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/predictor_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/quant_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/syntax_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/token_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/tree_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/vp8l_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/webp_enc.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/mux/anim_encode.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/mux/muxedit.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/mux/muxinternal.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/mux/muxread.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/bit_reader_utils.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/bit_writer_utils.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/color_cache_utils.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/filters_utils.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/huffman_encode_utils.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/huffman_utils.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/palette.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/quant_levels_dec_utils.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/quant_levels_utils.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/random_utils.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/rescaler_utils.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/thread_utils.c
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/utils.c
  ${PROJECT_SOURCE_DIR}/LibWebP/sharpyuv/sharpyuv.h
  ${PROJECT_SOURCE_DIR}/LibWebP/sharpyuv/sharpyuv_cpu.h
  ${PROJECT_SOURCE_DIR}/LibWebP/sharpyuv/sharpyuv_csp.h
  ${PROJECT_SOURCE_DIR}/LibWebP/sharpyuv/sharpyuv_dsp.h
  ${PROJECT_SOURCE_DIR}/LibWebP/sharpyuv/sharpyuv_gamma.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/alphai_dec.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/common_dec.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/vp8i_dec.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/vp8li_dec.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/vp8_dec.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dec/webpi_dec.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/common_sse2.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/common_sse41.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/cpu.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/dsp.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/lossless_common.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/mips_macro.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/msa_macro.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/neon.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/quant.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/dsp/yuv.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/backward_references_enc.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/cost_enc.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/histogram_enc.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/vp8i_enc.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/enc/vp8li_enc.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/mux/animi.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/mux/muxi.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/bit_reader_inl_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/bit_reader_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/bit_writer_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/bounds_safety.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/color_cache_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/endian_inl_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/filters_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/huffman_encode_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/huffman_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/palette.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/quant_levels_dec_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/quant_levels_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/random_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/rescaler_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/thread_utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/utils/utils.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/webp/decode.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/webp/demux.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/webp/encode.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/webp/format_constants.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/webp/mux.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/webp/mux_types.h
  ${PROJECT_SOURCE_DIR}/LibWebP/src/webp/types.h
)

set(ZLIB_SOURCES
  ${PROJECT_SOURCE_DIR}/ZLib/adler32.c
  ${PROJECT_SOURCE_DIR}/ZLib/compress.c
  ${PROJECT_SOURCE_DIR}/ZLib/crc32.c
  ${PROJECT_SOURCE_DIR}/ZLib/deflate.c
  ${PROJECT_SOURCE_DIR}/ZLib/gzclose.c
  ${PROJECT_SOURCE_DIR}/ZLib/gzlib.c
  ${PROJECT_SOURCE_DIR}/ZLib/gzread.c
  ${PROJECT_SOURCE_DIR}/ZLib/gzwrite.c
  ${PROJECT_SOURCE_DIR}/ZLib/infback.c
  ${PROJECT_SOURCE_DIR}/ZLib/inffast.c
  ${PROJECT_SOURCE_DIR}/ZLib/inflate.c
  ${PROJECT_SOURCE_DIR}/ZLib/inftrees.c
  ${PROJECT_SOURCE_DIR}/ZLib/trees.c
  ${PROJECT_SOURCE_DIR}/ZLib/uncompr.c
  ${PROJECT_SOURCE_DIR}/ZLib/zutil.c
)

set(MINIZ_SOURCES
  ${PROJECT_SOURCE_DIR}/miniz/miniz.c
  ${PROJECT_SOURCE_DIR}/miniz/miniz_tdef.c
  ${PROJECT_SOURCE_DIR}/miniz/miniz_tinfl.c
  ${PROJECT_SOURCE_DIR}/miniz/miniz_zip.c
  ${PROJECT_SOURCE_DIR}/miniz/miniz.h
  ${PROJECT_SOURCE_DIR}/miniz/miniz_common.h
  ${PROJECT_SOURCE_DIR}/miniz/miniz_export.h
  ${PROJECT_SOURCE_DIR}/miniz/miniz_tdef.h
  ${PROJECT_SOURCE_DIR}/miniz/miniz_tinfl.h
  ${PROJECT_SOURCE_DIR}/miniz/miniz_zip.h
)

set(SIMPLEINI_SOURCES
  ${PROJECT_SOURCE_DIR}/simpleini/SimpleIni.h
  )

set(DATEMASTER_SOURCES
  ${PROJECT_SOURCE_DIR}/date-master/include/date/date.h
  )


set(TEXT_RESOURCE_FILES
  ${PROJECT_SOURCE_DIR}/txt/twaininfo.txt
  ${PROJECT_SOURCE_DIR}/txt/twainlanguage.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_dutch.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_english.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_french.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_german.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_greek.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_italian.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_japanese.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_korean.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_portuguese.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_portuguese_br.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_romanian.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_russian.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_simplified_chinese.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_spanish.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_traditional_chinese.txt
  ${PROJECT_SOURCE_DIR}/txt/twainresourcestrings_turkish.txt
)

set(DTWDEMO_SOURCES
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/DebugUtils.c
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/DebugUtils.h
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/dibdisplay.c
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/dibdisplay.h
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/dibdisplayres.h
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/dtwdemo.c
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/DTWDEMO.h
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/DTWDEMO.ico
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/dtwdemo.rc
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/SourceProperties.c
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/SourceProperties.h
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/StringUtils.c
  ${PROJECT_SOURCE_DIR}/Examples/c/dtwdemo/StringUtils.h
  ${PROJECT_SOURCE_DIR}/Examples/c/dibview/DIB.c
  ${PROJECT_SOURCE_DIR}/Examples/c/dibview/DIB.h
  ${PROJECT_SOURCE_DIR}/Examples/c/dibview/DIBFUNC.h
  ${PROJECT_SOURCE_DIR}/Examples/c/dibview/ERRORS.c
  ${PROJECT_SOURCE_DIR}/Examples/c/dibview/ERRORS.h
  ${PROJECT_SOURCE_DIR}/Examples/c/dibview/MASTER.h
  ${PROJECT_SOURCE_DIR}/Examples/c/dibview/PAINT.c
  ${PROJECT_SOURCE_DIR}/Examples/c/dibview/PAINT.h
  ${PROJECT_SOURCE_DIR}/Examples/c/dibview/PALETTE.h
)

add_executable(dtwdemo WIN32
    ${DTWDEMO_SOURCES}
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DTWAIN_BITNESS "64")
else()
    set(DTWAIN_BITNESS "32")
endif()

if(DTWAIN_BUILD_UNICODE)
    set(DTWAIN_BUILD_TYPE_SUFFIX "u")
else()
    set(DTWAIN_BUILD_TYPE_SUFFIX "")
endif()

set_target_properties(dtwdemo PROPERTIES
    OUTPUT_NAME_DEBUG
        "dtwdemo${DTWAIN_BITNESS}${DTWAIN_BUILD_TYPE_SUFFIX}d"

    OUTPUT_NAME_RELEASE
        "dtwdemo${DTWAIN_BITNESS}${DTWAIN_BUILD_TYPE_SUFFIX}"

    OUTPUT_NAME_MINSIZEREL
        "dtwdemo${DTWAIN_BITNESS}${DTWAIN_BUILD_TYPE_SUFFIX}"

    OUTPUT_NAME_RELWITHDEBINFO
        "dtwdemo${DTWAIN_BITNESS}${DTWAIN_BUILD_TYPE_SUFFIX}"
)

target_include_directories(dtwdemo PRIVATE
    ${PROJECT_SOURCE_DIR}/h
)

target_link_libraries(dtwdemo PRIVATE
    dtwain
)

target_compile_definitions(dtwdemo PRIVATE
    $<$<BOOL:${DTWAIN_BUILD_UNICODE}>:UNICODE>
    $<$<BOOL:${DTWAIN_BUILD_UNICODE}>:_UNICODE>
    $<$<NOT:$<BOOL:${DTWAIN_BUILD_UNICODE}>>:_MBCS>
    $<$<CONFIG:Debug>:USE_DTWAIN_DEBUG_LIB>
)

add_library(dtwain_libpng STATIC
    ${LIBPNG_SOURCES}
)

target_include_directories(dtwain_libpng PRIVATE
    ${PROJECT_SOURCE_DIR}/LibPNG
    ${PROJECT_SOURCE_DIR}/ZLib
)

target_compile_definitions(dtwain_libpng PRIVATE
    PNG_USE_READ_MACROS
    _CRT_SECURE_NO_WARNINGS
)

set_target_properties(dtwain_libpng PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_BINARY_DIR}/intermediate/libpng/Debug"

    ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL
        "${CMAKE_BINARY_DIR}/intermediate/libpng/MinSizeRel"

    ARCHIVE_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_BINARY_DIR}/intermediate/libpng/Release"

    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO
        "${CMAKE_BINARY_DIR}/intermediate/libpng/RelWithDebInfo"
)

add_library(dtwain SHARED
    ${MAIN_INL}
    ${MAIN_HEADERS}
    ${MAIN_SOURCES}
    ${LIBGIF_SOURCES}
    ${LIBJPEG_SOURCES}
    ${LIBJXR_SOURCES}
    ${LIBOPENJPEG_SOURCES}
    ${LIBTIFF_SOURCES}
    ${LIBWEBP_SOURCES}
    ${ZLIB_SOURCES}
    ${MINIZ_SOURCES}
    ${SIMPLEINI_SOURCES}
    ${DATEMASTER_SOURCES}
)

target_link_libraries(dtwain PRIVATE
    dtwain_libpng
)

#
# Build output name:
#
# dtwain<32|64><u?><d?>
#


if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DTWAIN_BITNESS "64")
else()
    set(DTWAIN_BITNESS "32")
endif()

if(DTWAIN_BUILD_UNICODE)
    set(DTWAIN_UNICODE_SUFFIX "u")
    target_compile_definitions(dtwain PRIVATE
        UNICODE
        _UNICODE)
else()
    set(DTWAIN_UNICODE_SUFFIX "")
    target_compile_definitions(dtwain PRIVATE
        _MBCS)
endif()

set(DTWAIN_SELECTED_DEF_FILE
    "${PROJECT_SOURCE_DIR}/def/dtwain${DTWAIN_BITNESS}${DTWAIN_BUILD_TYPE_SUFFIX}$<$<CONFIG:Debug>:d>.def"
)

file(GLOB DTWAIN_DEF_FILES
    "${PROJECT_SOURCE_DIR}/def/*.def"
)

set(DTWAIN_DEF_RELEASE
    "${PROJECT_SOURCE_DIR}/def/dtwain${DTWAIN_BITNESS}${DTWAIN_UNICODE_SUFFIX}.def"
)

set(DTWAIN_DEF_DEBUG
    "${PROJECT_SOURCE_DIR}/def/dtwain${DTWAIN_BITNESS}${DTWAIN_UNICODE_SUFFIX}d.def"
)

target_sources(dtwain PRIVATE
    ${DTWAIN_DEF_FILES}
)

set_source_files_properties(
    ${DTWAIN_DEF_FILES}
    PROPERTIES
        HEADER_FILE_ONLY TRUE
)

source_group(
    "Module Definition Files"
    FILES ${DTWAIN_DEF_FILES}
)

set_target_properties(dtwain PROPERTIES
    LINK_FLAGS_DEBUG "/DEF:${DTWAIN_DEF_DEBUG}"
    LINK_FLAGS_MINSIZEREL "/DEF:${DTWAIN_DEF_RELEASE}"
    LINK_FLAGS_RELEASE "/DEF:${DTWAIN_DEF_RELEASE}"
    LINK_FLAGS_RELWITHDEBINFO "/DEF:${DTWAIN_DEF_RELEASE}"
)

set_target_properties(dtwain PROPERTIES
    OUTPUT_NAME_DEBUG
        "dtwain${DTWAIN_BITNESS}${DTWAIN_UNICODE_SUFFIX}d"

    OUTPUT_NAME_RELEASE
        "dtwain${DTWAIN_BITNESS}${DTWAIN_UNICODE_SUFFIX}"

    OUTPUT_NAME_RELWITHDEBINFO
        "dtwain${DTWAIN_BITNESS}${DTWAIN_UNICODE_SUFFIX}"

    OUTPUT_NAME_MINSIZEREL
        "dtwain${DTWAIN_BITNESS}${DTWAIN_UNICODE_SUFFIX}"

    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

file(GLOB DTWAIN_DEF_FILES
    "${PROJECT_SOURCE_DIR}/def/*.def"
)

target_sources(dtwain PRIVATE ${DTWAIN_DEF_FILES})
source_group( "Module Definition Files" FILES ${DTWAIN_DEF_FILES})
target_sources(dtwain PRIVATE ${PROJECT_SOURCE_DIR}/rc/twil32.rc)
target_sources(dtwain PRIVATE ${TEXT_RESOURCE_FILES})

set(DTWAIN_RC_DEFINES /D_WIN32)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    list(APPEND DTWAIN_RC_DEFINES /D_WIN64)
endif()

if(DTWAIN_BUILD_UNICODE)
    list(APPEND DTWAIN_RC_DEFINES /DUNICODE /D_UNICODE)
else()
    list(APPEND DTWAIN_RC_DEFINES /D_MBCS)
endif()

if(MSVC)
    target_compile_options(dtwain PRIVATE /Zi)
    target_link_options(dtwain PRIVATE /DEBUG)
endif()

set_target_properties(dtwain PROPERTIES
    PDB_NAME_DEBUG "dtwain${DTWAIN_BITNESS}${DTWAIN_BUILD_TYPE_SUFFIX}d"
    PDB_NAME_RELEASE "dtwain${DTWAIN_BITNESS}${DTWAIN_BUILD_TYPE_SUFFIX}"
    PDB_NAME_MINSIZEREL "dtwain${DTWAIN_BITNESS}${DTWAIN_BUILD_TYPE_SUFFIX}"
    PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

if(DTWAIN_USE_DYNAMIC_CRT)
    list(APPEND DTWAIN_RC_DEFINES /D_DLL)
endif()

set_source_files_properties(
    ${PROJECT_SOURCE_DIR}/rc/twil32.rc
    PROPERTIES
        COMPILE_OPTIONS "${DTWAIN_RC_DEFINES}"
)

source_group("Resource Files" FILES
    ${DTWAIN_RESOURCE_FILES}
)

target_compile_definitions(dtwain PRIVATE
    $<$<CONFIG:Debug>:NDEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_definitions(dtwain PRIVATE
    PNG_STATIC
    DTWAIN_LIBPNG_PRIVATE
)

if(MSVC)
    target_compile_options(dtwain PRIVATE
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/Zi>
        $<$<CONFIG:Debug>:/DNDEBUG>
        $<$<CONFIG:MinSizeRel>:/Zi>
        $<$<COMPILE_LANGUAGE:CXX>:/EHsc>
        /Zc:__cplusplus
    )

    target_link_options(dtwain PRIVATE
        $<$<CONFIG:Debug>:/DEBUG>
        $<$<CONFIG:Debug>:/OPT:NOREF>
        $<$<CONFIG:Debug>:/OPT:NOICF>
    )

    target_link_options(dtwain PRIVATE
        $<$<CONFIG:MinSizeRel>:/DEBUG>
        $<$<CONFIG:MinSizeRel>:/OPT:REF>
        $<$<CONFIG:MinSizeRel>:/OPT:ICF>
     )

    target_compile_options(dtwdemo PRIVATE
        $<$<CONFIG:MinSizeRel>:/GL>
        $<$<CONFIG:Release>:/GL>
    )

    target_link_options(dtwdemo PRIVATE
        $<$<CONFIG:MinSizeRel>:/LTCG>
        $<$<CONFIG:Release>:/LTCG>
    )

    target_link_options(dtwain PRIVATE
      "/DEF:${DTWAIN_SELECTED_DEF_FILE}"
)
endif()

target_compile_features(dtwain PRIVATE cxx_std_17)

target_include_directories(dtwain PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated
    ${PROJECT_SOURCE_DIR}/h
    ${PROJECT_SOURCE_DIR}/h/aeslib
    ${PROJECT_SOURCE_DIR}/h/imagefun
    ${PROJECT_SOURCE_DIR}/cpp
    ${PROJECT_SOURCE_DIR}/c
    ${PROJECT_SOURCE_DIR}/date-master/include
    ${PROJECT_SOURCE_DIR}/dtwinver
    ${PROJECT_SOURCE_DIR}/h/ImageFun/dtwpdf
    ${PROJECT_SOURCE_DIR}/inl
    ${PROJECT_SOURCE_DIR}/LibGIF
    ${PROJECT_SOURCE_DIR}/LibJPEG
    ${PROJECT_SOURCE_DIR}/LibJXR/image/sys
    ${PROJECT_SOURCE_DIR}/LibJXR/image/decode
    ${PROJECT_SOURCE_DIR}/LibJXR/image/encode
    ${PROJECT_SOURCE_DIR}/LibJXR/jxrgluelib
    ${PROJECT_SOURCE_DIR}/LibOpenJPEG
    ${PROJECT_SOURCE_DIR}/LibPNG
    ${PROJECT_SOURCE_DIR}/LibTIFF4
    ${PROJECT_SOURCE_DIR}/LibWebP
    ${PROJECT_SOURCE_DIR}/LibWebP/src
    ${PROJECT_SOURCE_DIR}/h/ocrengine
    ${PROJECT_SOURCE_DIR}/h/pdflib32
    ${PROJECT_SOURCE_DIR}/ZLib
    ${PROJECT_SOURCE_DIR}/miniz
    ${PROJECT_SOURCE_DIR}/simpleini
    ${DTWAIN_BOOST_INCLUDE_DIR}
)

set_target_properties(dtwain PROPERTIES
    OUTPUT_NAME "dtwain"
)

target_compile_features(dtwain PRIVATE cxx_std_17)

target_compile_definitions(dtwain PRIVATE
    $<$<BOOL:${DTWAIN_BUILD_UNICODE}>:UNICODE>
    $<$<BOOL:${DTWAIN_BUILD_UNICODE}>:_UNICODE>
    $<$<NOT:$<BOOL:${DTWAIN_BUILD_UNICODE}>>:_MBCS> 
    WIN32
    _WINDOWS
    _USRDLL
    WINVER=0x0601
    _WIN32_WINNT=0x0601
    NOMINMAX
    DTWAIN_SUPPORT_AES
    OPJ_STATIC
    DISABLE_PERF_MEASUREMENT
    DTWAIN_OPENSOURCE_DISTRIBUTION
    BUILDING_JPEG_DLL
    _DEBUGNDEBUG
    STRICT
    FUNCTION_MACRO_DEFINED
    PDFLIB_INTERNAL
    INCLUDE_DTWIMG32
    _CRT_SECURE_NO_WARNINGS
    DTWAIN_UNLICENSED_DISTRIBUTION_SPECIAL
    BUILDING_DTWAINDLL
    DTWAIN_DLL
    $<$<CONFIG:Debug>:USE_DTWAIN_DEBUG_LIB>
)

target_link_directories(dtwain PRIVATE
    ${DTWAIN_BOOST_LIBRARY_DIR}
)

source_group(
    TREE "${PROJECT_SOURCE_DIR}/h"
    PREFIX "Main Header Files"
    FILES ${MAIN_HEADERS}
)

source_group(
    TREE "${PROJECT_SOURCE_DIR}/inl"
    PREFIX "Inline Files"
    FILES ${MAIN_INL}
)

source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${MAIN_INL})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${LIBGIF_SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${LIBJPEG_SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${LIBJXR_SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${LIBOPENJPEG_SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${LIBPNG_SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${LIBTIFF_SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${LIBWEBP_SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${ZLIB_SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${MINIZ_SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${SIMPLEINI_SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${DATEMASTER_SOURCES})

if(MSVC)
    if(DTWAIN_USE_DYNAMIC_CRT)
        set(DTWAIN_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set(DTWAIN_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()

    set_property(TARGET dtwain PROPERTY
        MSVC_RUNTIME_LIBRARY "${DTWAIN_MSVC_RUNTIME_LIBRARY}")

    set_property(TARGET dtwain_libpng PROPERTY
        MSVC_RUNTIME_LIBRARY "${DTWAIN_MSVC_RUNTIME_LIBRARY}")

    set_property(TARGET dtwdemo PROPERTY
        MSVC_RUNTIME_LIBRARY "$<$<CONFIG:MinSizeRel>:MultiThreaded>")
endif()

if(MSVC)
    if(DTWAIN_USE_DYNAMIC_CRT)
        set_property(TARGET dtwain PROPERTY
            DTWAIN_MSVC_RUNTIME_LIBRARY
            "$<$<CONFIG:Debug>:MultiThreadedDebug>$<$<NOT:$<CONFIG:Debug>>:MultiThreadedDLL>"
        )
    else()
        set_property(TARGET dtwain PROPERTY
            DTWAIN_MSVC_RUNTIME_LIBRARY
            "$<$<CONFIG:Debug>:MultiThreadedDebug>$<$<NOT:$<CONFIG:Debug>>:MultiThreaded>"
        )
    endif()
endif()

if(MSVC)
    if(DTWAIN_ENABLE_SEH_EXCEPTIONS)
        set(DTWAIN_EXCEPTION_OPTION "/EHa")
    else()
        set(DTWAIN_EXCEPTION_OPTION "/EHsc")
    endif()

    target_compile_options(dtwain PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:${DTWAIN_EXCEPTION_OPTION}>
    )
endif()

set(DTWAIN_NATVIS_FILES
    ${PROJECT_SOURCE_DIR}/Natvis/dtwain.natvis
)

target_sources(dtwain PRIVATE
    ${DTWAIN_NATVIS_FILES}
)

source_group("Natvis" FILES
    ${DTWAIN_NATVIS_FILES}
)

source_group(
    "Text Resources"
    FILES
        ${TEXT_RESOURCE_FILES}
)

if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    find_program(DTWAIN_IMPLIB_EXE
        NAMES implib.exe implib
        DOC "Borland/Embarcadero implib.exe import-library tool"
    )

    if(DTWAIN_IMPLIB_EXE)
        add_custom_command(
            TARGET dtwain
            POST_BUILD
            COMMAND "${DTWAIN_IMPLIB_EXE}"
                    "$<TARGET_FILE_DIR:dtwain>/$<TARGET_FILE_BASE_NAME:dtwain>_embarcadero.lib"
                    "$<TARGET_FILE:dtwain>"
            COMMENT "Creating Embarcadero import library with implib"
            VERBATIM
        )
    else()
        message(STATUS "implib.exe not found; Embarcadero import library will not be generated.")
    endif()
endif()
