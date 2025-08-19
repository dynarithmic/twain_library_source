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
#ifndef AES_256_ECB_H__
#define AES_256_ECB_H__

#define AES_BLOCK_SIZE 16
#define AES_KEY_SIZE_256 32

typedef struct {
	unsigned int roundkey[60];
} AES_CTX_256_ECB;

// Wrap the static methods in a struct
struct AES256_ECB
{
	static void AES_EncryptInit(AES_CTX_256_ECB* ctx, const unsigned char* key, const unsigned char* iv = nullptr);
	static void AES_Encrypt(AES_CTX_256_ECB* ctx, const unsigned char in_data[AES_BLOCK_SIZE], unsigned char out_data[AES_BLOCK_SIZE]);
	static void AES_CTX_Free(AES_CTX_256_ECB* ctx);
    static void AES_DecryptInit(AES_CTX_256_ECB* ctx, const unsigned char* key, const unsigned char* iv);
    static void AES_Decrypt(AES_CTX_256_ECB* ctx, const unsigned char in_data[AES_BLOCK_SIZE], unsigned char out_data[AES_BLOCK_SIZE]);
};

#if 0
void AES_DecryptInit(AES_CTX_256_ECB *ctx, const unsigned char *key) {
	AES_EncryptInit(ctx, key);
	
	unsigned int temp;
	
	// Next, invert the order of the round keys.
    for (unsigned char i = 0, j = 56; i < j; i += 4, j -= 4) {
        temp = ctx->roundkey[i + 0];
        ctx->roundkey[i + 0] = ctx->roundkey[j + 0];
        ctx->roundkey[j + 0] = temp;

        temp = ctx->roundkey[i + 1];
        ctx->roundkey[i + 1] = ctx->roundkey[j + 1];
        ctx->roundkey[j + 1] = temp;

        temp = ctx->roundkey[i + 2];
        ctx->roundkey[i + 2] = ctx->roundkey[j + 2];
        ctx->roundkey[j + 2] = temp;

        temp = ctx->roundkey[i + 3];
        ctx->roundkey[i + 3] = ctx->roundkey[j + 3];
        ctx->roundkey[j + 3] = temp;
    }

    // Finally, apply the inverse MixColumn transform to all round keys except the first and last.
    for (unsigned char index = 4; index < 56; index += 4) {
        ctx->roundkey[index] =
            Td0[Te4[(ctx->roundkey[index] >> 24) & 0xff] & 0xff] ^
            Td1[Te4[(ctx->roundkey[index] >> 16) & 0xff] & 0xff] ^
            Td2[Te4[(ctx->roundkey[index] >>  8) & 0xff] & 0xff] ^
            Td3[Te4[(ctx->roundkey[index] >>  0) & 0xff] & 0xff];
        ctx->roundkey[index + 1] =
            Td0[Te4[(ctx->roundkey[index + 1] >> 24) & 0xff] & 0xff] ^
            Td1[Te4[(ctx->roundkey[index + 1] >> 16) & 0xff] & 0xff] ^
            Td2[Te4[(ctx->roundkey[index + 1] >>  8) & 0xff] & 0xff] ^
            Td3[Te4[(ctx->roundkey[index + 1] >>  0) & 0xff] & 0xff];
        ctx->roundkey[index + 2] =
            Td0[Te4[(ctx->roundkey[index + 2] >> 24) & 0xff] & 0xff] ^
            Td1[Te4[(ctx->roundkey[index + 2] >> 16) & 0xff] & 0xff] ^
            Td2[Te4[(ctx->roundkey[index + 2] >>  8) & 0xff] & 0xff] ^
            Td3[Te4[(ctx->roundkey[index + 2] >>  0) & 0xff] & 0xff];
        ctx->roundkey[index + 3] =
            Td0[Te4[(ctx->roundkey[index + 3] >> 24) & 0xff] & 0xff] ^
            Td1[Te4[(ctx->roundkey[index + 3] >> 16) & 0xff] & 0xff] ^
            Td2[Te4[(ctx->roundkey[index + 3] >>  8) & 0xff] & 0xff] ^
            Td3[Te4[(ctx->roundkey[index + 3] >>  0) & 0xff] & 0xff];
    }
}


void AES_Decrypt(AES_CTX_256_ECB *ctx, const unsigned char in_data[AES_BLOCK_SIZE], unsigned char out_data[AES_BLOCK_SIZE]) {
	unsigned int s0, s1, s2, s3, t0, t1, t2, t3;
	
	s0 = GETU32(in_data + 0) ^ ctx->roundkey[0];
	s1 = GETU32(in_data + 4) ^ ctx->roundkey[1];
	s2 = GETU32(in_data + 8) ^ ctx->roundkey[2];
	s3 = GETU32(in_data + 12) ^ ctx->roundkey[3];
	
	t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ ctx->roundkey[ 4];
	t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ ctx->roundkey[ 5];
	t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ ctx->roundkey[ 6];
	t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ ctx->roundkey[ 7];
	
	s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >>  8) & 0xff] ^ Td3[t1 & 0xff] ^ ctx->roundkey[ 8];
	s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >>  8) & 0xff] ^ Td3[t2 & 0xff] ^ ctx->roundkey[ 9];
	s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >>  8) & 0xff] ^ Td3[t3 & 0xff] ^ ctx->roundkey[10];
	s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >>  8) & 0xff] ^ Td3[t0 & 0xff] ^ ctx->roundkey[11];
	
	t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ ctx->roundkey[12];
	t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ ctx->roundkey[13];
	t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ ctx->roundkey[14];
	t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ ctx->roundkey[15];
	
	s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >>  8) & 0xff] ^ Td3[t1 & 0xff] ^ ctx->roundkey[16];
	s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >>  8) & 0xff] ^ Td3[t2 & 0xff] ^ ctx->roundkey[17];
	s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >>  8) & 0xff] ^ Td3[t3 & 0xff] ^ ctx->roundkey[18];
	s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >>  8) & 0xff] ^ Td3[t0 & 0xff] ^ ctx->roundkey[19];
	
	t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ ctx->roundkey[20];
	t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ ctx->roundkey[21];
	t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ ctx->roundkey[22];
	t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ ctx->roundkey[23];
	
	s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >>  8) & 0xff] ^ Td3[t1 & 0xff] ^ ctx->roundkey[24];
	s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >>  8) & 0xff] ^ Td3[t2 & 0xff] ^ ctx->roundkey[25];
	s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >>  8) & 0xff] ^ Td3[t3 & 0xff] ^ ctx->roundkey[26];
	s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >>  8) & 0xff] ^ Td3[t0 & 0xff] ^ ctx->roundkey[27];
	
	t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ ctx->roundkey[28];
	t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ ctx->roundkey[29];
	t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ ctx->roundkey[30];
	t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ ctx->roundkey[31];
	
	s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >>  8) & 0xff] ^ Td3[t1 & 0xff] ^ ctx->roundkey[32];
	s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >>  8) & 0xff] ^ Td3[t2 & 0xff] ^ ctx->roundkey[33];
	s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >>  8) & 0xff] ^ Td3[t3 & 0xff] ^ ctx->roundkey[34];
	s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >>  8) & 0xff] ^ Td3[t0 & 0xff] ^ ctx->roundkey[35];
	
	t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ ctx->roundkey[36];
	t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ ctx->roundkey[37];
	t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ ctx->roundkey[38];
	t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ ctx->roundkey[39];
	
	s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >>  8) & 0xff] ^ Td3[t1 & 0xff] ^ ctx->roundkey[40];
	s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >>  8) & 0xff] ^ Td3[t2 & 0xff] ^ ctx->roundkey[41];
	s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >>  8) & 0xff] ^ Td3[t3 & 0xff] ^ ctx->roundkey[42];
	s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >>  8) & 0xff] ^ Td3[t0 & 0xff] ^ ctx->roundkey[43];
	
	t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ ctx->roundkey[44];
	t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ ctx->roundkey[45];
	t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ ctx->roundkey[46];
	t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ ctx->roundkey[47];
	
	s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >>  8) & 0xff] ^ Td3[t1 & 0xff] ^ ctx->roundkey[48];
	s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >>  8) & 0xff] ^ Td3[t2 & 0xff] ^ ctx->roundkey[49];
	s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >>  8) & 0xff] ^ Td3[t3 & 0xff] ^ ctx->roundkey[50];
	s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >>  8) & 0xff] ^ Td3[t0 & 0xff] ^ ctx->roundkey[51];
	
	t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >>  8) & 0xff] ^ Td3[s1 & 0xff] ^ ctx->roundkey[52];
	t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >>  8) & 0xff] ^ Td3[s2 & 0xff] ^ ctx->roundkey[53];
	t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >>  8) & 0xff] ^ Td3[s3 & 0xff] ^ ctx->roundkey[54];
	t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >>  8) & 0xff] ^ Td3[s0 & 0xff] ^ ctx->roundkey[55];
	
	s0 = (Td4[(t0 >> 24) & 0xff] & 0xff000000) ^ (Td4[(t3 >> 16) & 0xff] & 0x00ff0000) ^ (Td4[(t2 >>  8) & 0xff] & 0x0000ff00) ^ (Td4[(t1 >>  0) & 0xff] & 0x000000ff) ^ ctx->roundkey[56];
	s1 = (Td4[(t1 >> 24) & 0xff] & 0xff000000) ^ (Td4[(t0 >> 16) & 0xff] & 0x00ff0000) ^ (Td4[(t3 >>  8) & 0xff] & 0x0000ff00) ^ (Td4[(t2 >>  0) & 0xff] & 0x000000ff) ^ ctx->roundkey[57];
	s2 = (Td4[(t2 >> 24) & 0xff] & 0xff000000) ^ (Td4[(t1 >> 16) & 0xff] & 0x00ff0000) ^ (Td4[(t0 >>  8) & 0xff] & 0x0000ff00) ^ (Td4[(t3 >>  0) & 0xff] & 0x000000ff) ^ ctx->roundkey[58];
	s3 = (Td4[(t3 >> 24) & 0xff] & 0xff000000) ^ (Td4[(t2 >> 16) & 0xff] & 0x00ff0000) ^ (Td4[(t1 >>  8) & 0xff] & 0x0000ff00) ^ (Td4[(t0 >>  0) & 0xff] & 0x000000ff) ^ ctx->roundkey[59];
	
	PUTU32(out_data +  0, s0);
	PUTU32(out_data +  4, s1);
	PUTU32(out_data +  8, s2);
	PUTU32(out_data + 12, s3);
}
#endif
#endif