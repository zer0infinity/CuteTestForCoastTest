/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MockAccessManager.h"
#include "Tracer.h"

//---- MockAccessManager ----------------------------------------------------------------
RegisterAccessManager(MockAccessManager);

bool MockAccessManager::Validate(String &uid)
{
	StartTrace1(MockAccessManager.Validate, "uid = " << uid);
	bool bRet ( Lookup( "AcceptedUsers" ).IsDefined(uid) );
	Trace((bRet ? "User exists." : "User unknown."));
	return bRet;
}

bool MockAccessManager::Validate(Context &ctx, String uid)
{
	StartTrace(MockAccessManager.Validate);
	return Validate(uid);
}

bool MockAccessManager::AuthenticateWeak(String uid, String passwd, String &newRole)
{
	StartTrace1(MockAccessManager.AuthenticateWeak, "uid = " << uid << " passwd = " << passwd);
	bool ret( false );
	if ( ((ROAnything)fMockData)["AcceptedUsers"].IsDefined( uid ) ) {
		ret = ((ROAnything)fMockData)["AcceptedUsers"][uid].AsString("iMp0Ss1blE_Va1LuE").IsEqual(passwd);
	} else {
		ret = Lookup( "AcceptedUsers" )[uid].AsString("iMp0Ss1blE_Va1LuE").IsEqual(passwd);
	}
	newRole = (ret ? Lookup( "AuthSuccessRole", "AuthSuccessRole") : Lookup( "AuthFailRole", "AuthFailRole" ) );
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

	Trace("otp = " << otp);
	Trace("window = " << window << " [ignored]");

	bool ret ( AuthenticateWeak(uid, passwd, newRole) && Lookup( "AcceptedCodes" ).Contains(otp) );
	newRole = (ret ? Lookup( "AuthSuccessRole", "AuthSuccessRole") : Lookup( "AuthFailRole", "AuthFailRole" ) );

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

	String dummy;
	Trace("uid = " << uid);
	Trace("oldpwd = " << oldpwd);
	Trace("newpwd = " << newpwd);

	if ( AuthenticateWeak(uid, oldpwd, dummy) ) {
		if ( IsAllowed(uid, Lookup( "RightForPasswdChange" ).AsString()) ) {
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
	StartTrace1(MockAccessManager.ResetPassword, "uid = " << uid);

	if ( Validate(uid) ) {
		if ( IsAllowed(uid, Lookup( "RightForPasswdReset" ).AsString()) ) {
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
	StartTrace1(MockAccessManager.GetAllowedEntitiesFor, "who = " << who.AsString());
	String whoStr( who.AsString() );
	if ( Lookup( "Rights" ).IsDefined(whoStr) ) {
		allowed = Lookup( "Rights" )[whoStr].DeepClone();
		TraceAny(allowed, "Granted rights:");
		return true;
	} else {
		Trace("User/role/group '" << whoStr << "' not defined in rights list.");
		allowed = Anything(Anything::ArrayMarker());
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
	StartTrace1(MockAccessManager.IsAllowed, "who = " << who << " right = " << right);
	bool ret( Lookup( "Rights" )[who].Contains(right) );
	Trace("Right " << (ret ? "granted." : "denied."));
	return ret;
}

bool MockAccessManager::IsAllowed(Context &ctx, String who, String entity)
{
	StartTrace(MockAccessManager.IsAllowed);
	return IsAllowed(who, entity);
}
