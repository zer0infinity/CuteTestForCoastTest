/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _blowfish_h_
#define _blowfish_h_

#include "config_security.h"
#include "SecurityModule.h"

//---- BlowfishScrambler -----------------------------------------------------------
class EXPORTDECL_SECURITY BlowfishScrambler :  public Scrambler
{
public:
	BlowfishScrambler(const char *name);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) BlowfishScrambler(fName);
	}
	// Scramble a string of any length. Mode used is blowfish ECB mode.
	// See SSLeavy blowfish header file for further explanations.
	virtual void DoEncode(String &scrambledText, const String &cleartext) const;
	virtual bool DoDecode(String &cleartext, const String &scrambledText) const;
#define BF_ROUNDS       16
#define BF_BLOCK        8
#if !defined(BF_LONG)
#define BF_LONG unsigned long
#endif
	struct BlowfishKey {
		BF_LONG P[BF_ROUNDS+2];
		BF_LONG S[4*256];
	};

protected:
	//! pad to BF_BLOCK bytes, Coast default is to append 0-bytes plus a counting byte
	virtual void PadToBlockSize(String &result, const String &cleartext) const;
	//! remove padding added, but do a sanity check, return false if padding looks insane
	virtual bool UnPad(String &clearText) const;

	//! hook for already padded String for encryption, classes select ECB or CBC
	virtual void DoBlowfishEncrypt(String &padded) const;
	//! hook for decryption of String, classes select ECB or CBC
	virtual bool DoBlowfishDecrypt(String &s) const;

	// Set the key. Recommended key lengths vary from 16 to 20 Bytes.
	void InitKey(const String &key);
	// Blowfish ECB (electronic cookbook base encryption)
	void BF_ECB_encrypt(unsigned char *in,  unsigned char *out) const;
	// Blowfish ECB (electronic cookbook base decryption)
	void BF_ECB_decrypt(unsigned char *in,  unsigned char *out) const;
	// Blowfish ECB mutiple 8 block encryption
	void DoECB_multipleBlockEncrypt(unsigned char *Ptr, unsigned int N_Bytes) const;
	// Blowfish ECB mutiple 8 block decryption
	bool DoECB_multipleBlockDecrypt(unsigned char *Ptr, unsigned int N_Bytes) const;
	// The actual implementation of Blowfish ECB symmetric encryption
	void BF_encrypt(BF_LONG *data) const;
	void BF_decrypt(BF_LONG *data) const;
	BlowfishKey fKey;

private:
	BlowfishScrambler();
	BlowfishScrambler(const BlowfishScrambler &);
	BlowfishScrambler &operator=(const BlowfishScrambler &);
	friend class BlowfishTest;
};

//! do blowfish string encryption in CBC mode with initialization vector of 0
//! block padding is done in front to increase randomness
class EXPORTDECL_SECURITY BlowfishCBCScrambler : public BlowfishScrambler
{
public:
	BlowfishCBCScrambler(const char *name): BlowfishScrambler(name), fIvec0(0), fIvec1(0) {}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) BlowfishCBCScrambler(fName);
	}
protected:
	//! pad to BF_BLOCK bytes, Coast default is to append 0-bytes plus a counting byte
	virtual void PadToBlockSize(String &result, const String &cleartext) const;
	//! remove padding added, but do a sanity check, return false if padding looks insane
	virtual bool UnPad(String &clearText) const;
	//! hook for already padded String for encryption, CBC encryption
	virtual void DoBlowfishEncrypt(String &padded) const;
	//! hook for decryption of String, classes select CBC decryption
	virtual bool DoBlowfishDecrypt(String &s) const;
	//!Init hook, for initializing fIvec from config slot /InitVec0 /InitVec1
	virtual bool Init(ROAnything config);
	BF_LONG	fIvec0, fIvec1; // 64bits of "initialization vector", adding to the key
private:
	BlowfishCBCScrambler();
	BlowfishCBCScrambler(const BlowfishCBCScrambler &);
	BlowfishCBCScrambler &operator=(const BlowfishCBCScrambler &);
	friend class BlowfishTest;

};

#endif
