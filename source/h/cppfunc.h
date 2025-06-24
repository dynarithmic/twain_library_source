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
#ifndef CPPFUNC_H
#define CPPFUNC_H

#include "dtwain_config.h"
#include <funcmac.h>
#include "logwriterutils.h"
#include <boost/version.hpp>
#include "dtwain_exception.h"

 /* This indicates that the build requires Visual C++ runtime if set to 1*/
 /* Since there is no way to set the runtime to use at compile time, this */
 /* setting must match the link-time options */
#if DTWAIN_BUILD_NEEDVCRUNTIME == 0
    #ifdef _MSC_VER
        #pragma message ("Build does not require external Visual C++ runtime")
    #endif
#else
    #ifdef _MSC_VER
        #pragma message ("Build requires external Visual C++ runtime")
    #endif
#endif

#ifdef _MSC_VER
    #pragma message ("Building using Boost Library version " BOOST_LIB_VERSION)
#endif

#if (BOOST_VERSION / 100000) == 1
    #if (BOOST_VERSION / 100) % 1000 < 80
        #error "Error:  Boost library version must be version 1.80 or greater"
    #endif
#else
    #if (BOOST_VERSION / 100000) < 1
        #error "Error:  Boost library version must be version 1.80 or greater"
    #endif
#endif

#define NAG_FOR_LICENSE (0)

#define THROW_EXCEPTION \
    { if ( CTL_StaticData::IsThrowExceptions() )  DTWAIN_InternalThrowException(); }

     #define STRING_PARAM_LIST(x) _T(#x)

#if  DTWAIN_BUILD_LOGCALLSTACK == 1
    #ifdef _MSC_VER
        #pragma message ("Building DTWAIN with call stack logging")
    #endif
    #define TRY_BLOCK try {
    #define LOG_FUNC_STRING(x) \
        if ( CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_CALLSTACK) { \
        CTL_LogFunctionCallA(DTWAIN_LOG_CALLSTACK, (""), LOG_INDENT_CONSOLE, (#x)); \
        CTL_LogFunctionCallA(DTWAIN_LOG_CALLSTACK, (""), LOG_INDENT_OUT, (#x)); }

    #define LOG_FUNC_VALUES(x) \
        if ( CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_CALLSTACK) {\
        CTL_LogFunctionCallA(DTWAIN_LOG_CALLSTACK,(""), LOG_INDENT_CONSOLE, (x)); \
        CTL_LogFunctionCallA(DTWAIN_LOG_CALLSTACK,(""), LOG_INDENT_OUT, (#x)); }


    #define LOG_FUNC_ENTRY_PARAMS_ISTWAINMSG(argVals) \
        TRY_BLOCK \
        CTL_LogFunctionCallAndParamsIn(DTWAIN_LOG_ISTWAINMSG, FUNC_MACRO, #argVals, std::make_tuple argVals);

    #define LOG_FUNC_EXIT_PARAMS_ISTWAINMSG(x) { \
        CTL_LogFunctionCallAndParamsOut(DTWAIN_LOG_ISTWAINMSG, FUNC_MACRO, std::make_tuple(x)); \
        return(x); \
        }

    #define LOG_FUNC_ENTRY_PARAMS(argVals) \
        TRY_BLOCK \
        CTL_LogFunctionCallAndParamsIn(DTWAIN_LOG_CALLSTACK,FUNC_MACRO, #argVals, std::make_tuple argVals);

    #define LOG_FUNC_ENTRY_NONAME_PARAMS(...) \
        TRY_BLOCK LogValue(FUNC_MACRO, true, int(0), __VA_ARGS__);

    #if DTWAIN_BUILD_LOGPOINTERS == 1
    #ifdef _MSC_VER
        #pragma message ("Building DTWAIN with logging pointer dereferencing on return")
    #endif
        #define LOG_FUNC_EXIT_DEREFERENCE_POINTERS(argVals) \
            CTL_LogFunctionDerefParams(FUNC_MACRO, #argVals, std::make_tuple argVals);
    #else
        #ifdef _MSC_VER
            #pragma message ("Building DTWAIN with no logging pointer dereferencing on return")
        #endif
        #define LOG_FUNC_EXIT_DEREFERENCE_POINTERS(argVals) 
    #endif

    #define LOG_FUNC_EXIT_NONAME_PARAMS(x) { LogValue(FUNC_MACRO, false, x); return(x); }

    #define LOG_FUNC_EXIT_PARAMS(x) { \
        CTL_LogFunctionCallAndParamsOut(DTWAIN_LOG_CALLSTACK, FUNC_MACRO, std::make_tuple(x)); \
        return(x); \
            }

    #define LOG_FUNC_VALUES_EX(argvals) { \
        CTL_LogFunctionCallAndParamsIn(DTWAIN_LOG_CALLSTACK,FUNC_MACRO, #argvals, std::make_tuple argvals); \
    }

    #define CATCH_BLOCK_END }

    #define CATCH_BLOCK(type) \
            CATCH_BLOCK_END \
            catch(const std::exception& ex_) \
            {\
                return ProcessCatch(type, ex_, FUNC_MACRO); \
            }\
            catch(...) {\
                LogExceptionErrorA(FUNC_MACRO, true); \
                return(type); \
            }

    #define CATCH_BLOCK_LOG_PARAMS(type) \
            CATCH_BLOCK_END \
            catch(const std::exception& ex_) \
            {\
                ProcessCatch(type, ex_, FUNC_MACRO); \
                LOG_FUNC_EXIT_PARAMS(type) \
            }\
            catch(...) {\
                LogExceptionErrorA(FUNC_MACRO, true); \
                LOG_FUNC_EXIT_PARAMS(type) \
            }
#else
    #ifdef _MSC_VER
        #pragma message ("Building DTWAIN with minimal logging code")
    #endif

    #define CATCH_BLOCK_END }

    #define TRY_BLOCK try {
    #define LOG_FUNC_STRING(x)

    #define LOG_FUNC_VALUES(x)

    #define LOG_FUNC_ENTRY_PARAMS(argVals) TRY_BLOCK

    #define LOG_FUNC_ENTRY_PARAMS_NO_CHECK(argvals) TRY_BLOCK

    #define LOG_FUNC_ENTRY_PARAMS_ISTWAINMSG(x) TRY_BLOCK
    
    #define LOG_FUNC_ENTRY_NONAME_PARAMS(...) TRY_BLOCK

    #define LOG_FUNC_EXIT_PARAMS(x) { return(x); }

    #define LOG_FUNC_EXIT_NONAME_PARAMS(x) { return(x); }

    #define LOG_FUNC_EXIT_PARAMS_ISTWAINMSG(x) { return(x); }

    #define LOG_FUNC_VALUES_EX(argvals)

    #define LOG_FUNC_EXIT_DEREFERENCE_POINTERS(argVals) 

    #define CATCH_BLOCK(type) \
            CATCH_BLOCK_END \
            catch(const std::exception& ex_) \
            {\
                return ProcessCatch(type, ex_, FUNC_MACRO); \
            }\
            catch(...) \
            { \
                return(type); \
            }

    #define CATCH_BLOCK_LOG_PARAMS(type) CATCH_BLOCK(type)

#endif
#include "ctllogfunctioncall.h"
#endif

