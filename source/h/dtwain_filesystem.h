#ifndef DTWAIN_FILESYSTEM_H
#define DTWAIN_FILESYSTEM_H
#ifdef DTWAIN_USE_BOOST_FILESYSTEM
    #pragma warning (push)
    #pragma warning (disable : 4714)
    #include <boost/filesystem.hpp>
    namespace filesys = boost::filesystem;
    #pragma warning (pop)
#else
    #include <filesystem>
    namespace filesys = std::filesystem;
#endif
#endif
