/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2024 Dynarithmic Software.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    FOR ANY PART OF THE COVERED WORK IN WHICH THE COPYRIGHT IS OWNED BY
    DYNARITHMIC SOFTWARE. DYNARITHMIC SOFTWARE DISCLAIMS THE WARRANTY OF NON INFRINGEMENT
    OF THIRD PARTY RIGHTS.
 */
#ifndef CTLIFACE_H
#define CTLIFACE_H

#ifdef _MSC_VER
#pragma warning( disable : 4786)
#pragma warning (disable : 4786)
#pragma warning (disable : 4127)
#endif
/*#include <algorithm>
#include <array>
#include <bitset>
#include <cstring>
#include <deque>
#include <list>
#include <queue>
#include <set>
#include <stack>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <cstdlib>
#include <boost/functional/hash.hpp>
#include <boost/variant2/variant.hpp>

#include "../tsl/ordered_map.h"*/
#include <mutex>
#include "ctltrp.h"
#include "dtwain_raii.h"
#include "ocrinterface.h"
#include "pdffont_basic.h"
#include "ctlres.h"
#include "dtwain.h"
#include "twainframe.h"
#include <boost/functional/hash.hpp>

#include "notimpl.h"
#include "sourceacquireopts.h"
#ifdef _WIN32
#include "winlibraryloader_impl.inl"
#else
#include "linuxlibraryloader_impl.inl"
#endif
#undef min
#undef max
template <typename T>
struct dtwain_library_loader : library_loader_impl
{
    static T get_func_ptr(void *handle, const char *name)
    {
        return static_cast<T>(get(handle, name));
    }
};

#include "capstruc.h"
#include "ctltmpl4.h"
#include "ctltwses.h"
#include "dtwain_resource_constants.h"
#include "errstruc.h"
#include "logmsg.h"
#include "winconst.h"
#include <map>

namespace dynarithmic
{
    class CTL_TwainDLLHandle;
    class CTL_ITwainSource;
    class CTL_TwainAppMgr;
    class CTL_TwainDibArray;
    struct SourceSelectionOptions;
    struct SourceAcquireOptions;

    #define  TWMSG_CancelSourceSelected         1100
    #define  TWMSG_TwainFailureConditionCode    1101
    #define  TWMSG_TwainAcquireImage            1102
        /* DTWAIN Source UI Close Modes */
    #define DTWAIN_SourceCloseModeFORCE           0
    #define DTWAIN_SourceCloseModeBYPASS          1

    #define DSM_STATE_NONE      1
    #define DSM_STATE_LOADED    2
    #define DSM_STATE_OPENED    3
        // Select source wParam's 
    #define  DTWAIN_SelectSourceFailed                1016
    #define  DTWAIN_AcquireSourceClosed               1017
    #define  DTWAIN_TN_ACQUIRECANCELLED_EX            1200
    #define  DTWAIN_TN_ACQUIREDONE_EX                 1205
    #define  DTWAIN_RETRY_EX                          9997

        // modal processing messages
    #define DTWAIN_TN_SETUPMODALACQUISITION           1300
    #define DTWAIN_TN_MESSAGELOOPERROR                1500
    #define REGISTERED_DTWAIN_MSG _T("DTWAIN_NOTIFY-{37AE5C3E-34B6-472f-A0BC-74F3CB199F2B}")

        // Availability flags
    #define DTWAIN_BASE_AVAILABLE       0
    #define DTWAIN_PDF_AVAILABLE        1
    #define DTWAIN_TWAINSAVE_AVAILABLE  2
    #define DTWAIN_OCR_AVAILABLE        3
    #define DTWAIN_BARCODE_AVAILABLE    4

        /* Transfer started */
        /* Scanner already has physically scanned a page.
         This is sent only once (when TWAIN actually does the transformation of the
         scanned image to the DIB) */
    #define  DTWAIN_TWAINAcquireStarted               1019

        /* Sent when DTWAIN_Acquire...() functions are about to return */
    #define  DTWAIN_AcquireTerminated                 1020
    #ifdef _WIN32
    #define  TWAINDLLVERSION_1    _T("TWAIN_32.DLL")
    #define  TWAINDLLVERSION_2    _T("TWAINDSM.DLL")
    #else
    #define  TWAINDLLVERSION_1    ""
    #define  TWAINDLLVERSION_2    "/usr/local/lib/libtwaindsm.so"
    #endif

    template <typename CallbackType, typename UserType>
    struct CallbackInfo
    {
        CallbackType Fn;
        UserType UserData;
        LRESULT retvalue;
        CallbackInfo(CallbackType theFn=NULL, UserType theUserData=0) :
        Fn(theFn), UserData(theUserData), retvalue(1)
        {}
    };

    // Note -- must have distinct key / value pairs.
    template <typename Key_, typename Value_>
    struct BiDirectionalMap
    {
        typedef std::map<Key_, Value_> left_map;
        typedef std::map<Value_, Key_> right_map;

        private:
            left_map left;
            right_map right;

        public:
            std::tuple<typename left_map::iterator,
                       typename right_map::iterator, bool>
            insert(const std::pair<Key_, Value_>& val)
            {
                auto it1 = left.insert({val.first, val.second});
                if ( it1.second )
                {
                    auto it2 = right.insert({val.second, val.first});
                    return {it1.first, it2.first, true};
                }
                return {it1.first, right.find(val.second), false};
            }

            std::tuple<typename left_map::iterator, typename right_map::iterator, bool>
            erase(const Key_& key)
            {
                auto it1 = left.find(key);
                if ( it1 != left.end())
                {
                    auto it2 = right.find(it1->second);
                    if ( it2 != right.end())
                    {
                        auto er1 = left.erase(it1);
                        auto er2 = right.erase(it2);
                        return {er1, er2, true};
                    }
                }
                return {left.end(), right.end(), false};
            }

            void clear()
            {
                left.clear();
                right.clear();
            }

            size_t size() const
            {
               return left.size();
            }

            bool empty() const
            {
                return left.empty();
            }

            const left_map& Left() { return left; }
            const right_map& Right() { return right; }
    };

    #include "capstruc.h"

    typedef CTL_ClassValues10<CTL_EnumCapability,/* Capability*/
                             UINT             , /* Container for Get */
                             UINT             , /* Container for Set*/
                             UINT             ,  /* Data Type */
                             UINT             ,  /* Available cap support */
                             UINT             ,  /* Container for Get Current */
                             UINT             ,   /* Container for Get Default  */
                             UINT             ,  /* Container for Set Constraint */
                             UINT             ,  /* Container for Reset */
                             UINT                 /* Container for Query Support*/
                            > CTL_CapInfo;

    #define CAPINFO_IDX_CAPABILITY 0    
    #define CAPINFO_IDX_GETCONTAINER 1    
    #define CAPINFO_IDX_SETCONTAINER 2    
    #define CAPINFO_IDX_DATATYPE     3    
    #define CAPINFO_IDX_SUPPORTEDOPS 4    
    #define CAPINFO_IDX_GETCURRENTCONTAINER 5    
    #define CAPINFO_IDX_GETDEFAULTCONTAINER 6    
    #define CAPINFO_IDX_SETCONSTRAINTCONTAINER 7    
    #define CAPINFO_IDX_RESETCONTAINER 8    
    #define CAPINFO_IDX_QUERYSUPPORT 9    

    typedef std::unordered_map<unsigned long, std::shared_ptr<CTL_TwainDLLHandle>> CTL_MapThreadToDLLHandle;
    typedef std::unordered_map<LONG, int> CTL_LongToIntMap;
    typedef BiDirectionalMap<std::pair<int, int>, std::string> CTL_TwainNameMap;
    typedef std::unordered_map<CTL_StringType, CTL_ITwainSource*> CTL_StringToSourcePtrMap;
    typedef std::unordered_map<CTL_StringType, int> CTL_StringToIntMap;
    typedef std::unordered_map<LONG, HMODULE> CTL_LongToHMODULEMap;
    typedef std::unordered_map<CTL_EnumCapability, CTL_CapInfo> CTL_EnumCapToInfoMap;
    typedef std::vector<CallbackInfo<DTWAIN_CALLBACK_PROC, LONG> > CTL_CallbackProcArray;
    typedef std::vector<CallbackInfo<DTWAIN_CALLBACK_PROC, LONGLONG> > CTL_CallbackProcArray64;
    typedef std::unordered_map<LONG, CTL_StringType> CTL_StringToLongMap;
    typedef std::map<LONG, std::string> CTL_LongToStringMap;
    typedef std::unordered_map<std::string, CTL_LongToStringMap> CTL_StringToMapLongToStringMap;
    typedef std::unordered_map<LONG, std::vector<LONG> > CTL_LongToVectorLongMap;
    typedef std::vector<CTL_MapThreadToDLLHandle>     CTL_HookInfoArray;

    // Create these dynamically whenever a new source is opened
    // and source cap info does not exist.  Add cap info statically.
    typedef std::unordered_map<CTL_EnumCapability, CTL_CapInfo>  CTL_CapInfoArray;
    typedef std::shared_ptr<CTL_CapInfoArray> CTL_CapInfoArrayPtr;

    // Create this statically when initializing.  Initialize the second
    // value with the dynamically created CTL_CapInfoArray above
    typedef CTL_ClassValues10<CTL_StringType, /* Product Name */
                             CTL_CapInfoArrayPtr, /* Array of cap info*/
                             int,       /* dummy */
                             int,        /* dummy */
                             char,
                             char,
                             char,
                             char,
                             char,
                             char> CTL_SourceCapInfo;

    // Add the statically created CTL_SourceCapInfo to this array
    typedef std::vector<CTL_SourceCapInfo> CTL_SourceCapInfoArray;

    constexpr int DTWAIN_MaxErrorSize=256;
    class CTLTwainDibArray;


    // define a vector that holds OCREngine interfaces
    typedef std::vector<OCREnginePtr> OCRInterfaceContainer;
    typedef std::unordered_map<std::string, OCREnginePtr> OCRProductNameToEngineMap;

    struct CustomPlacement
    {
        LONG nOptions;
        int xpos;
        int ypos;
        HWND hWndParent;
        CTL_StringType sTitle;
        std::vector<CTL_StringType> aIncludeNames;
        std::vector<CTL_StringType> aExcludeNames;
        std::unordered_map<CTL_StringType, CTL_StringType> mapNames;
        CustomPlacement() : nOptions(0), xpos(0), ypos(0), hWndParent(nullptr) {}
    };

    struct SelectStruct
    {
        CTL_StringType SourceName;
        CustomPlacement CS;
        CTL_TwainDLLHandle* pHandle;
        int nItems;
    };

    template <typename T>
    struct SmartPointerFinder
    {
        typedef typename T::element_type *ptr_type;
        ptr_type m_ptr;
        SmartPointerFinder(ptr_type p) : m_ptr(p) {}
        bool operator()(const T& value) const
        { return value.get() == m_ptr; }
    };

    struct ImageModuleDef
    {
        CTL_StringType sName;
        LONG    ImgType;
        HMODULE hMod;
        bool  bIsLoaded;
    };

    inline bool operator==(const TwainFrameInternal& lhs, const TwainFrameInternal& rhs)
    {
        return lhs.m_FrameComponent == rhs.m_FrameComponent;
    }

    inline bool operator!=(const TwainFrameInternal& lhs, const TwainFrameInternal& rhs)
    {
        return !(lhs.m_FrameComponent == rhs.m_FrameComponent);
    }

    typedef std::vector<ImageModuleDef> CTL_IMAGEDLLINFO;
    typedef std::shared_ptr<CTL_TwainDLLHandle> CTL_TwainDLLHandlePtr;

    template <typename T, int enumType=0>
    struct CTL_EnumeratorNode
    {
        typedef std::vector<T>  container_base_type;
        typedef container_base_type* container_pointer_type;
        typedef typename container_base_type::iterator container_iterator_type;
        int m_EnumType;
        container_base_type m_Array;
        CTL_EnumeratorNode(int nSize) : m_EnumType(enumType), m_Array(nSize) {}
        int GetEnumType() const { return m_EnumType; }
        void SetEnumType(int EnumType) { m_EnumType = EnumType; }
        enum {ENUMTYPE = enumType};
    };

    /* Fixed point structure type. */

    typedef std::string EnumStringTypeA;
    typedef std::wstring EnumStringTypeW;
    typedef CTL_ITwainSource* CTL_ITwainSourcePtr;
    
    typedef std::list<TwainFrameInternal> DTWAINFrameList;

    inline bool operator == (TW_FRAME lhs, TW_FRAME rhs)
    {
        return std::tie(lhs.Bottom.Frac, lhs.Bottom.Whole,
            lhs.Left.Frac, lhs.Left.Whole,
            lhs.Right.Frac, lhs.Right.Whole,
            lhs.Top.Frac, lhs.Top.Whole)
            ==
            std::tie(rhs.Bottom.Frac, rhs.Bottom.Whole,
                rhs.Left.Frac, rhs.Left.Whole,
                rhs.Right.Frac, rhs.Right.Whole,
                rhs.Top.Frac, rhs.Top.Whole);
    }

    inline bool operator!=(TW_FRAME f1, TW_FRAME f2)
    {
        return !(operator==(f1, f2));
    }

    struct CTL_ArrayFactory;
    typedef std::shared_ptr<CTL_ArrayFactory> CTL_ArrayFactoryPtr;

    class CTL_TwainDynMemoryHandler
    {
            TW_HANDLE m_handle;
            TW_MEMREF m_memPtr;
            TW_UINT32 m_memSize;

        public:
            TW_HANDLE getHandle() const { return m_handle; }
            TW_MEMREF getMemoryPtr() const { return m_memPtr; }
            TW_UINT32 getMemorySize() const { return m_memSize; }
            void setHandle(TW_HANDLE h) { m_handle = h; }
            void setMemoryPtr(TW_MEMREF p) { m_memPtr = p; }
            void setMemorySize(TW_UINT32 s) { m_memSize = s; }
            CTL_TwainDynMemoryHandler(TW_HANDLE h=nullptr, TW_MEMREF p=nullptr, TW_UINT32 memSize = 0)
                    : m_handle(h), m_memPtr(p), m_memSize(memSize) {}
    };

    // mimics 2.0 memory function pointers
    class CTL_TwainMemoryFunctions
    {
        public:
            virtual ~CTL_TwainMemoryFunctions() = default;
            virtual TW_HANDLE AllocateMemory(TW_UINT32 size) = 0;
            virtual void      FreeMemory(TW_HANDLE h) = 0;
            virtual TW_MEMREF LockMemory(TW_HANDLE h) = 0;
            virtual void      UnlockMemory(TW_HANDLE h) = 0;

            TW_MEMREF AllocateMemoryPtr(TW_UINT32 size, TW_HANDLE* pHandle = nullptr)
            {
                const TW_HANDLE h = AllocateMemory(size);
                if ( h )
                {
                    if ( pHandle )
                       *pHandle = h;
                    return LockMemory(h);
                }
                if ( pHandle )
                  *pHandle = nullptr;
                return nullptr;
            }

            TW_MEMREF ReallocateMemory(CTL_TwainDynMemoryHandler& memHandler, TW_UINT32 newSize)
            {
                // Allocate new memory
                const TW_HANDLE newHandle = AllocateMemory(newSize);
                if (!newHandle)
                    return nullptr;

                // copy old memory to new memory
                const TW_MEMREF oldMem = memHandler.getMemoryPtr();
                const TW_MEMREF newMem = LockMemory(newHandle);
                memcpy(newMem, oldMem, (std::min)(newSize, memHandler.getMemorySize()));
                UnlockMemory(newMem);

                // delete the old memory
                UnlockMemory(memHandler.getHandle());
                FreeMemory(memHandler.getHandle());

                // copy memHandler by constructing a new memory handler
                memHandler = CTL_TwainDynMemoryHandler(newHandle, newMem, newSize);
                return newHandle;
            }
    };


    class CTL_LegacyTwainMemoryFunctions : public CTL_TwainMemoryFunctions
    {
        public:
        #ifdef WIN32
            TW_HANDLE AllocateMemory(TW_UINT32 size) override { return GlobalAlloc(GHND, size); }
            void      FreeMemory(TW_HANDLE h) override { if (h) GlobalFree( h ); }
            TW_MEMREF LockMemory(TW_HANDLE h) override
            { if (h) return GlobalLock(h); return nullptr; }
            void      UnlockMemory(TW_HANDLE h) override { if (h) GlobalUnlock(h); }
        #else
            TW_HANDLE AllocateMemory(TW_UINT32) { return nullptr; }
            void      FreeMemory(TW_HANDLE) { }
            TW_MEMREF LockMemory(TW_HANDLE) { return nullptr; }
            void      UnlockMemory(TW_HANDLE) { }
        #endif
    };


    class CTL_Twain2MemoryFunctions : public CTL_TwainMemoryFunctions
    {
        public:
            TW_ENTRYPOINT m_EntryPoint;
            TW_HANDLE AllocateMemory(TW_UINT32 size) override { return m_EntryPoint.DSM_MemAllocate(size); }
            void      FreeMemory(TW_HANDLE h) override { if (h) m_EntryPoint.DSM_MemFree(h); }
            TW_MEMREF LockMemory(TW_HANDLE h) override
            { if (h) return m_EntryPoint.DSM_MemLock(h); return nullptr; }
            void      UnlockMemory(TW_HANDLE h) override { if (h) m_EntryPoint.DSM_MemUnlock(h); }
    };

    struct FileFormatNode
    {
        std::string m_formatName;
        std::vector<std::string> m_vExtensions;
        FileFormatNode(std::string name, std::vector<std::string> vExt) :
            m_formatName(std::move(name)), m_vExtensions(std::move(vExt)) {}
    };

    struct ImageResamplerData
    {
        std::string m_sImageType;
        std::vector<uint16_t> m_vNoSamples;
        std::map<uint16_t, uint16_t> m_mapFromTo;
    };

    struct SourceStatus
    {
        enum { SOURCE_STATUS_OPEN, SOURCE_STATUS_SELECECTED, SOURCE_STATUS_UNKNOWN };

        std::bitset<3> m_Status;
        std::string m_ThreadId;
        CTL_ITwainSource* m_pSource;
        SourceStatus() : m_Status(), m_ThreadId{}, m_pSource{} {}
        SourceStatus& SetStatus(int Status, bool bSet) { m_Status[Status] = bSet; return *this; }
        bool GetStatus(int Status) const { return m_Status[Status]; }
        bool IsSelected() const { return m_Status[SOURCE_STATUS_SELECECTED]; }
        bool IsOpen() const { return m_Status[SOURCE_STATUS_OPEN]; }
        bool IsClosed() const { return !IsOpen(); }
        bool IsUnknown() const { return m_Status[SOURCE_STATUS_UNKNOWN]; }
        SourceStatus& SetThreadID(std::string threadId) { m_ThreadId = threadId; return *this; }
        SourceStatus& SetSourceHandle(CTL_ITwainSource* Source) { m_pSource = Source; return *this; }
        std::string GetThreadID() const { return m_ThreadId; }
        CTL_ITwainSource* GetSourceHandle() const { return m_pSource; }
    };

    struct CacheKeyHash 
    {
        std::size_t operator()(const std::pair<LONG, std::string>& key) const 
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, key.first);
            boost::hash_combine(seed, key.second);
            return seed;
        }
    };

    typedef std::map<std::string, SourceStatus> SourceStatusMap;
    typedef std::map<int, ImageResamplerData> ImageResamplerMap;
    typedef std::unordered_map<LONG, std::pair<std::string, std::string>> CTL_PDFMediaMap;
    typedef tsl::ordered_map<LONG, FileFormatNode> CTL_AvailableFileFormatsMap;
    typedef tsl::ordered_map<LONG, std::string> CTL_TwainConstantToStringMapNode;
    typedef std::unordered_map<LONG, CTL_TwainConstantToStringMapNode> CTL_TwainConstantsMap;
    typedef std::unordered_map<LONG, std::string> CTL_TwainLongToStringMap;
    typedef std::unordered_map<int32_t, std::string> CTL_ErrorToExtraInfoMap;
    typedef std::unordered_map<std::string, unsigned long> CTL_ThreadMap;
    typedef std::unordered_map<std::pair<LONG, std::string>, std::string, CacheKeyHash> CTL_PairToStringMap;

    struct CTL_GeneralResourceInfo
    {
        CTL_StringType sResourceName;
        bool bIsFromRC = false;
    };

    struct CTL_StaticData
    {
        static CTL_StringToMapLongToStringMap s_AllLoadedResourcesMap;
        static CTL_GeneralResourceInfo         s_ResourceInfo;
        static CTL_PDFMediaMap          s_PDFMediaMap;
        static CTL_TwainLongToStringMap s_TwainCountryMap;
        static CTL_TwainNameMap         s_TwainNameMap;
        static CTL_AvailableFileFormatsMap s_AvailableFileFormatsMap;
        static CTL_TwainConstantsMap s_TwainConstantsMap;
        static CTL_TwainLongToStringMap s_TwainLanguageMap;
        static bool s_bCheckHandles;
        static CTL_StringType           s_strResourcePath;  // path to the DTWAIN resource strings
        static CTL_StringType           s_DLLPath;
        static CTL_StringType           s_sINIPath;
        static bool                     s_multipleThreads;
        static CTL_LongToStringMap      s_ErrorCodes;
        static CTL_StringType           s_VersionString;
        static HFONT                    s_DialogFont;
        static CTL_ErrorToExtraInfoMap  s_mapExtraErrorInfo;
        static CTL_GeneralCapInfo       s_mapGeneralCapInfo;
        static LONG                     s_nRegisteredDTWAINMsg;
        static std::mutex               s_mutexInitDestroy;
        static CTL_MapThreadToDLLHandle s_mapThreadToDLLHandle;
        static CTL_ThreadMap            s_ThreadMap;
        static bool                     s_bThrowExceptions;
        static HINSTANCE                s_DLLInstance;
        static std::unordered_set<HWND> s_appWindowsToDisable;
        static CTL_CallbackProcArray    s_aAllCallbacks;
        static CTL_StringType           s_strLangResourcePath;
        static CTL_GeneralErrorInfo     s_mapGeneralErrorInfo;
        static long                     s_lErrorFilterFlags;
        static CLogSystem               s_appLog;
        static UINT_PTR                 s_nTimeoutID;
        static UINT                     s_nTimeoutMilliseconds;
        static bool                     s_bTimerIDSet;
        static bool                     s_ResourcesInitialized;
        static ImageResamplerMap        s_ImageResamplerMap;
        static SourceStatusMap          s_SourceStatusMap;
        static CTL_StringType           s_ResourceVersion;
        static std::string              s_CurrentResourceKey;
        static CTL_PairToStringMap      s_ResourceCache;

        static CTL_PairToStringMap& GetResourceCache() { return s_ResourceCache; }
        static CTL_StringToMapLongToStringMap& GetAllLanguagesResourceMap() { return s_AllLoadedResourcesMap; }
        static CTL_LongToStringMap* GetLanguageResource(std::string sLang);
        static std::string&         GetCurrentLanguageResourceKey() { return s_CurrentResourceKey; }
        static void SetCurrentLanguageResourceKey(std::string sLang) { s_CurrentResourceKey = sLang; }
        static CTL_LongToStringMap* GetCurrentLanguageResource();
        static CTL_GeneralResourceInfo& GetGeneralResourceInfo() { return s_ResourceInfo; }
        static CTL_PDFMediaMap& GetPDFMediaMap() { return s_PDFMediaMap; }
        static CTL_TwainLongToStringMap& GetTwainCountryMap() { return s_TwainCountryMap; }
        static CTL_TwainNameMap& GetTwainNameMap() { return s_TwainNameMap; }
        static std::string GetTwainNameFromResource(int nWhichResourceID, int nWhichItem);
        static CTL_AvailableFileFormatsMap& GetAvailableFileFormatsMap() { return s_AvailableFileFormatsMap; }
        static CTL_TwainConstantsMap& GetTwainConstantsMap() { return s_TwainConstantsMap; }
        static CTL_TwainConstantToStringMapNode& GetTwainConstantsStrings(LONG nWhich) { return s_TwainConstantsMap[nWhich]; }
        static CTL_TwainLongToStringMap& GetTwainLanguageMap() { return s_TwainLanguageMap; }
        static bool IsCheckHandles() { return s_bCheckHandles; }
        static int GetIDFromTwainName(std::string sName);
        static int GetDGResourceID() { return 8890; }
        static int GetDATResourceID() { return 8891; }
        static int GetMSGResourceID() { return 8892; }
        static CTL_StringType& GetResourcePath() { return s_strResourcePath; }
        static CTL_StringType& GetDLLPath() { return s_DLLPath; }
        static CTL_StringType& GetINIPath() { return s_sINIPath; }
        static bool IsUsingMultipleThreads() { return s_multipleThreads; }
        static CTL_LongToStringMap& GetErrorCodes() { return s_ErrorCodes; }
        static CTL_GeneralCapInfo& GetGeneralCapInfo() { return s_mapGeneralCapInfo; }
        static HINSTANCE GetDLLInstanceHandle() { return s_DLLInstance; }
        static CTL_GeneralErrorInfo& GetGeneralErrorInfo() { return s_mapGeneralErrorInfo; }
        static long GetErrorFilterFlags() { return s_lErrorFilterFlags; }
        static ImageResamplerMap& GetImageResamplerMap() { return s_ImageResamplerMap; }
        static SourceStatusMap& GetSourceStatusMap() { return s_SourceStatusMap;  }
        static CTL_StringType& GetResourceVersion() { return s_ResourceVersion; }
        static CTL_StringType GetTwainNameFromConstant(int lConstantType, int lTwainConstant);
    };

    struct CTL_LoggerCallbackInfo
    {
        DTWAIN_LOGGER_PROC  m_pLoggerCallback = nullptr;
        DTWAIN_LOGGER_PROCA  m_pLoggerCallbackA = nullptr;
        DTWAIN_LOGGER_PROCW  m_pLoggerCallbackW = nullptr;
        DTWAIN_LONG64  m_pLoggerCallback_UserData = 0;
        DTWAIN_LONG64  m_pLoggerCallback_UserDataA = 0;
        DTWAIN_LONG64  m_pLoggerCallback_UserDataW = 0;
    };

    struct CTL_OnSourceOpenProperties
    {
        bool m_bCheckFeederStatusOnOpen;
        bool m_bQueryBestCapContainer;
        bool m_bQueryCapOperations;
    };

    class CTL_TwainDLLHandle
    {
        public:
            static constexpr int NumTwainMapValues = DTWAIN_CONSTANT_LAST;

            CTL_TwainDLLHandle();
            ~CTL_TwainDLLHandle() = default;
            static void    NotifyWindows( UINT nMsg, WPARAM wParam, LPARAM lParam );
            void    RemoveAllEnumerators();
            void    RemoveAllSourceCapInfo();
            void    RemoveAllSourceMaps();
            void    InitializeResourceRegistry();
            std::pair<CTL_ResourceRegistryMap::iterator, bool> AddResourceToRegistry(LPCSTR pLangDLL, bool bClear);
            CTL_ResourceRegistryMap& GetResourceRegistry() { return m_ResourceRegistry; }
            CTL_StringType GetVersionString() const { return  m_VersionString; }
            void        SetVersionString(CTL_StringType s) { m_VersionString = std::move(s); }

            DTWAIN_ACQUIRE          GetNewAcquireNum();
            void                    EraseAcquireNum(DTWAIN_ACQUIRE nNum);

            CTL_TwainAppMgr* m_pAppMgr;

            struct tagSessionStruct
            {
                TW_UINT16 nMajorNum;
                TW_UINT16 nMinorNum;
                TW_UINT16 nLanguage;
                TW_UINT16 nCountry;
                CTL_StringType szVersion;
                CTL_StringType szManufact;
                CTL_StringType szFamily;
                CTL_StringType szProduct;
                CTL_StringType DSMName;
                int nSessionType;
                tagSessionStruct() : nMajorNum(1),
                                     nMinorNum(0),
                                     nLanguage(TwainLanguage_USAENGLISH),
                                     nCountry(TwainCountry_USA),
                                     szVersion(_T("<?>")),
                                     szManufact(_T("<?>")),
                                     szFamily(_T("<?>")),
                                     szProduct(_T("<?>")),
                                     #ifdef _WIN64
                                     DSMName(TWAINDLLVERSION_2),
                                     nSessionType(DTWAIN_TWAINDSM_VERSION2)
                                     #else
                                     DSMName(TWAINDLLVERSION_1),
                                     nSessionType(DTWAIN_TWAINDSM_LEGACY)
                                     #endif
                                    {}
            };

            tagSessionStruct m_SessionStruct;
            CTL_ResourceRegistryMap m_ResourceRegistry;
            CTL_ITwainSession* m_pTwainSession;
            CTL_StringType   m_VersionString;
            CTL_StringType   m_strTWAINPath;     // path to the TWAIN Data Source Manager that is being used
            CTL_StringType   m_strTWAINPath2;   // path to the TWAIN Data Source Manager 2.x that is being used
            CTL_StringType   m_strLibraryPath;   // path to the DTWAIN Library being used
            CTL_StringType   m_sWindowsVersionInfo; // Windows version information, cached.
            CTL_StringType   m_strDefaultSource; // Current default TWAIN source
            CTL_LoggerCallbackInfo m_LoggerCallbackInfo;

            HINSTANCE           m_hInstance;
            HWND                m_hWndTwain;
            HWND                m_hNotifyWnd;
            #ifdef _WIN32
            WNDPROC             m_hOrigProc;
            HWND                m_hWndDummy;
            #endif
            int                 m_nCurrentDibPos;
            bool                m_bSessionAllocated;
            bool                m_bDummyWindowCreated;
            bool                m_bTransferDone;
            bool                m_bSourceClosed;    // Used for "WAIT" mode
            DTWAIN_CALLBACK     m_CallbackMsg;
            DTWAIN_CALLBACK     m_CallbackError;
            LONG                m_lLastError;
            LONG                m_lLastAcqError;
            LONG                m_lAcquireMode;
            bool                m_nSourceCloseMode;
            int                 m_nUIMode;
            bool                m_bNotificationsUsed;
            bool                m_bNotifyTripletsUsed;
            std::deque<int>     m_vErrorBuffer;
            unsigned int        m_nErrorBufferThreshold = 50;
            unsigned int        m_nErrorBufferReserve = 1000;
            DTWAIN_CALLBACK_PROC m_pCallbackFn;
            DTWAIN_CALLBACK_PROC64 m_pCallbackFn64;
            DTWAIN_ERROR_PROC   m_pErrorProcFn;
            DTWAIN_ERROR_PROC64 m_pErrorProcFn64;
            LONG                m_lErrorProcUserData;
            LONG64              m_lErrorProcUserData64;
            LONG                m_lCallbackData;
            LONGLONG            m_lCallbackData64;
            OCRInterfaceContainer m_OCRInterfaceArray;
            OCRProductNameToEngineMap m_OCRProdNameToEngine;
            OCREnginePtr          m_pOCRDefaultEngine;
            CTL_OnSourceOpenProperties  m_OnSourceOpenProperties = {};

            // File Save As information
            #ifdef _WIN32
            std::unique_ptr<OPENFILENAME>  m_pofn;
            LONG                m_nSaveAsFlags;
            POINT               m_SaveAsPos;
            LPOFNHOOKPROC       m_pSaveAsDlgProc;
            CustomPlacement     m_CustomPlacement;
            #endif
            CTL_TEXTELEMENTMAP m_mapPDFTextElement;
            bool                m_bUseProxy;
            CTL_SourceCapInfoArray   m_aSourceCapInfo;
            CTL_StringToSourcePtrMap       m_mapStringToSource;
            std::vector<LONG_PTR>           m_aAcquireNum;
            short int                       m_nDSMState = DSM_STATE_NONE;
            int                             m_nDSMVersion = DTWAIN_TWAINDSM_LEGACY;
            bool                            m_bOpenSourceOnSelect;
            CTL_StringType                  m_sTempFilePath;
            CTL_ArrayFactoryPtr             m_ArrayFactory;
            CTL_LongToVectorLongMap         m_mapDTWAINArrayToTwainType;
            CTL_TwainMemoryFunctions* m_TwainMemoryFunc = nullptr;
            CTL_LegacyTwainMemoryFunctions  m_TwainLegacyFunc;
            CTL_Twain2MemoryFunctions       m_Twain2Func;
            DTWAIN_DIBUPDATE_PROC           m_pDibUpdateProc = nullptr;
            std::unordered_set<DTWAIN_SOURCE> m_aFeederSources;
            int                             m_TwainDSMSearchOrder = DTWAIN_TWAINDSMSEARCH_WSO;
            std::string                     m_TwainDSMSearchOrderStr = "CWSOU";
            CTL_StringType                  m_TwainDSMUserDirectory;
            CTL_StringType                  m_strSessionDetails;
            CTL_StringType                  m_strSourceDetails;
    };

    template <typename T>
    T IsDLLHandleValid(CTL_TwainDLLHandle *pHandle, T bCheckSession = T(1))
    {
        // See if DLL Handle exists
        if (!pHandle)
            return {};
        // Check handles registered to the thread id's
        if (std::find_if(CTL_StaticData::s_mapThreadToDLLHandle.begin(),
            CTL_StaticData::s_mapThreadToDLLHandle.end(), [&](auto& pr) { return pr.second.get() == pHandle; }) ==
            CTL_StaticData::s_mapThreadToDLLHandle.end())
            return {};
        if (!pHandle->m_bSessionAllocated && bCheckSession)
            return {};
        return {1};
    }

    template <int CapInfoIdx>
    void SetCapabilityInfo(CTL_TwainDLLHandle* pHandle, DTWAIN_SOURCE Source, LONG value, LONG lCap)
    {
        CTL_ITwainSource* pSource = reinterpret_cast<CTL_ITwainSource*>(Source);
        const CTL_CapInfoArrayPtr pArray = GetCapInfoArray(pHandle, pSource);

        // Get the cap array values
        const auto iter = pArray->find(static_cast<TW_UINT16>(lCap));
        if (iter != pArray->end())
        {
            CTL_CapInfo* CapInfo = &iter->second;

            // Replace the cap information with the updated information
            std::get<CapInfoIdx>(*CapInfo) = value;
        }
    }

    #define DTWAIN_VERIFY_DLLHANDLE  1
    #define DTWAIN_VERIFY_SOURCEHANDLE  2
    #define DTWAIN_TEST_NOTHROW 4
    #define DTWAIN_TEST_SETLASTERROR 8

    CTL_TwainDLLHandle* FindHandle(HWND hWnd, bool bIsDisplay);
    CTL_TwainDLLHandle* FindHandle(HINSTANCE hInst);
    std::pair<CTL_TwainDLLHandle*, CTL_ITwainSource*> VerifyHandles(DTWAIN_SOURCE Source, int Testing = DTWAIN_VERIFY_DLLHANDLE | DTWAIN_VERIFY_SOURCEHANDLE | DTWAIN_TEST_SETLASTERROR);
    int GetResolutions(DTWAIN_HANDLE DLLHandle, DTWAIN_SOURCE Source, void* pArray,CTL_EnumGetType GetType);
    bool GetImageSize( DTWAIN_HANDLE DLLHandle,
                       DTWAIN_SOURCE Source,
                       double *pLeft,
                       double *pRight,
                       double *pTop,
                       double *pBottom,
                       CTL_EnumGetType GetType);

    bool SetImageSize( DTWAIN_HANDLE DLLHandle,
                       DTWAIN_SOURCE Source,
                       double dLeft,
                       double dRight,
                       double dTop,
                       double dBottom,
                       CTL_EnumSetType SetType,
                       std::vector<double>& rArray);

    bool GetNativeResolution(DTWAIN_HANDLE DLLHandle,
                             DTWAIN_SOURCE Source,
                             double *pRes,
                             CTL_EnumCapability Cap);

    int SetResolutions(DTWAIN_HANDLE DLLHandle, DTWAIN_SOURCE Source, void** pResolutions,
                        int nRes, void (*ResProc)(const CTL_ITwainSource *pSource,
                                                  std::vector<double>& pArray ));
    bool CenterWindow(HWND hwnd, HWND hwndParent);

    bool IsIntCapType(TW_UINT16 nCap);
    bool IsFloatCapType(TW_UINT16 nCap);
    bool IsStringCapType(TW_UINT16 nCap);
    bool IsFrameCapType(TW_UINT16 nCap);
    LONG GetArrayTypeFromCapType(TW_UINT16 CapType);
    LONG GetCustomCapDataType(DTWAIN_SOURCE Source, TW_UINT16 nCap);
    LONG GetCapContainer(CTL_ITwainSource* pSource, LONG nCap, LONG lCapType);
    LONG GetCapArrayType(CTL_TwainDLLHandle* pHandle, CTL_ITwainSource* pSource, LONG nCap);

    DTWAIN_BOOL    DTWAIN_ArrayFirst(DTWAIN_ARRAY pArray, LPVOID pVariant);

    DTWAIN_BOOL    DTWAIN_ArrayNext(DTWAIN_ARRAY pArray, LPVOID pVariant);
    LONG           DTWAIN_ArrayType(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY pArray);
    bool           DTWAINFRAMEToTWFRAME(DTWAIN_FRAME pDdtwil, pTW_FRAME pTwain);
    bool           TWFRAMEToDTWAINFRAME(TW_FRAME pTwain, DTWAIN_FRAME pDdtwil);

    #ifdef __cplusplus
    extern "C" {
    #endif
    #ifdef _WIN32
    LRESULT CALLBACK_DEF DTWAIN_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    DTWAIN_BOOL DTWAIN_SetCallbackProc( DTWAIN_CALLBACK fnCall, LONG nWhich);
    #endif
    void DTWAIN_AcquireProc(DTWAIN_HANDLE DLLHandle, DTWAIN_SOURCE Source, WPARAM Data1, LPARAM Data2);
    #ifdef __cplusplus
    }
    #endif

    void DTWAIN_InvokeCallback( int nWhich, DTWAIN_HANDLE pHandle, DTWAIN_SOURCE pSource, WPARAM lData1, LPARAM lData2 );
    DTWAIN_BOOL DTWAIN_GetAllSourceDibs(DTWAIN_SOURCE Source, DTWAIN_ARRAY pArray);

    void OutputDTWAINError(CTL_TwainDLLHandle *pHandle, LPCSTR pFunc=nullptr);
    void OutputDTWAINErrorA(CTL_TwainDLLHandle *pHandle, LPCSTR pFunc=nullptr);
    void OutputDTWAINErrorW(CTL_TwainDLLHandle *pHandle, LPCWSTR pFunc=nullptr);

    void LogExceptionErrorA(LPCSTR fname, const char *sAdditionalText=nullptr);
    void LogDTWAINMessage(HWND, UINT, WPARAM, LPARAM, bool bCallback=false);
    bool UserDefinedLoggerExists(CTL_TwainDLLHandle* pHandle);
    bool AnyLoggerExists(CTL_TwainDLLHandle* pHandle);
    void WriteUserDefinedLogMsg(CTL_TwainDLLHandle* pHandle, LPCTSTR sz);
    void WriteUserDefinedLogMsgA(CTL_TwainDLLHandle* pHandle, LPCSTR sz);
    void WriteUserDefinedLogMsgW(CTL_TwainDLLHandle* pHandle, LPCWSTR sz);
    bool GetSupportString(DTWAIN_SOURCE Source, LPTSTR sz, LONG nLen, LONG Cap, LONG GetType);
    bool EnumSupported(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY pArray, LONG Cap);
    LONG CheckEnabled(DTWAIN_SOURCE Source, LONG CapVal);
    bool SetSupportArray(DTWAIN_SOURCE Source, DTWAIN_ARRAY Array, LONG Cap);
    bool GetSupportArray(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY Array, LONG Cap, LONG GetType=DTWAIN_CAPGET);
    CTL_StringType GetDTWAINTempFilePath(CTL_TwainDLLHandle* pHandle);
    size_t GetResourceStringA(UINT nResNumber, LPSTR buffer, LONG bufSize);
    size_t GetResourceStringW(UINT nResNumber, LPWSTR buffer, LONG bufSize);
    size_t GetResourceString(UINT nResNumber, LPTSTR buffer, LONG bufSize);
    bool LoadLanguageResourceXML(LPCTSTR sLangDLL);
    bool LoadLanguageResourceXMLImpl(LPCTSTR szFile);
    void DumpArrayContents(DTWAIN_ARRAY Array, LONG lCap);
    void LogWin32Error(DWORD lastError);
    void LoadOCRInterfaces(CTL_TwainDLLHandle *pHandle);
    void UnloadOCRInterfaces(CTL_TwainDLLHandle *pHandle);
    CTL_StringType GetVersionString();
    CTL_StringType GetDTWAINExecutionPath();
    CTL_StringType GetDTWAINDLLPath();
    void LogToDebugMonitorA(std::string sMsg);
    void LogToDebugMonitorW(std::wstring sMsg);
    void LogToDebugMonitor(CTL_StringType sMsg);
    typedef DTWAIN_BOOL (DLLENTRY_DEF *SetByStringFn)(DTWAIN_SOURCE, DTWAIN_FLOAT);
    typedef DTWAIN_BOOL (DLLENTRY_DEF *SetByStringFn2)(DTWAIN_SOURCE, DTWAIN_FLOAT, DTWAIN_BOOL);
    typedef DTWAIN_BOOL(DLLENTRY_DEF *GetByStringFn)(DTWAIN_SOURCE, LPDTWAIN_FLOAT);
    typedef DTWAIN_BOOL (*CapSetterByStringFn)(DTWAIN_SOURCE, LPCTSTR, SetByStringFn);

    DTWAIN_BOOL DTWAIN_SetDeviceCapByString(DTWAIN_SOURCE Source, LPCTSTR strVal, SetByStringFn fn);
    DTWAIN_BOOL DTWAIN_GetDeviceCapByString(DTWAIN_SOURCE Source, LPTSTR strVal, GetByStringFn fn);
    DTWAIN_BOOL DTWAIN_SetDeviceCapByString2(DTWAIN_SOURCE Source, LPCTSTR strVal, bool bExtra, SetByStringFn2 fn);

    DTWAIN_BOOL DTWAIN_CacheCapabilityInfo(CTL_ITwainSource *p, CTL_TwainDLLHandle *pHandle, TW_UINT16 nCapToCache);
    DTWAIN_BOOL DTWAIN_CacheCapabilityInfo(CTL_ITwainSource *pSource, CTL_TwainDLLHandle *pHandle, CTL_EnumeratorNode<LONG>::container_pointer_type vCaps);
    CTL_CapInfoArrayPtr GetCapInfoArray(CTL_TwainDLLHandle* pHandle, const CTL_ITwainSource *p);
    DTWAIN_SOURCE SourceSelect(CTL_TwainDLLHandle* pHandle, const SourceSelectionOptions& options);
    DTWAIN_ARRAY  SourceAcquire(SourceAcquireOptions& opts);
    bool AcquireExHelper(SourceAcquireOptions& opts);
    bool AcquireFileHelper(SourceAcquireOptions& opts, LONG AcquireType);
    DTWAIN_ARRAY SourceAcquireWorkerThread(SourceAcquireOptions& opts);
    DTWAIN_ACQUIRE  LLAcquireImage(SourceAcquireOptions& opts);
    void LLSetupUIOnly(CTL_ITwainSource* pSource);
    DTWAIN_HANDLE GetDTWAINHandle_Internal();
    bool TileModeOn(DTWAIN_SOURCE Source);
    void DestroyArrayFromFactory(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY pArray);
    void DestroyFrameFromFactory(CTL_TwainDLLHandle* pHandle, DTWAIN_FRAME Frame);
    DTWAIN_ARRAY CreateArrayFromFactory(CTL_TwainDLLHandle* pHandle, LONG nEnumType, LONG nInitialSize);
    DTWAIN_ARRAY CreateArrayCopyFromFactory(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY Source);
    DTWAIN_FRAME CreateFrameArray(CTL_TwainDLLHandle* pHandle, double Left, double Top, double Right, double Bottom);
    void SetArrayValueFromFactory(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY pArray, size_t lPos, LPVOID pVariant);
    DTWAIN_ARRAY CreateArrayFromCap(CTL_TwainDLLHandle* pHandle, CTL_ITwainSource* pSource, LONG lCapType, LONG lSize);
    LONG GetCapDataType(CTL_ITwainSource* pSource, LONG nCap);

    typedef CTL_StringType(CTL_ITwainSource::* SOURCEINFOFUNC)() const;
    LONG GetSourceInfo(CTL_ITwainSource* p, SOURCEINFOFUNC pFunc, LPTSTR szInfo, LONG nMaxLen);

    //#ifdef DTWAIN_DEBUG_CALL_STACK
    std::string CTL_LogFunctionCallHelper(LPCSTR pFuncName, int nWhich, LPCSTR pOptionalString=nullptr);
    std::string CTL_LogFunctionCallA(LPCSTR pFuncName, int nWhich, LPCSTR pOptionalString=nullptr);
    //#endif

    // outputs parameter and return values
    class ParamOutputter
    {
        StringArray aParamNames;
        size_t nWhich;
        std::string argNames;
        std::ostringstream strm;
        bool m_bIsReturnValue;

    private:
        void LogInputType(std::string outStr, const char *ptr)
        {
            // ptr is a valid string supplied by the user, so just write it out
            if ( ptr )
                strm << outStr << "=" << ptr;
            else
                strm << outStr << "=" << "(null)";
        }

        void LogInputType(std::string outStr, const wchar_t *ptr)
        {
            if (ptr)
                strm << outStr << "=" << StringConversion::Convert_WidePtr_To_Ansi(ptr);
            else
                strm << outStr << "=" << "(null)";
        }

        void LogInputType(std::string outStr, char *ptr)
        {
            // ptr is a valid string supplied by the user, but we can't ensure it is null terminated
            // (It doesn't have to be null-terminated, as the DTWAIN function will eventually put the NULL
            //  terminated value into the output string).
            // So for now, we just output the pointer value of the string
            strm << outStr << "=" << static_cast<void*>(ptr);
        }

        void LogInputType(std::string outStr, wchar_t *ptr)
        {
            // ptr is a valid string supplied by the user, but we can't ensure it is null terminated
            // (It doesn't have to be null-terminated, as the DTWAIN function will eventually put the NULL
            //  terminated value into the output string).
            // So for now, we just output the pointer value of the string
            strm << outStr << "=" << static_cast<void*>(ptr);
        }

        template <typename T>
        void LogInputType(std::string outStr, T t, std::enable_if_t<std::is_pointer_v<T> >* = nullptr)
        {
            if (t)
                strm << outStr << "=" << t;
            else
                strm << outStr << "=" << "(null)";
        }

        template <typename T>
        void LogInputType(std::string outStr, T t, std::enable_if_t<!std::is_pointer_v<T> >* = nullptr)
        {
            strm << outStr << "=" << t;
        }

    public:
        ParamOutputter(const std::string& s, bool isReturnValue = false) : nWhich(0), m_bIsReturnValue(isReturnValue)
        {
            StringWrapperA::Tokenize(s, "(, )", aParamNames);
            if (!m_bIsReturnValue)
                strm << "(";
            else
                strm << s << " " << dynarithmic::GetResourceStringFromMap(IDS_LOGMSG_RETURNTEXT) << " ";
        }

        template <typename T, typename ...P>
        ParamOutputter& outputParam(T t, P ...p)
        {
            if (aParamNames.empty() && !m_bIsReturnValue)
                return *this;
            const bool bIsNull = std::is_pointer_v<T> && !t;
            if (!m_bIsReturnValue)
            {
                // Make sure we log input types correctly, especially character pointers.
                // User may supply to us a writable char buffer that is not null-terminated!
                LogInputType(aParamNames[nWhich], t);
            }
            else
            {
                if ( bIsNull )
                    strm << "(null)";
                else
                    strm << t;
            }
            if (!m_bIsReturnValue)
            {
              if (nWhich < aParamNames.size() - 1)
                strm << ", ";
              else
                strm << ")";
            }
            ++nWhich;
            if (sizeof...(p))
                outputParam(p...);
            return *this;
        }

        ParamOutputter& outputParam()
        {
            strm << ")"; return *this;
        }

        std::string getString() const { return strm.str(); }
    };

/*    struct DTWAINArrayPtr_DestroyTraits
    {
        static void Destroy(DTWAIN_ARRAY* a)
        {
            if (a && *a)
               DestroyArrayFromFactory(*a);
        }
        void operator()(DTWAIN_ARRAY* a) { Destroy(a); }
    };
    */
    struct DTWAINGlobalHandle_CloseTraits
    {
        static void Destroy(HANDLE h)
        {
            #ifdef _WIN32
            if ( h )
                ImageMemoryHandler::GlobalUnlock(h);
            #endif
        }
        void operator()(HANDLE h) { Destroy(h); }
    };

    struct DTWAINGlobalHandle_ClosePtrTraits
    {
        static void Destroy(HANDLE* h)
        {
            #ifdef _WIN32
            if (h && *h)
                ImageMemoryHandler::GlobalUnlock(*h);
            #endif
        }
        void operator()(HANDLE* h) { Destroy(h); }
    };

    struct DTWAINGlobalHandle_CloseFreeTraits
    {
        static void Destroy(HANDLE h)
        {
            #ifdef _WIN32
            if (h)
            {
                ImageMemoryHandler::GlobalUnlock(h);
                ImageMemoryHandler::GlobalFree(h);
            }
            #endif
        }
        void operator()(HANDLE h) { Destroy(h); }
    };

    using DSMPair = std::pair<CTL_TwainDLLHandle*, HANDLE>;

    struct DSM2UnlockTraits
    {
        static void Unlock(DSMPair* pr)
        {
            pr->first->m_TwainMemoryFunc->UnlockMemory(pr->second);
        }
    };

    struct DSM2NoFreeTraits
    {
        static void Free(DSMPair /*h*/)
        {
        }
    };

    struct DSM2FreeTraits
    {
        static void Free(DSMPair* pr)
        {
            pr->first->m_TwainMemoryFunc->FreeMemory(pr->second);
        }
    };

    template <typename T, typename UnLockFn, typename FreeFn>
    struct DTWAINGlobalHandle_GenericUnlockFreeTraits
    {
        static void Destroy(T* h)
        {
            UnLockFn::Unlock(h);
            FreeFn::Free(h);
        }
        void operator()(T* h) { Destroy(h); }
    };

    struct DTWAINGlobalHandle_ReleaseDCTraits
    {
        static void Destroy(std::pair<HWND, HDC>& val)
        {
            #ifdef _WIN32
            if (val.second)
                ReleaseDC(val.first, val.second);
            #endif
        }
        void operator()(std::pair<HWND, HDC>* val) { Destroy(*val); }
    };

    struct DTWAINFileHandle_CloseTraits
    {
        static void Destroy(HANDLE h)
        {
            #ifdef _WIN32
            if (h)
                CloseHandle(h);
            #endif
        }
        void operator()(HANDLE h) { Destroy(h); }
    };

    struct DTWAINResource_UnlockFreeTraits
    {
        static void Destroy(HGLOBAL h)
        {
            #ifdef _WIN32
            if (h)
                FreeResource(h);
            #endif
        }
        void operator()(HGLOBAL h) { Destroy(h); }
    };

    struct DTWAINResource_DeleteObjectTraits
    {
        static void Destroy(HBITMAP* h)
        {
#ifdef _WIN32
            if (h && *h)
                DeleteObject(*h);
#endif
        }
        void operator()(HBITMAP* h) { Destroy(h); }
    };

    template <typename ArrayType>
    struct DTWAINArrayLowLevel_RAII_Impl
    {
        CTL_TwainDLLHandle* m_pHandle;
        ArrayType m_Array;
        bool m_bDestroy;
        DTWAINArrayLowLevel_RAII_Impl(CTL_TwainDLLHandle* pHandle, ArrayType a) : m_pHandle(pHandle), m_Array(a), m_bDestroy(true) {}
        void SetDestroy(bool bSet) { m_bDestroy = bSet; }
        void SetArray(ArrayType arr) { m_Array = arr; }
        void Destroy()
        {
            if (m_bDestroy && m_Array)
            {
                m_pHandle->m_ArrayFactory->destroy(CTL_ArrayFactory::from_void(m_Array));
                m_Array = {};
            }
        }
        ~DTWAINArrayLowLevel_RAII_Impl()
        {
            Destroy();
        }
    };

    using DTWAINArrayLowLevel_RAII = DTWAINArrayLowLevel_RAII_Impl<DTWAIN_ARRAY>;
    using DTWAINArrayLowLevelPtr_RAII = DTWAINArrayLowLevel_RAII_Impl<DTWAIN_ARRAY*>;

    // RAII Classes
    using DTWAINDeviceContextRelease_RAII = std::unique_ptr<std::pair<HWND, HDC>, DTWAINGlobalHandle_ReleaseDCTraits>;
    using DTWAINGlobalHandlePtr_RAII = std::unique_ptr<HANDLE, DTWAINGlobalHandle_ClosePtrTraits>;
    using DTWAINFileHandle_RAII = std::unique_ptr<void, DTWAINFileHandle_CloseTraits>;
    using DTWAINResourceUnlockFree_RAII = std::unique_ptr<void, DTWAINResource_UnlockFreeTraits>;
    using DTWAINHBITMAPFree_RAII = std::unique_ptr<HBITMAP, DTWAINResource_DeleteObjectTraits>;
    using DTWAINGlobalHandle_RAII = std::unique_ptr<void, DTWAINGlobalHandle_CloseTraits>;
    using DTWAINArrayPtr_RAII = DTWAINArrayLowLevelPtr_RAII;// std::unique_ptr<DTWAIN_ARRAY, DTWAINArrayPtr_DestroyTraits>;
    using DTWAINGlobalHandleUnlockFree_RAII = std::unique_ptr<void, DTWAINGlobalHandle_CloseFreeTraits>;
    using DTWAINDSM2Lock_RAII = std::unique_ptr<void, 
            DTWAINGlobalHandle_GenericUnlockFreeTraits<HANDLE, DSM2UnlockTraits, DSM2NoFreeTraits>>;
    using DTWAINDSM2LockAndFree_RAII = std::unique_ptr<DSMPair,
        DTWAINGlobalHandle_GenericUnlockFreeTraits<DSMPair, DSM2UnlockTraits, DSM2FreeTraits>>;

    // RAII Class for turning on/off logging locally
    struct DTWAINScopedLogController
    {
        long m_ErrorFilterFlags;
        DTWAINScopedLogController(long newFilter) : m_ErrorFilterFlags(CTL_StaticData::s_lErrorFilterFlags)
        { CTL_StaticData::s_lErrorFilterFlags = newFilter; }
        ~DTWAINScopedLogController() { CTL_StaticData::s_lErrorFilterFlags = m_ErrorFilterFlags; }
        DTWAINScopedLogController(DTWAINScopedLogController&) = delete;
        DTWAINScopedLogController& operator=(DTWAINScopedLogController&) = delete;
    };

    struct HandleRAII
    {
        LPBYTE m_pByte;
        DTWAINGlobalHandle_RAII m_raii;
        HandleRAII(HANDLE h) : m_raii(h), m_pByte(static_cast<LPBYTE>(GlobalLock(h))) {}
        LPBYTE getData() const { return m_pByte; }
        HandleRAII(HandleRAII&) = delete;
        HandleRAII& operator=(HandleRAII&) = delete;
    };

    struct LogTraitsOff
    { static long Apply(long turnOff) { return CTL_StaticData::s_lErrorFilterFlags &~turnOff; } };

    struct LogTraitsOn
    { static long Apply(long turnOn) { return CTL_StaticData::s_lErrorFilterFlags  | turnOn; } };

    template <typename LogTraits>
    struct DTWAINScopedLogControllerEx
    {
        DTWAINScopedLogController m_controller;
        DTWAINScopedLogControllerEx(long newValue) : m_controller(LogTraits::Apply(newValue)) {}
        DTWAINScopedLogControllerEx(DTWAINScopedLogControllerEx&) = delete;
        DTWAINScopedLogControllerEx& operator=(DTWAINScopedLogControllerEx&) = delete;
    };

    typedef DTWAINScopedLogControllerEx<LogTraitsOff> DTWAINScopedLogControllerExclude;
    typedef DTWAINScopedLogControllerEx<LogTraitsOn>  DTWAINScopedLogControllerInclude;

    #define THIS_FUNCTION_PROTO_THROWS  ;
    #define THIS_FUNCTION_THROWS

    void  DTWAIN_InternalThrowException() THIS_FUNCTION_PROTO_THROWS

    LONG  TS_Command(LPCTSTR lpCommand);

    #define IDS_DTWAIN_APPTITLE       9700

    #define IDS_LIMITEDFUNCMSG1     8894
    #define IDS_LIMITEDFUNCMSG2     8895
    #define IDS_LIMITEDFUNCMSG3     8896


    #define CHECK_FOR_PDF_TYPE() \
        (lFileType == DTWAIN_PDF) || \
        (lFileType == DTWAIN_PDFMULTI) || \
        (lFileType == DTWAIN_POSTSCRIPT1) || \
        (lFileType == DTWAIN_POSTSCRIPT2) || \
        (lFileType == DTWAIN_POSTSCRIPT3) || \
        (lFileType == DTWAIN_POSTSCRIPT1MULTI) || \
        (lFileType == DTWAIN_POSTSCRIPT2MULTI) || \
        (lFileType == DTWAIN_POSTSCRIPT3MULTI)

    #define INVALID_LICENSE (0)
}
#endif
