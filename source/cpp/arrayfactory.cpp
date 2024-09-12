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

#include "arrayfactory.h"
#include "dtwaindefs.h"
#include "dtwtype.h"
#include "ctlobstr.h"
#include "ctliface.h"

namespace dynarithmic
{
    template <typename ArraySourceT, typename ConversionFunc>
    static void ArrayToNativeArray(CTL_TwainDLLHandle* pHandle, 
                                   DTWAIN_ARRAY ArraySource,
                                   DTWAIN_ARRAY ArrayDest,
                                   int ArraySourceType,
                                   ConversionFunc fn)
    {
        const auto& factory = pHandle->m_ArrayFactory;
        const auto TypeSource = factory->tag_type(CTL_ArrayFactory::from_void(ArraySource));
        const auto TypeDest = factory->tag_type(CTL_ArrayFactory::from_void(ArrayDest));
        if (TypeSource != ArraySourceType)
            return;
        if (!(TypeDest == CTL_ArrayFactory::arrayTag::StringType || TypeDest == CTL_ArrayFactory::arrayTag::WStringType))
            return;
        const auto theSourceTag = static_cast<CTL_ArrayFactory::arrayTag*>(ArraySource);
        const auto theDestTag = static_cast<CTL_ArrayFactory::arrayTag*>(ArrayDest);
        auto& vSource = factory->underlying_container_t<typename ArraySourceT::value_type>(theSourceTag);
        for (auto& str : vSource)
        {
            CTL_StringType sVal;
            sVal = fn(str);
            factory->add_to_back(theDestTag, &sVal, 1);
        }
    }

    void ArrayCopyWideToNative(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY ArraySource, DTWAIN_ARRAY ArrayDest)
    {
        ArrayToNativeArray<CTL_ArrayFactory::tagged_array_wstring>(pHandle, ArraySource, ArrayDest, CTL_ArrayFactory::arrayTag::WStringType,
            [](const std::wstring& val) { return StringConversion::Convert_Wide_To_Native(val); });
    }

    void ArrayCopyAnsiToNative(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY ArraySource, DTWAIN_ARRAY ArrayDest)
    {
        ArrayToNativeArray<CTL_ArrayFactory::tagged_array_string>(pHandle, ArraySource, ArrayDest, CTL_ArrayFactory::arrayTag::StringType,
            [](const std::string& val) { return StringConversion::Convert_Ansi_To_Native(val); });
    }

    CTL_ArrayFactory::CTL_ArrayFactory()
    {
        m_vfnAddMap.insert({ arrayTag::LongType, [&](arrayTag* tag, std::size_t nSize, void* value) { simple_pushback<tagged_array_long>(tag, value, nSize); } });
        m_vfnAddMap.insert({ arrayTag::Long64Type, [&](arrayTag* tag, std::size_t nSize, void* value) { simple_pushback<tagged_array_long64>(tag, value, nSize); } });
        m_vfnAddMap.insert({ arrayTag::DoubleType, [&](arrayTag* tag, std::size_t nSize,void* value) { simple_pushback<tagged_array_double>(tag, value, nSize); } });
        m_vfnAddMap.insert({ arrayTag::StringType, [&](arrayTag* tag, std::size_t nSize, void* value) { string_pushback<tagged_array_string>(tag, value, nSize); } });
        m_vfnAddMap.insert({ arrayTag::WStringType, [&](arrayTag* tag, std::size_t nSize, void* value) { string_pushback<tagged_array_wstring>(tag, value, nSize); } });
        m_vfnAddMap.insert({ arrayTag::SourceType, [&](arrayTag* tag, std::size_t nSize, void* value) { pointer_pushback<tagged_array_sourceptr>(tag, value, nSize); } });
        m_vfnAddMap.insert({ arrayTag::VoidPtrType, [&](arrayTag* tag, std::size_t nSize, void* value) { pointer_pushback<tagged_array_voidptr>(tag, value, nSize); } });
        m_vfnAddMap.insert({ arrayTag::ArrayOfArrayOfVoidPtrType, [&](arrayTag* tag, std::size_t nSize, void* value) { pointer_pushback<tagged_array_tagged_array_voidptr>(tag, value, nSize); } });
        m_vfnAddMap.insert({ arrayTag::FrameType, [&](arrayTag* tag, std::size_t nSize, void* value) { simple_pushback<tagged_array_frame>(tag, value, nSize); } });
        m_vfnAddMap.insert({ arrayTag::TWFrameType, [&](arrayTag* tag, std::size_t nSize, void* value) { simple_pushback<tagged_array_twframe>(tag, value, nSize); } });
        m_vfnAddMap.insert({ arrayTag::Fix32Type, [&](arrayTag* tag, std::size_t nSize, void* value) { simple_pushback<tagged_array_fix32>(tag, value, nSize); } });

        m_vfnGetMap.insert({ arrayTag::LongType, [&](arrayTag* tag, std::size_t nWhere, void* value) { return simple_getter<tagged_array_long>(tag, nWhere, value); } });
        m_vfnGetMap.insert({ arrayTag::Long64Type, [&](arrayTag* tag, std::size_t nWhere, void* value) { return simple_getter<tagged_array_long64>(tag, nWhere, value); } });
        m_vfnGetMap.insert({ arrayTag::DoubleType, [&](arrayTag* tag, std::size_t nWhere, void* value) { return simple_getter<tagged_array_double>(tag, nWhere, value); } });
        m_vfnGetMap.insert({ arrayTag::StringType, [&](arrayTag* tag, std::size_t nWhere, void* value) { return simple_getter<tagged_array_string>(tag, nWhere, value); } });
        m_vfnGetMap.insert({ arrayTag::WStringType, [&](arrayTag* tag, std::size_t nWhere, void* value) { return simple_getter<tagged_array_wstring>(tag, nWhere, value); } });
        m_vfnGetMap.insert({ arrayTag::SourceType, [&](arrayTag* tag, std::size_t nWhere, void* value) { return pointer_getter<tagged_array_sourceptr>(tag, nWhere, value); } });
        m_vfnGetMap.insert({ arrayTag::VoidPtrType, [&](arrayTag* tag, std::size_t nWhere, void* value) { return pointer_getter<tagged_array_voidptr>(tag, nWhere, value); } });
        m_vfnGetMap.insert({ arrayTag::ArrayOfArrayOfVoidPtrType, [&](arrayTag* tag, std::size_t nWhere, void* value) { return pointer_getter<tagged_array_tagged_array_voidptr>(tag, nWhere, value); } });
        m_vfnGetMap.insert({ arrayTag::FrameType, [&](arrayTag* tag, std::size_t nWhere, void* value) { return simple_getter<tagged_array_frame>(tag, nWhere, value); } });
        m_vfnGetMap.insert({ arrayTag::TWFrameType, [&](arrayTag* tag, std::size_t nWhere, void* value) { return simple_getter<tagged_array_twframe>(tag, nWhere, value); } });
        m_vfnGetMap.insert({ arrayTag::Fix32Type, [&](arrayTag* tag, std::size_t nWhere, void* value) { return simple_getter<tagged_array_fix32>(tag, nWhere, value); } });

        m_vfnFindMap.insert({ arrayTag::LongType, [&](arrayTag* tag, void* value, double) { return simple_finder<tagged_array_long>(tag, value, {}, simple_finder_t()); } });
        m_vfnFindMap.insert({ arrayTag::Long64Type, [&](arrayTag* tag, void* value, double) { return simple_finder<tagged_array_long64>(tag, value, {}, simple_finder_t()); } });
        m_vfnFindMap.insert({ arrayTag::DoubleType, [&](arrayTag* tag, void* value, double val) { return simple_finder<tagged_array_double>(tag, value, {}, double_finder_t(val)); } });
        m_vfnFindMap.insert({ arrayTag::StringType, [&](arrayTag* tag, void* value, double) { return simple_finder<tagged_array_string>(tag, value, {}, simple_finder_t()); } });
        m_vfnFindMap.insert({ arrayTag::WStringType, [&](arrayTag* tag, void* value, double) { return simple_finder<tagged_array_wstring>(tag, value, {}, simple_finder_t()); } });
        m_vfnFindMap.insert({ arrayTag::SourceType, [&](arrayTag* tag, void* value, double) { return simple_finder<tagged_array_sourceptr>(tag, value, {}, pointer_finder_t()); } });
        m_vfnFindMap.insert({ arrayTag::VoidPtrType, [&](arrayTag* tag, void* value, double) { return simple_finder<tagged_array_voidptr>(tag, value, {}, pointer_finder_t()); } });
        m_vfnFindMap.insert({ arrayTag::ArrayOfArrayOfVoidPtrType, [&](arrayTag* tag, void* value, double) { return simple_finder<tagged_array_tagged_array_voidptr>(tag, value, {}, pointer_finder_t()); } });
        m_vfnFindMap.insert({ arrayTag::FrameType, [&](arrayTag* tag, void* value, double) { return simple_finder<tagged_array_frame>(tag, value, {}, simple_finder_t()); } });
        m_vfnFindMap.insert({ arrayTag::TWFrameType, [&](arrayTag* tag, void* value, double) { return simple_finder<tagged_array_twframe>(tag, value, {}, frame_finder_t()); } });
        m_vfnFindMap.insert({ arrayTag::Fix32Type, [&](arrayTag* tag, void* value, double) { return simple_finder<tagged_array_fix32>(tag, value, {}, simple_finder_t()); } });

        m_vfnInserterMap.insert({ arrayTag::LongType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) { simple_inserter<tagged_array_long>(tag, nWhere, nCount, value); } });
        m_vfnInserterMap.insert({ arrayTag::Long64Type, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) { simple_inserter<tagged_array_long64>(tag, nWhere, nCount, value); } });
        m_vfnInserterMap.insert({ arrayTag::DoubleType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) { simple_inserter<tagged_array_double>(tag, nWhere, nCount, value); } });
        m_vfnInserterMap.insert({ arrayTag::StringType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) { string_inserter<tagged_array_string>(tag, nWhere, nCount, value); } });
        m_vfnInserterMap.insert({ arrayTag::WStringType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) { string_inserter<tagged_array_wstring>(tag, nWhere, nCount, value); } });
        m_vfnInserterMap.insert({ arrayTag::FrameType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) { simple_inserter<tagged_array_frame>(tag, nWhere, nCount, value); } });
        m_vfnInserterMap.insert({ arrayTag::TWFrameType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) { simple_inserter<tagged_array_twframe>(tag, nWhere, nCount, value); } });
        m_vfnInserterMap.insert({ arrayTag::Fix32Type, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) { simple_inserter<tagged_array_fix32>(tag, nWhere, nCount, value); } });
        m_vfnInserterMap.insert({ arrayTag::SourceType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) { pointer_inserter<tagged_array_sourceptr>(tag, nWhere, nCount, value); } });
        m_vfnInserterMap.insert({ arrayTag::VoidPtrType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) { pointer_inserter<tagged_array_voidptr>(tag, nWhere, nCount, value); } });
        m_vfnInserterMap.insert({ arrayTag::ArrayOfArrayOfVoidPtrType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount, void* value) { pointer_inserter<tagged_array_tagged_array_voidptr>(tag, nWhere, nCount, value); } });

        m_vfnCopierMap.insert({ arrayTag::LongType, [&](arrayTag* tagDest, arrayTag* tagSrc) { simple_copier<tagged_array_long>(tagDest, tagSrc); } });
        m_vfnCopierMap.insert({ arrayTag::Long64Type, [&](arrayTag* tagDest, arrayTag* tagSrc) { simple_copier<tagged_array_long64>(tagDest, tagSrc); } });
        m_vfnCopierMap.insert({ arrayTag::DoubleType, [&](arrayTag* tagDest, arrayTag* tagSrc) { simple_copier<tagged_array_double>(tagDest, tagSrc); } });
        m_vfnCopierMap.insert({ arrayTag::StringType, [&](arrayTag* tagDest, arrayTag* tagSrc) { simple_copier<tagged_array_string>(tagDest, tagSrc); } });
        m_vfnCopierMap.insert({ arrayTag::WStringType, [&](arrayTag* tagDest, arrayTag* tagSrc) { simple_copier<tagged_array_wstring>(tagDest, tagSrc); } });
        m_vfnCopierMap.insert({ arrayTag::FrameType, [&](arrayTag* tagDest, arrayTag* tagSrc) { simple_copier<tagged_array_frame>(tagDest, tagSrc); } });
        m_vfnCopierMap.insert({ arrayTag::TWFrameType, [&](arrayTag* tagDest, arrayTag* tagSrc) { simple_copier<tagged_array_twframe>(tagDest, tagSrc); } });
        m_vfnCopierMap.insert({ arrayTag::Fix32Type, [&](arrayTag* tagDest, arrayTag* tagSrc) { simple_copier<tagged_array_fix32>(tagDest, tagSrc); } });
        m_vfnCopierMap.insert({ arrayTag::SourceType, [&](arrayTag* tagDest, arrayTag* tagSrc) { simple_copier<tagged_array_sourceptr>(tagDest, tagSrc); } });
        m_vfnCopierMap.insert({ arrayTag::VoidPtrType, [&](arrayTag* tagDest, arrayTag* tagSrc) { simple_copier<tagged_array_voidptr>(tagDest, tagSrc); } });
        m_vfnCopierMap.insert({ arrayTag::ArrayOfArrayOfVoidPtrType, [&](arrayTag* tagDest, arrayTag* tagSrc) { simple_copier<tagged_array_tagged_array_voidptr>(tagDest, tagSrc); } });

        m_vfnRemoverMap.insert({ arrayTag::LongType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount) { simple_remover<tagged_array_long>(tag, nWhere, nCount); } });
        m_vfnRemoverMap.insert({ arrayTag::Long64Type, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount) { simple_remover<tagged_array_long64>(tag, nWhere, nCount); } });
        m_vfnRemoverMap.insert({ arrayTag::DoubleType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount) { simple_remover<tagged_array_double>(tag, nWhere, nCount); } });
        m_vfnRemoverMap.insert({ arrayTag::StringType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount) { simple_remover<tagged_array_string>(tag, nWhere, nCount); } });
        m_vfnRemoverMap.insert({ arrayTag::WStringType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount) { simple_remover<tagged_array_wstring>(tag, nWhere, nCount); } });
        m_vfnRemoverMap.insert({ arrayTag::FrameType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount) { simple_remover<tagged_array_frame>(tag, nWhere, nCount); } });
        m_vfnRemoverMap.insert({ arrayTag::TWFrameType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount) { simple_remover<tagged_array_twframe>(tag, nWhere, nCount); } });
        m_vfnRemoverMap.insert({ arrayTag::Fix32Type, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount) { simple_remover<tagged_array_fix32>(tag, nWhere, nCount); } });
        m_vfnRemoverMap.insert({ arrayTag::SourceType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount) { simple_remover<tagged_array_sourceptr>(tag, nWhere, nCount); } });
        m_vfnRemoverMap.insert({ arrayTag::VoidPtrType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount) { simple_remover<tagged_array_voidptr>(tag, nWhere, nCount); } });
        m_vfnRemoverMap.insert({ arrayTag::ArrayOfArrayOfVoidPtrType, [&](arrayTag* tag, std::size_t nWhere, std::size_t nCount) { simple_remover<tagged_array_tagged_array_voidptr>(tag, nWhere, nCount); } });

        m_vfnClearerMap.insert({ arrayTag::LongType, [&](arrayTag* tag) { simple_clearer<tagged_array_long>(tag); } });
        m_vfnClearerMap.insert({ arrayTag::Long64Type, [&](arrayTag* tag) { simple_clearer<tagged_array_long64>(tag); } });
        m_vfnClearerMap.insert({ arrayTag::DoubleType, [&](arrayTag* tag) { simple_clearer<tagged_array_double>(tag); } });
        m_vfnClearerMap.insert({ arrayTag::StringType, [&](arrayTag* tag) { simple_clearer<tagged_array_string>(tag); } });
        m_vfnClearerMap.insert({ arrayTag::WStringType, [&](arrayTag* tag) { simple_clearer<tagged_array_wstring>(tag); } });
        m_vfnClearerMap.insert({ arrayTag::FrameType, [&](arrayTag* tag) { simple_clearer<tagged_array_frame>(tag); } });
        m_vfnClearerMap.insert({ arrayTag::TWFrameType, [&](arrayTag* tag) { simple_clearer<tagged_array_twframe>(tag); } });
        m_vfnClearerMap.insert({ arrayTag::Fix32Type, [&](arrayTag* tag) { simple_clearer<tagged_array_fix32>(tag); } });
        m_vfnClearerMap.insert({ arrayTag::SourceType, [&](arrayTag* tag) { simple_clearer<tagged_array_sourceptr>(tag); } });
        m_vfnClearerMap.insert({ arrayTag::VoidPtrType, [&](arrayTag* tag) { simple_clearer<tagged_array_voidptr>(tag); } });
        m_vfnClearerMap.insert({ arrayTag::ArrayOfArrayOfVoidPtrType, [&](arrayTag* tag) { simple_clearer<tagged_array_tagged_array_voidptr>(tag); } });

        m_vfnResizerMap.insert({ arrayTag::LongType, [&](arrayTag* tag, std::size_t num) { simple_resizer<tagged_array_long>(tag, num); } });
        m_vfnResizerMap.insert({ arrayTag::Long64Type, [&](arrayTag* tag, std::size_t num) { simple_resizer<tagged_array_long64>(tag, num); } });
        m_vfnResizerMap.insert({ arrayTag::DoubleType, [&](arrayTag* tag, std::size_t num) { simple_resizer<tagged_array_double>(tag, num); } });
        m_vfnResizerMap.insert({ arrayTag::StringType, [&](arrayTag* tag, std::size_t num) { simple_resizer<tagged_array_string>(tag, num); } });
        m_vfnResizerMap.insert({ arrayTag::WStringType, [&](arrayTag* tag, std::size_t num) { simple_resizer<tagged_array_wstring>(tag, num); } });
        m_vfnResizerMap.insert({ arrayTag::FrameType, [&](arrayTag* tag, std::size_t num) { simple_resizer<tagged_array_frame>(tag, num); } });
        m_vfnResizerMap.insert({ arrayTag::TWFrameType, [&](arrayTag* tag, std::size_t num) { simple_resizer<tagged_array_twframe>(tag, num); } });
        m_vfnResizerMap.insert({ arrayTag::Fix32Type, [&](arrayTag* tag, std::size_t num) { simple_resizer<tagged_array_fix32>(tag, num); } });
        m_vfnResizerMap.insert({ arrayTag::SourceType, [&](arrayTag* tag, std::size_t num) { simple_resizer<tagged_array_sourceptr>(tag, num); } });
        m_vfnResizerMap.insert({ arrayTag::VoidPtrType, [&](arrayTag* tag, std::size_t num) { simple_resizer<tagged_array_voidptr>(tag, num); } });
        m_vfnResizerMap.insert({ arrayTag::ArrayOfArrayOfVoidPtrType, [&](arrayTag* tag, std::size_t num) { simple_resizer<tagged_array_tagged_array_voidptr>(tag, num); } });

        m_vfnCounterMap.insert({ arrayTag::LongType, [&](arrayTag* tag) { return simple_counter<tagged_array_long>(tag); } });
        m_vfnCounterMap.insert({ arrayTag::Long64Type, [&](arrayTag* tag) { return simple_counter<tagged_array_long64>(tag); } });
        m_vfnCounterMap.insert({ arrayTag::DoubleType, [&](arrayTag* tag) { return simple_counter<tagged_array_double>(tag); } });
        m_vfnCounterMap.insert({ arrayTag::StringType, [&](arrayTag* tag) { return simple_counter<tagged_array_string>(tag); } });
        m_vfnCounterMap.insert({ arrayTag::WStringType, [&](arrayTag* tag) { return simple_counter<tagged_array_wstring>(tag); } });
        m_vfnCounterMap.insert({ arrayTag::FrameType, [&](arrayTag* tag) { return simple_counter<tagged_array_frame>(tag); } });
        m_vfnCounterMap.insert({ arrayTag::TWFrameType, [&](arrayTag* tag) { return simple_counter<tagged_array_twframe>(tag); } });
        m_vfnCounterMap.insert({ arrayTag::Fix32Type, [&](arrayTag* tag) { return simple_counter<tagged_array_fix32>(tag); } });
        m_vfnCounterMap.insert({ arrayTag::SourceType, [&](arrayTag* tag) { return simple_counter<tagged_array_sourceptr>(tag); } });
        m_vfnCounterMap.insert({ arrayTag::VoidPtrType, [&](arrayTag* tag) { return simple_counter<tagged_array_voidptr>(tag); } });
        m_vfnCounterMap.insert({ arrayTag::ArrayOfArrayOfVoidPtrType, [&](arrayTag* tag) { return simple_counter<tagged_array_tagged_array_voidptr>(tag); } });

        m_vfnSetterMap.insert({ arrayTag::LongType, [&](arrayTag* tag, size_t nWhere, void* value) { simple_setter<tagged_array_long>(tag, nWhere, value); } });
        m_vfnSetterMap.insert({ arrayTag::Long64Type, [&](arrayTag* tag, size_t nWhere, void* value) { simple_setter<tagged_array_long64>(tag, nWhere, value); } });
        m_vfnSetterMap.insert({ arrayTag::DoubleType, [&](arrayTag* tag, size_t nWhere, void* value) { simple_setter<tagged_array_double>(tag, nWhere, value); } });
        m_vfnSetterMap.insert({ arrayTag::StringType, [&](arrayTag* tag, size_t nWhere, void* value) { string_setter<tagged_array_string>(tag, nWhere, value); } });
        m_vfnSetterMap.insert({ arrayTag::WStringType, [&](arrayTag* tag, size_t nWhere, void* value) { string_setter<tagged_array_wstring>(tag, nWhere, value); } });
        m_vfnSetterMap.insert({ arrayTag::FrameType, [&](arrayTag* tag, size_t nWhere, void* value) { simple_setter<tagged_array_frame>(tag, nWhere, value); } });
        m_vfnSetterMap.insert({ arrayTag::TWFrameType, [&](arrayTag* tag, size_t nWhere, void* value) { simple_setter<tagged_array_twframe>(tag, nWhere, value); } });
        m_vfnSetterMap.insert({ arrayTag::Fix32Type, [&](arrayTag* tag, size_t nWhere, void* value) { simple_setter<tagged_array_fix32>(tag, nWhere, value); } });
        m_vfnSetterMap.insert({ arrayTag::SourceType, [&](arrayTag* tag, size_t nWhere, void* value) { pointer_setter<tagged_array_sourceptr>(tag, nWhere, value); } });
        m_vfnSetterMap.insert({ arrayTag::VoidPtrType, [&](arrayTag* tag, size_t nWhere, void* value) { pointer_setter<tagged_array_voidptr>(tag, nWhere, value); } });
        m_vfnSetterMap.insert({ arrayTag::ArrayOfArrayOfVoidPtrType, [&](arrayTag* tag, size_t nWhere, void* value) { pointer_setter<tagged_array_tagged_array_voidptr>(tag, nWhere, value); } });

        m_vfnGetBufferMap.insert({ arrayTag::LongType, [&](arrayTag* tag, size_t nWhere) { return simple_buffer_getter<tagged_array_long>(tag, nWhere); } });
        m_vfnGetBufferMap.insert({ arrayTag::Long64Type, [&](arrayTag* tag, size_t nWhere) { return simple_buffer_getter<tagged_array_long64>(tag, nWhere); } });
        m_vfnGetBufferMap.insert({ arrayTag::DoubleType, [&](arrayTag* tag, size_t nWhere) { return simple_buffer_getter<tagged_array_double>(tag, nWhere); } });
        m_vfnGetBufferMap.insert({ arrayTag::StringType, [&](arrayTag* tag, size_t nWhere) { return simple_buffer_getter<tagged_array_string>(tag, nWhere); } });
        m_vfnGetBufferMap.insert({ arrayTag::WStringType, [&](arrayTag* tag, size_t nWhere) { return simple_buffer_getter<tagged_array_wstring>(tag, nWhere); } });
        m_vfnGetBufferMap.insert({ arrayTag::FrameType, [&](arrayTag* tag, size_t nWhere) { return simple_buffer_getter<tagged_array_frame>(tag, nWhere); } });
        m_vfnGetBufferMap.insert({ arrayTag::TWFrameType, [&](arrayTag* tag, size_t nWhere) { return simple_buffer_getter<tagged_array_twframe>(tag, nWhere); } });
        m_vfnGetBufferMap.insert({ arrayTag::Fix32Type, [&](arrayTag* tag, size_t nWhere) { return simple_buffer_getter<tagged_array_fix32>(tag, nWhere); } });
        m_vfnGetBufferMap.insert({ arrayTag::SourceType, [&](arrayTag* tag, size_t nWhere) { return simple_buffer_getter<tagged_array_sourceptr>(tag, nWhere); } });
        m_vfnGetBufferMap.insert({ arrayTag::VoidPtrType, [&](arrayTag* tag, size_t nWhere) { return simple_buffer_getter<tagged_array_voidptr>(tag, nWhere); } });
        m_vfnGetBufferMap.insert({ arrayTag::ArrayOfArrayOfVoidPtrType, [&](arrayTag* tag, size_t nWhere) { return simple_buffer_getter<tagged_array_tagged_array_voidptr>(tag, nWhere); } });
    }

    // return a new tag that corresponds to the array type
    CTL_ArrayFactory::arrayTag* CTL_ArrayFactory::create_array(CTL_ArrayType ArrayType, int* pStatus, size_t nInitialSize)
    {
        arrayTag* pNewArray = nullptr;
        if (pStatus)
            *pStatus = DTWAIN_NO_ERROR;

        if (ArrayType == CTL_ArrayStringType)
        {
#ifdef _UNICODE
            ArrayType = CTL_ArrayWideStringType;
#else
            ArrayType = CTL_ArrayANSIStringType;
#endif
        }
        switch (ArrayType)
        {
            case CTL_ArrayIntType:
                pNewArray = generic_array_creator<tagged_array_long>(arrayTag::LongType, nInitialSize);
                break;
            case CTL_ArrayInt64Type:
                pNewArray = generic_array_creator<tagged_array_long64>(arrayTag::Long64Type, nInitialSize);
                break;
            case CTL_ArrayDoubleType:
                pNewArray = generic_array_creator<tagged_array_double>(arrayTag::DoubleType, nInitialSize);
                break;
            case CTL_ArrayHandleType:
            case CTL_ArrayPtrType:
                pNewArray = generic_array_creator<tagged_array_voidptr>(arrayTag::VoidPtrType, nInitialSize);
                break;
            case CTL_ArrayANSIStringType:
                pNewArray = generic_array_creator<tagged_array_string>(arrayTag::StringType, nInitialSize);
                break;
            case CTL_ArrayWideStringType:
                pNewArray = generic_array_creator<tagged_array_wstring>(arrayTag::WStringType, nInitialSize);
                break;
            case CTL_ArrayDTWAINFrameType:
                pNewArray = generic_array_creator<tagged_array_frame>(arrayTag::FrameType, nInitialSize);
                break;
            case CTL_ArrayTWFIX32Type:
                pNewArray = generic_array_creator<tagged_array_fix32>(arrayTag::Fix32Type, nInitialSize);
                break;
            case CTL_ArraySourceType:
                pNewArray = generic_array_creator<tagged_array_sourceptr>(arrayTag::SourceType, nInitialSize);
                break;
            case CTL_ArrayToHandleArray:
                pNewArray = generic_array_creator<tagged_array_tagged_array_voidptr>(arrayTag::ArrayOfArrayOfVoidPtrType, nInitialSize);
                break;
            case CTL_ArrayFrameSingleType:
                pNewArray = generic_single_creator<tagged_array_frame>(arrayTag::FrameSingleType);
                break;
            default:
                if (pStatus)
                    *pStatus = DTWAIN_ERR_WRONG_ARRAY_TYPE;
                break;
        }
        return pNewArray;
    }

    bool CTL_ArrayFactory::is_valid(arrayTag* pTag) const
    {
        return m_tagMap.find(const_cast<arrayTag*>(pTag)) != m_tagMap.end();
    }

    bool CTL_ArrayFactory::is_frame_valid(const void *frame) const
    {
        const auto pTag = static_cast<const arrayTag*>(frame);
        const auto iter = m_tagMap.find(const_cast<arrayTag*>(pTag));
        if (iter == m_tagMap.end())
            return false;
        return pTag->getTag() == arrayTag::FrameSingleType;
    }

    bool CTL_ArrayFactory::is_valid(arrayTag* pTag, int tagType) const
    {
        if (is_valid(pTag))
            return pTag->getTag() == tagType;
        return false;
    }

    int CTL_ArrayFactory::tag_type(arrayTag * pTag) const
    {
        if (is_valid(pTag))
        {
            const auto pTagTemp = static_cast<arrayTag*>(pTag);
            return pTagTemp->getTag();
        }
        return -1;
    }

    void CTL_ArrayFactory::copy(arrayTag *pTagDest, arrayTag* pTagSource)
    {
        if (!is_valid(pTagDest) || !is_valid(pTagSource))
            return;
        if (pTagDest->getTag() != pTagSource->getTag())
            return;
        m_vfnCopierMap[pTagDest->getTag()](pTagDest, pTagSource);
    }

    void CTL_ArrayFactory::destroy(arrayTag *pTag)
    {
        if (!is_valid(pTag))
            return;
        const auto iter = m_tagMap.find(pTag);
        if (iter != m_tagMap.end())
            m_tagMap.erase(iter);
    }

    void CTL_ArrayFactory::add_to_back(arrayTag *pTag, void *value, size_t num)
    {
        if (!is_valid(pTag))
            return;
        m_vfnAddMap[pTag->getTag()](pTag, num, value);
    }

    void* CTL_ArrayFactory::get_value(arrayTag *pTag, size_t nWhere, void *value) const
    {
        if (!is_valid(pTag))
            return nullptr;
        const auto iter = m_vfnGetMap.find(pTag->getTag());
        return iter->second(pTag, nWhere, value);
    }

    void CTL_ArrayFactory::insert(arrayTag* pTag, void* value, size_t nWhere, size_t num)
    {
        if (!is_valid(pTag))
            return;
        m_vfnInserterMap[pTag->getTag()](pTag, nWhere, num, value);
    }

    void CTL_ArrayFactory::remove(arrayTag* pTag, std::size_t nWhere, std::size_t num)
    {
        if (!is_valid(pTag))
            return;
        m_vfnRemoverMap[pTag->getTag()](pTag, nWhere, num);
    }

    void CTL_ArrayFactory::clear(arrayTag *pTag)
    {
        if (!is_valid(pTag))
            return;
        m_vfnClearerMap[pTag->getTag()](pTag);
    }


    void CTL_ArrayFactory::resize(arrayTag* pTag, std::size_t num)
    {
        if (!is_valid(pTag))
            return;
        m_vfnResizerMap[pTag->getTag()](pTag, num);
    }

    size_t CTL_ArrayFactory::size(arrayTag* pTag) const
    {
        if (!is_valid(pTag))
            return 0;
        const auto iter = m_vfnCounterMap.find(pTag->getTag());
        return iter->second(pTag);
    }

    size_t CTL_ArrayFactory::find(arrayTag *pTag, void *value, double tol)
    {
        if (!is_valid(pTag))
            return 0;
        if (pTag->getTag() == arrayTag::DoubleType)
            return m_vfnFindMap[arrayTag::DoubleType](pTag, value, tol);
        return m_vfnFindMap[pTag->getTag()](pTag, value, {});
    }

    void CTL_ArrayFactory::set_value(arrayTag *pTag, std::size_t nWhere, void *value)
    {
        if (!is_valid(pTag))
            return;
        m_vfnSetterMap[pTag->getTag()](pTag, nWhere, value);
    }

    void* CTL_ArrayFactory::get_buffer(arrayTag *pTag, std::size_t nWhere)
    {
        if (!is_valid(pTag))
            return nullptr;
        return m_vfnGetBufferMap[pTag->getTag()](pTag, nWhere);
    }

    CTL_ArrayFactory::arrayTag* CTL_ArrayFactory::create_frame(double left, double top, double right, double bottom)
    {
        int status;
        const auto frame = create_array(CTL_ArrayFrameSingleType, &status, 1);
        auto& vect = underlying_container_t<TwainFrameInternal>(frame);
        auto& frameInst = vect.front();
        frameInst.m_FrameComponent[0] = left;
        frameInst.m_FrameComponent[1] = top;
        frameInst.m_FrameComponent[2] = right;
        frameInst.m_FrameComponent[3] = bottom;
        return frame;
    }

    void CTL_ArrayFactory::destroy_frame(arrayTag* frame)
    {
        destroy(frame);
    }

    int CTL_ArrayFactory::arraytype_to_tagtype(CTL_ArrayType arrayType)
    {
        static std::unordered_map<CTL_ArrayType, int> mapArrayTypeToTag = {
            {CTL_ArrayIntType, arrayTag::LongType},
            {CTL_ArrayInt64Type, arrayTag::Long64Type},
            {CTL_ArrayDoubleType, arrayTag::DoubleType},
            {CTL_ArrayHandleType,arrayTag::VoidPtrType},
            {CTL_ArrayPtrType, arrayTag::VoidPtrType},
            {CTL_ArrayANSIStringType, arrayTag::StringType},
            {CTL_ArrayWideStringType, arrayTag::WStringType},
            {CTL_ArrayDTWAINFrameType, arrayTag::FrameType},
            {CTL_ArrayTWFIX32Type, arrayTag::Fix32Type},
            {CTL_ArraySourceType, arrayTag::SourceType},
            {CTL_ArrayToHandleArray, arrayTag::ArrayOfArrayOfVoidPtrType},
            {CTL_ArrayFrameSingleType, arrayTag::FrameSingleType} };

        const auto iter = mapArrayTypeToTag.find(arrayType);
        if (iter != mapArrayTypeToTag.end())
            return iter->second;
        return arrayTag::UnknownType;
    }

    CTL_ArrayType CTL_ArrayFactory::tagtype_to_arraytype(int tag)
    {
        static std::unordered_map<int, CTL_ArrayType> mapTagToArrayType = {
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
            {arrayTag::FrameSingleType,                     CTL_ArrayFrameSingleType} };

        const auto iter = mapTagToArrayType.find(tag);
        if (iter != mapTagToArrayType.end())
            return iter->second;
        return CTL_ArrayInvalid;
    }
}