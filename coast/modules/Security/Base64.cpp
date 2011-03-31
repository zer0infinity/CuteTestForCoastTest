/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Base64.h"
#include "Anything.h"
#include "Dbg.h"
#include "SystemLog.h"
#define CHAR64(c)  (index_64[(unsigned char)(c)])

#define XX 127

// ------------------- Base64 ---------------------------------------------
RegisterEncoder(Base64);
RegisterAliasSecurityItem(b64, Base64);
RegisterEncoder(Base64Regular);
RegisterAliasSecurityItem(b64r, Base64Regular);

// base64 encoder, neglects newline after 73 characters
// since it is not used for file encoding, but only for string encoding
void Base64::DoEncode(String &encStr, const String &str) const
{
	StartTrace(Base64.DoEncode);
	Trace("input = " << str);
	const char *basis_64 = DoGetBasis();

	const int cBlockSize = 3;
	unsigned char c1, c2, c3;
	long blocks = str.Length() / cBlockSize;
	long remBytes = str.Length() % cBlockSize;
	long index = 0;

	const char *buf = str;
	for ( long i = 0; i < blocks; ++i, index += 3 ) {
		c1 = buf[index+0L];
		c2 = buf[index+1L];
		c3 = buf[index+2L];

		encStr.Append((char)basis_64[c1>>2]);
		encStr.Append((char)basis_64[((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4)]);
		encStr.Append((char)basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)]);
		encStr.Append((char)basis_64[c3 & 0x3F] );
	}

	if ( remBytes != 0 ) {
		c1 = buf[index];
		c2 = (index + 1L < str.Length()) ? buf[index+1L] : 0;
		c3 = 0;

		encStr.Append(basis_64[c1>>2]);
		encStr.Append(basis_64[((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4)]);

		if ( remBytes == 2 ) {
			encStr.Append(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)]);
			encStr.Append('=');
		} else {
			encStr.Append('=');
			encStr.Append('=');
		}
	}
	Trace("Result =" << encStr);
}

// base64 decoder, neglects newline after 73 characters
// since it is not used for file encoding, but only for string encoding
bool Base64::DoDecode(String &str, const String &encStr) const
{
	StartTrace(Base64.DoDecode);
	Trace("input = " << encStr);
	const int cBlockSize = 4;
	unsigned char c1, c2, c3, c4;
	unsigned char cr1, cr2, cr3, cr4;
	unsigned char byte1, byte2, byte3;
	long blocks = encStr.Length() / cBlockSize;
	long remBytes = encStr.Length() % cBlockSize; // should be 0
	long index = 0;

	for ( long i = 0; i < blocks; ++i, index += 4 ) {
		cr1 = encStr[index+0L];
		cr2 = encStr[index+1L];
		cr3 = encStr[index+2L];
		cr4 = encStr[index+3L];

		c1 = CHAR64(cr1);
		c2 = CHAR64(cr2);
		c3 = (cr3 == '=') ? 0 : CHAR64(cr3);
		c4 = (cr4 == '=') ? 0 : CHAR64(cr4);

		byte1 = ((c1 << 2) | ((c2 & 0x30) >> 4));
		str.Append((char) byte1 );
		if (cr3 == '=') {
			// we're done
		} else {
			byte2 = ((c2 & 0x0F) << 4) | ((c3 & 0x3C) >> 2);
			str.Append((char) (byte2) );

			if (cr4 != '=') {
				byte3 = (((c3 & 0x03) << 6) | c4);
				str.Append((char) (byte3) );
			}
		}
	}

	if ( remBytes != 0 ) {
		Trace("invalid aligned input string");
		SystemLog::Warning(String("Warning encoded String is corrupted - In str:") << str << " EncStr:" << encStr);
		return false;
	}
	return true;
}

// basis64 table defines conversion table for Base64 encoding scheme.
// The last character '/' is replaced with '$' since a / is recognized by
// the browser and has to be escaped
const char Base64::basis_64_wd[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-$";	// replace '+' with '-' and '/' with $
const char Base64Regular::basis_64_orig[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
 * Table for decoding base64
 */
const char Base64::index_64[256] = {
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, 63, XX, XX, XX, XX, XX, XX, 62, XX, 62, XX, 63,			// CR #24 replace '+' with '-'
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, XX, XX, XX, XX, XX, XX,
	XX, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, XX, XX, XX, XX, XX,
	XX, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
};

