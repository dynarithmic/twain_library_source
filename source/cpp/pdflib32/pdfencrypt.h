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
#include <string_view>

#ifdef DTWAIN_SUPPORT_AES
    #include "..\aeslib\AES.h"
#endif

#include <ctlobstr.h>
#include <ctlconstexprutils.h>

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
        UCHARArray m_OwnerKey;

        /** The encryption key for the user */
        UCHARArray m_UserKey;

        /** The UE encryption key (PDF 2.0 only) */
        UCHARArray m_UserKeyE;

        /** The OE encryption key (PDF 2.0 only) */
        UCHARArray m_OwnerKeyE;

        /** The Perms encryption key (PDF 2.0 only) */
        UCHARArray m_PermsKey;

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
        void CreateAESV3Info(std::string userPassword, std::string ownerPassword, int permissions);

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

        UCHARArray ComputeHashAESV3(std::string pswd, std::string salt, std::string uValue);
        void ComputeUserOrOwnerKeyAESV3(const std::string& pswd, UCHARArray& Key,
                                        UCHARArray& KeyE, bool useUserKey);
        void ComputeUserKeyAESV3(const std::string& userpswd);
        void ComputeOwnerKeyAESV3(const std::string& ownerpswd);
        void ComputePermsKey(int permissions);

        UCHARArray& GetUserKey() { return m_UserKey; }
        UCHARArray& GetOwnerKey() { return m_OwnerKey; }
        UCHARArray& GetUserKeyE() { return m_UserKeyE; }
        UCHARArray& GetOwnerKeyE() { return m_OwnerKeyE; }
        UCHARArray& GetEncryptionKey() { return m_EncryptionKey; }
        UCHARArray& GetPermsKey() { return m_PermsKey; }
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
template <typename ClassName, typename CTXType, bool UseIV>
struct AESEncryptorTraits
{
    typedef std::vector<unsigned char> UCHARArray;
    static const bool s_bUseIV = UseIV;

    static void Initialize(CTXType* ctx, const UCHARArray& localKey, const unsigned char* iv)
    {
        ClassName::AES_EncryptInit(ctx, localKey.data(), iv);
    }

    static void EncryptBlock(CTXType* ctx, uint8_t* chunk)
    {
        ClassName::AES_Encrypt(ctx, chunk, chunk);
    }
};

class PDFEncryptionAES: public PDFEncryption
{
    protected:
        UCHARArray GetExtendedKey(int number, int generation);

    private:
        unsigned char m_ivValue[AES_BLOCK_SIZE];
        bool m_bIsPaddingUsed = true;
        bool m_bIsIVAttached = true;
        void EncryptInternal(std::string dataIn, std::string& dataOut,
                             AESMode aesMode, AESKeyLength keyLength);

    public:
        void Encrypt(const std::string& dataIn, std::string& dataOut) override;
        void Encrypt(char *dataIn, int len) override;
        void SetPaddingUsed(bool bSet) { m_bIsPaddingUsed = bSet; }
        void SetIVAttached(bool bSet) { m_bIsIVAttached = bSet; }
        bool IsIVAttached() const { return m_bIsIVAttached; }
        bool IsPaddingUsed() const { return m_bIsPaddingUsed; }
        void PrepareKey() override;
        void PrepareKey(const unsigned char* key, size_t keySize, const unsigned char* iv);

        void EncryptAES128CBC(const std::string& dataIn, std::string& dataOut);
        void EncryptAES128ECB(const std::string& dataIn, std::string& dataOut);
        void EncryptAES256CBC(const std::string& dataIn, std::string& dataOut);
        void EncryptAES256ECB(const std::string& dataIn, std::string& dataOut);
};
#endif
#endif