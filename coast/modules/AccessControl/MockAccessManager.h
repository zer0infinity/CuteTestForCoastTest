/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MockAccessManager_H
#define _MockAccessManager_H

//---- baseclass include -------------------------------------------------
#include "config_AccessControl.h"
#include "AccessManager.h"

//---- MockAccessManager ----------------------------------------------------------
//! <B>Mock Access Manager to simplify testing.</B>
/*! Config:<pre>
{
	/AcceptedCodes {
		# list of accepted otp codes
		"33920023"
	}
	/AcceptedUsers {
		# uid/password pairs
		/user1	"secret"
		/tester	"dingdong"
	}
	/Rights {
		# rights for users/roles only (no groups)
		/user1 {
			"resetPW"
			"someOtherRight"
		}
		/user2 {
			"changePW"
			"resetPW"
		}
		/tester {
			# has no rights
		}
	}

	# roles to return by auth. request
	/AuthSuccessRole	loginOk
	/AuthFailRole		stayOut

	# IsAllowed(uid, right) must succeed with the following (see /Rights above)
	/RightForPasswdChange	"changePW"
	/RightForPasswdReset	"resetPW"
}
</pre>
Performs weak and strong authentication. All data is taken from config.
For the strong authentication, count and seed are not relevant, the
code is simply looked up in a list. User/password pairs are also looked
up from a list (see examples above).
*/
class EXPORTDECL_ACCESSCONTROL MockAccessManager : public AccessManager
{
public:
	//--- constructors
	MockAccessManager(const char *name) : AccessManager(name) {}
	~MockAccessManager() {}
	IFAObject *Clone() const {
		return new MockAccessManager(fName);
	}

	//! check if user exists in AcceptUsers list
	virtual bool Validate(String uid);
	virtual bool Validate(Context &ctx, String uid);

	//! perform weak authentication with uid + password
	//! \return true, if the uid/pwd pair is found in config of this AM
	virtual bool AuthenticateWeak(String uid, String passwd, String &newRole);
	virtual bool AuthenticateWeak(Context &ctx, String uid, String passwd, String &newRole);

	//! perform strong authentication with uid, password + otp
	//! \return true, if authentication was successful
	virtual bool AuthenticateStrong(String uid, String passwd, String otp, long window, String &newRole);
	virtual bool AuthenticateStrong(Context &ctx, String uid, String passwd, String otp, long window, String &newRole);

	//! change password (in memory only), old password must be correct
	virtual bool ChangePassword(String uid, String newpwd, String oldpwd = "");
	virtual bool ChangePassword(Context &ctx, String uid, String newpwd, String oldpwd = "");

	//! reset password to username (in memory only)
	virtual bool ResetPassword(String uid);
	virtual bool ResetPassword(Context &ctx, String uid);

	//! get rights for user/group/role (who must not be a list)
	virtual bool GetAllowedEntitiesFor(Anything who, Anything &allowed);
	virtual bool GetAllowedEntitiesFor(Context &ctx, Anything who, Anything &allowed);

	//! check if user/group/role is allowed to do something
	virtual bool IsAllowed(String who, String right);
	virtual bool IsAllowed(Context &ctx, String who, String entity);

private:
	Anything &GetData() {
		if (fMockData.IsNull()) {
			fMockData = fConfig.DeepClone();
		}
		return fMockData;
	}
	Anything fMockData;
};

#endif
