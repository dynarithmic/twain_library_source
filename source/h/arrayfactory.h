/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2025 Dynarithmic Software.

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

/* The array factory allows creation of DTWAIN Arrays using std::vector<type> as 
   the backing container for all DTWAIN arrays. 
*/   
#ifndef ARRAYFACTORY_H
#define ARRAYFACTORY_H

#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <boost/container/flat_map.hpp>
#include "twainframe.h"
#include "twainfix32.h"
#include "ctlconstexprfind.h"

namespace dynarithmic
{
    // All the array types supported by the DTWAIN library and internal functions
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

    /* The CTL_ArrayFactory contains the infrastructure that maintains all of the 
    DTWAIN Arrays created.*/
    struct CTL_ArrayFactory
    {
        // The arrayTag is used to distinguish the type of DTWAIN array that was
        // created, i.e. LONG, double, string, etc.
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

        // This is the basic class that is created for each DTWAIN Array
        template <typename T>
        struct tagged_array : arrayTag
        {
            using value_type = T;  // The underlying array type
            using container_type = std::vector<T>; // the backing container for the array values
            container_type vData;
            container_type& get_container() noexcept { return vData; }  // returns the vector
            void push_back(const value_type& value) { vData.push_back(value); } // adds a new item to the vector
            void push_back_v(T *value) { vData.push_back(*value); } // Adds a dereferenced value to the vector
            void push_back_ptr(T value) { vData.push_back(value); } // Adds a pointer to the value
        };

        // all of the DTWAIN array types.
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

        // Map type that maps the array tag to a function that manipulates the 
        // DTWAIN array
        #define FNMAPTYPE boost::container::flat_map

        // Function that adds a value to the DTWAIN Array
        using voidAddFn = std::function<void(arrayTag*, std::size_t, void*)>;
        using voidAddFnMap = FNMAPTYPE<int, voidAddFn>;

        // Function that gets a value to the DTWAIN Array
        using voidGetFn = std::function<void*(arrayTag*, std::size_t, void*)>;
        using voidGetFnMap = FNMAPTYPE<int, voidGetFn>;

        // Function that finds a value to the DTWAIN Array
        using voidFindFn = std::function<std::size_t(arrayTag*, void*, double)>;
        using voidFindFnMap = FNMAPTYPE<int, voidFindFn>;

        // Function that inserts a value to the DTWAIN Array
        using voidInserterFn = std::function<void(arrayTag*, std::size_t, std::size_t, void*)>;
        using voidInserterFnMap = FNMAPTYPE<int, voidInserterFn>;

        // Function that copies from one array to another of the same type
        using voidCopierFn = std::function<void(arrayTag*, arrayTag*)>;
        using voidCopierFnMap = FNMAPTYPE<int, voidCopierFn>;

        // Function that removes a value from the DTWAIN Array
        using voidRemoverFn = std::function<void(arrayTag*, std::size_t, std::size_t)>;
        using voidRemoverFnMap = FNMAPTYPE<int, voidRemoverFn>;

        // Function that clears the DTWAIN Array
        using voidClearerFn = std::function<void(arrayTag*)>;
        using voidClearerFnMap = FNMAPTYPE<int, voidClearerFn>;

        // Function that resizes the DTWAIN Array
        using voidResizerFn = std::function<void(arrayTag*, std::size_t)>;
        using voidResizerFnMap = FNMAPTYPE<int, voidResizerFn>;

        // Function that returns the size of the DTWAIN array
        using intCounterFn = std::function<std::size_t(arrayTag*)>;
        using intCounterFnMap = FNMAPTYPE<int, intCounterFn>;

        // Function that sets an existing value in the DTWAIN Array
        using voidSetterFn = std::function<void(arrayTag*, std::size_t, void* value)>;
        using voidSetterFnMap = FNMAPTYPE<int, voidSetterFn>;

        // Function that gets a pointer to the underlying DTWAIN Array buffer
        using voidGetBufferFn = std::function<void*(arrayTag*, std::size_t)>;
        using voidGetBufferFnMap = FNMAPTYPE<int, voidGetBufferFn>;

        // Returns the underlying std::vector that the DTWAIN Array is wrapping.
        template <typename T>
        auto& underlying_container_t(arrayTag* pTag) const
        {
            // Get the tagged_array<T> that represents the DTWAIN Array
	        if (auto* p = static_cast<tagged_array<T>*>(pTag))
                return p->get_container(); // Return the std::vector
            throw (std::invalid_argument::invalid_argument("underlying_container_t argument cannot be nullptr"));
        }

        template <typename T>
        auto& underlying_container_t(void* pTag) const
        {
            // Given a void*, convert to an array_tag and get
            // the underlying container
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

        // Adds a value to a DTWAIN Array that is a fundamental type,
        // i.e. int, double, etc.
        // The nSize is the number of copies to add
        template <typename T>
        void simple_pushback(arrayTag* tag, void* value, size_t nSize) const
        {
            // Convert the void* to the actual type.  The value is a pointer
            // to the value we will add.
            auto vvalue = static_cast<typename T::value_type *>(value);

            // Get the tagged_array type from the tag
            auto p = static_cast<T*>(tag);

            // Add the values
            for (size_t i = 0; i < nSize; ++i)
                p->push_back(*vvalue);
        }

        // Adds a std::string or std::wstring to a DTWAIN Array.
        template <typename T>
        void string_pushback(arrayTag* tag, void* value, size_t nSize) const
        {
            // Convert the void* to the actual string type.  Note that
            // we need to dereference the pointer to get the string.
            auto vvalue = *static_cast<typename T::value_type *>(value);

            // Get the tagged_array type
            auto p = static_cast<T*>(tag);

            // Add nSize copies to the array
            for (size_t i = 0; i < nSize; ++i)
                p->push_back(vvalue); 
        }

        // Adds a pointer to a DTWAIN Array
        template <typename T>
        void pointer_pushback(arrayTag* tag, void* value, size_t nSize) const
        {
            // The type is already a pointer, so value is the type we want
            // to store
            auto vvalue = static_cast<typename T::value_type>(value);

            // Get the array type
            auto p = static_cast<T*>(tag);

            // Call the specialized push_back for pointers
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
        FNMAPTYPE<arrayTag*, std::pair<int, std::shared_ptr<arrayTag>>> m_tagMap;
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

        static constexpr int arraytype_to_tagtype(CTL_ArrayType arrayType)
        {
            constexpr std::array<std::pair<CTL_ArrayType, int>, 12> mapArrayTypeToTag =
            { {
                {CTL_ArrayIntType, arrayTag::LongType},
                {CTL_ArrayInt64Type, arrayTag::Long64Type},
                {CTL_ArrayDoubleType, arrayTag::DoubleType},
                {CTL_ArrayHandleType,arrayTag::VoidPtrType},
                {CTL_ArrayPtrType, arrayTag::VoidPtrType},
                {CTL_ArrayANSIStringType, arrayTag::StringType},
                {CTL_ArrayWideStringType, arrayTag::WStringType},
                {CTL_ArrayDTWAINFrameType, arrayTag::FrameSingleType},
                {CTL_ArrayTWFIX32Type, arrayTag::Fix32Type},
                {CTL_ArraySourceType, arrayTag::SourceType},
                {CTL_ArrayToHandleArray, arrayTag::ArrayOfArrayOfVoidPtrType},
                {CTL_ArrayFrameSingleType, arrayTag::FrameSingleType} } };
            auto iter = dynarithmic::generic_array_finder_if(mapArrayTypeToTag, [&](const auto& pr) { return pr.first == arrayType; });
            if (iter.first)
                return mapArrayTypeToTag[iter.second].second;
            return arrayTag::UnknownType;
        }

        static constexpr CTL_ArrayType tagtype_to_arraytype(int tag)
        {
            constexpr std::array<std::pair<int, CTL_ArrayType>, 12> mapTagToArrayType = { {
                {arrayTag::LongType,                            CTL_ArrayIntType},
                {arrayTag::Long64Type,                          CTL_ArrayInt64Type},
                {arrayTag::DoubleType,                          CTL_ArrayDoubleType},
                {arrayTag::VoidPtrType,                         CTL_ArrayHandleType},
                {arrayTag::VoidPtrType,                         CTL_ArrayPtrType},
                {arrayTag::StringType,                          CTL_ArrayANSIStringType},
                {arrayTag::WStringType,                         CTL_ArrayWideStringType},
                {arrayTag::FrameType,                           CTL_ArrayDTWAINFrameType},
                {arrayTag::Fix32Type,                           CTL_ArrayTWFIX32Type},
                {arrayTag::SourceType,                          CTL_ArraySourceType},
                {arrayTag::ArrayOfArrayOfVoidPtrType,           CTL_ArrayToHandleArray},
                {arrayTag::FrameSingleType,                     CTL_ArrayDTWAINFrameType}}};

            auto iter = dynarithmic::generic_array_finder_if(mapTagToArrayType, [&](const auto& pr) { return pr.first == tag; });
            if (iter.first)
                return mapTagToArrayType[iter.second].second;
            return CTL_ArrayInvalid;
        }
    };
}
#endif
