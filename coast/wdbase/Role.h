/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ROLE_H
#define _ROLE_H

#include "WDModule.h"
#include "Registry.h"
#include "Tracer.h"

class Session;
class Context;

class RolesModule: public WDModule {
public:
	RolesModule(const char *name) :
		WDModule(name) {
	}
	virtual bool Init(const ROAnything config);
	virtual bool ResetFinis(const ROAnything);
	virtual bool Finis();
};

/*!abstraction of authentication level and manager of the sites possible navigation paths for this level
A role represents the <b>authentication level</b> a session has.
A request is always checked against the necessary authentication level a session must have.
If it fails, reauthentication is required before processing the request.
It manages the possible navigation paths a user can have according to his authentication levels.
The default algorithm in GetNewPageName uses a Map defined in the role's configuration.
This configuration defines mappings from a transition token to a new page name, optionally defining a preprocess action */
class Role: public HierarchConfNamed {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	Role(const char *name) :
		HierarchConfNamed(name) {
	}
    /*! use Map from config to determine new page name, implements a state machine
	 subclasses use hook DoGetNewPageName if they implement the state machine
	 other than from the Role's configuration
	 \param c the context that might be used for additional decision
	 \param transition direction to go in state machine (2nd index to map)
	 \param pagename page we come from (1st index to map) and page we go to if true
	 \return true if pagename is set, false if transition couldn't find a page */
	virtual bool GetNewPageName(Context & c, String & transition, String & pagename) const;
	/*! get the action page map config from the role's configuration file
	 \param entry token-specific config from map or Null if not defined
	 \param transition direction to go in state machine (2nd index to map)
	 \param pagename page we come from (1st index to map) and page we go to if true
	 \return true if corresponding token-config was found */
	bool GetNextActionConfig(ROAnything & entry, String & transition, String & pagename) const;
	/*! check if the given transition is a StayOnSamePage-token
	 These tokens can be specified on a per Role basis using the slot StayOnSamePageTokens
	 \param transition token to check
	 \return true if the given token was defined as StayOnSamePage token */
	bool IsStayOnSamePageToken(String & transition) const;
	//!copies information from query or query["fields"] into the tmp store; if they are not already defined there; this takes place before processing the request
	virtual void PrepareTmpStore(Context & c); // copies state contained in the query to the TmpStore
	//!copies information from the tmp store into the query according to a <i>StateFull</i> list defined in the role's configuration
	virtual void CollectLinkState(Anything & a, Context & c); // copies state contained in the TmpStore to the URLState
	// manage 'role store' in session after creation or relogin
	virtual bool Init(Context & c);
	//!clean up role store in Session s, hook for subclasses as a replacement for destructor
	virtual void Finis(Session & s, Role *newrole);
	//!hook to synchronize the rolestore with the required content to process the query at hand; default implementation does nothing
	virtual bool Synchronize(Context & c) const;
	//! test if this role is valid and if the query is well formed for the role
	bool Verify(Context & c, String & transition, String & pagename) const;
	/*! return the session timeout value for this role
	 This is retrieved from the roles configuration slot "SessionTimeout", if not configured the default is 60 seconds. Configuration hierarchy is used
	 \return session timeout value for this role */
	virtual long GetTimeout() const;
	RegCacheDef(Role);

	//! Like Role, but when FindRole(role_name) fails, try FileRole(dflt)
	static Role *FindRoleWithDefault(const char *role_name, Context &ctx, const char *dflt = "Role");

	/*! Default implementation of DefaultRole mechanism, renders content of /DefaultRole slot */
	static String GetDefaultRoleName(Context &ctx);

	String GetRequestRoleName(Context &ctx, String const &transition) const {
		return DoGetRequestRoleName(ctx, transition);
	}
    /*! Get a number of the given Role's hierarchy depth, based on the configured hierarchy
	 * @param pRole Role to get the level for
	 * @return number which tells how many hierarchy levels we are away from the top level Role, or -1 if given Role is not valid
	 */
	static long GetRoleLevel(const Role *pRole);

	static const char* gpcCategory;
	static const char* gpcConfigPath;

protected:
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) Role(fName);
	}
	/*! security API hooks for subclasses
	 Checks role levels by name. Default implementation uses the role hierarchy to find mismatches.
	 \param name the name of the role we want to test for its level
	 \return returns false if this doesn't have privileges of role with name */
	virtual bool CheckLevel(const String &name) const;

	//! Subclass hook to verify Role
	/*!	Default implementation does nothing and returns true
	 * @param c Context that might be used for decision
	 * @param transition Current direction to go
	 * @param pagename Page we come from
	 * @return true always */
	virtual bool DoVerify(Context &c, String &transition, String &pagename) const;

	/*! check if the transition/action is valid for this role, regardless of the rest
	 default implementation allows "Logout" for all roles
	 \param transition paramater from query["action"] (called action for historical reasons) */
	virtual bool TransitionAlwaysOK(const String &transition) const;

	virtual String DoGetRequestRoleName(Context &ctx, String const &transition) const;
};

#define RegisterRole(name) RegisterObject(name, Role)

#endif
