/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BasicAccessManager_H
#define _BasicAccessManager_H

//--- includes
#include "config_AccessControl.h"
#include "AccessManager.h"

//--- forward decls
class UserDataAccessController;
class EntityDataAccessController;
class TokenDataAccessController;
class OTPList;

//---- BasicAccessManager ----------------------------------------------------------
//! <B>Performs WEAK authentication only.</B><BR>
/*! Configuration:
<PRE>
{
	/UserDataAccessController	String,	mandatory, 	name of udac to use
	/EntityDataAccessController	String, mandatory, 	name of edac to use
	/TokenDataAccessController	String, mandatory, 	name of tdac to use
	/OTPList					String, optional,	name of otp list to use for verification - if not specified, will always return false for strong auth
	/AuthFailRole				String, optional, 	name of role to return if authentication fails (default="nobody")
	/AuthSuccessRole			String,	optional, 	name of role to return if authentication succeeds (default="user")
	/RightForPasswdChange		String, optional,	IsAllowed(user,right) must succeed in order to change pwd. if not defined, pwd change is allowed always
	/RightForPasswdReset		String, optional,	IsAllowed(user,right) must succeed in order to change pwd. if not defined, pwd reset is allowed always
}</PRE>

This access manager performs weak and strong authentication.
It uses the specified AccessControllers to get user- and rights-data
from desired resources. The specified OTPList is used to perform
strong authentication (i.e. to verify an otp code). */
class EXPORTDECL_ACCESSCONTROL BasicAccessManager : public AccessManager
{
public:
	//--- constructors
	BasicAccessManager(const char *name) : AccessManager(name) {}
	~BasicAccessManager() {}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) BasicAccessManager(fName);
	}

	//! check if given id is a valid user id
	/*!	\param uid user id to check
		\return true if user id is accepted */
	virtual bool Validate(String &uid);
	virtual bool Validate(Context &ctx, String uid);

	//! authenticate given user with password
	/*!	\param uid user id to check
		\param passwd password for the given user
		\param newRole role assigned after successful or failing authentication
		\return true if authentication was successful, false otherwise */
	virtual bool AuthenticateWeak(String uid, String passwd, String &newRole);
	virtual bool AuthenticateWeak(Context &ctx, String uid, String passwd, String &newRole);

	//! authenticate given user with password, otp and window
	/*!	\param uid user id to check
		\param passwd password for the given user
		\param otp one time password token for the given user
		\param window size of window for otp check (should be >= 1)
		\param newRole role assigned after successful or failing authentication
		\return true if authentication was successful, false otherwise */
	virtual bool AuthenticateStrong(String uid, String passwd, String otp, long window, String &newRole);
	virtual bool AuthenticateStrong(Context &ctx, String uid, String passwd, String otp, long window, String &newRole);

	//! change password of the given user
	/*!	\param uid user id to change password for
		\param newpwd new password for the given user
		\param oldpwd old password for the given user
		\return true, if the change was successful */
	virtual bool ChangePassword(String uid, String newpwd, String oldpwd = "");
	virtual bool ChangePassword(Context &ctx, String uid, String newpwd, String oldpwd = "");

	//! reset password of the given user (if supported) to a value defined by the behavior of the utilized UserDataAccessManager
	/*!	\param uid user id to reset password for
		\return true, if the reset was successful */
	virtual bool ResetPassword(String uid);
	virtual bool ResetPassword(Context &ctx, String uid);

	//! check if permission can be given for entity
	/*! \param who uid, role or group name
		\param entity to be allowed: can be a role, a service or a permissible entity or something similar
		\return true if the entity is allowed for the given 'who' */
	virtual bool IsAllowed(String who, String entity);
	virtual bool IsAllowed(Context &ctx, String who, String entity);

	//! return all ressources (permissible entitites, services, ...) that a specific user, role, group or set of groups is allowed to use
	/*! \param who uid, role, group name or set of group names to get allowed entities for
		\param allowed allowed entities for the given who
		\return false if either the given 'who' is not valid or if no entities were found, true otherwise */
	virtual bool GetAllowedEntitiesFor(Anything who, Anything &allowed);
	virtual bool GetAllowedEntitiesFor(Context &ctx, Anything who, Anything &allowed);

protected:
	virtual UserDataAccessController *GetUdac();
	virtual EntityDataAccessController *GetEdac();
	virtual TokenDataAccessController *GetTdac();
	virtual OTPList *GetOTPList();

	//! return role for failed authentication.
	/*! \return look for definition of /AuthFailRole in config or returns "nobody" if not defined */
	virtual String GetAuthFailRole();

	//! return role for successful authentication (may be based on user name).
	/*! this implementation does not care about the user name.
		\return simply look for a definition of /AuthSuccessRole in config or returns "user" if not defined. */
	virtual String GetAuthSuccessRole(String uid = "");

private:
	bool ChangePwdAllowed(String uid);
	bool ResetPwdAllowed(String uid);

	// block the following default elements of this class
	// because they're not allowed to be used
	BasicAccessManager();
	BasicAccessManager(const BasicAccessManager &);
	BasicAccessManager &operator=(const BasicAccessManager &);
};

#endif
