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
#include <ctlrandnumutils.h>

#include <boost/multiprecision/cpp_int.hpp> // Include for uint128_t

#undef min
#undef max
#include "pdfencrypt.h"
#include "../hashlib/md5.h"
#include "ctlhashutils.h"
#include "ctlstringutils.h"
#include "ctlobstr.h"

#define STRINGER_2_(x) #x
#define STRINGER_(x) STRINGER_2_(x)

std::string GetSystemTimeInMilliseconds();
#ifdef _MSC_VER
#pragma warning (disable:4244)
#endif

using namespace boost::multiprecision;

// Function to convert 16-byte big-endian char array to int64_t
static uint128_t bigEndianBytesToInt(const unsigned char* bytes, size_t numBytes)
{
    uint128_t value = 0;

    // Iterate through the bytes and shift them into the value
    // Big-endian means the most significant byte is first (at index 0)
    for (size_t i = 0; i < numBytes; ++i) { // For an 8-byte (64-bit) integer
        value = (value << 8) | bytes[i];
    }
    return value;
}

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
                                m_nKeySize{}, m_nActualKeyLength(5), 
                                m_nMaxPasswordLength(PasswordLength),
                                m_nPermissions{}
{
    m_OwnerKey.resize(m_nMaxPasswordLength);
    m_UserKey.resize(m_nMaxPasswordLength);
}

void PDFEncryption::SetMaxPasswordLength(uint32_t maxLen)
{
    m_nMaxPasswordLength = maxLen;
    m_OwnerKey.resize(m_nMaxPasswordLength);
    m_UserKey.resize(m_nMaxPasswordLength);
}

void PDFEncryption::SetupAllKeys(const std::string& DocID,
                                 const std::string& userPassword,
                                 const std::string& ownerPassword, 
                                 int permissionsValue,
                                 bool strength128Bits)
{
    UCHARArray uP(userPassword.length());
    std::copy(userPassword.begin(), userPassword.end(), uP.begin());
    UCHARArray oP(ownerPassword.length());
    std::copy(ownerPassword.begin(), ownerPassword.end(), oP.begin());
    SetupAllKeys(DocID, uP, oP, permissionsValue, strength128Bits);
}


void PDFEncryption::SetupAllKeys(const std::string& DocID,
                                UCHARArray& userPassword,
                                UCHARArray& ownerPassword,
                                int permissionsParam,
                                bool strength128Bits)
{
    bool isAES256 = (m_nActualKeyLength == 32 && dynamic_cast<PDFEncryptionAES*>(this));
    if (ownerPassword.empty())
        ownerPassword = userPassword;

    // See PDF reference manual -- user access permissions bits:
    // 
    // For 40-bit encryption strength, bits 8 through 11 of the 
    // permissions value are unused, so just set them to 1.
    // For 128-bit encryption strength, leave those bits alone
    permissionsParam |= strength128Bits ? 0xfffff0c0 : 0xffffffc0;
    permissionsParam &= 0xfffffffc;

    if (!isAES256)
    {
        // (old) PDF reference 3.5.2 Standard Security Handler, Algorithm 3.3-1
        // 
        // If there is no owner password, use the user password instead.
        const UCHARArray userPad = PadPassword(userPassword);
        const UCHARArray ownerPad = PadPassword(ownerPassword);

        m_OwnerKey = ComputeOwnerKey(userPad, ownerPad, strength128Bits);
        SetupByUserPad(DocID, userPad, m_OwnerKey, permissionsParam, strength128Bits);
    }
    else
    {
        // Make sure that the passwords are not greater than 127 characters
        ownerPassword.resize(std::min(static_cast<size_t>(127), ownerPassword.size()));
        userPassword.resize(std::min(static_cast<size_t>(127), userPassword.size()));

        // Create all of the information blocks that will be written to the PDF
        // file in the Encryption dictionary (U, O, UE, OE, Perms, and the file encryption key)
        CreateAESV3Info(dynarithmic::StringWrapperA::StringFromUChars(ownerPassword.data(), ownerPassword.size()),
                        dynarithmic::StringWrapperA::StringFromUChars(userPassword.data(), userPassword.size()),
                        permissionsParam);
    }
}

void PDFEncryption::CreateAESV3Info(std::string userPassword, std::string ownerPassword, int permissions)
{
    // Generate file encryption key
    m_EncryptionKey = dynarithmic::CreateRandomDigits(32);

    // Compute U and UE values
    ComputeUserKeyAESV3(userPassword);

    // Compute O and OE value
    ComputeOwnerKeyAESV3(ownerPassword);

    // Compute the permissions
    // Compute the Perms key
    ComputePermsKey(permissions);
}

void PDFEncryption::ComputeUserKeyAESV3(const std::string& userpswd)
{
    ComputeUserOrOwnerKeyAESV3(userpswd, m_UserKey, m_UserKeyE, false);
}

void PDFEncryption::ComputeOwnerKeyAESV3(const std::string& ownerpswd)
{
    ComputeUserOrOwnerKeyAESV3(ownerpswd, m_OwnerKey, m_OwnerKeyE, true);
}

void PDFEncryption::ComputePermsKey(int permissions)
{
    // Compute the Perms key
    /*  Fill a 16 - byte block as follows :
            Extend the permissions(contents of the P integer) to 64 bits by setting the upper 32 bits to all 1’s.
                NOTE This allows for future extension without changing the format.
                b) Record the 8 bytes of permission in the bytes 0 - 7 of the block, low order byte first.
                c) Set byte 8 to the ASCII character "T" or "F" according to the EncryptMetadata boolean.
                d) Set bytes 9 - 11 to the ASCII characters '"a", "d", "b".
                e) Set bytes 12 - 15 to 4 bytes of random data, which will be ignored.
                f) Encrypt the 16 - byte block using AES - 256 in ECB mode with an initialization vector of zero, using the file
                encryption key as the key.The result(16 bytes) is stored as the Perms string, and checked for validity
                when the file is opened.*/

    m_nPermissions = permissions;
    unsigned char PermBlock[16] = {};
    PermBlock[3] = ((unsigned)permissions >> 24) & 0xFF;
    PermBlock[2] = ((unsigned)permissions >> 16) & 0xFF;
    PermBlock[1] = ((unsigned)permissions >> 8) & 0xFF;
    PermBlock[0] = ((unsigned)permissions >> 0) & 0xFF;

    PermBlock[4] = 0xFF;
    PermBlock[5] = 0xFF;
    PermBlock[6] = 0xFF;
    PermBlock[7] = 0xFF;

    PermBlock[8] = 'T';
    PermBlock[9] = 'a';
    PermBlock[10] = 'd';
    PermBlock[11] = 'b';

    PermBlock[12] = 0;
    PermBlock[13] = 0;
    PermBlock[14] = 0;
    PermBlock[15] = 0;
    auto randomData = dynarithmic::CreateRandomDigits(4);
    std::copy_n(randomData.begin(), 4, PermBlock + 12);

    PDFEncryptionAES aes;

    // Prepare the AES-256 key with the file encryption key
    unsigned char iv[16] = {};
    aes.PrepareKey(m_EncryptionKey.data(), 32, iv);
    aes.SetPaddingUsed(false);

    // Use the encryption key for the AES-256 hash.
    std::string sPermsKey;
    std::string sPermBlock = dynarithmic::StringWrapperA::StringFromUChars(PermBlock, 16);
    aes.EncryptAES256ECB(sPermBlock, sPermsKey);

    m_PermsKey = dynarithmic::StringWrapperA::UCharsFromString(sPermsKey);

}
void PDFEncryption::ComputeUserOrOwnerKeyAESV3(const std::string& pswd, // Password
                                               UCHARArray& Key, // U or O entry
                                               UCHARArray& KeyE, // UE or OE entry
                                               bool useUserKey) // use the user key
{
    // Generate Salts
    auto vSalt = dynarithmic::CreateRandomDigits(8);
    auto kSalt = dynarithmic::CreateRandomDigits(8); 
    auto vSaltString = dynarithmic::StringWrapperA::StringFromUChars(vSalt.data(), 8);
    auto kSaltString = dynarithmic::StringWrapperA::StringFromUChars(kSalt.data(), 8);

    // If we need to use the userkey, set the userString
    std::string userString;
    if (useUserKey)
        userString = dynarithmic::StringWrapperA::StringFromUChars(m_UserKey.data(), m_UserKey.size());

    // Generate hash for U or O
    auto hashValueOut = ComputeHashAESV3(pswd, vSaltString, userString);

    auto KeyString = dynarithmic::StringWrapperA::StringFromUChars(hashValueOut.data(), hashValueOut.size()) +
                                                                   vSaltString + kSaltString;

    Key = dynarithmic::StringWrapperA::UCharsFromString(KeyString);

    // Generate hash for UE or OE
    hashValueOut = ComputeHashAESV3(pswd, kSaltString, userString);

    // UE or OE = AES-256 encoded file encryption key with key=hash
    // CBC mode, no padding, init vector=0
    unsigned char iv[16] = {};
    PDFEncryptionAES aesEncrypt;
    IVGenerator ivGen;
    aesEncrypt.PrepareKey(hashValueOut.data(), 32, iv);
    aesEncrypt.SetPaddingUsed(false);
    aesEncrypt.SetIVAttached(false);
    std::string dataOut;
    aesEncrypt.EncryptAES256CBC(
        dynarithmic::StringWrapperA::StringFromUChars(m_EncryptionKey.data(), m_EncryptionKey.size()), 
        dataOut);
    KeyE.assign(dataOut.begin(), dataOut.end());
}

PDFEncryption::UCHARArray PDFEncryption::ComputeHashAESV3(std::string pswd, std::string salt, std::string uValue)
{
    PDFEncryption::UCHARArray outValue;
    std::string hasherKey = pswd + salt + uValue;
    auto K = dynarithmic::SHA2Hash(hasherKey, dynarithmic::SHA2HashType::SHA256);

    // AES-256 according to PDF 1.7 Adobe Extension Level 8 (PDF 2.0)
    std::string E;
    PDFEncryptionAES aesEncryptor;
    for (unsigned i = 0; i < 64 ||
        static_cast<unsigned int>(E.back()) > (i - 32); i++)
    {
        std::string oneValue = pswd +
            dynarithmic::StringWrapperA::StringFromUChars(K.data(), K.size()) 
            + uValue;
        std::string K1;
        for (int j = 1; j <= 64; ++j)
            K1 += oneValue;

        aesEncryptor.PrepareKey(K.data(), 16, &K[16]);
        aesEncryptor.SetPaddingUsed(false);
        aesEncryptor.SetIVAttached(false);
        aesEncryptor.EncryptAES128CBC(K1, E);
        auto ETemp = dynarithmic::StringWrapperA::UCharsFromString(E);
        auto K1Temp = dynarithmic::StringWrapperA::UCharsFromString(K1);
        auto remainder = static_cast<int>(bigEndianBytesToInt(ETemp.data(), 16) % 3);

        if (remainder == 0)
        {
            K = dynarithmic::SHA2Hash(E, dynarithmic::SHA2HashType::SHA256);
        }
        else if (remainder == 1)
        {
            K = dynarithmic::SHA2Hash(E, dynarithmic::SHA2HashType::SHA384);
        }
        else
        {
            K = dynarithmic::SHA2Hash(E, dynarithmic::SHA2HashType::SHA512);
        }
    }
    K.resize(32);
    return K;
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
    this->m_OwnerKey = ownerKeyParam;
    this->m_nPermissions = permissionsParam;
    m_documentID = documentID;
    m_EncryptionKey.resize(m_nActualKeyLength);

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
    if (m_EncryptionKey.size() >= 16)
    {
        for (int k = 0; k < 50; ++k)
        {
            md5.reset();
            md5.add(testbuf, MD5::HashBytes);
            md5.getHash(testbuf);
        }
    }
    auto minToCopy = std::min(static_cast<size_t>(MD5::HashBytes), m_EncryptionKey.size());
    std::copy(testbuf, testbuf + minToCopy, m_EncryptionKey.begin());
}


PDFEncryption::UCHARArray PDFEncryption::ComputeOwnerKey(const UCHARArray& userPad,
                                                         const UCHARArray& ownerPad,
                                                         bool strength128Bits)
{
    UCHARArray ownerKeyValue(m_nMaxPasswordLength);
    UCHARArray digest(16);

    MD5 md5;
    md5.add(ownerPad.data(), ownerPad.size());
    md5.getHash(digest.data());

    if (m_nActualKeyLength >= 16)
    {
        for (int k = 0; k < 50; ++k)
        {
            md5.reset();
            md5.add(digest.data(), MD5::HashBytes);
            md5.getHash(digest.data());
        }

        UCHARArray mkeyValue(m_nActualKeyLength);
        ArrayCopy(userPad, 0, ownerKeyValue, 0, m_nMaxPasswordLength);
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
    if (m_EncryptionKey.size() >= 16)
    {
        UCHARArray digest(32);
        MD5 md5;
        md5.add(&pad[0], sizeof pad);

        // step 3
        const UCHARArray test = StringToHexArray(m_documentID);
        md5.add(test.data(), test.size());
        md5.getHash(digest.data());

        // step 4
        PrepareRC4Key(m_EncryptionKey, 0, static_cast<int>(m_EncryptionKey.size()));
        EncryptRC4(digest, 0, 16);

        for (int k = 16; k < 32; ++k)
            m_UserKey[k] = 0;

        // step 5
        UCHARArray tempkey = digest;
        for (int i = 1; i <= 19; ++i)
        {

            // Make encryption key
            for (UCHARArray::size_type j = 0; j < m_EncryptionKey.size(); ++j)
                tempkey[j] = m_EncryptionKey[j] ^ i;

            PrepareRC4Key(tempkey, 0, static_cast<int>(m_EncryptionKey.size()));

            // Call RC4 using
            EncryptRC4(digest, 0, 16);
            tempkey = digest;
        }
        m_UserKey = std::move(tempkey);
    }
    else
    {
        const UCHARArray vectorPad(pad, pad + std::size(pad));
        PrepareRC4Key(m_EncryptionKey);
        EncryptRC4(vectorPad, m_UserKey);
    }
}

PDFEncryption::UCHARArray PDFEncryption::PadPassword(const UCHARArray& pass) const
{
    UCHARArray userPad(m_nMaxPasswordLength);
    const UCHARArray vectorPad(pad, pad + std::size(pad));
    if ( pass.empty() )
    {
        ArrayCopy(vectorPad, 0, userPad, 0, m_nMaxPasswordLength);
    }
    else
    {
         ArrayCopy(pass, 0, userPad, 0, static_cast<int>((std::min)(pass.size(), static_cast<size_t>(m_nMaxPasswordLength))));
         if (pass.size() < m_nMaxPasswordLength)
         {
             ArrayCopy(vectorPad, 0, userPad, static_cast<int>(pass.size()), static_cast<int>(m_nMaxPasswordLength - pass.size()));
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
    m.write(reinterpret_cast<const char*>(m_EncryptionKey.data()), m_EncryptionKey.size());
    m.write(reinterpret_cast<const char*>(extra.data()),extra.size());
    const std::string sTemp = m.str();

    const UCHARArray tempArr = StringToByteArray(sTemp);
    MD5 md5;
    md5.add(tempArr.data(), tempArr.size());
    unsigned char tempbuf[MD5::HashBytes];
    md5.getHash(tempbuf);

    m_LocalKey.resize(32);

    std::copy(tempbuf, tempbuf + MD5::HashBytes, m_LocalKey.begin());

    m_nKeySize = static_cast<int>(m_EncryptionKey.size()) + 5;
    auto maxKeySize = std::max(16U, m_nActualKeyLength);
    if (m_nKeySize > maxKeySize)
        m_nKeySize = maxKeySize;
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
    PrepareRC4Key(m_LocalKey, 0, m_nKeySize);
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
    PrepareRC4Key(m_LocalKey, 0, m_nKeySize);
    IVGenerator iv;
    PDFEncryption::UCHARArray arr = iv.getIV(AES_BLOCK_SIZE);
    memcpy(m_ivValue, &arr[0], AES_BLOCK_SIZE);
}

void PDFEncryptionAES::PrepareKey(const unsigned char* key, size_t keySize, const unsigned char * iv)
{
    m_LocalKey.clear();
    std::copy_n(key, keySize, std::back_inserter(m_LocalKey));
    memcpy(m_ivValue, iv, AES_BLOCK_SIZE);
}

void PDFEncryptionAES::PrepareKey(const unsigned char* key, size_t keySize)
{
    m_LocalKey.clear();
    std::copy_n(key, keySize, std::back_inserter(m_LocalKey));
    IVGenerator iv;
    PDFEncryption::UCHARArray arr = iv.getIV(AES_BLOCK_SIZE);
    memcpy(m_ivValue, &arr[0], AES_BLOCK_SIZE);
}

// save encrypted data to new string
void PDFEncryptionAES::Encrypt(const std::string& dataIn, std::string& dataOut)
{
    switch (m_nKeySize)
    {
        case 16:
            EncryptAES128CBC(dataIn, dataOut);
        break;
        case 32:
            EncryptAES256CBC(dataIn, dataOut);
        break;
    }
}

void PDFEncryptionAES::EncryptAES128CBC(const std::string& dataIn, std::string& dataOut)
{
    EncryptInternal(dataIn, dataOut, AESMode::AES_CBC, AESKeyLength::AES_128);
}

void PDFEncryptionAES::EncryptAES256CBC(const std::string& dataIn, std::string& dataOut)
{
    EncryptInternal(dataIn, dataOut, AESMode::AES_CBC, AESKeyLength::AES_256);
}

void PDFEncryptionAES::EncryptAES256ECB(const std::string& dataIn, std::string& dataOut)
{
    EncryptInternal(dataIn, dataOut, AESMode::AES_ECB, AESKeyLength::AES_256);
}

void PDFEncryptionAES::EncryptAES128ECB(const std::string& dataIn, std::string& dataOut)
{
    EncryptInternal(dataIn, dataOut, AESMode::AES_ECB, AESKeyLength::AES_128);
}

void PDFEncryptionAES::EncryptInternal(std::string dataIn, std::string& dataOut, 
                                       AESMode aesMode, AESKeyLength keyLength)
{
    // Convert input string to byte array
    std::vector<unsigned char> origDataAsUChars = dynarithmic::StringWrapperA::UCharsFromString(dataIn);

    // Adjust the input string, depending on the padding.
    unsigned char paddingByte = 0;
    bool extraPadding = false;
    unsigned char paddingToAdd[16] = {};
    if (m_bIsPaddingUsed)
    {
        // we need to add padding bytes (PKCS#7)
        extraPadding = true;
        auto nearest16 = dynarithmic::RoundUpToNearest(static_cast<uint32_t>(dataIn.size()), 16U);
        if (dataIn.size() % 16 == 0)
        {
            paddingByte = 0x10;
        }
        else
        {
            paddingByte = static_cast<unsigned char>(nearest16 - dataIn.size());
        }
    }

    // Add padding byte to end of the data we will encrypt
    if (extraPadding && paddingByte == 0x10)
    {
        memset(paddingToAdd, 0x10, sizeof(paddingToAdd));
        origDataAsUChars.insert(origDataAsUChars.end(), paddingToAdd, paddingToAdd + AES_BLOCK_SIZE);
    }
    else
    if (extraPadding)
    {
        memset(paddingToAdd, paddingByte, sizeof(paddingToAdd));
        origDataAsUChars.insert(origDataAsUChars.end(), paddingToAdd, paddingToAdd + paddingByte);
    }

    AES aes(keyLength); // The lower-level AES encryption instance
    std::vector<unsigned char> vEncryptedData;

    if (aesMode == AESMode::AES_ECB)
    {
        // Start the encryption (no iv in ECB mode)
        vEncryptedData = aes.EncryptECB(origDataAsUChars, m_LocalKey);
    }
    else
    if (aesMode == AESMode::AES_CBC)
    {
        // Convert initialization vector to a compatible std::vector
        std::vector<unsigned char> vIv(m_ivValue, m_ivValue + 16);

        // Start the encryption
        vEncryptedData = aes.EncryptCBC(origDataAsUChars, m_LocalKey, vIv);

        // Add the initialization vector at the beginning of the encrypted stream
        // but only if 1) the user wants to attach the iv, 2) there is an iv, 
        // and 3) the iv is not all 0
        if (m_bIsIVAttached && std::any_of(vIv.begin(), vIv.end(), [](auto ch) { return ch != 0; }))
        {
            vEncryptedData.insert(vEncryptedData.begin(), vIv.begin(), vIv.end());
        }
    }
    // Convert encrypted data to a std::string and we are done.
    dataOut = dynarithmic::StringWrapperA::StringFromUChars(vEncryptedData.data(), vEncryptedData.size());
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


