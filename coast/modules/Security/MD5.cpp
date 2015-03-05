/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MD5.h"

// ------------------- MD5 ---------------------------------------------
/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 */

#include <cstring>		/* for memcpy() */
#ifdef __alpha
typedef unsigned int uint32;
#else
typedef unsigned long uint32;
#endif

class MD5Context {
	MD5Context(const MD5Context &);
	MD5Context &operator=(const MD5Context &);
public:
	MD5Context();

	void Init();
	void Update(const unsigned char *buf, unsigned len);
	void Final(unsigned char digest[16]);
	void Transform(uint32 buf[4], const uint32 in[16]);

protected:
	uint32 fBuf[4];
	uint32 fBits[2];
	unsigned char fIn[64];
};

//!@FIXME there is no way to determine processor byte order
// by the preprocessor.
#ifdef __sparc__
#define HIGHFIRST
#endif
// do not know for others yet, intel is definitely LOWFIRST

#ifndef HIGHFIRST
#define byteReverse(buf, len)	/* Nothing */
#else
void byteReverse(unsigned char *buf, unsigned longs);

#ifndef ASM_MD5
/*
 * Note: this code is harmless on little-endian machines.
 */
void byteReverse(unsigned char *buf, unsigned longs)
{
	uint32 t;
	do {
		t = (uint32) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
		((unsigned) buf[1] << 8 | buf[0]);
		*(uint32 *) buf = t;
		buf += 4;
	}while (--longs);
}
#endif
#endif
MD5Context::MD5Context() {
	Init();
}

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
void MD5Context::Init() {
	fBuf[0] = 0x67452301;
	fBuf[1] = 0xefcdab89;
	fBuf[2] = 0x98badcfe;
	fBuf[3] = 0x10325476;

	fBits[0] = 0;
	fBits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void MD5Context::Update(unsigned char const *buf, unsigned len) {
	uint32 t;

	/* Update bitcount */

	t = fBits[0];
	if ((fBits[0] = t + ((uint32) len << 3)) < t) {
		++fBits[1]; /* Carry from low to high */
	}
	fBits[1] += len >> 29;

	t = (t >> 3) & 0x3f; /* Bytes already in shsInfo->data */

	/* Handle any leading odd-sized chunks */

	if (t) {
		unsigned char *p = (unsigned char *) fIn + t;

		t = 64 - t;
		if (len < t) {
			memcpy(p, buf, len);
			return;
		}
		memcpy(p, buf, t);
		byteReverse(fIn, 16);
		Transform(this->fBuf, (uint32 *) fIn);
		buf += t;
		len -= t;
	}
	/* Process data in 64-byte chunks */

	while (len >= 64) {
		memcpy(fIn, buf, 64);
		byteReverse(fIn, 16);
		Transform(this->fBuf, (uint32 *) fIn);
		buf += 64;
		len -= 64;
	}

	/* Handle any remaining bytes of data. */

	memcpy(fIn, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void MD5Context::Final(unsigned char digest[16]) {
	unsigned count;
	unsigned char *p;

	/* Compute number of bytes mod 64 */
	count = (fBits[0] >> 3) & 0x3F;

	/* Set the first char of padding to 0x80.  This is safe since there is
	 always at least one byte free */
	p = fIn + count;
	*p++ = 0x80;

	/* Bytes of padding needed to make 64 bytes */
	count = 64 - 1 - count;

	/* Pad out to 56 mod 64 */
	if (count < 8) {
		/* Two lots of padding:  Pad the first block to 64 bytes */
		memset(p, 0, count);
		byteReverse(fIn, 16);
		Transform(fBuf, (uint32 *) fIn);

		/* Now fill the next block with 56 bytes */
		memset(fIn, 0, 56);
	} else {
		/* Pad block to 56 bytes */
		memset(p, 0, count - 8);
	}byteReverse(fIn, 14);

	/* Append length in bits and transform */
	((uint32 *) fIn)[14] = fBits[0];
	((uint32 *) fIn)[15] = fBits[1];

	Transform(fBuf, (uint32 *) fIn);
	byteReverse((unsigned char *) fBuf, 4);
	memcpy(digest, fBuf, 16);
}

#ifndef ASM_MD5

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#ifdef __PUREC__
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f /*(x, y, z)*/ + data,  w = w<<s | w>>(32-s),  w += x )
#else
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )
#endif

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
void MD5Context::Transform(uint32 buf[4], uint32 const in[16]) {
	register uint32 a, b, c, d;

	a = buf[0];
	b = buf[1];
	c = buf[2];
	d = buf[3];

#ifdef __PUREC__	/* PureC Weirdness... (GG) */
	MD5STEP(F1(b, c, d), a, b, c, d, in[0] + 0xd76aa478L, 7);
	MD5STEP(F1(a, b, c), d, a, b, c, in[1] + 0xe8c7b756L, 12);
	MD5STEP(F1(d, a, b), c, d, a, b, in[2] + 0x242070dbL, 17);
	MD5STEP(F1(c, d, a), b, c, d, a, in[3] + 0xc1bdceeeL, 22);
	MD5STEP(F1(b, c, d), a, b, c, d, in[4] + 0xf57c0fafL, 7);
	MD5STEP(F1(a, b, c), d, a, b, c, in[5] + 0x4787c62aL, 12);
	MD5STEP(F1(d, a, b), c, d, a, b, in[6] + 0xa8304613L, 17);
	MD5STEP(F1(c, d, a), b, c, d, a, in[7] + 0xfd469501L, 22);
	MD5STEP(F1(b, c, d), a, b, c, d, in[8] + 0x698098d8L, 7);
	MD5STEP(F1(a, b, c), d, a, b, c, in[9] + 0x8b44f7afL, 12);
	MD5STEP(F1(d, a, b), c, d, a, b, in[10] + 0xffff5bb1L, 17);
	MD5STEP(F1(c, d, a), b, c, d, a, in[11] + 0x895cd7beL, 22);
	MD5STEP(F1(b, c, d), a, b, c, d, in[12] + 0x6b901122L, 7);
	MD5STEP(F1(a, b, c), d, a, b, c, in[13] + 0xfd987193L, 12);
	MD5STEP(F1(d, a, b), c, d, a, b, in[14] + 0xa679438eL, 17);
	MD5STEP(F1(c, d, a), b, c, d, a, in[15] + 0x49b40821L, 22);

	MD5STEP(F2(b, c, d), a, b, c, d, in[1] + 0xf61e2562L, 5);
	MD5STEP(F2(a, b, c), d, a, b, c, in[6] + 0xc040b340L, 9);
	MD5STEP(F2(d, a, b), c, d, a, b, in[11] + 0x265e5a51L, 14);
	MD5STEP(F2(c, d, a), b, c, d, a, in[0] + 0xe9b6c7aaL, 20);
	MD5STEP(F2(b, c, d), a, b, c, d, in[5] + 0xd62f105dL, 5);
	MD5STEP(F2(a, b, c), d, a, b, c, in[10] + 0x02441453L, 9);
	MD5STEP(F2(d, a, b), c, d, a, b, in[15] + 0xd8a1e681L, 14);
	MD5STEP(F2(c, d, a), b, c, d, a, in[4] + 0xe7d3fbc8L, 20);
	MD5STEP(F2(b, c, d), a, b, c, d, in[9] + 0x21e1cde6L, 5);
	MD5STEP(F2(a, b, c), d, a, b, c, in[14] + 0xc33707d6L, 9);
	MD5STEP(F2(d, a, b), c, d, a, b, in[3] + 0xf4d50d87L, 14);
	MD5STEP(F2(c, d, a), b, c, d, a, in[8] + 0x455a14edL, 20);
	MD5STEP(F2(b, c, d), a, b, c, d, in[13] + 0xa9e3e905L, 5);
	MD5STEP(F2(a, b, c), d, a, b, c, in[2] + 0xfcefa3f8L, 9);
	MD5STEP(F2(d, a, b), c, d, a, b, in[7] + 0x676f02d9L, 14);
	MD5STEP(F2(c, d, a), b, c, d, a, in[12] + 0x8d2a4c8aL, 20);

	MD5STEP(F3(b, c, d), a, b, c, d, in[5] + 0xfffa3942L, 4);
	MD5STEP(F3(a, b, c), d, a, b, c, in[8] + 0x8771f681L, 11);
	MD5STEP(F3(d, a, b), c, d, a, b, in[11] + 0x6d9d6122L, 16);
	MD5STEP(F3(c, d, a), b, c, d, a, in[14] + 0xfde5380cL, 23);
	MD5STEP(F3(b, c, d), a, b, c, d, in[1] + 0xa4beea44L, 4);
	MD5STEP(F3(a, b, c), d, a, b, c, in[4] + 0x4bdecfa9L, 11);
	MD5STEP(F3(d, a, b), c, d, a, b, in[7] + 0xf6bb4b60L, 16);
	MD5STEP(F3(c, d, a), b, c, d, a, in[10] + 0xbebfbc70L, 23);
	MD5STEP(F3(b, c, d), a, b, c, d, in[13] + 0x289b7ec6L, 4);
	MD5STEP(F3(a, b, c), d, a, b, c, in[0] + 0xeaa127faL, 11);
	MD5STEP(F3(d, a, b), c, d, a, b, in[3] + 0xd4ef3085L, 16);
	MD5STEP(F3(c, d, a), b, c, d, a, in[6] + 0x04881d05L, 23);
	MD5STEP(F3(b, c, d), a, b, c, d, in[9] + 0xd9d4d039L, 4);
	MD5STEP(F3(a, b, c), d, a, b, c, in[12] + 0xe6db99e5L, 11);
	MD5STEP(F3(d, a, b), c, d, a, b, in[15] + 0x1fa27cf8L, 16);
	MD5STEP(F3(c, d, a), b, c, d, a, in[2] + 0xc4ac5665L, 23);

	MD5STEP(F4(b, c, d), a, b, c, d, in[0] + 0xf4292244L, 6);
	MD5STEP(F4(a, b, c), d, a, b, c, in[7] + 0x432aff97L, 10);
	MD5STEP(F4(d, a, b), c, d, a, b, in[14] + 0xab9423a7L, 15);
	MD5STEP(F4(c, d, a), b, c, d, a, in[5] + 0xfc93a039L, 21);
	MD5STEP(F4(b, c, d), a, b, c, d, in[12] + 0x655b59c3L, 6);
	MD5STEP(F4(a, b, c), d, a, b, c, in[3] + 0x8f0ccc92L, 10);
	MD5STEP(F4(d, a, b), c, d, a, b, in[10] + 0xffeff47dL, 15);
	MD5STEP(F4(c, d, a), b, c, d, a, in[1] + 0x85845dd1L, 21);
	MD5STEP(F4(b, c, d), a, b, c, d, in[8] + 0x6fa87e4fL, 6);
	MD5STEP(F4(a, b, c), d, a, b, c, in[15] + 0xfe2ce6e0L, 10);
	MD5STEP(F4(d, a, b), c, d, a, b, in[6] + 0xa3014314L, 15);
	MD5STEP(F4(c, d, a), b, c, d, a, in[13] + 0x4e0811a1L, 21);
	MD5STEP(F4(b, c, d), a, b, c, d, in[4] + 0xf7537e82L, 6);
	MD5STEP(F4(a, b, c), d, a, b, c, in[11] + 0xbd3af235L, 10);
	MD5STEP(F4(d, a, b), c, d, a, b, in[2] + 0x2ad7d2bbL, 15);
	MD5STEP(F4(c, d, a), b, c, d, a, in[9] + 0xeb86d391L, 21);
#else
	MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
	MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
	MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
	MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
	MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
	MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
	MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
	MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
	MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
	MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
	MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
	MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
	MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
	MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
	MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
	MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

	MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
	MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
	MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
	MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
	MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
	MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
	MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
	MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
	MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
	MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
	MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
	MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
	MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
	MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
	MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
	MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

	MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
	MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
	MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
	MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
	MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
	MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
	MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
	MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
	MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
	MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
	MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
	MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
	MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
	MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
	MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
	MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

	MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
	MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
	MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
	MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
	MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
	MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
	MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
	MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
	MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
	MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
	MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
	MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
	MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
	MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
	MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
	MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);
#endif

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}

#endif

RegisterSigner(MD5Signer);
RegisterAliasSecurityItem(md5, MD5Signer);

MD5Signer::MD5Signer(const char *name) :
		Signer(name) {
	InitKey(fgcLegacyMasterKey);
}

void MD5Signer::InitKey(const String &key) {
	StartTrace1(MD5Signer.InitKey, fName);
	Trace("Configured name: " << fName << " using key:" << key);
	fKey = key;
}

void MD5Signer::DoHash(const String &cleartext, String &hashvalue) {
	StartTrace(MD5Signer.DoHash);
	Trace("Cleartext: <" << cleartext << ">");

	MD5Context md5ctx;
	unsigned char digest[16];

	md5ctx.Update((const unsigned char *) (const char *) cleartext, cleartext.Length());
	md5ctx.Final(digest);

	hashvalue = "";
	hashvalue.Append((void *) digest, 16);
//	MD5 md5ctx;
//
//	md5ctx.Update((const unsigned char *)(const char *)cleartext, cleartext.Length());
//	md5ctx.Finalize(hashvalue);

	Trace("Hashvalue: <" << String().AppendAsHex((const unsigned char *)(const char *)hashvalue, hashvalue.Length(), ' ') << ">");
}
void MD5Signer::DoEncode(String &scrambledText, const String &cleartext) const {
	DoSign(fKey, scrambledText, cleartext);
}
bool MD5Signer::DoDecode(String &cleartext, const String &scrambledText) const {
	return DoCheck(fKey, cleartext, scrambledText);
}

void MD5Signer::DoSign(const String &strkey, String &scrambledText, const String &cleartext) {
	StartTrace(MD5Signer.DoSign);
	Trace("key: <" << strkey << ">");
	Trace("Cleartext: <" << cleartext << ">");

	MD5Context md5ctx;
	unsigned char digest[16];
	const char *msg = cleartext;
	const char *key = strkey;

	md5ctx.Update((const unsigned char *) msg, cleartext.Length());
	md5ctx.Update((const unsigned char *) key, strkey.Length());
	md5ctx.Final(digest);

	String strDigest((void *) digest, 16);

	scrambledText << strDigest << cleartext << '$'; // end marker

}

bool MD5Signer::DoCheck(const String &strkey, String &cleartext, const String &scrambledText) {
	StartTrace(MD5Signer.DoCheck);
	Trace("key: <" << strkey << ">");
	Trace("Scrambled: <" << String().AppendAsHex((const unsigned char *)(const char *)scrambledText, scrambledText.Length(), ' ') << ">");

	long effLength = scrambledText.StrRChr('$');
	Trace("EffLength: " << effLength);

	if (effLength == -1) {
		effLength = scrambledText.Length();
	}
	if (effLength <= 16) {
		return false;
	}Trace("EffLength: " << effLength);
	MD5Context md5ctx;
	unsigned char digest[16];
	const char *msg = scrambledText;
	msg += 16;

	const char *key = strkey;

	Trace("Msg: <" << msg << ">");
	md5ctx.Update(((const unsigned char *) msg), (effLength - 16));
	md5ctx.Update((const unsigned char *) key, strkey.Length());
	md5ctx.Final(digest);
	Trace("Msg: <" << msg << ">");

	if (::memcmp(digest, (const char *) scrambledText, 16) == 0) {
		cleartext = scrambledText.SubString(16, effLength - 16);
		Trace("Cleartext: <" << cleartext << ">");
		Trace("returning true");
		return true;
	}Trace("returning false");

	return false;
}

