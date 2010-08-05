/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MockOTPList_h_
#define _MockOTPList_h_

#include "config_AccessControl.h"
#include "OTPList.h"

class TokenDataAccessController;

//---- MockOTPList -----------------------------------------------------------
//! Mock-Implementation of the OTPList API.
/*! Config:<pre>
{
	/TokenX {
		/OtpList	mandatory, list of one time passwords for Token X
		/Count		optional, initial index into otp list of Token X (default = 0)
	}
	/TokenY {
		...
	}
}
</pre>
Example:
<pre>
{
	/MyToken {
		/OtpList {
			"293ab3"
			"ff293g"
			"9301aa"
		}
		/Count	2
	}

	/YourToken {
		/OtpList {
			"9829r3"
			"883299"
			"md299d"
		}
		# no count given - defaults to 0
	}
}
</pre>
Keeps one time password lists in its config and updates count in memory only.
If no config is defined (i.e. no corresponding any file exists) then the
Mock implementation will always return true, no matter what the inputs are.
*/
class EXPORTDECL_ACCESSCONTROL MockOTPList : public OTPList
{
public:
	MockOTPList(const char *name) : OTPList(name) {};
	~MockOTPList() {};

	//-- Cloning interface
	IFAObject *Clone() const {
		return new MockOTPList(fName);
	}

	//! return true always (no matter what arguments are), if NO config is present.
	//! if a config IS present then:
	//! \return true if code is a valid otp for token tokenId within window. tokenData is ignored
	bool Verify(String tokenId, String otp, long window, TokenDataAccessController *tokenData);

	//! return 1 always (no matter what arguments are), if NO config is present.
	//! if a config IS present then:
	//! \return distance of code for token tokenId within window. tokenData is ignored. -1 if not valid
	long IsValid(String tokenId, String otp, long window, TokenDataAccessController *tokenData);

	long GetCount(String tokenId);
	void SetCount(String tokenId, long newCount);

private:
	Anything fCount;

	// block the following default elements of this class
	// because they're not allowed to be used
	MockOTPList();
	MockOTPList(const MockOTPList &);
	MockOTPList &operator=(const MockOTPList &);
};

#endif
