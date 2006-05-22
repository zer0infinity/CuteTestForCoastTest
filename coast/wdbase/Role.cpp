/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Role.h"

//--- standard modules used ----------------------------------------------------
#include "Session.h"
#include "Registry.h"
#include "Renderer.h"
#include "SysLog.h"
#include "Dbg.h"

//---- RolesModule -----------------------------------------------------------
RegisterModule(RolesModule);

RolesModule::RolesModule(const char *name) : WDModule(name)
{
}

RolesModule::~RolesModule()
{
}

bool RolesModule::Init(const Anything &config)
{
	if (config.IsDefined("Roles")) {
		HierarchyInstaller hi("Role");
		return RegisterableObject::Install(config["Roles"], "Role", &hi);
	}
	return false;
}

bool RolesModule::ResetFinis(const Anything &config)
{
	AliasTerminator at("Role");
	return RegisterableObject::ResetTerminate("Role", &at);
}

bool RolesModule::Finis()
{
	return StdFinis("Role", "Roles");
}

//---- Role --------------------------------------------------------------------------
RegisterRole(Role);

Role::Role(const char *name) : HierarchConfNamed(name)
{
}

Role::~Role()
{
	StatTrace(Role.Misc, "~Role: <" << fName << ">", Storage::Current());
}

IFAObject *Role::Clone() const
{
	return new Role(fName);
}

bool Role::Init(Context &)
{
	StartTrace1(Role.Init, fName << ": abstract returning always true" );
	return true;
}

void Role::Finis(Session &, Role *)
{
	StartTrace1(Role.Finis, fName << ": abstract - no clean up" );
}

bool Role::Synchronize(Context &)
{
	StartTrace1(Role.Synchronize, fName << ": abstract returning always true" );
	return true;
}

bool Role::CheckLevel(const String &queryRoleName) const
{
	StartTrace(Role.CheckLevel);
	String strRoleName(fName);
	Trace("my role name [" << strRoleName << "]");

	bool bLevelOk = false;
	if ( !( bLevelOk = queryRoleName.IsEqual(strRoleName) ) ) {
#if defined(DEBUG)
		// the role level is not really relevant because the role name decides if we are in the correct role
		// this code is just fo informational purposes
		GetRoleLevel( this );
		GetRoleLevel( Role::FindRole(queryRoleName) );
#endif
	}
	return bLevelOk;
}

long Role::GetRoleLevel(const Role *pRole) const
{
	StartTrace(Role.GetRoleLevel);
	long lLevel = 0;
	String strRoleName;
	if ( pRole ) {
		pRole->GetName(strRoleName);
	}
	while ( pRole && ( pRole = (Role *)pRole->GetSuper() ) ) {
		++lLevel;
	}
	Trace("Role <" << strRoleName << "> has Level " << lLevel);
	return lLevel;
}

long Role::GetTimeout()
{
	return Lookup("SessionTimeout", 60L);
}

void Role::PrepareTmpStore(Context &c)
{
	// now extract state from URL into TmpStore
	StartTrace1(Role.PrepareTmpStore, fName << ":");
	ROAnything stateFullList;

	if (Lookup("StateFull", stateFullList)) {
		TraceAny(stateFullList, "State full list");

		Anything query = c.GetQuery();
		Anything fields = query["fields"];
		Anything tmpStore = c.GetTmpStore();

		for (int i = 0, szf = stateFullList.GetSize(); i < szf; ++i) {
			const char *stateName = stateFullList[i].AsCharPtr(0);

			if (stateName) {
				//--- don't overwrite entries already there
				bool stateAlreadyDefined = tmpStore.IsDefined(stateName);
				stateAlreadyDefined = stateAlreadyDefined && (strlen(tmpStore[stateName].AsCharPtr("")) > 0);

				if ( !stateAlreadyDefined ) {
					if (query.IsDefined(stateName) && !query[stateName].IsNull()) {
						tmpStore[stateName] = query[stateName];
					}
					//Implicit value overriding
					if (fields.IsDefined(stateName) && !fields[stateName].IsNull()) {
						tmpStore[stateName] = fields[stateName];
					}
				}
			}
		}
		TraceAny(tmpStore, "Tempstore after");
	}
}

bool Role::GetNewPageName( Context &c, String &transition, String &pagename)
{
	// this method implements the default page resolving mechanism
	// it searches a new page through a lookup in the action/page map
	// table, defined in the role's *.any file

	StartTrace1(Role.GetNewPageName, "Rolename <" << fName << "> currentpage= <" << pagename << ">, transition= <" << transition << ">");

	if ( IsStayOnSamePageToken(transition) ) {
		return true;
	}

	ROAnything entry;
	if ( GetNextActionConfig(entry, transition, pagename) ) {
		// now look for new map entries consisting of page and action
		const char *newpagename = 0;
		if (entry.IsDefined("Page")) {
			newpagename = entry["Page"].AsCharPtr(0);
		} else {
			newpagename = entry[0L].AsCharPtr(0);
		}
		Trace("returning newPageName: <" << NotNull(newpagename) << ">");
		if (newpagename) {
			pagename = newpagename;
			if (entry.GetSize() > 1) {
				if (entry.IsDefined("Action")) {
					transition = entry["Action"].AsCharPtr(0);
				} else {
					transition = entry[1L].AsCharPtr(0);
				}
			} else {
				transition = "PreprocessAction";
			}
			return true;
		}
	}
	return false;
}

bool Role::GetNextActionConfig(ROAnything &entry, String &transition, String &pagename)
{
	StartTrace(Role.GetNextActionConfig);
	// get the action page map from the role's
	// configuration file
	// PS: should inherit map!
	String index("Map.");
	index << pagename << "." << transition;
	Trace("trying [" << index << "]");
	if (!Lookup(index, entry)) {
		String defaultindex("Map.Default.");
		defaultindex  << transition;
		Trace("trying [" << defaultindex << "]");
		Lookup(defaultindex, entry);
	}
	return !entry.IsNull();
}

bool Role::IsStayOnSamePageToken(String &transition)
{
	StartTrace(Role.IsStayOnSamePageToken);
	ROAnything tokens;
	if ( transition == "StayOnSamePage" || ( Lookup("StayOnSamePageTokens", tokens) && tokens.Contains(transition) ) ) {
		transition = "PreprocessAction";
		return true;
	}
	return false;
}

void Role::CollectLinkState(Anything &stateIn, Context &c)
{
	StartTrace(Role.CollectLinkState);

	CheckConfig("Role");

	// copy selected fields from TmpStore into Link
	// the symmetric operation is in GetNewPageName
	ROAnything stateFullList;

	if (Lookup("StateFull", stateFullList)) {
		Anything tmpStore = c.GetTmpStore();

		for (int i = 0, szf = stateFullList.GetSize(); i < szf; ++i) {
			const char *stateName = stateFullList[i].AsCharPtr(0);
			if (stateName) {
				if (!stateIn.IsDefined(stateName) && tmpStore.IsDefined(stateName)) {
					stateIn[stateName] = tmpStore[stateName];
				}
			}
		}
	}
	stateIn["role"] = fName;
	Session *s = c.GetSession();
	if (s) {
		s->CollectLinkState(stateIn, c);
	}
}

bool Role::TransitionAlwaysOK(const String &transition)
{
	return (transition ==  "Logout");
}

String Role::GetRequestRoleName(Context &ctx) const
{
	StartTrace(Role.GetRequestRoleName);
	String name;
	Anything query = ctx.GetQuery();
	if ( query.IsDefined("role") ) {
		name = query["role"].AsString(name);
		Trace("got Role <" << name << "> from query");
	} else {
		name = GetDefaultRoleName(ctx);
	}
	return name;
}

// this method verifies the authentication level of role
// if everything is ok it let's the subclass verify the
// detailed paramaters of the query in DoVerify
bool Role::Verify(Context &c, String &transition, String &pagename)
{
	StartTrace1(Role.Verify, "Rolename <" << fName << "> currentpage= <" << pagename << ">, transition= <" << transition << ">");
	// if the action is always possible (e.g. logout) no further checking
	// is necessary
	if (TransitionAlwaysOK(transition)) {
		return true;
	}

	// we check the role level by role name
	// if no role is defined in the query we
	// assume guest
	String name = GetRequestRoleName(c);

	// check the level of the role it is defined in the config
	// assuming some levels of roles (e.g. Guest < Customer < PaymentCustomer)
	if ( CheckLevel(name) ) {
		// We have the right role level
		// let's check the query parameters
		Trace("role level is OK");
		return DoVerify(c, transition, pagename);
	}

	// We have a level which is too low for this
	// query, we can't proceed without authentication
	Trace("role level too low");
	return false;
}

bool Role::DoVerify(Context &, String &, String &)
{
	StartTrace1(Role.DoVerify, fName << ": abstract returning always true" );
	return true;
}

RegCacheImpl(Role);	// FindRole()

String Role::GetDefaultRoleName(Context &ctx)
{
	String ret;
	ROAnything rspec;
	if (ctx.Lookup("DefaultRole", rspec)) {
		Renderer::RenderOnString(ret, ctx, rspec);
	} else {
		ret = "Role";
	}
	return ret;
}

Role *Role::FindRoleWithDefault(const char *role_name, Context &ctx, const char *dflt)
{
	Role *ret = Role::FindRole(role_name);

	if (ret == 0) {
		String msg;
		msg << "<" << ctx.GetSessionId() << "> "
			<< "no valid role <" << role_name << "> found; using <" << dflt << ">";
		SysLog::Info(msg);

		ret = Role::FindRole(dflt);
	}
	if (ret == 0) {
		String msg;
		msg << "<" << ctx.GetSessionId() << "> "
			<< "Role <" << role_name << "> and Role <" << dflt
			<< "> not registered -- Danger";
		SYSERROR(msg);
		// fake a new role...as a last resort before crash
		Assert(ret != 0); // fail in Debug mode
		::abort();
	}
	return ret;
}
