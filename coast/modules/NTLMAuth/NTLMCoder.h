/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _NTLMCoder_H
#define _NTLMCoder_H

#include "config_ntlmauth.h"
#include "ITOString.h"

//---- NTLMCoder ----------------------------------------------------------
//! <B>encode and decode NTLM HTTP Authenticate messages</B>
/*!
basic mechanics for implementing WWW-Authenticate: NTLM
*/
class EXPORTDECL_NTLMAUTH NTLMCoder
{
public:

	//--- public api
	//! Decode "Type1" message from a client to server
	//! assume messages String is base64 encoded as in
	//! Authorization: NTLM base64encodedresponsetype1containingdomainandhost
	//! Authorization: NTLM TlRMTVNTUAABAAAAA7IAAAoACgApAAAACQAJACAAAABMSUdIVENJVFlVUlNBLU1JTk9S
	//! host name "LIGHTCITY" domain "URSA-MINOR"
	//! see http://www.innovation.ch/java/ntlm.html
	//! \param base64ms base64 encoded message as passed by client
	//! \param domain return value of decoded domain
	//! \param host  return value of decoded host name (not FQDN)
	//! \return if message conforms to format
	static bool DecodeClientMsg(const String &base64msg, String &domain, String &host);
	static String EncodeClientMsg(String domain, String host);
	static String EncodeServerNonce(String nonce, long flags = 0x8201);
	static bool DecodeServerNonce(const String &base64msg, String &nonce);
	static bool DecodeResponse(const String &base64msg, const String &nonce,
							   const String &lmhash, const String &nthash,
							   String &domain, String &host, String &user);
	static String EncodeResponse(const String &nonce,
								 const String &lmhash, const String &nthash,
								 const String &domain, const String &host, const String &user);

	static String EncodeLMPassword(String password);
	static String EncodeNTPassword(String password);
	static String MakeUtf16(const String &input);
	static String StripUtf16(const String &input);
	static String CalculateNonceHash(const String &keys, const String &plaintext);

	static String MakeBase64(const String &buffer);
	static bool DecodeBase64(String &result, const String &input );
	static String MakeUtf16Upper(const String &in);
protected:

private:
	//--- constructors -- only a static API
	NTLMCoder() {}
	NTLMCoder(const NTLMCoder &) {}
	~NTLMCoder() {}
};

#endif
