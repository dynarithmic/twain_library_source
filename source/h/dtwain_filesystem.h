#ifndef DTWAIN_FILESYSTEM_H
#define DTWAIN_FILESYSTEM_H

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
    #include <filesystem>
    namespace filesys = std::filesystem;
#else
    #error Must use a C++17 compliant compiler
#endif
#endif
