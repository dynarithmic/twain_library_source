#ifndef DTWAIN_ANYUTILS_H
#define DTWAIN_ANYUTILS_H

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
    #include <any>
    using anytype_ = std::any;
    #define ANYTYPE_NAMESPACE std::
#else
    #pragma warning (push)
    #pragma warning (disable : 4714)
    #include <boost/any.hpp>
    using anytype_ = boost::any;
    #define ANYTYPE_NAMESPACE boost::
#pragma warning (pop)
#endif
#endif
