/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _OTPList_h_
#define _OTPList_h_

#include "config_AccessControl.h"
#include "IFAConfObject.h"
#include "WDModule.h"

class TokenDataAccessController;

//---- OTPListsModule -----------------------------------------------------------
//! Manages OneTimePasswordList-Singletons
class EXPORTDECL_ACCESSCONTROL OTPListsModule : public WDModule
{
public:
	OTPListsModule(const char *name) : WDModule(name) {};
	~OTPListsModule() {};

	virtual bool Init(const ROAnything config);
	virtual bool Finis();
};

//---- OTPList -----------------------------------------------------------
//! \em Abstract OneTimePassword List. Do not instantiate directly.
class EXPORTDECL_ACCESSCONTROL OTPList : public ConfNamedObject
{
public:
	OTPList(const char *name) : ConfNamedObject(name) {};
	~OTPList() {};

	//---- registry interface
	RegCacheDef(OTPList);	// FindOTPList()

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		Assert(false);
		return 0;
	}

	//! Verify an otp for a given token, allow window for validation.
	//! Perform any necessary necessary steps on token data after validation
	//! (E.g. increase count)
	virtual bool Verify(String tokenId, String otp, long window, TokenDataAccessController *tokenData) = 0;

	//! Check if an otp is valid. No changes are performed on the tokenData.
	//! \returns distance (window>x>=0) to current position, -1 if not a valid token within window
	virtual long IsValid(String tokenId, String otp, long window, TokenDataAccessController *tokenData) = 0;

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	OTPList();
	OTPList(const OTPList &);
	OTPList &operator=(const OTPList &);
};

#define RegisterOTPList(name) RegisterObject(name, OTPList)

#endif
