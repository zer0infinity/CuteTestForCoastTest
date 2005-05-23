/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _base64_h_
#define _base64_h_

#include "config_security.h"
#include "SecurityModule.h"

//---- Base64 -----------------------------------------------------------
//! Coasts Base64 encoding with the default URL style using -$ instead
/*! registered as Base64 and b64 */
class EXPORTDECL_SECURITY Base64 :  public Encoder
{
public:
	Base64(const char *name): Encoder(name) {}
	virtual ~Base64() {}
	virtual IFAObject *Clone() const {
		return new Base64(fName);
	}

	//! hook from SecurityItem doing the base 64 encoding
	virtual void DoEncode(String &encodedText, const String &cleartext) const;
	//! hook from SecurityItem decoding base 64 encoded string
	virtual bool DoDecode(String &cleartext, const String &scrambledText) const;

protected:
	//! hook for using the two different mappings, this one uses URL style
	virtual const char *DoGetBasis() const {
		return Base64::basis_64_wd;
	}
	static const char basis_64_wd[];
	static const char index_64[256];

private:
	Base64();
	Base64(const Base64 &);
	Base64 &operator=(const Base64 &);
};

//! normal Base64 encoding as used everywhere else (except inserting newlines)
/*! registered as Base64Regular and b64r */
class EXPORTDECL_SECURITY Base64Regular :  public Base64
{
public:
	Base64Regular(const char *name): Base64(name) {}
	virtual ~Base64Regular() {}
	virtual IFAObject *Clone() const {
		return new Base64Regular(fName);
	}

protected:
	//! hook for using the two different mappings, this one uses default style
	virtual const char *DoGetBasis() const {
		return Base64Regular::basis_64_orig;
	}
	static const char basis_64_orig[];

private:
	Base64Regular();
	Base64Regular(const Base64Regular &);
	Base64Regular &operator=(const Base64Regular &);
};

#endif
