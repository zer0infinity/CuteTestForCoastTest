/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AccessManager_H
#define _AccessManager_H

//---- WDModule include -------------------------------------------------
#include "config_AccessControl.h"
#include "WDModule.h"

class AccessManager;
class Context;

//---- AccessManagerModule ----------------------------------------------------------
//! Manages access managers
/*!
Installs configured access managers list from coastd config.
Gives access to default access manager implementation.
*/
class EXPORTDECL_ACCESSCONTROL AccessManagerModule : public WDModule
{
public:
	//--- constructors
	AccessManagerModule(const char *name) : WDModule(name) {};
	~AccessManagerModule() {};

	//! implementers should initialize module using config
	virtual bool Init(const ROAnything config);
	//! implementers should terminate module expecting destruction
	virtual bool Finis();

	static AccessManager *GetAccessManager(const char *name = 0);

protected:
	static AccessManager *fgMainAccessManager;
};

//---- AbstractAccessManager ----------------------------------------------------------
//! Interface for all AccessManagers.
class EXPORTDECL_ACCESSCONTROL AccessManager : public HierarchConfNamed
{
public:
	//--- constructors
	AccessManager(const char *name) : HierarchConfNamed(name) {}
	virtual ~AccessManager() {}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		Assert(false);    // do not instantiate
		return 0;
	}

	//--- registry interface
	RegCacheDef(AccessManager); 	// FindAccessManager() definition

	//! check if given id is a valid user id
	/*!	\param uid user id to check
		\return true if user id is accepted */
	virtual bool Validate(String &uid) = 0;

	//! check if given id is a valid user id, context version
	/*!	\param ctx Context to use for DataAccess etc.
		\param uid user id to check
		\return true if user id is accepted */
	virtual bool Validate(Context &ctx, String uid) {
		return Validate(uid);
	}

	//! authenticate given user with password
	/*!	\param uid user id to check
		\param passwd password for the given user
		\param newRole role assigned after successful or failing authentication
		\return true if authentication was successful, false otherwise */
	virtual bool AuthenticateWeak(String uid, String passwd, String &newRole) = 0;

	//! authenticate given user with password, context version
	/*!	\param ctx Context to use for DataAccess etc.
		\param uid user id to check
		\param passwd password for the given user
		\param newRole role assigned after successful or failing authentication
		\return true if authentication was successful, false otherwise */
	virtual bool AuthenticateWeak(Context &ctx, String uid, String passwd, String &newRole) {
		return AuthenticateWeak(uid, passwd, newRole);
	}

	//! authenticate given user with password, otp and window
	/*!	\param uid user id to check
		\param passwd password for the given user
		\param otp one time password token for the given user
		\param window size of window for otp check (should be >= 1)
		\param newRole role assigned after successful or failing authentication
		\return true if authentication was successful, false otherwise */
	virtual bool AuthenticateStrong(String uid, String passwd, String otp, long window, String &newRole) = 0;

	//! authenticate given user with password, otp and window, context version
	/*!	\param ctx Context to use for DataAccess etc.
		\param uid user id to check
		\param passwd password for the given user
		\param otp one time password token for the given user
		\param window size of window for otp check (should be >= 1)
		\param newRole role assigned after successful or failing authentication
		\return true if authentication was successful, false otherwise */
	virtual bool AuthenticateStrong(Context &ctx, String uid, String passwd, String otp, long window, String &newRole) {
		return AuthenticateStrong(uid, passwd, otp, window, newRole);
	}

	//! change password of the given user
	/*!	\param uid user id to change password for
		\param newpwd new password for the given user
		\param oldpwd old password for the given user
		\return true, if the change was successful */
	virtual bool ChangePassword(String uid, String newpwd, String oldpwd = "") = 0;

	//! change password of the given user, context version
	/*!	\param ctx Context to use for DataAccess etc.
		\param uid user id to change password for
		\param newpwd new password for the given user
		\param oldpwd old password for the given user
		\return true, if the change was successful */
	virtual bool ChangePassword(Context &ctx, String uid, String newpwd, String oldpwd = "") {
		return ChangePassword(uid, newpwd, oldpwd);
	}

	//! reset password of the given user (if supported) to a value defined by the behavior of the utilized UserDataAccessManager
	/*!	\param uid user id to reset password for
		\return true, if the reset was successful */
	virtual bool ResetPassword(String uid) = 0;

	//! reset password of the given user (if supported) to a value defined by the behavior of the utilized UserDataAccessManager, context version
	/*!	\param ctx Context to use for DataAccess etc.
		\param uid user id to reset password for
		\return true, if the reset was successful */
	virtual bool ResetPassword(Context &ctx, String uid) {
		return ResetPassword(uid);
	}

	//! check if permission can be given for entity
	/*! \param who uid, role or group name
		\param entity to be allowed: can be a role, a service or a permissible entity or something similar
		\return true if the entity is allowed for the given 'who' */
	virtual bool IsAllowed(String who, String entity) = 0;

	//! check if permission can be given for entity, context version
	/*! \param ctx Context to use for DataAccess etc.
		\param who uid, role or group name
		\param entity to be allowed: can be a role, a service or a permissible entity or something similar
		\return true if the entity is allowed for the given 'who' */
	virtual bool IsAllowed(Context &ctx, String who, String entity) {
		return IsAllowed(who, entity);
	}

	//! return all ressources (permissible entitites, services, ...) that a specific user, role, group or set of groups is allowed to use
	/*! \param who uid, role, group name or set of group names to get allowed entities for
		\param allowed allowed entities for the given who
		\return false if either the given 'who' is not valid or if no entities were found, true otherwise */
	virtual bool GetAllowedEntitiesFor(Anything who, Anything &allowed) = 0;

	//! return all ressources (permissible entitites, services, ...) that a specific user, role, group or set of groups is allowed to use, context version
	/*! \param ctx Context to use for DataAccess etc.
		\param who uid, role, group name or set of group names to get allowed entities for
		\param allowed allowed entities for the given who
		\return false if either the given 'who' is not valid or if no entities were found, true otherwise */
	virtual bool GetAllowedEntitiesFor(Context &ctx, Anything who, Anything &allowed) {
		return GetAllowedEntitiesFor(who, allowed);
	}

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	AccessManager();
	AccessManager(const AccessManager &);
	AccessManager &operator=(const AccessManager &);
};

#define RegisterAccessManager(name) RegisterObject(name, AccessManager)

#endif
