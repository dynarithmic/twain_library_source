#ifndef DTWAIN_FILESYSTEM_H
#define DTWAIN_FILESYSTEM_H

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
    #include <filesystem>
    namespace filesys = std::filesystem;
#else
    #pragma warning (push)
    #pragma warning (disable : 4714)
    #include <boost/filesystem.hpp>
    namespace filesys = boost::filesystem;
    #pragma warning (pop)
#endif
#endif
