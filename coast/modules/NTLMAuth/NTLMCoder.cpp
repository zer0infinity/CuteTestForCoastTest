/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "NTLMCoder.h"
#include "Base64.h"
#include <openssl/des.h> // be careful to really use our 3rdparty openssl instead of an older one
#include <openssl/md4.h>

static long decodeshort(const String &dm, long pos) {
	return (unsigned char) dm[pos] + ((unsigned char) dm[pos + 1L] << 8);
}
static void appendshort(String &buff, unsigned short val) {
	buff.Append((char) (val & 0xff)).Append((char) (val >> 8));
}
static void appendlong(String &buff, long val) {
	appendshort(buff, val & 0xffff);
	appendshort(buff, (val >> 16) & 0xffff);
}
String NTLMCoder::MakeBase64(const String &buff) {
	StartTrace(NTLMCoder.MakeBase64);
	String result;
	Base64Regular("NTLMb64").DoEncode(result, buff); //not an url
	Trace("result = " << result);
	return result;

}
bool NTLMCoder::DecodeBase64(String &result, const String &input) {
	Base64Regular b64("NTLMb64");
	return b64.DoDecode(result, input);
}

bool NTLMCoder::DecodeClientMsg(const String &base64msg, String &domain, String &host) {
	StartTrace(NTLMCoder.DecodeClientMsg);
	String dm;
	if (DecodeBase64(dm, base64msg) && "NTLMSSP" == dm.SubString(0L, 7L) && '\x01' == dm[8L] && '\x03' == dm[12L] && '\xb2' == dm[13L]) {
		Trace("decoded string len: " << dm.Length());
		long dlen = decodeshort(dm, 16L);
		long doff = decodeshort(dm, 20L);
		long hlen = decodeshort(dm, 24L);
		long hoff = decodeshort(dm, 28L);
		Trace("doff = " << doff << " dlen = " << dlen << " hoff (32) = " << hoff << " hlen= " << hlen);
		domain = dm.SubString(doff, dlen);
		host = dm.SubString(hoff, hlen);
		return true;
	}

	return false;
}
String NTLMCoder::EncodeClientMsg(String domain, String host) {
	StartTrace(NTLMCoder.EncodeClientMsg);
	String buff((void *) "NTLMSSP\0\x01\0\0\0\x03\xb2\0\0", 16L);
	unsigned short dlen = domain.Length();
	unsigned short hlen = host.Length();
	unsigned short doff = 32 + hlen;
	Trace("doff = " << long(doff) << " dlen = " << long(dlen) << " hlen= " << long(hlen));
	appendshort(buff, dlen);
	appendshort(buff, dlen);
	appendshort(buff, doff);
	appendshort(buff, 0);
	appendshort(buff, hlen);
	appendshort(buff, hlen);
	appendshort(buff, 0x20);
	appendshort(buff, 0);
	Trace("should be 32:" << buff.Length());
	Assert(32 == buff.Length());
	host.ToUpper();
	domain.ToUpper();
	Trace("host.ToUpper() = " << host << " domain.ToUpper() = " << domain);
	buff.Append(host);
	buff.Append(domain);
	Trace(buff.Length());
	Assert(buff.Length() == (32 + hlen + dlen));
	return MakeBase64(buff);
}

String NTLMCoder::EncodeServerNonce(String nonce, long flags) {
	StartTrace(NTLMCoder.EncodeServerNonce);
	String buff((void *) "NTLMSSP\0\x2\0\0\0\x0\0\0\0\x28\0\0\0", 20L);
	appendlong(buff, flags);
	long i;
	for (i = 0L; i < 8; i++) {
		buff.Append(nonce[i]);
	}
	for (i = 0; i < 8; i++) {
		buff.Append((char) 0);
	} Assert(40L == buff.Length());
	return MakeBase64(buff);
}

bool NTLMCoder::DecodeServerNonce(const String &base64msg, String &nonce) {
	StartTrace(NTLMCoder.DecodeServerNonce);
	String dm;
	if (DecodeBase64(dm, base64msg) && 40L == dm.Length() && "NTLMSSP" == dm.SubString(0L, 7L) && '\x02' == dm[8L] && '\x28' == dm[16L]
			&& '\0' == dm[17L] && '\x01' == dm[20L] && '\x82' == dm[21L]) {
		nonce = dm.SubString(24L, 8L);
		Trace("nonce = " << nonce);
		// should be hex?
		return true;
	}
	return false;
}

/*
 * turns a 56 bit key into the 64 bit, odd parity key and sets the key.
 * The key schedule ks is also set.
 */
static void setup_des_key(const String &keystr, DES_key_schedule *ks) {
	DES_cblock key;
	const unsigned char *key_56 = (const unsigned char *) (const char *) keystr;

	key[0] = key_56[0];
	key[1] = ((key_56[0] << 7) & 0xFF) | (key_56[1] >> 1);
	key[2] = ((key_56[1] << 6) & 0xFF) | (key_56[2] >> 2);
	key[3] = ((key_56[2] << 5) & 0xFF) | (key_56[3] >> 3);
	key[4] = ((key_56[3] << 4) & 0xFF) | (key_56[4] >> 4);
	key[5] = ((key_56[4] << 3) & 0xFF) | (key_56[5] >> 5);
	key[6] = ((key_56[5] << 2) & 0xFF) | (key_56[6] >> 6);
	key[7] = (key_56[6] << 1) & 0xFF;

	DES_set_odd_parity(&key);
	DES_set_key(&key, ks);
}

/*
 * takes a 21 byte array and treats it as 3 56-bit DES keys. The
 * 8 byte plaintext is encrypted with each key and the resulting 24
 * bytes are stored in the results array.
 */
String NTLMCoder::CalculateNonceHash(const String &keys, const String &plaintext) {
	DES_key_schedule ks;
	DES_cblock pt8;
	for (long i = 0; i < 8; i++) {
		pt8[i] = (unsigned char) plaintext[i];
	}

	String result;
	DES_cblock res;
	setup_des_key(keys.SubString(0L, 7L), &ks);
	DES_ecb_encrypt(&pt8, &res, &ks, DES_ENCRYPT);
	result.Append((void *) &res[0], 8);

	setup_des_key(keys.SubString(7L, 7L), &ks);
	DES_ecb_encrypt(&pt8, &res, &ks, DES_ENCRYPT);
	result.Append((void *) &res[0], 8);

	setup_des_key(keys.SubString(14L, 7L), &ks);
	DES_ecb_encrypt(&pt8, &res, &ks, DES_ENCRYPT);
	result.Append((void *) &res[0], 8);
	return result;
}

String NTLMCoder::EncodeLMPassword(String password) {
	StartTrace(NTLMCoder.EncodeLMPassword);
	Trace("password: '" << password << "'");
	password.Trim(14L);
	password.ToUpper();
	while (password.Length() < 14) {
		password.Append(char(0));
	}Trace("password as used:" << password);
	/* create LanManager hashed password */

	unsigned char magic[8] = { 0x4B, 0x47, 0x53, 0x21, 0x40, 0x23, 0x24, 0x25 };
	Trace("magic = " << String((const char *)magic, 8));
	DES_cblock lmhpw;
	Assert(8 == sizeof(lmhpw));
	des_key_schedule ks;
	String result;
	setup_des_key(password.SubString(0L, 7L), &ks);
	DES_ecb_encrypt(&magic, &lmhpw, &ks, DES_ENCRYPT);
	long i;
	for (i = 0; i < 8; i++) {
		result.Append((char) lmhpw[i]);
	}

	setup_des_key(password.SubString(7L, 7L), &ks);
	DES_ecb_encrypt(&magic, &lmhpw, &ks, DES_ENCRYPT);
	for (i = 0; i < 8; i++) {
		result.Append((char) lmhpw[i]);
	}
	while (result.Length() < 21) {
		result.Append(char(0));
	} Assert(result.Length() == 21);
	return result;
}
String NTLMCoder::EncodeNTPassword(String password) {
	StartTrace(NTLMCoder.EncodeNTPassword);
	Trace("password ='" << password << "'");

	String ntpw = MakeUtf16(password);

	unsigned char nt_hpw[21];
	MD4_CTX context;
	MD4_Init(&context);
	MD4_Update(&context, (const char *) ntpw, ntpw.Length());
	MD4_Final(nt_hpw, &context);
	String result((void *) nt_hpw, 16L);
	while (result.Length() < 21) {
		result.Append(char(0));
	} Assert(result.Length() == 21);
	return result;
}
String NTLMCoder::MakeUtf16(const String &input) {
	String result;
	for (long i = 0; i < input.Length(); i++) {
		result.Append(input[i]);
		result.Append((char) '\0'); // "dummy" UTF 16 encoding
	}
	return result;

}
static String cutfrommessage(const String dm, long lengthpos) {
	return dm.SubString(decodeshort(dm, lengthpos + 4), decodeshort(dm, lengthpos));
}
String NTLMCoder::StripUtf16(const String &input) {
	String result(input.Length() / 2);
	for (long i = 0; i < input.Length(); i += 2) {
		result.Append(input[i]);
		Assert(input[i+1L] == 0);
	}
	return result;
}
bool NTLMCoder::DecodeResponse(const String &base64msg, const String &nonce, const String &lmhash, const String &nthash, String &domain,
		String &host, String &user) {
	StartTrace(NTLMCoder.DecodeResponse);
	String dm;
	if (DecodeBase64(dm, base64msg) && "NTLMSSP" == dm.SubString(0L, 7L) && '\x03' == dm[8L] && '\x01' == dm[60L] && '\x82' == dm[61L]
			&& dm.Length() == decodeshort(dm, 56L)) {
		Trace("decoded string len: " << dm.Length());
		domain = StripUtf16(cutfrommessage(dm, 28L));
		user = StripUtf16(cutfrommessage(dm, 36L));
		host = StripUtf16(cutfrommessage(dm, 44L));
		Trace("domain =" << domain << " user =" << user << " host =" << host);
		return CalculateNonceHash(lmhash, nonce) == cutfrommessage(dm, 12L) && CalculateNonceHash(nthash, nonce) == cutfrommessage(dm, 20L);
	}
	return false;
}
static void appendlenoff(String &buff, long length, short off) {
	appendshort(buff, length);
	appendshort(buff, length);
	appendshort(buff, off);
	appendshort(buff, 0);
}
String NTLMCoder::MakeUtf16Upper(const String &in) {
	String s = in;
	s.ToUpper();
	return MakeUtf16(s);
}
String NTLMCoder::EncodeResponse(const String &nonce, const String &lmhash, const String &nthash, const String &domain, const String &host,
		const String &user) {
	StartTrace(NTLMCoder.EncodeResponse);
	String buff((void *) "NTLMSSP\0\x03\0\0\0", 12L);
	String lmresp = CalculateNonceHash(lmhash, nonce);
	String ntresp = CalculateNonceHash(nthash, nonce);
	String utfdom = MakeUtf16Upper(domain);
	String utfhost = MakeUtf16Upper(host);
	String utfuser = MakeUtf16(user);
	short uoff = 64 + utfdom.Length();
	short hoff = uoff + utfuser.Length();
	short lmoff = hoff + utfhost.Length();
	short msglen = lmoff + 24 + 24;
	appendlenoff(buff, lmresp.Length(), lmoff);
	appendlenoff(buff, ntresp.Length(), lmoff + lmresp.Length());
	appendlenoff(buff, utfdom.Length(), 64);
	appendlenoff(buff, utfuser.Length(), uoff);
	appendlenoff(buff, utfhost.Length(), hoff);
	appendlenoff(buff, 0, msglen);
	//--- 52 ---
	appendshort(buff, 0x8201);
	appendshort(buff, 0);
	Assert(64 == buff.Length());
	buff.Append(utfdom).Append(utfuser).Append(utfhost);
	buff.Append(lmresp).Append(ntresp);
	Assert(msglen == buff.Length());
	//--- done with message
	return MakeBase64(buff);

}
