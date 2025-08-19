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
#ifndef PDFENCRYPT_H
#define PDFENCRYPT_H
#include <vector>
#include <string>

#ifdef DTWAIN_SUPPORT_AES
    #include "..\aeslib\AES_128_CBC.h"
    #include "..\aeslib\AES_256_CBC.h"
#endif

#define ENCRYPTION_OK           0
#define ENCRYPTION_NOTSET       1
#define ENCRYPTION_ERROR        2
#define ENCRYPTION_UNAVAILABLE  3

class PDFEncryption
{
    public:
        typedef std:: vector<unsigned char> UCHARArray;
        static constexpr uint32_t PasswordLength = 32U;
        static constexpr uint32_t PasswordLength256 = 127U;

    protected:
    static unsigned char pad[32];
        UCHARArray state;
        int m_xRC4Component;
        int m_yRC4Component;

        /** The encryption key for a particular object/generation */
        UCHARArray m_LocalKey;

        /** The total encryption key length (keylength + 5) for a particular object/generation */
        uint32_t m_nKeySize;

        /** The key length for a particular object/generation */
        uint32_t m_nActualKeyLength;

        /** The global encryption key */
        UCHARArray m_EncryptionKey;

        /** The maximum user/owner password length */
        uint32_t m_nMaxPasswordLength;

        /** Alternate message digest */
        //CMD5Checksum MD5Alternate;

        /** The encryption key for the owner */
        UCHARArray m_nOwnerKey;

        /** The encryption key for the user */
        UCHARArray m_nUserKey;

        int m_nPermissions;

        std::string m_documentID;

        UCHARArray PadPassword(const UCHARArray& passw) const;
        void SetupByUserPad(const std::string& documentID,
                            const UCHARArray& userPad,
                            const UCHARArray& ownerKey,
                            int permissions,
                            bool strength128Bits);
        void SetupGlobalEncryptionKey(const std::string& documentID,
                                      const UCHARArray& userPad,
                                      const UCHARArray& ownerKey,
                                      int permissions,
                                       bool strength128Bits) ;
        void SetupUserKey();
        void PrepareRC4Key(const UCHARArray& key);
        void PrepareRC4Key(const UCHARArray& key, int off, int len);
        void EncryptRC4(const UCHARArray& dataIn, int off, int len, UCHARArray& dataOut);
        void EncryptRC4(UCHARArray& data, int off, int len);
        void EncryptRC4(const UCHARArray& dataIn, UCHARArray& dataOut);
        void EncryptRC4(UCHARArray& data);
        virtual UCHARArray GetExtendedKey(int number, int generation);

    public:
        PDFEncryption();
        virtual ~PDFEncryption() = default;
        UCHARArray ComputeOwnerKey(const UCHARArray& userPad,
                                   const UCHARArray& ownerPad,
                                   bool strength128Bits);

        void SetupAllKeys(const std::string& DocID,
                          const std::string& userPassword,
                          const std::string& ownerPassword, int permissions,
                          bool strength128Bits);
        void SetKeyLength(uint32_t keyLength) { m_nActualKeyLength = keyLength; }
        uint32_t GetKeyLength() const { return m_nActualKeyLength; }
        void SetMaxPasswordLength(uint32_t maxLen);
        void SetupAllKeys(const std::string& DocID,
                          const UCHARArray& userPassword, UCHARArray& ownerPassword,
                          int permissions, bool strength128Bits);
        virtual void PrepareKey() = 0;
        void SetHashKey(int number, int generation);
        virtual void Encrypt(const std::string& /*dataIn*/, std::string& /*dataOut*/) {}
        virtual void Encrypt(char * /*dataIn*/, int/* len*/) {}

        UCHARArray& GetUserKey() { return m_nUserKey; }
        UCHARArray& GetOwnerKey() { return m_nOwnerKey; }
        UCHARArray& GetEncryptionKey() { return m_EncryptionKey; }
        int GetPermissions() const { return m_nPermissions; }
};

class PDFEncryptionRC4 : public PDFEncryption
{
    protected:
        UCHARArray GetExtendedKey(int number, int generation) override;
    public:
        void Encrypt(const std::string& dataIn, std::string& dataOut) override;
        void Encrypt(char *dataIn, int len) override;
        void PrepareKey() override;

};

#ifdef DTWAIN_SUPPORT_AES
template <typename ClassName, typename CTXType>
struct AESEncryptorTraits
{
    typedef std::vector<unsigned char> UCHARArray;

    static void Initialize(CTXType* ctx, const UCHARArray& localKey, const unsigned char* iv)
    {
        ClassName::AES_EncryptInit(ctx, localKey.data(), iv);
    }

    static void EncryptBlock(CTXType* ctx, uint8_t* chunk)
    {
        ClassName::AES_Encrypt(ctx, chunk, chunk);
    }
};

template <typename CTXType, typename ClassName, typename EncryptorTraits>
class PDFAESGenericEncryptor
{
    typedef std::vector<unsigned char> UCHARArray;

    CTXType ctx;
    UCHARArray m_LocalKey;
    unsigned char* m_ivValue;
    public:
        PDFAESGenericEncryptor(UCHARArray localKey, unsigned char* ivValue) :
            m_LocalKey(localKey), m_ivValue(ivValue) {}

        void InitializeEngine()
        {
            EncryptorTraits::Initialize(&ctx, m_LocalKey, m_ivValue);
        }

        void EncryptBlock(uint8_t* chunk)
        {
            EncryptorTraits::EncryptBlock(&ctx, chunk);
        }

        void Encrypt(const std::string& dataIn, std::string& dataOut)
        {
            auto numBytes = dataIn.size();

            InitializeEngine();

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
                EncryptBlock(oneChunk);
                std::copy_n(oneChunk, AES_BLOCK_SIZE, std::back_inserter(dataOut));
                start += AES_BLOCK_SIZE;
                totalBytes -= AES_BLOCK_SIZE;
            }

            // Place the iv as the first 16 bytes
            dataOut.insert(0, (const char*)m_ivValue, AES_BLOCK_SIZE);
        }
};

using PDFEncryptionAES128 = PDFAESGenericEncryptor<AES_CTX_128, AES128, AESEncryptorTraits<AES128, AES_CTX_128>>;
using PDFEncryptionAES256 = PDFAESGenericEncryptor<AES_CTX_256, AES256, AESEncryptorTraits<AES256, AES_CTX_256>>;;

class PDFEncryptionAES: public PDFEncryption
{
    protected:
        UCHARArray GetExtendedKey(int number, int generation);
        void EncryptAES128(const std::string& dataIn, std::string& dataOut);
        void EncryptAES256(const std::string& dataIn, std::string& dataOut);
private:
        unsigned char m_ivValue[AES_BLOCK_SIZE];
    public:
        void Encrypt(const std::string& dataIn, std::string& dataOut) override;
        void Encrypt(char *dataIn, int len) override;
        void PrepareKey() override;
};
#endif
#endif