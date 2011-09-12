/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _NTLMCoder_H
#define _NTLMCoder_H

#include "ITOString.h"

//! encode and decode NTLM HTTP Authenticate messages
/*!
 basic mechanics for implementing WWW-Authenticate: NTLM
 */
namespace NTLMCoder {
	enum Flags {
		/*! Indicates that Unicode strings are supported for use in security
		 * buffer data. */
		NegotiateUnicode = 0x00000001,
		/*! Indicates that OEM strings are supported for use in security
		 * buffer data. */
		NegotiateOEM = 0x00000002,
		/*! Requests that the server's authentication realm be included in
		 * the Type 2 message. */
		RequestTarget = 0x00000004,
		/*! This flag's usage has not been identified. */
		unknown1 = 0x00000008,
		/*! Specifies that authenticated communication between the client and
		 * server should carry a digital signature (message integrity). */
		NegotiateSign = 0x00000010,
		/*! Specifies that authenticated communication between the client and
		 * server should be encrypted (message confidentiality). */
		NegotiateSeal = 0x00000020,
		/*! Indicates that datagram authentication is being used. */
		NegotiateDatagramStyle = 0x00000040,
		/*! Indicates that the Lan Manager Session Key should be used for
		 * signing and sealing authenticated communications. */
		NegotiateLanManagerKey = 0x00000080,
		/*! This flag's usage has not been identified. */
		NegotiateNetware = 0x00000100,
		/*! Indicates that NTLM authentication is being used. */
		NegotiateNTLM = 0x00000200,
		/*! This flag's usage has not been identified. */
		unknown2 = 0x00000400,
		/*! Sent by the client in the Type 3 message to indicate that an
		 * anonymous context has been established. This also affects the
		 * response fields (as detailed in the "Anonymous Response" section). */
		NegotiateAnonymous = 0x00000800,
		/*! Sent by the client in the Type 1 message to indicate that the
		 * name of the domain in which the client workstation has membership
		 * is included in the message. This is used by the server to determine
		 * whether the client is eligible for local authentication. */
		NegotiateDomainSupplied = 0x00001000,
		/*! Sent by the client in the Type 1 message to indicate that the
		 * client workstation's name is included in the message. This is used
		 * by the server to determine whether the client is eligible for local
		 * authentication. */
		NegotiateWorkstationSupplied = 0x00002000,
		/*! Sent by the server to indicate that the server and client are on
		 * the same machine. Implies that the client may use the established
		 * local credentials for authentication instead of calculating a
		 * response to the challenge. */
		NegotiateLocalCall = 0x00004000,
		/*! Indicates that authenticated communication between the client and
		 * server should be signed with a "dummy" signature. */
		NegotiateAlwaysSign = 0x00008000,
		/*! Sent by the server in the Type 2 message to indicate that the
		 * target authentication realm is a domain. */
		TargetTypeDomain = 0x00010000,
		/*! Sent by the server in the Type 2 message to indicate that the
		 * target authentication realm is a server. */
		TargetTypeServer = 0x00020000,
		/*! Sent by the server in the Type 2 message to indicate that the
		 * target authentication realm is a share. Presumably, this is for
		 * share-level authentication. Usage is unclear. */
		TargetTypeShare = 0x00040000,
		/*! Indicates that the NTLM2 signing and sealing scheme should be used
		 *  for protecting authenticated communications. Note that this refers
		 *  to a particular session security scheme, and is not related to the
		 *  use of NTLMv2 authentication. This flag can, however, have an
		 *  effect on the response calculations (as detailed in the
		 *  "NTLM2 Session Response" section). */
		NegotiateNTLM2Key = 0x00080000,
		/*! This flag's usage has not been identified. */
		RequestInitResponse = 0x00100000,
		/*! This flag's usage has not been identified. */
		RequestAcceptResponse = 0x00200000,
		/*! This flag's usage has not been identified. */
		RequestNonNTSessionKey = 0x00400000,
		/*! Sent by the server in the Type 2 message to indicate that it is
		 * including a Target Information block in the message. The Target
		 * Information block is used in the calculation of the NTLMv2
		 * response. */
		NegotiateTargetInfo = 0x00800000,
		/*! This flag's usage has not been identified. */
		unknown3 = 0x01000000,
		/*! This flag's usage has not been identified. */
		unknown4 = 0x02000000,
		/*! This flag's usage has not been identified. */
		unknown5 = 0x04000000,
		/*! This flag's usage has not been identified. */
		unknown6 = 0x08000000,
		/*! This flag's usage has not been identified. */
		unknown7 = 0x10000000,
		/*! Indicates that 128-bit encryption is supported. */
		Negotiate128 = 0x20000000,
		/*! Indicates that the client will provide an encrypted master key in
		 * the "Session Key" field of the Type 3 message. */
		NegotiateKeyExchange = 0x40000000,
		/*! Indicates that 56-bit encryption is supported. */
		Negotiate56 = 0x80000000,
	};

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
	bool DecodeClientMsg(const String &base64msg, String &domain, String &host);
	String EncodeClientMsg(String domain, String host);
	String EncodeServerNonce(String nonce, long flags = 0x8201);
	bool DecodeServerNonce(const String &base64msg, String &nonce);
	bool DecodeResponse(const String &base64msg, const String &nonce, const String &lmhash, const String &nthash, String &domain,
			String &host, String &user);
	String EncodeResponse(const String &nonce, const String &lmhash, const String &nthash, const String &domain, const String &host,
			const String &user);

	String EncodeLMPassword(String password);
	String EncodeNTPassword(String password);
	String MakeUtf16(const String &input);
	String StripUtf16(const String &input);
	String CalculateNonceHash(const String &keys, const String &plaintext);

	String MakeBase64(const String &buffer);
	bool DecodeBase64(String &result, const String &input);
	String MakeUtf16Upper(const String &in);
};

#endif
