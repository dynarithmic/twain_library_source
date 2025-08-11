/* MIT License

Copyright(c) 2024 Hallo Weeks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// File changes for DTWAIN done here.
// See above license for original author(s)
#ifndef AES_128_CBC_H__
#define AES_128_CBC_H__

#define AES_BLOCK_SIZE 16
#define AES_KEY_SIZE 16

#define GETU32(in_data) (((unsigned int)(in_data)[0] << 24) ^ \
						((unsigned int)(in_data)[1] << 16) ^ \
						((unsigned int)(in_data)[2] <<  8) ^ \
						((unsigned int)(in_data)[3] <<  0))

#define PUTU32(out_data, st) { (out_data)[0] = (unsigned char)((st) >> 24); \
							  (out_data)[1] = (unsigned char)((st) >> 16); \
							  (out_data)[2] = (unsigned char)((st) >>  8); \
							  (out_data)[3] = (unsigned char)((st) >>  0); }

typedef struct {
	unsigned int roundkey[44];
	unsigned int iv[4];
} AES_CTX;

// Wrap the static methods in a struct
struct AES128
{
	static void AES_EncryptInit(AES_CTX* ctx, const unsigned char* key, const unsigned char* iv);
	static void AES_Encrypt(AES_CTX* ctx, const unsigned char in_data[AES_BLOCK_SIZE], unsigned char out_data[AES_BLOCK_SIZE]);
    static void AES_DecryptInit(AES_CTX* ctx, const unsigned char* key, const unsigned char* iv);
    static void AES_Decrypt(AES_CTX* ctx, const unsigned char in_data[AES_BLOCK_SIZE], unsigned char out_data[AES_BLOCK_SIZE]);
};
#endif
