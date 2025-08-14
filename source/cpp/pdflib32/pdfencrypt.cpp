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
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#define BOOST_DATE_TIME_NO_LIB

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#undef min
#undef max
#include "pdfencrypt.h"
#include "../hashlib/md5.h"

#define STRINGER_2_(x) #x
#define STRINGER_(x) STRINGER_2_(x)

std::string GetSystemTimeInMilliseconds();
#ifdef _MSC_VER
#pragma warning (disable:4244)
#endif

template <typename T>
void ArrayCopy(const T& input_array, int start, T& output_array, int output_start, int nBytes)
{
    std::copy(input_array.begin() + start,
                            input_array.begin() + start + nBytes,
                            output_array.begin() + output_start);
}

static unsigned char ConvertToHex(unsigned char hi, unsigned char lo);
static PDFEncryption::UCHARArray StringToHexArray(const std::string& sSource);
static PDFEncryption::UCHARArray StringToByteArray(const std::string& sSource);

class ARCFOUREncryption
{
    private:
        PDFEncryption::UCHARArray state;
        int x;
        int y;

    public:
        ARCFOUREncryption() : state(256), x{}, y{} {}
        void prepareARCFOURKey(const PDFEncryption::UCHARArray& key)
        { prepareARCFOURKey(key, 0, static_cast<int>(key.size())); }

        void prepareARCFOURKey(const PDFEncryption::UCHARArray& key, int off, int len)
        {
            int index1 = 0;
            int index2 = 0;
            for (int k = 0; k < 256; ++k)
                state[k] = static_cast<unsigned char>(k);
            x = 0;
            y = 0;
            for (int k = 0; k < 256; ++k)
            {
                index2 = key[index1 + off] + state[k] + index2 & 255;
                unsigned char tmp = state[k];
                state[k] = state[index2];
                state[index2] = tmp;
                index1 = (index1 + 1) % len;
            }
        }

       void encryptARCFOUR(const PDFEncryption::UCHARArray& dataIn, int off, int len, PDFEncryption::UCHARArray& dataOut, int offOut)
       {
           const int length = len + off;
           for (int k = off; k < length; ++k)
            {
                x = x + 1 & 255;
                y = state[x] + y & 255;
                unsigned char tmp = state[x];
                state[x] = state[y];
                state[y] = tmp;
                dataOut[k - off + offOut] = static_cast<unsigned char>(dataIn[k] ^ state[state[x] + state[y] & 255]);
            }
        }

        void encryptARCFOUR(PDFEncryption::UCHARArray& data, int off, int len)
        { encryptARCFOUR(data, off, len, data, off); }

        void encryptARCFOUR(const PDFEncryption::UCHARArray& dataIn, PDFEncryption::UCHARArray& dataOut)
        { encryptARCFOUR(dataIn, 0, static_cast<int>(dataIn.size()), dataOut, 0); }

        void encryptARCFOUR(PDFEncryption::UCHARArray& data)
        { encryptARCFOUR(data, 0, static_cast<int>(data.size()), data, 0); }
    };

    class IVGenerator
    {
        private:
            ARCFOUREncryption arcfour;

        public:
            IVGenerator()
            {
                std::string time = "01234567890123";
                std::string time2 = "01234567890123";
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(time.begin(), time.end(), g);
                std::shuffle(time2.begin(), time2.end(), g);
                std::string revTime = time + "+" + time2;
                arcfour.prepareARCFOURKey(PDFEncryption::UCHARArray(revTime.begin(), revTime.end()));
            }

            PDFEncryption::UCHARArray getIV()
            {
                return getIV(16);
            }

            PDFEncryption::UCHARArray getIV(int len)
            {
                PDFEncryption::UCHARArray b(len);
                arcfour.encryptARCFOUR(b);
                return b;
            }
    };

static unsigned char ConvertToHex(unsigned char hi, unsigned char lo)
{
    char retval;
    int temp = toupper(hi);
    if ( temp >= '0' && temp <= '9' )
        retval = (temp - '0') << 4;
    else
        retval = (temp - 'A' + 10) << 4;

    temp = toupper(lo);
    if ( temp >= '0' && temp <= '9' )
        retval += temp - '0';
    else
        retval += temp - 'A' + 10;
    return retval;
}

static PDFEncryption::UCHARArray StringToHexArray(const std::string& sSource)
{
    PDFEncryption::UCHARArray aDest(sSource.size() / 2);
    const int nLen = static_cast<int>(sSource.size());
    int j = 0;
    for ( int i = 0; i < nLen; i+=2, j++ )
    {
        aDest[j] = ConvertToHex(sSource[i], sSource[i+1]);
    }
    return aDest;
}

static PDFEncryption::UCHARArray StringToByteArray(const std::string& sSource)
{
    PDFEncryption::UCHARArray aDest(sSource.size());
    const int nLen = static_cast<int>(sSource.size());
    for ( int i = 0; i < nLen; ++i )
        aDest[i] = sSource[i];
    return aDest;
}

unsigned char PDFEncryption::pad[] =
{
    0x28, 0xBF, 0x4E, 0x5E, 0x4E, 0x75,
    0x8A, 0x41, 0x64, 0x00, 0x4E, 0x56,
    0xFF, 0xFA, 0x01, 0x08, 0x2E, 0x2E,
    0x00, 0xB6, 0xD0, 0x68, 0x3E, 0x80,
    0x2F, 0x0C, 0xA9, 0xFE, 0x64, 0x53,
    0x69, 0x7A
};

PDFEncryption::PDFEncryption() : state(256), m_xRC4Component{}, m_yRC4Component{},
                                keySize{}, ownerKey(32), userKey(32), permissions{}
{
}

void PDFEncryption::SetupAllKeys(const std::string& DocID,
                                 const std::string& userPassword,
                                 const std::string& ownerPassword, int permissionsValue,
                                 bool strength128Bits)
{
    UCHARArray uP(userPassword.length());
    std::copy(userPassword.begin(), userPassword.end(), uP.begin());
    UCHARArray oP(ownerPassword.length());
    std::copy(ownerPassword.begin(), ownerPassword.end(), oP.begin());
    SetupAllKeys(DocID, uP, oP, permissionsValue, strength128Bits);
}


void PDFEncryption::SetupAllKeys(const std::string& DocID,
                                 const UCHARArray& userPassword,
                                 UCHARArray& ownerPassword,
                                 int permissionsParam,
                                 bool strength128Bits)
{
    if (ownerPassword.empty() )
        ownerPassword = userPassword;

    // See PDF reference manual -- user access permissions bits:
    // 
    // For 40-bit encryption strength, bits 8 through 11 of the 
    // permissions value are unused, so just set them to 1.
    // For 128-bit encryption strength or greater, leave those bits alone
    permissionsParam |= strength128Bits ? 0xfffff0c0 : 0xffffffc0;
    permissionsParam &= 0xfffffffc;

    // PDF reference 3.5.2 Standard Security Handler, Algorithm 3.3-1
    // 
    // If there is no owner password, use the user password instead.
    const UCHARArray userPad = PadPassword(userPassword);
    const UCHARArray ownerPad = PadPassword(ownerPassword);

    ownerKey = ComputeOwnerKey(userPad, ownerPad, strength128Bits);
    SetupByUserPad(DocID, userPad, ownerKey, permissionsParam, strength128Bits);
}

void PDFEncryption::SetupByUserPad(const std::string& documentID,
                                   const UCHARArray& userPad,
                                   const UCHARArray& ownerKeyParam,
                                   int permissionsParam,
                                   bool strength128Bits)
{
    SetupGlobalEncryptionKey(documentID, userPad, ownerKeyParam, permissionsParam, strength128Bits);
    SetupUserKey();
}

void PDFEncryption::SetupGlobalEncryptionKey(const std::string& documentID,
                                             const UCHARArray& userPad,
                                             const UCHARArray& ownerKeyParam,
                                             int permissionsParam,
                                             bool strength128Bits)
{
    this->ownerKey = ownerKeyParam;
    this->permissions = permissionsParam;
    m_documentID = documentID;
    mkey.resize(strength128Bits ? 16 : 5);

    UCHARArray digest(16);
    UCHARArray ext(4);

    ext[0] = static_cast<unsigned char>(permissionsParam);
    ext[1] = permissionsParam >> 8;
    ext[2] = permissionsParam >> 16;
    ext[3] = permissionsParam >> 24;

    // This version of the MD5 checksum mimics the PDF reference
    // create a new hashing object
    MD5 md5;
    md5.add(userPad.data(), userPad.size());
    md5.add(ownerKeyParam.data(), ownerKeyParam.size());
    md5.add(ext.data(), 4);

    {
        const UCHARArray test = StringToHexArray(documentID);
        md5.add(test.data(), test.size());
    }


    unsigned char testbuf[MD5::HashBytes];
    md5.getHash(testbuf);
    if (mkey.size() == 16)
    {
        for (int k = 0; k < 50; ++k)
        {
            md5.reset();
            md5.add(testbuf, MD5::HashBytes);
            md5.getHash(testbuf);
        }
    }
    auto minToCopy = std::min(static_cast<size_t>(MD5::HashBytes), mkey.size());
    std::copy(testbuf, testbuf + minToCopy, mkey.begin());
}


PDFEncryption::UCHARArray PDFEncryption::ComputeOwnerKey(const UCHARArray& userPad,
                                                         const UCHARArray& ownerPad,
                                                         bool strength128Bits)
{
    UCHARArray ownerKeyValue(32);
    UCHARArray digest(16);

    MD5 md5;
    md5.add(ownerPad.data(), ownerPad.size());
    md5.getHash(digest.data());

    if (strength128Bits)
    {

        for (int k = 0; k < 50; ++k)
        {
            md5.reset();
            md5.add(digest.data(), MD5::HashBytes);
            md5.getHash(digest.data());
        }

        UCHARArray mkeyValue(16);
        ArrayCopy(userPad, 0, ownerKeyValue, 0, 32);
        for (int i = 0; i < 20; ++i)
        {
            for (UCHARArray::size_type j = 0; j < mkeyValue.size(); ++j)
                mkeyValue[j] = digest[j] ^ i;
            PrepareRC4Key(mkeyValue);
            EncryptRC4(ownerKeyValue);
        }
    }
    else
    {
        PrepareRC4Key(digest, 0, 5);
        EncryptRC4(userPad, ownerKeyValue);
    }
    return ownerKeyValue;
}

void PDFEncryption::SetupUserKey()
{
    if (mkey.size() == 16)
    {
        UCHARArray digest(32);
        MD5 md5;
        md5.add(&pad[0], sizeof pad);

        // step 3
        const UCHARArray test = StringToHexArray(m_documentID);
        md5.add(test.data(), test.size());
        md5.getHash(digest.data());

        // step 4
        PrepareRC4Key(mkey, 0, static_cast<int>(mkey.size()));
        EncryptRC4(digest, 0, 16);

        for (int k = 16; k < 32; ++k)
            userKey[k] = 0;

        // step 5
        UCHARArray tempkey = digest;
        for (int i = 1; i <= 19; ++i)
        {

            // Make encryption key
            for (UCHARArray::size_type j = 0; j < mkey.size(); ++j)
                tempkey[j] = mkey[j] ^ i;

            PrepareRC4Key(tempkey, 0, static_cast<int>(mkey.size()));

            // Call RC4 using
            EncryptRC4(digest, 0, 16);
            tempkey = digest;
        }
        userKey = std::move(tempkey);
    }
    else
    {
        const UCHARArray vectorPad(pad, pad + std::size(pad));
        PrepareRC4Key(mkey);
        EncryptRC4(vectorPad, userKey);
    }
}

PDFEncryption::UCHARArray PDFEncryption::PadPassword(const UCHARArray& pass) const
{
    UCHARArray userPad(32);
    const UCHARArray vectorPad(pad, pad + std::size(pad));
    if ( pass.empty() )
    {
        ArrayCopy(vectorPad, 0, userPad, 0, 32);
    }
    else
    {
         ArrayCopy(pass, 0, userPad, 0, static_cast<int>((std::min)(pass.size(), static_cast<size_t>(32))));
         if (pass.size() < 32)
         {
             ArrayCopy(vectorPad, 0, userPad, static_cast<int>(pass.size()), static_cast<int>(32 - pass.size()));
         }
    }
    return userPad;
}

PDFEncryption::UCHARArray PDFEncryption::GetExtendedKey(int number, int generation)
{
    UCHARArray extra(5);
    extra[0] = static_cast<unsigned char>(number);
    extra[1] = number >> 8;
    extra[2] = number >> 16;
    extra[3] = static_cast<unsigned char>(generation);
    extra[4] = generation >> 8;
    return extra;
}

void PDFEncryption::SetHashKey(int number, int generation)
{
    /** Work area to prepare the object/generation bytes */
    const UCHARArray extra = GetExtendedKey(number, generation);
    std::ostringstream m;
    m.write(reinterpret_cast<const char*>(mkey.data()), mkey.size());
    m.write(reinterpret_cast<const char*>(extra.data()),extra.size());
    const std::string sTemp = m.str();

    const UCHARArray tempArr = StringToByteArray(sTemp);
    MD5 md5;
    md5.add(tempArr.data(), tempArr.size());
    unsigned char tempbuf[MD5::HashBytes];
    md5.getHash(tempbuf);

    key.resize(32);

    std::copy(tempbuf, tempbuf + MD5::HashBytes, key.begin());

    keySize = static_cast<int>(mkey.size()) + 5;
    if (keySize > 16)
        keySize = 16;
}

void PDFEncryption::PrepareRC4Key(const UCHARArray& keyParam)
{
    PrepareRC4Key(keyParam, 0, static_cast<int>(keyParam.size()));
}

void PDFEncryption::PrepareRC4Key(const UCHARArray& keyParam, int off, int len)
{
    int index1 = 0;
    int index2 = 0;
    int k;
    for (k = 0; k < 256; ++k)
        state[k] = static_cast<unsigned char>(k);
    m_xRC4Component = 0;
    m_yRC4Component = 0;
    for (k = 0; k < 256; ++k)
    {
        index2 = keyParam[index1 + off] + state[k] + index2 & 255;
        unsigned char tmp = state[k];
        state[k] = state[index2];
        state[index2] = tmp;
        index1 = (index1 + 1) % len;
    }
}

void PDFEncryption::EncryptRC4(const UCHARArray& dataIn, int off, int len, UCHARArray& dataOut)
{
    const int length = len + off;
    for (int k = off; k < length; ++k)
    {
        m_xRC4Component = m_xRC4Component + 1 & 255;
        m_yRC4Component = state[m_xRC4Component] + m_yRC4Component & 255;
        unsigned char tmp = state[m_xRC4Component];
        state[m_xRC4Component] = state[m_yRC4Component];
        state[m_yRC4Component] = tmp;
        dataOut[k] = dataIn[k] ^ state[state[m_xRC4Component] + state[m_yRC4Component] & 255];
    }
}

void PDFEncryption::EncryptRC4(UCHARArray& data, int off, int len)
{
    EncryptRC4(data, off, len, data);
}

void PDFEncryption::EncryptRC4(const UCHARArray& dataIn, UCHARArray& dataOut)
{
    EncryptRC4(dataIn, 0, static_cast<int>(dataIn.size()), dataOut);
}

void PDFEncryption::EncryptRC4(UCHARArray& data)
{
    EncryptRC4(data, 0, static_cast<int>(data.size()), data);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PDFEncryptionRC4::PrepareKey()
{
    PrepareRC4Key(key, 0, keySize);
}

PDFEncryption::UCHARArray PDFEncryptionRC4::GetExtendedKey(int number, int generation)
{
    return PDFEncryption::GetExtendedKey(number, generation);
}

void PDFEncryptionRC4::Encrypt(const std::string& dataIn, std::string& dataOut)
{
    UCHARArray dIn(dataIn.size());
    std::copy (dataIn.begin(), dataIn.end(), dIn.begin());
    UCHARArray dOut(dataIn.size());
    EncryptRC4(dIn, dOut);
    dataOut.clear();
    dataOut.append(reinterpret_cast<const char *>(dOut.data()), dOut.size());
}

void PDFEncryptionRC4::Encrypt(char *dataIn, int len)
{
    UCHARArray dIn(len);
    std::copy_n (dataIn, len, dIn.begin());
    EncryptRC4(dIn);
    std::copy(dIn.begin(), dIn.end(), dataIn);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
#ifdef DTWAIN_SUPPORT_AES
void PDFEncryptionAES::PrepareKey()
{
    PrepareRC4Key(key, 0, keySize);
    IVGenerator iv;
    PDFEncryption::UCHARArray arr = iv.getIV(AES_BLOCK_SIZE);
    memcpy(m_ivValue, &arr[0], AES_BLOCK_SIZE);
}

// save encrypted data to new string
void PDFEncryptionAES::Encrypt(const std::string& dataIn, std::string& dataOut)
{
    auto numBytes = dataIn.size();

    AES_CTX ctx;
    AES128::AES_EncryptInit(&ctx, key.data(), m_ivValue);

    size_t numchunks = numBytes / AES_BLOCK_SIZE + 1;
    if (numBytes > 0 && numBytes % AES_BLOCK_SIZE == 0)
        --numchunks;
    dataOut.clear();

    int start = 0;
    size_t totalBytes = numBytes;
    for (size_t i = 0; i < numchunks; ++i)
    {
        uint8_t oneChunk[AES_BLOCK_SIZE] = {};
        memcpy(oneChunk, dataIn.data() + start, std::min(totalBytes, static_cast<size_t>(AES_BLOCK_SIZE)));
        AES128::AES_Encrypt(&ctx, oneChunk, oneChunk);
        std::copy_n(oneChunk, AES_BLOCK_SIZE, std::back_inserter(dataOut));
        start += AES_BLOCK_SIZE;
        totalBytes -= AES_BLOCK_SIZE;
    }

    // Place the iv as the first 16 bytes
    dataOut.insert(0, (const char *)m_ivValue, AES_KEY_SIZE);
}

PDFEncryption::UCHARArray PDFEncryptionAES::GetExtendedKey(int number, int generation)
{
    // copies "sAlT" to extended key string
    constexpr char AESString[] = { 0x73, 0x41, 0x6c, 0x54 };

    // first get the original MD5 key string
    UCHARArray extra = PDFEncryption::GetExtendedKey(number, generation);

    // now attach "sAlT" string to this
    std::copy_n(AESString, 4, std::back_inserter(extra));

    return extra;
}


// save encrypted data
void PDFEncryptionAES::Encrypt(char *dataIn, int len)
{
}
#else
    #pragma message ("AES encryption is not supported")
#endif


