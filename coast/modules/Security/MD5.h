/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _md5_h_
#define _md5_h_

#include "config_security.h"
#include "SecurityModule.h"

//---- MD5Signer -----------------------------------------------------------
class EXPORTDECL_SECURITY MD5Signer :  public Signer
{
public:
	MD5Signer(const char *name);
	~MD5Signer();
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) MD5Signer(fName);
	}
	void InitKey(const String &key);
	//!calculate plain MD5 hash value, no signing
	static void DoHash(const String &cleartext, String &hashvalue);
	//! sign the cleartext with MD5 hash value, takes the key from configuration
	virtual void DoEncode(String &scrambledText, const String &cleartext) const;
	//! sign the cleartext with MD5 hash value, takes the key from configuration
	virtual bool DoDecode(String &cleartext, const String &scrambledText) const;
	//! sign the cleartext with MD5 hash value that includes secret key
	static void DoSign(const String &key, String &scrambledText, const String &cleartext);
	//! check the signed text scramledText with key strkey and return clear text if OK
	static bool DoCheck(const String &strkey, String &cleartext, const String &scrambledText);
protected:
	String fKey;
private:
	MD5Signer();
	MD5Signer(const MD5Signer &);
	MD5Signer &operator=(const MD5Signer &);
};

#endif
