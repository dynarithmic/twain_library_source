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

    For more information, the license file LICENSE.TXT that is located in the root
    directory of the DTWAIN installation covers the restrictions under the LGPL license.
    Please read this file before deploying or distributing any application using DTWAIN.
 */
#ifndef DTWAIN_RAII_H
#define DTWAIN_RAII_H

namespace dynarithmic
{
    template <typename T, typename DestroyTraits>
    struct RAII_Thrower
    {
        static void RAII_Destroy(T m_a)
        {
            try
            {
                DestroyTraits::Destroy(m_a);
            }
            catch (...)
            {
                LogExceptionErrorA("RAII failure");
            }
        }
    };

    template <typename T, typename DestroyTraits>
    struct RAII_NoThrower
    {
        static void RAII_Destroy(T m_a)
        {
            DestroyTraits::Destroy(m_a);
        }
    };

    template <typename T, typename DestroyTraits, bool canThrow=true>
    struct DTWAIN_RAII
    {
        T m_a;
        DTWAIN_RAII(T a = T()) : m_a(a) {}
        void SetObject(T a) { m_a = a; }
        void Disconnect() { m_a = T(); }
        ~DTWAIN_RAII()
        {
            canThrow?RAII_Thrower<T, DestroyTraits>::RAII_Destroy(m_a):
                     RAII_NoThrower<T, DestroyTraits>::RAII_Destroy(m_a);
        }
    };
}
#endif