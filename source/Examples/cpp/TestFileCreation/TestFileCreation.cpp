// TestFileCreation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include "dtwain.h"

void TestSingleFile(std::string outDir);
void TestMultiFile(std::string outDir);
void TestArrays();

int main(int argc, char* argv[])
{
    int value = 0;
    if (argc < 3)
    {
        std::cout << "Usage: TestImageFileCreation test-to-run[1,2,3] outout-directory\n\n";
        std::cout << "Example:\n    TestImageFileCreation 1 c:\\saved_images";
        return 0;
    }

    try
    {
        value = std::stoi(argv[1]);
    }
    catch (std::exception& e)
    {
        std::cout << "Error: " << e.what();
        return -1;
    }

    std::string outDir = argv[2];
    if (outDir.back() != '\\')
        outDir.push_back('\\');

    std::string testFile = outDir + "testDir";
    std::ofstream ofs(testFile.c_str());
    if (ofs)
    {
        ofs.close();
        ::remove(testFile.c_str());
    }
    else
    {
        std::cout << "Could not open test file " << testFile;
        return -2;
    }

    auto handle = DTWAIN_SysInitialize();
    if (!handle)
    {
        std::cout << "Could not initialize the DTWAIN library";
        return -3;
    }

    // Test array stuff
    TestArrays();
    switch (value)
    {
    case 1:
        TestSingleFile(outDir);
        break;
    case 2:
        TestMultiFile(outDir);
        break;
    default:
        TestSingleFile(outDir);
        TestMultiFile(outDir);
        break;
    }
    DTWAIN_SysDestroy();
    return 0;
}

void TestSingleFile(std::string outDir)
{
    auto Source = DTWAIN_SelectSource();
    if (Source)
    {
        auto ocr = DTWAIN_InitOCRInterface();
        DTWAIN_ARRAY aFiles3 = DTWAIN_EnumSupportedSinglePageFileTypes();
        DTWAIN_SetTempFileDirectoryA(outDir.c_str());
        const LONG* buffer = static_cast<LONG*>(DTWAIN_ArrayGetBuffer(aFiles3, 0));
        LONG sz = DTWAIN_ArrayGetCount(aFiles3);
        std::string filePrefix = outDir + "testSingle";
        for (LONG i = 0; i < sz; ++i)
        {
            char nameBuf[100] = {};
            char extBuf[100] = {};
            DTWAIN_GetFileTypeNameA(buffer[i], nameBuf, 99);
            DTWAIN_GetFileTypeExtensionsA(buffer[i], extBuf, 99);
            char* context;
            const char* extToUse = strtok_s(extBuf, "|", &context);
            LONG pt = DTWAIN_PT_DEFAULT;
            if (buffer[i] == DTWAIN_TIFFG3 ||
                buffer[i] == DTWAIN_TIFFG4 ||
                buffer[i] == DTWAIN_TEXT)
                pt = DTWAIN_PT_BW;
            if (buffer[i] == DTWAIN_GIF)
                pt = DTWAIN_PT_GRAY;
            std::string fileName = filePrefix + std::to_string(i + 1) + "." + std::string(extToUse);
            DTWAIN_AcquireFileA(Source, fileName.c_str(), buffer[i],
                DTWAIN_USELONGNAME, pt, 1, FALSE, TRUE, nullptr);
            std::cout << buffer[i] << " " << nameBuf << " " << extBuf << "\n";
        }
        DTWAIN_CloseSource(Source);
    }
}

void TestMultiFile(std::string outDir)
{
    auto Source = DTWAIN_SelectSource();
    if (Source)
    {
        auto ocr = DTWAIN_InitOCRInterface();
        DTWAIN_ARRAY aFiles3 = DTWAIN_EnumSupportedMultiPageFileTypes();
        DTWAIN_SetTempFileDirectoryA(outDir.c_str());
        const LONG* buffer = static_cast<LONG*>(DTWAIN_ArrayGetBuffer(aFiles3, 0));
        LONG sz = DTWAIN_ArrayGetCount(aFiles3);
        std::string filePrefix = outDir + "testMulti";
        for (LONG i = 0; i < sz; ++i)
        {
            char nameBuf[100] = {};
            char extBuf[100] = {};
            DTWAIN_GetFileTypeNameA(buffer[i], nameBuf, 99);
            DTWAIN_GetFileTypeExtensionsA(buffer[i], extBuf, 99);
            char* context;
            const char* extToUse = strtok_s(extBuf, "|", &context);
            LONG pt = DTWAIN_PT_DEFAULT;
            if (buffer[i] == DTWAIN_TIFFG3MULTI ||
                buffer[i] == DTWAIN_TIFFG4MULTI ||
                buffer[i] == DTWAIN_TEXTMULTI)
                pt = DTWAIN_PT_BW;
            std::string fileName = filePrefix + std::to_string(i + 1) + "." + std::string(extToUse);
            DTWAIN_AcquireFileA(Source, fileName.c_str(), buffer[i],
                DTWAIN_USELONGNAME, pt, 2, FALSE, TRUE, nullptr);
            std::cout << buffer[i] << " " << nameBuf << " " << extBuf << "\n";
        }
        DTWAIN_CloseSource(Source);
    }
}

void TestArrays()
{
}