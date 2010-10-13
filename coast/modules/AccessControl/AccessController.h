/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AccessController_h_
#define _AccessController_h_

#include "config_AccessControl.h"
#include "IFAConfObject.h"

//---- UserDataAccessController -----------------------------------------------------------
//! This should be the superclass of all user data access controller implementations.
//! Abstract class.
class EXPORTDECL_ACCESSCONTROL UserDataAccessController : public HierarchConfNamed
{
public:
	UserDataAccessController(const char *name) : HierarchConfNamed(name) {};
	virtual ~UserDataAccessController() {}

	RegCacheDef(UserDataAccessController);	// creates FindUserDataAccessController()

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		Assert(false);    // do not instantiate
		return 0;
	}

	//-- API (dummy implementations)
	virtual bool Exists(String userid) = 0;
	virtual String GetTokenId(String userid) = 0;

	virtual String GetPassword(String userid) = 0;
	virtual bool SetPassword(String userid, String newpasswd) = 0;
	virtual bool ResetPassword(String userid) = 0;

	virtual bool SetGroups(String userid, ROAnything newgroups) = 0;
	virtual bool GetGroups(String userid, Anything &groups) = 0;

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	UserDataAccessController();
	UserDataAccessController(const UserDataAccessController &);
	UserDataAccessController &operator=(const UserDataAccessController &);
};

#define RegisterUserDataAccessController(name) RegisterObject(name, UserDataAccessController)

//---- TokenDataAccessController -----------------------------------------------------------
//! This should be the superclass of all token data access controller implementations.
//! Abstract class.
class EXPORTDECL_ACCESSCONTROL TokenDataAccessController : public HierarchConfNamed
{
public:
	TokenDataAccessController(const char *name) : HierarchConfNamed(name) {};
	virtual ~TokenDataAccessController() {}

	RegCacheDef(TokenDataAccessController);	// creates FindTokenDataAccessController()

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		Assert(false);    // do not instantiate
		return 0;
	}

	//-- API (dummy implementations)
	virtual String GetSeed(String tokenid) = 0;
	virtual unsigned long GetCount(String tokenid) = 0;
	virtual bool IncCount(String tokenid, long by) = 0;

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	TokenDataAccessController();
	TokenDataAccessController(const TokenDataAccessController &);
	TokenDataAccessController &operator=(const TokenDataAccessController &);
};

#define RegisterTokenDataAccessController(name) RegisterObject(name, TokenDataAccessController)

//---- EntityDataAccessController -----------------------------------------------------------
//! This should be the superclass of all entity data access controller implementations.
//! Abstract class.
class EXPORTDECL_ACCESSCONTROL EntityDataAccessController : public HierarchConfNamed
{
public:
	EntityDataAccessController(const char *name) : HierarchConfNamed(name) {};
	virtual ~EntityDataAccessController() {}

	RegCacheDef(EntityDataAccessController);	// creates FindEntityDataAccessController()

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		Assert(false);    // do not instantiate
		return 0;
	}

	//-- API (dummy implementations)
	virtual bool GetAllowedEntitiesForGroup(String group, Anything &allowed) = 0;
	virtual bool GetAllowedEntitiesForGroups(Anything groups, Anything &allowed) = 0;
	virtual bool IsAllowed(String group, String entity) = 0;

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	EntityDataAccessController();
	EntityDataAccessController(const EntityDataAccessController &);
	EntityDataAccessController &operator=(const EntityDataAccessController &);
};

#define RegisterEntityDataAccessController(name) RegisterObject(name, EntityDataAccessController)

#endif
