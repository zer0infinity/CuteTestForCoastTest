/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MockAccessManager.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- MockAccessManager ----------------------------------------------------------------
RegisterAccessManager(MockAccessManager);

bool MockAccessManager::Validate(String uid)
{
	StartTrace(MockAccessManager.Validate);

	ROAnything roData(GetData());
	Trace("uid = " << uid);

	bool ret = roData["AcceptedUsers"].IsDefined(uid);
	Trace((ret ? "User exists." : "User unknown."));
	return ret;
}

bool MockAccessManager::Validate(Context &ctx, String uid)
{
	StartTrace(MockAccessManager.Validate);
	return Validate(uid);
}

bool MockAccessManager::AuthenticateWeak(String uid, String passwd, String &newRole)
{
	StartTrace(MockAccessManager.AuthenticateWeak);

	ROAnything roData(GetData());
	Trace("uid = " << uid);
	Trace("passwd = " << passwd);

	bool ret = roData["AcceptedUsers"][uid].AsString("iMp0Ss1blE_Va1LuE").IsEqual(passwd);
	newRole = (ret ? roData["AuthSuccessRole"].AsString("AuthSuccessRole") : roData["AuthFailRole"].AsString("authFailRole") );

	Trace("weak authentication = " << (ret ? "successful." : "failed.") << " new role = " << newRole);
	return ret;
}

bool MockAccessManager::AuthenticateWeak(Context &ctx, String uid, String passwd, String &newRole)
{
	StartTrace(MockAccessManager.AuthenticateWeak);
	return AuthenticateWeak(uid, passwd, newRole);
}

bool MockAccessManager::AuthenticateStrong(String uid, String passwd, String otp, long window, String &newRole)
{
	StartTrace(MockAccessManager.AuthenticateStrong);

	ROAnything roData(GetData());
	Trace("otp = " << otp);
	Trace("window = " << window << " [ignored]");
	// TraceAny(roData, "MockAM config = ");

	bool ret = AuthenticateWeak(uid, passwd, newRole) && roData["AcceptedCodes"].Contains(otp);
	newRole = (ret ? roData["AuthSuccessRole"].AsString("AuthSuccessRole") : roData["AuthFailRole"].AsString("authFailRole") );

	Trace("strong authentication = " << (ret ? "successful." : "failed.") << " new role = " << newRole);
	return ret;
}

bool MockAccessManager::AuthenticateStrong(Context &ctx, String uid, String passwd, String otp, long window, String &newRole)
{
	StartTrace(MockAccessManager.AuthenticateStrong);
	return AuthenticateStrong(uid, passwd, otp, window, newRole);
}

bool MockAccessManager::ChangePassword(String uid, String newpwd, String oldpwd)
{
	StartTrace(MockAccessManager.ChangePassword);

	ROAnything roData(GetData());
	String dummy;
	Trace("uid = " << uid);
	Trace("oldpwd = " << oldpwd);
	Trace("newpwd = " << newpwd);

	if ( AuthenticateWeak(uid, oldpwd, dummy) ) {
		if ( IsAllowed(uid, roData["RightForPasswdChange"].AsString()) ) {
			fMockData["AcceptedUsers"][uid] = newpwd;
			Trace("Password changed.");
			return true;
		}
		Trace("User '" << uid << "' has no right to change password.");
		return false;
	}
	Trace("User unknown or incorrect old password.");
	return false;
}

bool MockAccessManager::ChangePassword(Context &ctx, String uid, String newpwd, String oldpwd)
{
	StartTrace(MockAccessManager::ChangePassword);
	return ChangePassword(uid, newpwd, oldpwd);
}

bool MockAccessManager::ResetPassword(String uid)
{
	StartTrace(MockAccessManager.ResetPassword);

	ROAnything roData(GetData());
	Trace("uid = " << uid);

	if ( Validate(uid) ) {
		if ( IsAllowed(uid, roData["RightForPasswdReset"].AsString()) ) {
			fMockData["AcceptedUsers"][uid] = uid;
			Trace("Password reset.");
			return true;
		}
		Trace("User '" << uid << "' has no right to reset password.");
		return false;
	}
	Trace("User unknown.");
	return false;
}

bool MockAccessManager::ResetPassword(Context &ctx, String uid)
{
	StartTrace(MockAccessManager.ResetPassword);
	return ResetPassword(uid);
}

bool MockAccessManager::GetAllowedEntitiesFor(Anything who, Anything &allowed)
{
	StartTrace(MockAccessManager.GetAllowedEntitiesFor);

	ROAnything roData(GetData());
	String whoStr = who.AsString();
	Trace("who = " << whoStr);

	if ( roData["Rights"].IsDefined(whoStr) ) {
		allowed = roData["Rights"][whoStr].DeepClone();
		TraceAny(allowed, "Granted rights:");
		return true;
	} else {
		Trace("User/role/group '" << whoStr << "' not defined in rights list.");
		allowed = MetaThing().DeepClone();
		return false;
	}
}

bool MockAccessManager::GetAllowedEntitiesFor(Context &ctx, Anything who, Anything &allowed)
{
	StartTrace(MockAccessManager.GetAllowedEntitiesFor);
	return GetAllowedEntitiesFor(who, allowed);
}

bool MockAccessManager::IsAllowed(String who, String right)
{
	StartTrace(MockAccessManager.IsAllowed);

	ROAnything roData(GetData());
	Trace("who = " << who);
	Trace("right = " << right);

	bool ret = roData["Rights"][who].Contains(right);
	Trace("Right " << (ret ? "granted." : "denied."));
	return ret;
}

bool MockAccessManager::IsAllowed(Context &ctx, String who, String entity)
{
	StartTrace(MockAccessManager.IsAllowed);
	return IsAllowed(who, entity);
}
