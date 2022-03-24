/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2022 Dynarithmic Software.

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
#ifndef CTLIFACE_H_
#define CTLIFACE_H_

#ifdef _MSC_VER
#pragma warning( disable : 4786)
#pragma warning (disable : 4786)
#pragma warning (disable : 4127)
#endif
#include <algorithm>
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
#include <boost/functional/hash.hpp>
#include "../tsl/ordered_map.h"
#include "ctltrp.h"
#include "dtwain_raii.h"
#include "ocrinterface.h"
#include "pdffont_basic.h"
#include "ctlres.h"
#include "dtwain.h"

#ifdef _WIN32
#include "winlibraryloader_impl.inl"
#else
#include "linuxlibraryloader_impl.inl"
#endif
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
        /* Select source wParam's */
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

    typedef CTL_ClassValues7<CTL_EnumCapability,/* Capability*/
                             UINT             , /* Container for Get*/
                             UINT             , /* Container for Set*/
                             UINT             ,  /* Data Type */
                             UINT             ,  /* Available cap support */
                             UINT             ,  /* Container for Get Current */
                             UINT                /* Container for Get Default  */
                             > CTL_CapInfo;

    typedef CTL_ClassValues7<DWORD, HHOOK, HHOOK, CTL_TwainDLLHandle*, bool,char,char>  CTL_HookInfo;

    typedef std::unordered_map<LONG, int> CTL_LongToIntMap;
//    typedef std::unordered_map<std::pair<int, int>, std::string, boost::hash<std::pair<int, int>>> CTL_TwainNameMap;
    typedef BiDirectionalMap<std::pair<int, int>, std::string> CTL_TwainNameMap;
    typedef std::unordered_map<CTL_StringType, CTL_ITwainSource*> CTL_StringToSourcePtrMap;
    typedef std::unordered_map<CTL_StringType, int> CTL_StringToIntMap;
    typedef std::unordered_map<LONG, HMODULE> CTL_LongToHMODULEMap;
    typedef std::unordered_map<CTL_EnumCapability, CTL_CapInfo> CTL_EnumCapToInfoMap;
    typedef std::vector<CallbackInfo<DTWAIN_CALLBACK_PROC, LONG> > CTL_CallbackProcArray;
    typedef std::vector<CallbackInfo<DTWAIN_CALLBACK_PROC, LONGLONG> > CTL_CallbackProcArray64;
    typedef std::unordered_map<LONG, CTL_StringType> CTL_StringToLongMap;
    typedef std::unordered_map<LONG, std::string> CTL_LongToStringMap;
    typedef std::unordered_map<LONG, std::vector<LONG> > CTL_LongToVectorLongMap;
    typedef std::vector<CTL_HookInfo>     CTL_HookInfoArray;
    typedef std::unordered_map<LONG, std::string> CTL_TwainLongToStringMap;

    // Create these dynamically whenever a new source is opened
    // and source cap info does not exist.  Add cap info statically.
    typedef std::unordered_map<CTL_EnumCapability, CTL_CapInfo>  CTL_CapInfoArray;
    typedef std::shared_ptr<CTL_CapInfoArray> CTL_CapInfoArrayPtr;

    // Create this statically when initializing.  Initialize the second
    // value with the dynamically created CTL_CapInfoArray above
    typedef CTL_ClassValues7<CTL_StringType, /* Product Name */
                             CTL_CapInfoArrayPtr, /* Array of cap info*/
                             int,       /* dummy */
                             int,        /* dummy */
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
    #define DTWAINFrameInternalGUID _T("80301C36-4E51-48C3-B2C9-B04E28D5C5FD")
    struct DTWAINFrameInternal
    {
        std::array<double, 4> m_FrameComponent;
        std::array<TCHAR, sizeof DTWAINFrameInternalGUID / sizeof(TCHAR)> s_id;
        DTWAINFrameInternal(double left=0, double top=0, double right=0, double bottom=0) : m_FrameComponent{}
        {
            m_FrameComponent[DTWAIN_FRAMELEFT] = left;
            m_FrameComponent[DTWAIN_FRAMETOP] = top;
            m_FrameComponent[DTWAIN_FRAMERIGHT] = right;
            m_FrameComponent[DTWAIN_FRAMEBOTTOM] = bottom;
            std::copy_n(DTWAINFrameInternalGUID, sizeof DTWAINFrameInternalGUID / sizeof(TCHAR), s_id.begin());
            s_id.back() = _T('\0');
        }
    };

    inline bool operator==(const DTWAINFrameInternal& lhs, const DTWAINFrameInternal& rhs)
    {
        return lhs.m_FrameComponent == rhs.m_FrameComponent;
    }

    inline bool operator!=(const DTWAINFrameInternal& lhs, const DTWAINFrameInternal& rhs)
    {
        return !(lhs.m_FrameComponent == rhs.m_FrameComponent);
    }

    typedef std::vector<ImageModuleDef> CTL_IMAGEDLLINFO;
    typedef std::shared_ptr<CTL_TwainDLLHandle> CTL_TwainDLLHandlePtr;

    enum CTL_EnumeratorType { CTL_EnumeratorPtrType     = 1,
                                CTL_EnumeratorIntType       = 2,
                                CTL_EnumeratorDoubleType    = 3,
                                CTL_EnumeratorHandleType     = 4,
                                CTL_EnumeratorSourceType    = 5,
                                CTL_EnumeratorStringType  = 6,
                                CTL_EnumeratorDTWAINFrameType   = 7,
                                CTL_EnumeratorLongStringType = 8,
                                CTL_EnumeratorUnicodeStringType = 9,
                                CTL_EnumeratorInt64Type   = 10,
                                CTL_EnumeratorANSIStringType = 11,
                                CTL_EnumeratorWideStringType = 12,
                                CTL_EnumeratorTWFIX32Type = 200,
                                CTL_EnumeratorTWFrameType = 500,
                                CTL_EnumeratorAnyType     = 1000,
                                CTL_EnumeratorInvalid     = -1
    };

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

    typedef std::string EnumStringTypeA;
    typedef std::wstring EnumStringTypeW;

    typedef CTL_ITwainSource* CTL_ITwainSourcePtr;
    typedef CTL_EnumeratorNode<int, CTL_EnumeratorIntType>                  CTL_Enumerator_int;
    typedef CTL_EnumeratorNode<LONG64, CTL_EnumeratorInt64Type>             CTL_Enumerator_LONG64;
    typedef CTL_EnumeratorNode<double, CTL_EnumeratorDoubleType>            CTL_Enumerator_double;
    typedef CTL_EnumeratorNode<HANDLE, CTL_EnumeratorHandleType>               CTL_Enumerator_HANDLE;
    typedef CTL_EnumeratorNode<CTL_ITwainSourcePtr, CTL_EnumeratorSourceType> CTL_Enumerator_CTL_ITwainSourcePtr;
    typedef CTL_EnumeratorNode<LPVOID, CTL_EnumeratorPtrType>               CTL_Enumerator_LPVOID;
    typedef CTL_EnumeratorNode<CTL_StringType, CTL_EnumeratorStringType>        CTL_Enumerator_CTL_StringType;
    typedef CTL_EnumeratorNode<EnumStringTypeA, CTL_EnumeratorANSIStringType>        CTL_Enumerator_EnumStringTypeA;
    typedef CTL_EnumeratorNode<EnumStringTypeW, CTL_EnumeratorWideStringType>        CTL_Enumerator_EnumStringTypeW;
    typedef CTL_EnumeratorNode<DTWAINFrameInternal, CTL_EnumeratorDTWAINFrameType> CTL_Enumerator_DTWAINFrameInternal;
    typedef CTL_EnumeratorNode<TW_FRAME, CTL_EnumeratorTWFrameType>         CTL_Enumerator_TW_FRAME;
    typedef CTL_EnumeratorNode<TW_FIX32, CTL_EnumeratorTWFIX32Type>         CTL_Enumerator_TW_FIX32;

    typedef std::shared_ptr<TW_FIX32> TW_FIX32Ptr;

    //typedef std::list<DTWAINFrameInternalPtr> DTWAINFrameList;
    typedef std::list<DTWAINFrameInternal> DTWAINFrameList;

    struct CTL_EnumeratorFactory
    {
        // Make these lists
        std::list< CTL_Enumerator_int >         m_EnumeratorList_int;
        std::list< CTL_Enumerator_LONG64 >      m_EnumeratorList_LONG64;
        std::list< CTL_Enumerator_double>       m_EnumeratorList_double;
        std::list< CTL_Enumerator_HANDLE>       m_EnumeratorList_HANDLE;
        std::list< CTL_Enumerator_CTL_ITwainSourcePtr> m_EnumeratorList_CTL_ITwainSourcePtr;
        std::list< CTL_Enumerator_LPVOID>       m_EnumeratorList_LPVOID;
        std::list< CTL_Enumerator_CTL_StringType >  m_EnumeratorList_CTL_StringType;
        std::list< CTL_Enumerator_EnumStringTypeA >  m_EnumeratorList_EnumStringTypeA;
        std::list< CTL_Enumerator_EnumStringTypeW>  m_EnumeratorList_EnumStringTypeW;
        std::list< CTL_Enumerator_DTWAINFrameInternal> m_EnumeratorList_DTWAINFrameInternal;
        std::list< CTL_Enumerator_TW_FRAME >    m_EnumeratorList_TW_FRAME;
        std::list< CTL_Enumerator_TW_FIX32 >    m_EnumeratorList_TW_FIX32;

        // special list for TW_FIX32 individual instances
        std::list<TW_FIX32Ptr>                    m_AvailableFix32Values;
        DTWAINFrameList                           m_AvailableFrameValues;
    };

    typedef std::shared_ptr<CTL_EnumeratorFactory> CTL_EnumeratorFactoryPtr;

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

    class TwainMessageLoopImpl
    {
        private:
            CTL_TwainDLLHandle* m_pDLLHandle;

        protected:
            virtual bool IsSourceOpen(CTL_ITwainSource* pSource, bool bUIOnly);
            virtual bool CanEnterDispatch(MSG * /*pMsg*/) { return true; }

        public:
            TwainMessageLoopImpl(CTL_TwainDLLHandle* pHandle) : m_pDLLHandle(pHandle) {}
            TwainMessageLoopImpl(const TwainMessageLoopImpl&) = delete;
            TwainMessageLoopImpl& operator=(const TwainMessageLoopImpl&) = delete;
            TwainMessageLoopImpl(TwainMessageLoopImpl&& rhs) noexcept :
                    m_pDLLHandle(rhs.m_pDLLHandle) { rhs.m_pDLLHandle = nullptr; }
            TwainMessageLoopImpl& operator=(TwainMessageLoopImpl&& rhs) = delete;

            virtual ~TwainMessageLoopImpl() = default;
            virtual void PrepareLoop() {}
            virtual void PerformMessageLoop(CTL_ITwainSource * /*pSource*/, bool /*bUIOnly*/) {}
    };

    class TwainMessageLoopWindowsImpl : public TwainMessageLoopImpl
    {
        public:
            TwainMessageLoopWindowsImpl(CTL_TwainDLLHandle* pHandle) : TwainMessageLoopImpl(pHandle) {}
            void PrepareLoop() override
            {
            #ifdef WIN32
                MSG msg;
                // Call this so that we have a queue to deal with
                PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
            #endif
            }

            void PerformMessageLoop(CTL_ITwainSource *pSource, bool bUIOnly) override;
    };

    class TwainMessageLoopV1 : public TwainMessageLoopWindowsImpl
    {
        public:
            TwainMessageLoopV1(CTL_TwainDLLHandle* pHandle) : TwainMessageLoopWindowsImpl(pHandle) {}
            bool CanEnterDispatch(MSG *pMsg) override { return !DTWAIN_IsTwainMsg(pMsg); }
    };

    class TwainMessageLoopV2 : public TwainMessageLoopWindowsImpl
    {
        public:
            static std::queue<MSG> s_MessageQueue;

            static TW_UINT16 TW_CALLINGSTYLE TwainVersion2MsgProc(
                pTW_IDENTITY pOrigin,
                pTW_IDENTITY pDest,
                TW_UINT32 DG_,
                TW_UINT16 DAT_,
                TW_UINT16 MSG_,
                TW_MEMREF pData
                );

            TwainMessageLoopV2(CTL_TwainDLLHandle* pHandle) : TwainMessageLoopWindowsImpl(pHandle) {}
            void PrepareLoop() override
            {
                // remove elements from the queue
                std::queue<MSG> empty;
                std::swap(s_MessageQueue, empty);
            }

            bool IsSourceOpen(CTL_ITwainSource* pSource, bool bUIOnly) override;
            bool CanEnterDispatch(MSG *pMsg) override { return !DTWAIN_IsTwainMsg(pMsg); }
    };

    class CTL_TwainDLLHandle
    {
        public:
            struct FileFormatNode
            {
                std::string m_formatName;
                std::vector<std::string> m_vExtensions;
                FileFormatNode(std::string name, std::vector<std::string> vExt) :
                                m_formatName(std::move(name)), m_vExtensions(std::move(vExt)) {}
            };

            typedef std::unordered_map<LONG, std::pair<std::string, std::string>> CTL_PDFMediaMap;
            typedef tsl::ordered_map<LONG, FileFormatNode> CTL_AvailableFileFormatsMap;
            CTL_TwainDLLHandle();
            ~CTL_TwainDLLHandle();
            static void    NotifyWindows( UINT nMsg, WPARAM wParam, LPARAM lParam );
            static void    RemoveAllEnumerators();
            void    RemoveAllSourceCapInfo();
            void    RemoveAllSourceMaps();
            void    InitializeResourceRegistry();
            std::pair<CTL_ResourceRegistryMap::iterator, bool> AddResourceToRegistry(LPCSTR pLangDLL);
            CTL_ResourceRegistryMap& GetResourceRegistry() { return m_ResourceRegistry; }
            static CTL_TwainLongToStringMap& GetTwainCountryMap() { return s_TwainCountryMap;  }
            static CTL_TwainLongToStringMap& GetTwainLanguageMap() { return s_TwainLanguageMap; }
            CTL_StringType GetVersionString() const { return  m_VersionString; }
            void        SetVersionString(CTL_StringType s) { m_VersionString = s; }

            static DTWAIN_ACQUIRE     GetNewAcquireNum();
            static void             EraseAcquireNum(DTWAIN_ACQUIRE nNum);
            static std::string       GetTwainNameFromResource(int nWhichResourceID, int nWhichItem);
            static int              GetIDFromTwainName(std::string sName);
            static int              GetDGResourceID()  { return 8890; }
            static int              GetDATResourceID() { return 8891; }
            static int              GetMSGResourceID() { return 8892; }
            static long             GetErrorFilterFlags() { return s_lErrorFilterFlags; }
            static CTL_PDFMediaMap& GetPDFMediaMap() { return s_PDFMediaMap; }
            static CTL_AvailableFileFormatsMap& GetAvailableFileFormatsMap() { return s_AvailableFileFormatsMap; }

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
            static CTL_StringType   s_strResourcePath;  // path to the DTWAIN resource strings
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
            DTWAIN_CALLBACK_PROC m_pCallbackFn;
            DTWAIN_CALLBACK_PROC64 m_pCallbackFn64;
            DTWAIN_ERROR_PROC   m_pErrorProcFn;
            DTWAIN_ERROR_PROC64 m_pErrorProcFn64;
            LONG                m_lErrorProcUserData;
            LONG64              m_lErrorProcUserData64;
            static DTWAIN_LOGGER_PROC  s_pLoggerCallback;
            static DTWAIN_LOGGER_PROCA  s_pLoggerCallbackA;
            static DTWAIN_LOGGER_PROCW  s_pLoggerCallbackW;
            static DTWAIN_LONG64  s_pLoggerCallback_UserData;
            static DTWAIN_LONG64  s_pLoggerCallback_UserDataA;
            static DTWAIN_LONG64  s_pLoggerCallback_UserDataW;
            LONG                m_lCallbackData;
            LONGLONG            m_lCallbackData64;
            CTL_ITwainSource*   m_pDummySource;
            OCRInterfaceContainer m_OCRInterfaceArray;
            OCRProductNameToEngineMap m_OCRProdNameToEngine;
            OCREnginePtr          m_pOCRDefaultEngine;
            static CTL_PDFMediaMap s_PDFMediaMap;
            static CTL_AvailableFileFormatsMap s_AvailableFileFormatsMap;
            std::set<CTL_TwainTriplet::TwainTripletComponents> m_setLogFilterComponents;

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
            static CTL_TwainNameMap    s_TwainNameMap;
            static CTL_TwainLongToStringMap s_TwainCountryMap;
            static CTL_TwainLongToStringMap s_TwainLanguageMap;

            static HINSTANCE         s_DLLInstance;

            // static arrays
            static bool                     s_ResourcesInitialized;
            static std::vector<CTL_TwainDLLHandlePtr> s_DLLHandles;
            static std::unordered_set<DTWAIN_SOURCE> s_aFeederSources;
            static CTL_HookInfoArray        s_aHookInfo;
            static std::vector<int>              s_aAcquireNum;
            static bool                     s_bCheckReentrancy;
            static CTL_GeneralCapInfo       s_mapGeneralCapInfo;
            static CTL_GeneralErrorInfo     s_mapGeneralErrorInfo;
            static short int                s_nDSMState;
            static int                      s_nDSMVersion;
            static long                     s_lErrorFilterFlags;
            static bool                     s_bProcessError;
            static CLogSystem               s_appLog;
            static LONG                     s_nRegisteredDTWAINMsg;
            static CTL_StringType        s_sINIPath;
            static std::string           s_CurLangResource;
            static CTL_StringType        s_TempFilePath;
            static CTL_StringType        s_ImageDLLFilePath;
            static CTL_StringType        s_LangResourcePath;
            static CTL_StringType        s_VersionString;
            static CTL_StringType        s_DLLPath;
            static UINT_PTR                 s_nTimerID;
            static UINT_PTR                 s_nTimeoutID;
            static UINT                     s_nTimeoutMilliseconds;
            static bool                     s_bTimerIDSet;
            static bool                     s_bThrowExceptions;
            static CTL_CallbackProcArray    s_aAllCallbacks;
            static CTL_LongToStringMap      s_ErrorCodes;
            static CTL_LongToStringMap      s_ResourceStrings;
            static CTL_EnumeratorFactoryPtr s_EnumeratorFactory;
            static bool                     s_UsingCustomResource;
            static bool                     s_DemoInitialized;
            static int                      s_TwainDSMSearchOrder;
            static std::string               s_TwainDSMSearchOrderStr;
            static CTL_StringType           s_TwainDSMUserDirectory;
            static bool                     s_multipleThreads;
            static HFONT                    s_DialogFont;

            static std::unordered_set<HWND>   s_appWindowsToDisable;
            bool                            m_bOpenSourceOnSelect;
            static bool                     s_bCheckHandles;
            static bool                     s_bQuerySupport;
            static DTWAIN_DIBUPDATE_PROC    s_pDibUpdateProc;
            static std::deque<int> s_vErrorBuffer;
            static unsigned int             s_nErrorBufferThreshold;
            static unsigned int             s_nErrorBufferReserve;
            static std::stack<unsigned long, std::deque<unsigned long> > s_vErrorFlagStack;
            static CTL_TwainMemoryFunctions*      s_TwainMemoryFunc;
            static CTL_LegacyTwainMemoryFunctions s_TwainLegacyFunc;
            static CTL_Twain2MemoryFunctions s_Twain2Func;
            static bool                     s_TwainCallbackSet;
            /*static */CTL_LongToVectorLongMap  m_mapDTWAINArrayToTwainType;
            static CTL_IMAGEDLLINFO         s_ImageDLLInfo;
            static std::bitset<10>  g_AvailabilityFlags;
    #ifndef DTWAIN_RETAIL
    //        static PROCESS_INFORMATION      s_ProcessInfo;
    #endif

            /////////////////////////////////////////////////////////////////////////////
            // protection
           #ifdef WIN32
                static  CRITICAL_SECTION        s_critLogCall;
                static  CRITICAL_SECTION        s_critFileCreate;
                static  CRITICAL_SECTION        s_critStaticInit;
                static  bool                    s_bCritSectionCreated;
                static  bool                    s_bFileCritSectionCreated;
                static  bool                    s_bCritStaticCreated;

           #endif
    };

    template <typename T>
    T IsDLLHandleValid(CTL_TwainDLLHandle *pHandle, T bCheckSession = T(1))
    {
        // See if DLL Handle exists
        if (!pHandle)
            return {};
        if (std::find_if(CTL_TwainDLLHandle::s_DLLHandles.begin(),
            CTL_TwainDLLHandle::s_DLLHandles.end(), SmartPointerFinder<CTL_TwainDLLHandlePtr>(pHandle)) ==
            CTL_TwainDLLHandle::s_DLLHandles.end())
            return {};
        if (!pHandle->m_bSessionAllocated && bCheckSession)
            return {};
        return {1};
    }

    CTL_TwainDLLHandle* FindHandle(HWND hWnd, bool bIsDisplay);
    CTL_TwainDLLHandle* FindHandle(HINSTANCE hInst);
    CTL_ITwainSource* VerifySourceHandle( DTWAIN_HANDLE DLLHandle, DTWAIN_SOURCE Source );



    int GetResolutions(DTWAIN_HANDLE DLLHandle, DTWAIN_SOURCE Source, void* pArray,
                       CTL_EnumGetType GetType);
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

    DTWAIN_BOOL    DTWAIN_ArrayFirst(DTWAIN_ARRAY pArray, LPVOID pVariant);

    DTWAIN_BOOL    DTWAIN_ArrayNext(DTWAIN_ARRAY pArray, LPVOID pVariant);
    LONG           DTWAIN_ArrayType(DTWAIN_ARRAY pArray, bool bGetReal=true);
    bool           DTWAINFRAMEToTWFRAME(DTWAIN_FRAME pDdtwil, pTW_FRAME pTwain);
    bool           TWFRAMEToDTWAINFRAME(const TW_FRAME& pTwain, DTWAIN_FRAME pDdtwil);

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
    bool UserDefinedLoggerExists();
    void WriteUserDefinedLogMsg(LPCTSTR sz);
    void WriteUserDefinedLogMsgA(LPCSTR sz);
    void WriteUserDefinedLogMsgW(LPCWSTR sz);
    bool GetSupportString(DTWAIN_SOURCE Source, LPTSTR sz, LONG nLen, LONG Cap, LONG GetType);
    bool EnumSupported(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY pArray, LONG Cap);
    LONG CheckEnabled(DTWAIN_SOURCE Source, LONG CapVal);
    bool SetSupportArray(DTWAIN_SOURCE Source, DTWAIN_ARRAY Array, LONG Cap);
    bool GetSupportArray(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY Array, LONG Cap, LONG GetType=DTWAIN_CAPGET);
    CTL_StringType& GetDTWAINTempFilePath();
    size_t GetResourceStringA(UINT nResNumber, LPSTR buffer, LONG bufSize);
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
    void DTWAIN_CollectCapabilityInfo(CTL_ITwainSource *p, TW_UINT16 nCap, CTL_CapInfoArray& pArray);
    CTL_CapInfoArrayPtr GetCapInfoArray(CTL_TwainDLLHandle* pHandle, const CTL_ITwainSource *p);
    DTWAIN_SOURCE SourceSelect(const SourceSelectionOptions& options);
    DTWAIN_ARRAY  SourceAcquire(SourceAcquireOptions& opts);
    bool AcquireExHelper(SourceAcquireOptions& opts);
    bool AcquireFileHelper(SourceAcquireOptions& opts, LONG AcquireType);
    DTWAIN_ARRAY SourceAcquireWorkerThread(SourceAcquireOptions& opts);
    DTWAIN_ACQUIRE  LLAcquireImage(SourceAcquireOptions& opts);
    DTWAIN_HANDLE GetDTWAINHandle_Internal();
    bool TileModeOn(DTWAIN_SOURCE Source);

    //#ifdef DTWAIN_DEBUG_CALL_STACK
    std::string CTL_LogFunctionCallHelper(LPCSTR pFuncName, int nWhich, LPCSTR pOptionalString=nullptr);
    std::string CTL_LogFunctionCallA(LPCSTR pFuncName, int nWhich, LPCSTR pOptionalString=nullptr);
    //#endif

    // outputs parameter and return values
    class ParamOutputter
    {
        StringArray sv;
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
            StringWrapperA::Tokenize(s, "(, )", sv);
            if (!m_bIsReturnValue)
                strm << "(";
            else
                strm << s << " " << CTL_TwainDLLHandle::s_ResourceStrings[IDS_LOGMSG_RETURNTEXT] << " ";
        }

        template <typename T, typename ...P>
        ParamOutputter& outputParam(T t, P ...p)
        {
            if (sv.empty() && !m_bIsReturnValue)
                return *this;
            const bool bIsNull = std::is_pointer_v<T> && !t;
            if (!m_bIsReturnValue)
            {
                // Make sure we log input types correctly, especially character pointers.
                // User may supply to us a writable char buffer that is not null-terminated!
                LogInputType(sv[nWhich], t);
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
              if (nWhich < sv.size() - 1)
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

    struct DTWAINArray_DestroyTraits
    {
        static void Destroy(DTWAIN_ARRAY a)
        {
            if (a)
                DTWAIN_ArrayDestroy(a);
        }
    };

    struct DTWAINArrayPtr_DestroyTraits
    {
        static void Destroy(DTWAIN_ARRAY* a)
        {
            if (a && *a)
                DTWAIN_ArrayDestroy(*a);
        }
    };

    struct DTWAINGlobalHandle_CloseTraits
    {
        static void Destroy(HANDLE h)
        {
            #ifdef _WIN32
            if ( h )
                ImageMemoryHandler::GlobalUnlock(h);
            #endif
        }
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
    };

    struct DTWAINFrame_DestroyTraits
    {
        static void Destroy(DTWAIN_FRAME f)
        {
            if (f)
                DTWAIN_FrameDestroy(f);
        }
    };

    struct DTWAINGlobalHandle_ReleaseDCTraits
    {
        static void Destroy(std::pair<HWND, HDC> val)
        {
            #ifdef _WIN32
            if (val.second)
                ReleaseDC(val.first, val.second);
            #endif
        }
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
    };

    // RAII Class for DTWAIN_ARRAY
    typedef DTWAIN_RAII<DTWAIN_ARRAY, DTWAINArray_DestroyTraits> DTWAINArray_RAII;
    typedef DTWAIN_RAII<DTWAIN_ARRAY*, DTWAINArrayPtr_DestroyTraits> DTWAINArrayPtr_RAII;
    typedef DTWAIN_RAII<DTWAIN_FRAME, DTWAINFrame_DestroyTraits> DTWAINFrame_RAII;
    typedef DTWAIN_RAII<HANDLE, DTWAINGlobalHandle_CloseTraits> DTWAINGlobalHandle_RAII;
    typedef DTWAIN_RAII<HANDLE*, DTWAINGlobalHandle_ClosePtrTraits> DTWAINGlobalHandlePtr_RAII;
    typedef DTWAIN_RAII<HANDLE, DTWAINGlobalHandle_CloseFreeTraits> DTWAINGlobalHandleUnlockFree_RAII;
    typedef DTWAIN_RAII<std::pair<HWND, HDC>, DTWAINGlobalHandle_ReleaseDCTraits> DTWAINDeviceContextRelease_RAII;
    typedef DTWAIN_RAII<HANDLE, DTWAINFileHandle_CloseTraits> DTWAINFileHandle_RAII;
    typedef DTWAIN_RAII<HGLOBAL, DTWAINResource_UnlockFreeTraits> DTWAINResourceUnlockFree_RAII;
    typedef DTWAIN_RAII<HBITMAP*, DTWAINResource_DeleteObjectTraits> DTWAINHBITMAPFree_RAII;

    // RAII Class for turning on/off logging locally
    struct DTWAINScopedLogController
    {
        long m_ErrorFilterFlags;
        DTWAINScopedLogController(long newFilter) : m_ErrorFilterFlags(CTL_TwainDLLHandle::s_lErrorFilterFlags)
        { CTL_TwainDLLHandle::s_lErrorFilterFlags = newFilter; }
        ~DTWAINScopedLogController() { CTL_TwainDLLHandle::s_lErrorFilterFlags = m_ErrorFilterFlags; }
    };

    struct LogTraitsOff
    { static long Apply(long turnOff) { return CTL_TwainDLLHandle::s_lErrorFilterFlags &~turnOff; } };

    struct LogTraitsOn
    { static long Apply(long turnOn) { return CTL_TwainDLLHandle::s_lErrorFilterFlags  | turnOn; } };

    template <typename LogTraits>
    struct DTWAINScopedLogControllerEx
    {
        DTWAINScopedLogController m_controller;
        DTWAINScopedLogControllerEx(long newValue) : m_controller(LogTraits::Apply(newValue)) {}
    };

    template <typename T, bool Value=false>
    struct NotImpl
    { bool operator !() const { return Value; } };

    typedef DTWAINScopedLogControllerEx<LogTraitsOff> DTWAINScopedLogControllerExclude;
    typedef DTWAINScopedLogControllerEx<LogTraitsOn>  DTWAINScopedLogControllerInclude;

    #ifdef USE_EXCEPTION_SPEC
        #define THIS_FUNCTION_PROTO_THROWS throw(...);
        #define THIS_FUNCTION_THROWS throw(...)
    #else
        #define THIS_FUNCTION_PROTO_THROWS  ;
        #define THIS_FUNCTION_THROWS
    #endif

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
