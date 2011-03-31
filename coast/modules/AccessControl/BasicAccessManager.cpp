/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "BasicAccessManager.h"
#include "AccessController.h"
#include "OTPList.h"
#include "Registry.h"
#include "Dbg.h"

//---- BasicAccessManager -----------------------------------------------------------

RegisterAccessManager(BasicAccessManager);

bool BasicAccessManager::Validate(String &uid)
{
	StartTrace(BasicAccessManager.Validate);

	UserDataAccessController *udac = GetUdac();

	if (udac && udac->Exists(uid)) {
		Trace("User '" << uid << "' exists.");
		return true;
	}
	Trace("Unknown user '" << uid << "'.");
	return false;
}

bool BasicAccessManager::Validate(Context &ctx, String uid)
{
	StartTrace(BasicAccessManager.Validate);
	return Validate(uid);
}

bool BasicAccessManager::AuthenticateWeak(String uid, String passwd, String &newRole)
{
	StartTrace(BasicAccessManager.AuthenticateWeak);

	UserDataAccessController *udac = GetUdac();
	newRole = GetAuthFailRole();

	// check uid + password
	Trace("uid = " << uid);
	Trace("passwd = " << passwd);

	if ( Validate(uid) && udac && (udac->GetPassword(uid)).IsEqual(passwd) ) {
		newRole = GetAuthSuccessRole();
		Trace("weak authentication SUCCEEDED. New role = " << newRole);
		return true;
	}

	Trace("weak authentication FAILED. New role = " << newRole);
	return false;
}

bool BasicAccessManager::AuthenticateWeak(Context &ctx, String uid, String passwd, String &newRole)
{
	StartTrace(BasicAccessManager.AuthenticateWeak);
	return AuthenticateWeak(uid, passwd, newRole);
}

bool BasicAccessManager::AuthenticateStrong(String uid, String passwd, String otp, long window, String &newRole)
{
	StartTrace(BasicAccessManager.AuthenticateStrong);

	// perform weak authentication first
	bool weakAuthOk = AuthenticateWeak(uid, passwd, newRole);
	Trace("result of weak authentication = " << (weakAuthOk ? "ok" : "fail"));

	// ignore/delete role that comes back from weak authentication
	newRole = GetAuthFailRole();

	// perform strong authentication
	if (weakAuthOk) {
		Trace("acter token = " << otp);
		Trace("window = " << window);

		UserDataAccessController *udac = GetUdac();
		TokenDataAccessController *tdac = GetTdac();
		OTPList *verifier = GetOTPList();

		if ( udac && tdac && verifier ) {
			if ( verifier->Verify(udac->GetTokenId(uid), otp, window, tdac) ) {
				newRole = GetAuthSuccessRole();
				Trace("strong authentication SUCCEEDED. new role = " << newRole);
				return true;
			}
		}
	}
	Trace("strong authentication FAILED. new role = " << newRole);
	return false;
}

bool BasicAccessManager::AuthenticateStrong(Context &ctx, String uid, String passwd, String otp, long window, String &newRole)
{
	StartTrace(BasicAccessManager.AuthenticateStrong);
	return AuthenticateStrong(uid, passwd, otp, window, newRole);
}

bool BasicAccessManager::ChangePassword(String uid, String newpwd, String oldpwd)
{
	StartTrace(BasicAccessManager.ChangePassword);

	UserDataAccessController *udac = GetUdac();
	if (udac) {
		Trace("old passwd = " << oldpwd);
		Trace("new passwd = " << newpwd);
		Trace("old passwd from db = " << udac->GetPassword(uid));

		if ( !ChangePwdAllowed(uid) ) {
			Trace("ChangePassword FAILED. User '" << uid << "' has no right.");
			return false;
		}
		// old password is required (must be correct)
		if ( !(udac->GetPassword(uid)).IsEqual(oldpwd) ) {
			Trace("ChangePassword FAILED. Old password is wrong.");
			return false;
		}
		return udac->SetPassword(uid, newpwd);
	}
	return false;
}

bool BasicAccessManager::ChangePassword(Context &ctx, String uid, String newpwd, String oldpwd)
{
	StartTrace(BasicAccessManager.ChangePassword);
	return ChangePassword(uid, newpwd, oldpwd);
}

bool BasicAccessManager::ResetPassword(String uid)
{
	StartTrace(BasicAccessManager.ResetPassword);

	if ( !ResetPwdAllowed(uid) ) {
		Trace("ResetPassword FAILED. User '" << uid << "' has no right.");
		return false;
	}

	UserDataAccessController *udac = GetUdac();
	if (udac) {
		return udac->SetPassword(uid, uid);
	}
	return false;
}

bool BasicAccessManager::ResetPassword(Context &ctx, String uid)
{
	StartTrace(BasicAccessManager.ResetPassword);
	return ResetPassword(uid);
}

bool BasicAccessManager::IsAllowed(String who, String right)
{
	StartTrace(BasicAccessManager.IsAllowed);

	Trace("who = " << who);
	Trace("right = " << right);

	Anything allowed;
	if ( !GetAllowedEntitiesFor(who, allowed) ) {
		return false;
	}
	return allowed.Contains(right);
}

bool BasicAccessManager::IsAllowed(Context &ctx, String who, String entity)
{
	StartTrace(BasicAccessManager.IsAllowed);
	return IsAllowed(who, entity);
}

bool BasicAccessManager::GetAllowedEntitiesFor(Anything who, Anything &allowed)
{
	StartTrace(BasicAccessManager.GetAllowedEntitiesFor);

	EntityDataAccessController *edac = GetEdac();
	UserDataAccessController *udac = GetUdac();
	if ( !(udac && edac) ) {
		return false;
	}

	TraceAny(who, "who =");

	// is who a set of groups?
	if (who.GetType() == AnyArrayType) {
		return edac->GetAllowedEntitiesForGroups(who, allowed);
	}

	// is who an user?
	if (udac->Exists(who.AsCharPtr())) {
		Anything groups;
		if ( !udac->GetGroups(who.AsCharPtr(), groups) ) {
			Trace("WARNING: No groups given for user '" << who.AsCharPtr() << "'");
			allowed = Anything(Anything::ArrayMarker());
		}
		return edac->GetAllowedEntitiesForGroups(groups, allowed);
	}
	// must be a role or group (or not defined) then...
	return edac->GetAllowedEntitiesForGroup(who.AsCharPtr(), allowed);
}

bool BasicAccessManager::GetAllowedEntitiesFor(Context &ctx, Anything who, Anything &allowed)
{
	StartTrace(BasicAccessManager.GetAllowedEntitiesFor);
	return GetAllowedEntitiesFor(who, allowed);
}

// ----------------------- protected helpers -----------------------------
UserDataAccessController *BasicAccessManager::GetUdac()
{
	StartTrace(BasicAccessManager.GetUdac);
	const char *udacName = Lookup("UserDataAccessController", "");
	UserDataAccessController *udac = UserDataAccessController::FindUserDataAccessController(udacName);
	if ( !udac ) {
		Trace("UserDataAccessController '" << udacName << "' not found. Specified correctly in AccessManager config?");
	}
	return udac;
}

EntityDataAccessController *BasicAccessManager::GetEdac()
{
	StartTrace(BasicAccessManager.GetEdac);
	const char *edacName = Lookup("EntityDataAccessController", "");
	EntityDataAccessController *edac = EntityDataAccessController::FindEntityDataAccessController(edacName);
	if ( !edac ) {
		Trace("EntityDataAccessController '" << edacName << "' not found. Specified correctly in AccessManager config?");
	}
	return edac;
}

TokenDataAccessController *BasicAccessManager::GetTdac()
{
	StartTrace(BasicAccessManager.GetTdac);
	const char *tdacName = Lookup("TokenDataAccessController", "");
	TokenDataAccessController *tdac = TokenDataAccessController::FindTokenDataAccessController(tdacName);
	if ( !tdac ) {
		Trace("TokenDataAccessController '" << tdacName << "' not found. Specified correctly in AccessManager config?");
	}
	return tdac;
}

OTPList *BasicAccessManager::GetOTPList()
{
	StartTrace(BasicAccessManager.GetOTPList);
	const char *otpListName = Lookup("OTPList", "");
	OTPList *list = OTPList::FindOTPList(otpListName);
	if ( !list ) {
		Trace("OTPList '" << otpListName << "' not found. Specified correctly in AccessManager config?");
	}
	return list;
}

String BasicAccessManager::GetAuthFailRole()
{
	StartTrace(BasicAccessManager.GetAuthFailRole);
	String authFailRole = Lookup("AuthFailRole", "nobody");
	Trace("auth fail role = " << authFailRole);
	return authFailRole;
}

String BasicAccessManager::GetAuthSuccessRole(String)
{
	StartTrace(BasicAccessManager.GetAuthSuccessRole);
	String authSuccRole = Lookup("AuthSuccessRole", "user");
	Trace("auth success role = " << authSuccRole);
	return authSuccRole;
}

bool BasicAccessManager::ChangePwdAllowed(String uid)
{
	StartTrace(BasicAccessManager.ChangePwdAllowed);
	ROAnything roaRight;
	if ( Lookup("RightForPasswdChange", roaRight) ) {
		return IsAllowed( uid, roaRight.AsCharPtr() );
	}
	return true;
}

bool BasicAccessManager::ResetPwdAllowed(String uid)
{
	StartTrace(BasicAccessManager.ResetPwdAllowed);
	ROAnything roaRight;
	if ( Lookup("RightForPasswdReset", roaRight) ) {
		return IsAllowed( uid, roaRight.AsCharPtr() );
	}
	return true;
}
