/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2023 Dynarithmic Software.

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
#ifndef ARRAYFACTORY_H
#define ARRAYFACTORY_H

#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>

#include "twainframe.h"
#include "twainfix32.h"

namespace dynarithmic
{
    enum CTL_ArrayType
    {
        CTL_ArrayPtrType = 1,
        CTL_ArrayIntType = 2,
        CTL_ArrayDoubleType = 3,
        CTL_ArrayHandleType = 4,
        CTL_ArraySourceType = 5,
        CTL_ArrayStringType = 6,
        CTL_ArrayDTWAINFrameType = 7,
        CTL_ArrayLongStringType = 8,
        CTL_ArrayUnicodeStringType = 9,
        CTL_ArrayInt64Type = 10,
        CTL_ArrayANSIStringType = 11,
        CTL_ArrayWideStringType = 12,
        CTL_ArrayTWFIX32Type = 200,
        CTL_ArrayTWFrameType = 500,
        CTL_ArrayAnyType = 1000,
        CTL_ArrayToHandleArray = 2000,
        CTL_ArrayFrameSingleType = 3000,
        CTL_ArrayInvalid = -1
    };

    class CTL_ITwainSource;

    struct CTL_ArrayFactory
    {
        struct arrayTag
        {
            static constexpr int LongType = 0;
            static constexpr int DoubleType = 1;
            static constexpr int StringType = 2;
            static constexpr int WStringType = 3;
            static constexpr int VoidPtrType = 4;
            static constexpr int SourceType = 5;
            static constexpr int ArrayOfArrayOfVoidPtrType = 6;
            static constexpr int FrameType = 7;
            static constexpr int Fix32Type = 8;
            static constexpr int Long64Type = 9;
            static constexpr int FrameSingleType = 10;
            static constexpr int TWFrameType = 11;
            static constexpr int UnknownType = -1;

            int nTag;
            int getTag() const noexcept { return nTag; }
            void setTag(int tag) noexcept { nTag = tag; }
            explicit arrayTag(int tag = 0) noexcept : nTag(tag) {}
        };

        template <typename T>
        struct tagged_array : arrayTag
        {
            using value_type = T;
            using container_type = std::vector<T>;
            container_type vData;
            container_type& get_container() noexcept { return vData; }
            void push_back(const value_type& value) { vData.push_back(value); }
            void push_back_v(T *value) { vData.push_back(*value); }
            void push_back_ptr(T value) { vData.push_back(value); }
        };

        using tagged_array_long = tagged_array<LONG>;
        using tagged_array_double = tagged_array<double>;
        using tagged_array_string = tagged_array<std::string>;
        using tagged_array_wstring = tagged_array<std::wstring>;
        using tagged_array_voidptr = tagged_array<void*>;
        using tagged_array_sourceptr = tagged_array<CTL_ITwainSource*>;
        using tagged_array_tagged_array_voidptr = tagged_array<tagged_array_voidptr*>;
        using tagged_array_frame = tagged_array<TwainFrameInternal>;
        using tagged_array_twframe = tagged_array<TW_FRAME>;
        using tagged_array_fix32 = tagged_array<TW_FIX32Ex>;
        using tagged_array_long64 = tagged_array<LONG64>;

        using voidAddFn = std::function<void(arrayTag*, std::size_t, void*)>;
        using voidAddFnMap = std::unordered_map<int, voidAddFn>;

        using voidGetFn = std::function<void*(arrayTag*, std::size_t, void*)>;
        using voidGetFnMap = std::unordered_map<int, voidGetFn>;

        using voidFindFn = std::function<std::size_t(arrayTag*, void*, double)>;
        using voidFindFnMap = std::unordered_map<int, voidFindFn>;

        using voidInserterFn = std::function<void(arrayTag*, std::size_t, std::size_t, void*)>;
        using voidInserterFnMap = std::unordered_map<int, voidInserterFn>;

        using voidCopierFn = std::function<void(arrayTag*, arrayTag*)>;
        using voidCopierFnMap = std::unordered_map<int, voidCopierFn>;

        using voidRemoverFn = std::function<void(arrayTag*, std::size_t, std::size_t)>;
        using voidRemoverFnMap = std::unordered_map<int, voidRemoverFn>;

        using voidClearerFn = std::function<void(arrayTag*)>;
        using voidClearerFnMap = std::unordered_map<int, voidClearerFn>;

        using voidResizerFn = std::function<void(arrayTag*, std::size_t)>;
        using voidResizerFnMap = std::unordered_map<int, voidResizerFn>;

        using intCounterFn = std::function<std::size_t(arrayTag*)>;
        using intCounterFnMap = std::unordered_map<int, intCounterFn>;

        using voidSetterFn = std::function<void(arrayTag*, std::size_t, void* value)>;
        using voidSetterFnMap = std::unordered_map<int, voidSetterFn>;

        using voidGetBufferFn = std::function<void*(arrayTag*, std::size_t)>;
        using voidGetBufferFnMap = std::unordered_map<int, voidGetBufferFn>;

        template <typename T>
        auto& underlying_container_t(arrayTag* pTag) const
        {
	        if (auto* p = static_cast<tagged_array<T>*>(pTag))
                return p->get_container();
            throw (std::invalid_argument::invalid_argument("underlying_container_t argument cannot be nullptr"));
        }

        template <typename T>
        auto& underlying_container_t(void* pTag) const
        {
            return underlying_container_t<T>(from_void(pTag));
        }


        template <typename T>
        auto& underlying_container(arrayTag* tag)
        {
            return underlying_container_t<typename T::value_type>(tag);
        }

        template <typename T>
        auto& underlying_container(void* tag)
        {
            return underlying_container<T>(from_void(tag));
        }

        template <typename T>
        void simple_pushback(arrayTag* tag, void* value, size_t nSize) const
        {
            auto vvalue = static_cast<typename T::value_type *>(value);
            auto p = static_cast<T*>(tag);
            for (size_t i = 0; i < nSize; ++i)
                p->push_back(*vvalue);
        }

        template <typename T>
        void string_pushback(arrayTag* tag, void* value, size_t nSize) const
        {
            auto vvalue = *static_cast<typename T::value_type *>(value);
            auto p = static_cast<T*>(tag);
            for (size_t i = 0; i < nSize; ++i)
                p->push_back(vvalue); 
        }

        template <typename T>
        void pointer_pushback(arrayTag* tag, void* value, size_t nSize) const
        {
            auto vvalue = static_cast<typename T::value_type>(value);
            auto p = static_cast<T*>(tag);
            for (size_t i = 0; i < nSize; ++i)
                p->push_back_ptr(vvalue);
        }

        template <typename T>
        void simple_clearer(arrayTag* tag) const
        {
            auto p = static_cast<T*>(tag);
            p->get_container().clear();
        }

        template <typename T>
        void* simple_getter(arrayTag* tag, size_t nWhere, void* value) const
        {
            auto pRet = static_cast<typename T::value_type*>(value);
            auto p = static_cast<T*>(tag);
            *pRet = p->get_container()[nWhere];
            return value;
        }

        template <typename T>
        void* pointer_getter(arrayTag* tag, size_t nWhere, void* value) const
        {
            auto p = static_cast<T*>(tag);
            return p->get_container()[nWhere];
        }

        template <typename T>
        void simple_setter(arrayTag* tag, size_t nWhere, void* value) const
        {
            auto pRet = static_cast<typename T::value_type*>(value);
            auto p = static_cast<T*>(tag);
            p->get_container()[nWhere] = *pRet;
        }

        template <typename T>
        void pointer_setter(arrayTag* tag, size_t nWhere, void* value) const
        {
            auto p = static_cast<T*>(tag);
            p->get_container()[nWhere] = static_cast<typename T::value_type>(value);
        }

        template <typename T>
        void string_setter(arrayTag* tag, size_t nWhere, void* value) const
        {
            auto p = static_cast<T*>(tag);
            p->get_container()[nWhere] = *static_cast<typename T::value_type*>(value);
        }

        struct simple_finder_t
        {
            template <typename container, typename ValueType>
            typename container::iterator find(container& c, ValueType value, double)
            {
                return std::find(c.begin(), c.end(), value);
            }
        };

        struct frame_finder_t
        {
            template <typename container>
            typename container::iterator find(container& c, TW_FRAME& value, double)
            {
                return std::find_if(c.begin(), c.end(), [&](const TW_FRAME& frm) 
                    { return frm == value; });
            }
        };

        struct double_finder_t
        {
            double tolerance;
            explicit double_finder_t(double tol = 1.0e-08) noexcept : tolerance(tol) {}

            template <typename container>
            typename container::iterator find(container& c, double value, double)
            {
                return std::find_if(c.begin(), c.end(), [&](double val) { return fabs(val - tolerance) <= 1.0e-8; });
            }
        };

        struct pointer_finder_t
        {
            template <typename container>
            typename container::iterator find(container& c, void* value, double)
            {
                return std::find(c.begin(), c.end(), value);
            }
        };

        template <typename T, typename U>
        size_t simple_finder(arrayTag* tag, void* value, double val, U finder_traits)
        {
            auto pRet = *static_cast<typename T::value_type*>(value);
            auto p = static_cast<T*>(tag);
            auto& v = p->get_container();
            auto iter = finder_traits.find(v, pRet, val);
            if (iter == v.end())
                return (std::numeric_limits<int>::max)();
            return std::distance(v.begin(), iter);
        }

        template <typename T>
        void simple_resizer(arrayTag* tag, std::size_t nCount) const
        {
            auto p = static_cast<T*>(tag);
            auto& v = p->get_container();
            v.resize(nCount);
        }

        template <typename T>
        void simple_inserter(arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) const
        {
            auto pRet = static_cast<typename T::value_type*>(value);
            auto p = static_cast<T*>(tag);
            auto& v = p->get_container();
            v.insert(v.begin() + nWhere, nCount, *pRet);
        }

        template <typename T>
        void string_inserter(arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) const
        {
            auto p = static_cast<T*>(tag);
            auto& v = p->get_container();
            typename T::value_type str(*static_cast<typename T::value_type*>(value));
            v.insert(v.begin() + nWhere, nCount, str);
        }

        template <typename T>
        void pointer_inserter(arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) const
        {
            auto p = static_cast<T*>(tag);
            auto& v = p->get_container();
            v.insert(v.begin() + nWhere, nCount, static_cast<typename T::value_type>(value));
        }

        template <typename T>
        void simple_remover(arrayTag* tag, std::size_t nWhere, std::size_t nCount) const
        {
            auto p = static_cast<T*>(tag);
            auto& v = p->get_container();
            v.erase(v.begin() + nWhere, v.begin() + nWhere + nCount);
        }

        template <typename T>
        size_t simple_counter(arrayTag* tag) const
        {
            auto p = static_cast<T*>(tag);
            auto& v = p->get_container();
            return v.size();
        }

        template <typename T>
        void simple_copier(arrayTag* tag, arrayTag* tag2) const
        {
            auto pDest = static_cast<T*>(tag);
            auto& vDest = pDest->get_container();
            auto pSrc = static_cast<T*>(tag2);
            auto& vSrc = pSrc->get_container();
            vDest = vSrc;
        }

        template <typename T>
        void* simple_buffer_getter(arrayTag* tag, std::size_t nWhere) const
        {
            auto p = static_cast<T*>(tag);
            auto& v = p->get_container();
            return v.data() + nWhere;
        }

        template <typename T, typename U>
        arrayTag* generic_array_creator(U tagType, std::size_t nInitialSize)
        {
            auto sp = std::make_shared<T>();
            sp->setTag(tagType);
            m_tagMap.insert({ sp.get(), {tagType,sp} });
            auto& v = underlying_container<T>(sp.get());
            v.resize(nInitialSize);
            return sp.get();
        }

        template <typename T, typename U>
        arrayTag* generic_single_creator(U tagType)
        {
            auto sp = std::make_shared<T>();
            sp->setTag(tagType);
            m_tagMap.insert({ sp.get(), {tagType,sp} });
            auto& v = underlying_container<T>(sp.get());
            v.resize(1);
            return sp.get();
        }

    private:
        std::unordered_map <arrayTag*, std::pair<int, std::shared_ptr<arrayTag>>> m_tagMap;
        voidAddFnMap m_vfnAddMap;
        voidGetFnMap m_vfnGetMap;
        voidFindFnMap m_vfnFindMap;
        voidInserterFnMap m_vfnInserterMap;
        voidCopierFnMap m_vfnCopierMap;
        voidRemoverFnMap m_vfnRemoverMap;
        voidClearerFnMap m_vfnClearerMap;
        voidResizerFnMap m_vfnResizerMap;
        intCounterFnMap m_vfnCounterMap;
        voidSetterFnMap m_vfnSetterMap;
        voidGetBufferFnMap m_vfnGetBufferMap;

    public:
        CTL_ArrayFactory();
        arrayTag* create_array(CTL_ArrayType ArrayType, int *pStatus, size_t nInitialSize);
        static arrayTag* from_void(void *ptr) { return static_cast<arrayTag*>(ptr); }

        bool is_valid(arrayTag* pTag) const;
        bool is_valid(void* pTag) const { return is_valid(from_void(pTag)); }
        bool is_valid(arrayTag* pTag, int tagType) const;
        bool is_valid(void* pTag, int tagType) const { return is_valid(from_void(pTag), tagType); }


        std::size_t size(arrayTag *pTag) const;
        std::size_t size(void *pTag) const { return size(from_void(pTag)); }

        bool empty(arrayTag* pTag) const { return size(pTag) == 0; }
        bool empty(void * pTag) const { return size(pTag) == 0; }

        int tag_type(void* pTag) const { return tag_type(from_void(pTag)); }
        int tag_type(arrayTag* pTag) const;
        void clear(arrayTag *pTag);
        void clear(void *pTag) { clear(from_void(pTag)); }

        void destroy(arrayTag* pTag);
        void destroy(void* pTag) { destroy(from_void(pTag)); }

        static int arraytype_to_tagtype(CTL_ArrayType ArrayType);
        static CTL_ArrayType tagtype_to_arraytype(int tag);

        void copy(arrayTag* pTagDest, arrayTag* pTagSource);
        void copy(void * pTagDest, void* pTagSource) { return copy(from_void(pTagDest), from_void(pTagSource)); }

        void add_to_back(arrayTag *pTag, void *value, std::size_t num);
        void add_to_back(void *pTag, void *value, std::size_t num) { return add_to_back(from_void(pTag), value, num); }

        void insert(arrayTag* pTag, void* value, std::size_t nWhere, std::size_t num);
        void insert(void* pTag, void* value, std::size_t nWhere, std::size_t num) { insert(from_void(pTag), value, nWhere, num); }

        void remove(arrayTag* pTag, std::size_t nWhere, std::size_t num);
        void remove(void* pTag, std::size_t nWhere, std::size_t num) { remove(from_void(pTag), nWhere, num); }

        void resize(arrayTag* pTag, std::size_t num);
        void resize(void* pTag, std::size_t num) { return resize(from_void(pTag), num); }

        std::size_t find(arrayTag *pTag, void *value, double tol = 1.0e-08);
        std::size_t find(void *pTag, void *value, double tol = 1.0e-08) { return find(from_void(pTag), value, tol); }

        void set_value(arrayTag *pTag, std::size_t nWhere, void *value);
        void set_value(void *pTag, std::size_t nWhere, void *value) { set_value(from_void(pTag), nWhere, value); }

        void* get_value(arrayTag *pTag, size_t nWhere, void *value) const;
        void* get_value(void *pTag, size_t nWhere, void *value) const { return get_value(from_void(pTag), nWhere, value); }

        template <typename T>
        T get_value(arrayTag *av, size_t nWhere) const
        {
            T retVal = {};
            void *val = get_value(av, nWhere, &retVal);
            if ( val )
                return *static_cast<T*>(val);
            return {};
        }

        template <typename T>
        T get_value(void*av, size_t nWhere) const { return get_value<T>(from_void(av), nWhere); }

        void* get_buffer(arrayTag *pTag, std::size_t nWhere = 0);
        void* get_buffer(void* pTag, std::size_t nWhere = 0) { return get_buffer(from_void(pTag), nWhere); }

        arrayTag* create_frame(double left = 0, double top = 0, double right = 0, double bottom = 0);
        bool is_frame_valid(const void *frame) const;
        void destroy_frame(arrayTag *frame);
        void destroy_frame(void* frame) { destroy_frame(from_void(frame)); }
    };
}
#endif
