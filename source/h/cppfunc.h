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
#ifndef CPPFUNC_H
#define CPPFUNC_H

#include "dtwain_retail_def.h"
#include <funcmac.h>
#define LOG_INDENT_CONSOLE 0
#define LOG_NO_INDENT   1
#define LOG_INDENT_IN   2
#define LOG_INDENT_OUT  3

#define NAG_FOR_LICENSE (0)

#define THROW_EXCEPTION \
    { if ( CTL_StaticData::s_bThrowExceptions )  DTWAIN_InternalThrowException(); }

     #define STRING_PARAM_LIST(x) _T(#x)

#ifndef DTWAIN_LEAN_AND_MEAN
    #ifndef DTWAIN_NO_LOGGING
        #define TRY_BLOCK try {
        #define LOG_FUNC_STRING(x) \
            if ( CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_CALLSTACK) { \
            CTL_LogFunctionCallA((""), LOG_INDENT_CONSOLE, (#x)); \
            CTL_LogFunctionCallA((""), LOG_INDENT_OUT, (#x)); }

        #define LOG_FUNC_VALUES(x) \
            if ( CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_CALLSTACK) {\
            CTL_LogFunctionCallA((""), LOG_INDENT_CONSOLE, (x)); \
            CTL_LogFunctionCallA((""), LOG_INDENT_OUT, (#x)); }


        #define LOG_FUNC_ENTRY_PARAMS_ISTWAINMSG(argVals) \
            TRY_BLOCK \
            if ((CTL_StaticData::s_lErrorFilterFlags & (DTWAIN_LOG_CALLSTACK | DTWAIN_LOG_ISTWAINMSG)) == \
                    (DTWAIN_LOG_CALLSTACK | DTWAIN_LOG_ISTWAINMSG)) \
            CTL_TwainAppMgr::WriteLogInfoA(CTL_LogFunctionCallA(FUNC_MACRO,LOG_INDENT_IN) + ParamOutputter((#argVals)).outputParam argVals.getString());

        #define LOG_FUNC_EXIT_PARAMS_ISTWAINMSG(x) { \
            if ((CTL_StaticData::s_lErrorFilterFlags & (DTWAIN_LOG_CALLSTACK | DTWAIN_LOG_ISTWAINMSG)) == \
                    (DTWAIN_LOG_CALLSTACK | DTWAIN_LOG_ISTWAINMSG)) \
            CTL_TwainAppMgr::WriteLogInfoA(CTL_LogFunctionCallA(FUNC_MACRO, LOG_INDENT_OUT) + ParamOutputter((""), true).outputParam(x).getString()); \
            return(x); \
                }

        #define LOG_FUNC_ENTRY_PARAMS(argVals) \
            TRY_BLOCK \
            if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_CALLSTACK) \
            CTL_TwainAppMgr::WriteLogInfoA(CTL_LogFunctionCallA(FUNC_MACRO,LOG_INDENT_IN) + ParamOutputter((#argVals)).outputParam argVals.getString());

        #define LOG_FUNC_ENTRY_NONAME_PARAMS(...) \
            TRY_BLOCK LogValue(FUNC_MACRO, true, int(0), __VA_ARGS__);

        #define LOG_FUNC_EXIT_NONAME_PARAMS(x) { LogValue(FUNC_MACRO, false, x); return(x); }

        #define LOG_FUNC_EXIT_PARAMS(x) { \
            if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_CALLSTACK) \
            CTL_TwainAppMgr::WriteLogInfoA(CTL_LogFunctionCallA(FUNC_MACRO, LOG_INDENT_OUT) + ParamOutputter((""), true).outputParam(x).getString()); \
            return(x); \
                }

        #define LOG_FUNC_VALUES_EX(argvals) { \
            if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_CALLSTACK) \
            CTL_TwainAppMgr::WriteLogInfoA(CTL_LogFunctionCallA((""),LOG_INDENT_IN) + ParamOutputter((#argvals)).outputParam argvals.getString()); \
        }

        #define CATCH_BLOCK_END }

        #define CATCH_BLOCK(type) \
                CATCH_BLOCK_END \
                catch(const std::exception& ex_) \
                {\
                    LogExceptionErrorA(FUNC_MACRO, ex_.what()); \
                    THROW_EXCEPTION \
                    return(type); \
                }\
                catch(const decltype(type) var) { return var; }\
                catch(...) {\
                LogExceptionErrorA(FUNC_MACRO); \
                THROW_EXCEPTION \
                return(type); \
                }

        #define CATCH_BLOCK_LOG_PARAMS(type) \
                CATCH_BLOCK_END \
                catch(const std::exception& ex_) \
                {\
                    LogExceptionErrorA(FUNC_MACRO, ex_.what()); \
                    LOG_FUNC_EXIT_NONAME_PARAMS(type) \
                    THROW_EXCEPTION \
                    return(type); \
                }\
                catch(const decltype(type) var) { \
                    LOG_FUNC_EXIT_NONAME_PARAMS(type) \
                    return var; }\
                catch(...) {\
                    LogExceptionErrorA(FUNC_MACRO); \
                    LOG_FUNC_EXIT_NONAME_PARAMS(type) \
                    THROW_EXCEPTION \
                    return(type); \
                }
        #else
            #define CATCH_BLOCK_END }

            #define TRY_BLOCK try {
            #define LOG_FUNC_STRING(x)

            #define LOG_FUNC_VALUES(x)

            #define LOG_FUNC_ENTRY_PARAMS(argVals) TRY_BLOCK

            #define LOG_FUNC_ENTRY_PARAMS_NO_CHECK(argvals) TRY_BLOCK

            #define LOG_FUNC_ENTRY_PARAMS_ISTWAINMSG(x) TRY_BLOCK

            #define LOG_FUNC_EXIT_NONAME_PARAMS(x) { return(x); }

            #define LOG_FUNC_EXIT_PARAMS_ISTWAINMSG(x) { return(x); }

            #define LOG_FUNC_VALUES_EX(argvals)

            #define CATCH_BLOCK(type) \
                CATCH_BLOCK_END \
                catch(decltype(type) var) { return var; }\
                catch(...) {\
                THROW_EXCEPTION \
                return(type); \
                }
        #endif
#else
    #pragma message("Building DTWAIN without callstack logging...")
    #define TRY_BLOCK try {

    #define LOG_FUNC_ENTRY_PARAMS(x)

    #define LOG_FUNC_STRING(x)

    #define LOG_FUNC_VALUES(x)

    #define LOG_FUNC_ENTRY_PARAMS_ISTWAINMSG(x) 

    #define LOG_FUNC_VALUES_EX(x)

    #define LOG_FUNC_EXIT_NONAME_PARAMS(x) { return (x); }

    #define LOG_FUNC_EXIT_PARAMS_ISTWAINMSG(x) { return(x); }

    #define CATCH_BLOCK(type) \
                } \
                catch(decltype(type) var) { return var; }\
                catch(...) {\
                THROW_EXCEPTION \
                return(type); \
                }

    #define LOG_FUNC_ENTRY_PARAMS_NO_CHECK(argvals)
#endif
#include "dtwain_paramlogger.h"
#endif

