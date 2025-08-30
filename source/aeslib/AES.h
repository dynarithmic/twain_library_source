/*
MIT License

Copyright (c) 2019 SergeyBel

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

// Modified by Dynarithmic Software, 2025
#ifndef AES_H_
#define AES_H_

#include <vector>

enum class AESKeyLength { AES_128, AES_192, AES_256 };
enum class AESMode {AES_CBC, AES_ECB};
#define AES_BLOCK_SIZE 16

class AES {
 private:
  static constexpr unsigned int Nb = 4;
  static constexpr unsigned int blockBytesLen = 4 * Nb * sizeof(unsigned char);

  unsigned int Nk;
  unsigned int Nr;

  void SubBytes(unsigned char state[4][Nb]);

  void ShiftRow(unsigned char state[4][Nb], unsigned int i,
                unsigned int n);  // shift row i on n positions

  void ShiftRows(unsigned char state[4][Nb]);

  unsigned char xtime(unsigned char b);  // multiply on x

  void MixColumns(unsigned char state[4][Nb]);

  void AddRoundKey(unsigned char state[4][Nb], unsigned char *key);

  void SubWord(unsigned char *a);

  void RotWord(unsigned char *a);

  void XorWords(unsigned char *a, unsigned char *b, unsigned char *c);

  void Rcon(unsigned char *a, unsigned int n);

  void InvSubBytes(unsigned char state[4][Nb]);

  void InvMixColumns(unsigned char state[4][Nb]);

  void InvShiftRows(unsigned char state[4][Nb]);

  void CheckLength(unsigned int len);

  void KeyExpansion(const unsigned char key[], unsigned char w[]);

  void EncryptBlock(const unsigned char in[], unsigned char out[],
                    unsigned char *roundKeys);

  void DecryptBlock(const unsigned char in[], unsigned char out[],
                    unsigned char *roundKeys);

  void XorBlocks(const unsigned char *a, const unsigned char *b,
                 unsigned char *c, unsigned int len);

 public:
  explicit AES(const AESKeyLength keyLength = AESKeyLength::AES_256);

  std::vector<unsigned char> EncryptECB(const unsigned char in[], unsigned int inLen,
                                        const unsigned char key[]);

  std::vector<unsigned char> DecryptECB(const unsigned char in[], unsigned int inLen,
                                        const unsigned char key[]);

  std::vector<unsigned char> EncryptCBC(const unsigned char in[], unsigned int inLen,
                                        const unsigned char key[], const unsigned char *iv);

  std::vector<unsigned char> DecryptCBC(const unsigned char in[], unsigned int inLen,
                                        const unsigned char key[], const unsigned char *iv);

  std::vector<unsigned char> EncryptCFB(const unsigned char in[], unsigned int inLen,
                                        const unsigned char key[], const unsigned char *iv);

  std::vector<unsigned char> DecryptCFB(const unsigned char in[], unsigned int inLen,
                                        const unsigned char key[], const unsigned char *iv);

  std::vector<unsigned char> EncryptECB(const std::vector<unsigned char>& in,
                                        const std::vector<unsigned char>& key);

  std::vector<unsigned char> DecryptECB(const std::vector<unsigned char>& in,
                                        const std::vector<unsigned char>& key);

  std::vector<unsigned char> EncryptCBC(const std::vector<unsigned char>& in,
                                        const std::vector<unsigned char>& key,
                                        const std::vector<unsigned char>& iv);

  std::vector<unsigned char> DecryptCBC(const std::vector<unsigned char>& in,
                                        const std::vector<unsigned char>& key,
                                        const std::vector<unsigned char>& iv);

  std::vector<unsigned char> EncryptCFB(const std::vector<unsigned char>& in,
                                        const std::vector<unsigned char>& key,
                                        const std::vector<unsigned char>& iv);

  std::vector<unsigned char> DecryptCFB(const std::vector<unsigned char>& in,
                                        const std::vector<unsigned char>& key,
                                        const std::vector<unsigned char>& iv);
};
#endif
