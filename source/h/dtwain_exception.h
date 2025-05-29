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
/*********************************************************************/

#ifndef DTWAIN_EXCEPTION_H
#define DTWAIN_EXCEPTION_H

#include <exception>


namespace dynarithmic
{
    class DTWAINException : public std::exception
    {
        int m_Exception = {};
        public:
            static constexpr const char* whatString = "{d5d26bc5-d30f-4c34-a80d-032ab23989ae}";
            DTWAINException(int r) noexcept : m_Exception(r) {}
            DTWAINException(void* ) noexcept : m_Exception{} {}
            int GetReturnException() const noexcept { return m_Exception; }
            void SetReturnException(int r) noexcept { m_Exception = r; }
            const char* what() const override { return whatString; }
    };

    template <typename T>
    T ProcessCatch(T val, const std::exception& ex_, const char* fn = nullptr)
    {
        if (ex_.what() != DTWAINException::whatString)
            LogExceptionErrorA(fn, true); 
        return val;
    }
}
#endif