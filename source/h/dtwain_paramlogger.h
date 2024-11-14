#ifndef DTWAIN_PARAMLOGGER_H
#define DTWAIN_PARAMLOGGER_H

#if DTWAIN_BUILD_LOGCALLSTACK == 1
#include <string>
#include <sstream>
#include "dtwaindefs.h"
#include "dtwain_resource_constants2.h"
namespace dynarithmic
{
    class ParamOutputter2
    {
        int nWhich;
        std::string argNames;
        std::ostringstream strm;
        bool m_bIsReturnValue;
        int nArgs = 0;

    private:
        void LogInputType(std::string outStr, const char* ptr)
        {
            // ptr is a valid string supplied by the user, so just write it out
            if (ptr)
                strm << outStr << "=" << ptr;
            else
                strm << outStr << "=" << "(null)";
        }

        void LogInputType(std::string outStr, char* ptr)
        {
            // ptr is a valid string supplied by the user, but we can't ensure it is null terminated
            // (It doesn't have to be null-terminated, as the DTWAIN function will eventually put the NULL
            //  terminated value into the output string).
            // So for now, we just output the pointer value of the string
            strm << outStr << "=" << static_cast<void*>(ptr);
        }

        void LogInputType(std::string outStr, wchar_t* ptr)
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
    std::string LogValue(std::string func, bool isIn, T retValue, P ...p)
    {
        std::string s;
        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_CALLSTACK)
        {
            if (isIn)
                s = CTL_LogFunctionCallA(func.c_str(), LOG_INDENT_IN) + ParamOutputter2(false, std::forward<P>(p)...).getString();
            else
                s = CTL_LogFunctionCallA(func.c_str(), LOG_INDENT_OUT) + ParamOutputter2(true, retValue).getString();
            LogWriterUtils::WriteLogInfoA(s);
        }
        return s;
    }
}
#endif
#endif