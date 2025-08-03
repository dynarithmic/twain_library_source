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
#ifndef CTLLOGFUNCTIONCALL_H
#define CTLLOGFUNCTIONCALL_H

#include "dtwain_config.h"
#if DTWAIN_BUILD_LOGCALLSTACK == 1

#include <string.h>
#include <winconst.h>
#include <string>
#include "logwriterutils.h"
#include "ctlstringutils.h"
#include "dtwaindefs.h"
#include "dtwain_resource_constants2.h"
#include "ctlloadresources.h"
#include "ctllogcalls.h"

namespace dynarithmic
{
    // outputs parameter and return values
    class ParamOutputter
    {
    public:   
        StringArray aParamNames;
        size_t nWhich;
        std::string argNames;
        std::ostringstream strm;
        bool m_bIsReturnValue;
        bool m_bOutputAsString;

    private:
        void LogType(std::string_view outStr, const char* ptr)
        {
            // ptr must be a pointer to a valid null terminated string, or nullptr.
            if (ptr)
                strm << outStr << "=\"" << TruncateStringWithMore(ptr, 256)
                     << "\" (" << "0x" << std::hex << static_cast<const void*>(ptr) << ")" << std::dec;
            else
                strm << outStr << "=(null)";
        }

        void LogType(std::string_view outStr, const wchar_t* ptr)
        {
            // ptr must be a pointer to a valid null terminated string, or nullptr.
            if (ptr)
                strm << outStr << "=\"" << 
                TruncateStringWithMore(StringConversion::Convert_WidePtr_To_Ansi(ptr), 256) <<
                "\" (" << "0x" << std::hex << static_cast<const void*>(ptr) << ")" << std::dec;
            else
                strm << outStr << "=(null)";
        }

        template <typename T>
        void LogType(std::string_view outStr, const T* ptr)
        {
            // ptr is a valid pointer to double supplied by the user
            if (ptr)
            {
                if (!m_bOutputAsString)
                    strm << outStr << "=0x" << std::hex << static_cast<const void*>(ptr) << std::dec;
                else
                    strm << outStr << *ptr;
            }
            else
                strm << outStr << "=(null)";
        }

        template <typename T>
        void LogType(std::string_view outStr, T t, std::enable_if_t<std::is_pointer_v<T> >* = nullptr)
        {
            struct asStringRAII
            {
                bool* pAsString;
                bool orig;
                asStringRAII(bool* asString) : pAsString(asString), orig(*asString) {}
                ~asStringRAII() { *pAsString = orig; }
            };

            asStringRAII raii(&m_bOutputAsString);

            if constexpr (std::is_same_v<T, wchar_t*>)
            {
                if (m_bOutputAsString)
                    LogType(outStr, static_cast<const wchar_t*>(t));
                else
                if (t)
                    strm << outStr << "=0x" << std::hex << static_cast<void*>(t) << std::dec;
                else
                    strm << outStr << "=(null)";
            }
            else
            if constexpr (std::is_same_v<T, char*>)
            {
                if (m_bOutputAsString)
                    LogType(outStr, static_cast<const char*>(t));
                else
                if (t)
                    strm << outStr << "=0x" << std::hex << static_cast<void*>(t) << std::dec;
                else
                    strm << outStr << "=(null)";
            }
            else
            if constexpr (std::is_pointer_v<T> && !std::is_same_v<T, void*>
                && std::is_fundamental_v<std::remove_pointer_t<T>>)
            {
                if (m_bOutputAsString)
                {
                    if (t)
                        strm << outStr << "=" << *t;
                    else
                        strm << outStr << "=" << "(null)";
                }
                else
                {
                    if (t)
                        strm << outStr << "=0x" << std::hex << static_cast<void*>(t) << std::dec;
                    else
                        strm << outStr << "=" << "(null)";
                }
            }
            else
            if constexpr (std::is_pointer_v<T>)
            {
                if (t)
                    strm << outStr << "=0x" << std::hex << static_cast<const void*>(t) << std::dec;
                else
                    strm << outStr << "=" << "(null)";
            }
            else
                strm << outStr << "=" << t;
        }

        template <typename T>
        void LogType(std::string_view outStr, T t, std::enable_if_t<!std::is_pointer_v<T> >* = nullptr)
        {
            strm << outStr << "=" << t;
        }

    public:
        ParamOutputter(std::string_view s, bool isReturnValue = false) :
            nWhich(0), m_bIsReturnValue(isReturnValue), m_bOutputAsString(false)
        {
            StringWrapperA::Tokenize(s.data(), "(, )", aParamNames);
            if (!aParamNames.empty())
            {
                if (!m_bIsReturnValue)
                    strm << "(";
                else
                    strm << s << " " << dynarithmic::GetResourceStringFromMap(IDS_LOGMSG_RETURNTEXT) << " ";
            }
        }

        ParamOutputter& setOutputAsString(bool bSet) noexcept
        {
            m_bOutputAsString = bSet;
            return *this;
        }

        template <typename T>
        void appendToStream(size_t idx, T val)
        {
            if (aParamNames.empty() && !m_bIsReturnValue)
                return;
            const bool bIsNull = std::is_pointer_v<T> && !val;
            if (!m_bIsReturnValue)
            {
                // Make sure we log types correctly, especially character pointers.
                // User may supply to us a writable char buffer that is not null-terminated!
                LogType(aParamNames[idx], val);
            }
            else
            {
                if (bIsNull)
                    strm << "(null)";
                else
                {
                    if constexpr (std::is_same_v<wchar_t*, T> || std::is_same_v<const wchar_t*, T>)
                        strm << StringConversion::Convert_WidePtr_To_Ansi(val).c_str();
                    else
                        strm << val;
                }
            }
            if (!m_bIsReturnValue)
            {
                if (aParamNames.empty())
                    strm << ")";
                else
                if (idx < aParamNames.size() - 1)
                    strm << ", ";
                else
                    strm << ")";
            }
        }

        template <typename T, typename ...P>
        ParamOutputter& outputParam(T t, P ...p)
        {
            if (aParamNames.empty() && !m_bIsReturnValue)
                return *this;
            const bool bIsNull = std::is_pointer_v<T> && !t;
            if (!m_bIsReturnValue)
            {
                // Make sure we log types correctly, especially character pointers.
                // User may supply to us a writable char buffer that is not null-terminated!
                LogType(aParamNames[nWhich], t);
            }
            else
            {
                if (bIsNull)
                    strm << "(null)";
                else
                {
                    if constexpr (std::is_same_v<wchar_t*, T> || std::is_same_v<const wchar_t*, T>)
                        strm << StringConversion::Convert_WidePtr_To_Ansi(t).c_str();
                    else
                        strm << t;
                }
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

        template <typename TupleT>
        std::string processTupleArguments(const TupleT& tp, std::string_view paramList, bool IsIn, bool IsDeref)
        {
            if (!IsIn)
            {
                setOutputAsString(true);
                aParamNames.clear();
            }
            if (IsDeref)
            {
                setOutputAsString(true);
            }
            std::apply([&](const auto&... tupleArgs)
                {
                    size_t index = 0;
                    auto printElem = [&](const auto& x)
                    {
                        appendToStream(index, x);
                        ++index;
                    };

                    (printElem(tupleArgs), ...);
                }, tp);
            return getString();
        }
    };

    class ParamOutputter2
    {
        int nWhich;
        std::string argNames;
        std::ostringstream strm;
        bool m_bIsReturnValue;
        int nArgs = 0;

    private:
        void LogInputType(std::string_view outStr, const char* ptr)
        {
            // ptr is a valid string supplied by the user, so just write it out
            if (ptr)
                strm << outStr << "=" << ptr;
            else
                strm << outStr << "=" << "(null)";
        }

        void LogInputType(std::string_view outStr, char* ptr)
        {
            // ptr is a valid string supplied by the user, but we can't ensure it is null terminated
            // (It doesn't have to be null-terminated, as the DTWAIN function will eventually put the NULL
            //  terminated value into the output string).
            // So for now, we just output the pointer value of the string
            strm << outStr << "=" << static_cast<void*>(ptr);
        }

        void LogInputType(std::string_view outStr, wchar_t* ptr)
        {
            // ptr is a valid string supplied by the user, but we can't ensure it is null terminated
            // (It doesn't have to be null-terminated, as the DTWAIN function will eventually put the NULL
            //  terminated value into the output string).
            // So for now, we just output the pointer value of the string
            strm << outStr << "=" << static_cast<void*>(ptr);
        }

        template <typename T>
        void LogInputType(std::string_view outStr, T t, std::enable_if_t<std::is_pointer_v<T> >* = nullptr)
        {
            if (t)
            {
                if constexpr (std::is_same_v<wchar_t*, T> || std::is_same_v<const wchar_t*, T>)
                    strm << StringConversion::Convert_WidePtr_To_Ansi(t).c_str();
                else
                    strm << outStr << "=" << t;
            }
            else
                strm << outStr << "=" << "(null)";
        }

        template <typename T>
        void LogInputType(std::string_view outStr, T t, std::enable_if_t<!std::is_pointer_v<T> >* = nullptr)
        {
            strm << outStr << "=" << t;
        }

    public:
        template<typename ...Args>
        ParamOutputter2(bool bIsReturnValue, Args&&... theArgs) : nWhich(0), m_bIsReturnValue(bIsReturnValue)
        {
            nArgs = sizeof...(theArgs);
            if (!m_bIsReturnValue)
                strm << "(";
            else
                strm << " " << dynarithmic::GetResourceStringFromMap(IDS_LOGMSG_RETURNTEXT) << " ";
            outputParam(std::forward<Args>(theArgs)...);
        }

        template <typename T, typename ...P>
        ParamOutputter2& outputParam(T t, P ...p)
        {
            const bool bIsNull = std::is_pointer_v<T> && !t;
            if (!m_bIsReturnValue)
            {
                // Make sure we log input types correctly, especially character pointers.
                // User may supply to us a writable char buffer that is not null-terminated!
                LogInputType("param_" + std::to_string(nWhich + 1), t);
            }
            else
            {
                if (bIsNull)
                    strm << "(null)";
                else
                if constexpr (std::is_same_v<wchar_t*, T> || std::is_same_v<const wchar_t*, T>)
                    strm << StringConversion::Convert_WidePtr_To_Ansi(t).c_str();
                else
                    strm << t;
            }
            if (!m_bIsReturnValue)
            {
                if (nWhich < nArgs - 1)
                    strm << ", ";
                else
                    strm << ")";
            }
            ++nWhich;
            if (sizeof...(p))
                outputParam(p...);
            return *this;
        }

        ParamOutputter2& outputParam()
        {
            strm << ")"; return *this;
        }

        std::string getString() const { return strm.str(); }
    };

    template <typename T, typename ...P>
    std::string LogValue(std::string_view func, bool isIn, T retValue, P ...p)
    {
        std::string s;
        if (GetLogFilterFlags() & DTWAIN_LOG_CALLSTACK)
        {
            if (isIn)
                s = CTL_LogFunctionCallA(DTWAIN_LOG_CALLSTACK, func.data(), LOG_INDENT_IN) + ParamOutputter2(false, std::forward<P>(p)...).getString();
            else
                s = CTL_LogFunctionCallA(DTWAIN_LOG_CALLSTACK, func.data(), LOG_INDENT_OUT) + ParamOutputter2(true, retValue).getString();
            LogWriterUtils::WriteLogInfoA(s);
        }
        return s;
    }

    template <typename TupleType>
    std::string CTL_LogFunctionCallAndParamsIn_String(LONG filterFlags, LPCSTR pFuncName, std::string_view paramList, TupleType theArgs)
    {
        if (dynarithmic::GetLogFilterFlags() & filterFlags)
        {
            dynarithmic::ParamOutputter pm(paramList);
            return dynarithmic::CTL_LogFunctionCallA(DTWAIN_LOG_CALLSTACK, pFuncName, LOG_INDENT_IN) +
                                                     pm.processTupleArguments(theArgs, paramList, true, false); 
        }
        return {};
    }

    template <typename TupleType>
    void CTL_LogFunctionCallAndParamsIn(LONG filterFlags, LPCSTR pFuncName, std::string_view paramList, TupleType theArgs)
    {
        if (dynarithmic::GetLogFilterFlags() & filterFlags)
            dynarithmic::LogWriterUtils::WriteLogInfoA(CTL_LogFunctionCallAndParamsIn_String(filterFlags, pFuncName, paramList, theArgs));
    }

    template <typename TupleType>
    std::string CTL_LogFunctionCallAndParamsOut_String(LONG filterFlags, LPCSTR pFuncName, TupleType theArgs)
    {
        if (dynarithmic::GetLogFilterFlags() & filterFlags)
        {
            dynarithmic::ParamOutputter pm("", true);
            return dynarithmic::CTL_LogFunctionCallA(DTWAIN_LOG_CALLSTACK, pFuncName, LOG_INDENT_OUT) +
                                                    pm.processTupleArguments(theArgs, "", false, false);
        }
        return {};
    }

    template <typename TupleType>
    void CTL_LogFunctionCallAndParamsOut(LONG filterFlags, LPCSTR pFuncName, TupleType theArgs)
    {
        if (dynarithmic::GetLogFilterFlags() & filterFlags)
            dynarithmic::LogWriterUtils::WriteLogInfoA(CTL_LogFunctionCallAndParamsOut_String(filterFlags, pFuncName, theArgs));
    }

    template <typename TupleType>
    void CTL_LogFunctionDerefParams(LPCSTR pFuncName, std::string_view paramList, TupleType theArgs)
    {
        if (dynarithmic::GetLogFilterFlags() & DTWAIN_LOG_CALLSTACK)
        {
            dynarithmic::ParamOutputter pm(paramList);
            std::string s = dynarithmic::CTL_LogFunctionCallA(DTWAIN_LOG_CALLSTACK, pFuncName, LOG_INDENT_USELAST) +
                            pm.processTupleArguments(theArgs, paramList, true, true); 
            dynarithmic::LogWriterUtils::WriteLogInfoA(s);
        }
    }
}
#endif
#endif