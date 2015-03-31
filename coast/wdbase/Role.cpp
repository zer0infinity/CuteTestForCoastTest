/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "Role.h"
#include "Session.h"
#include "Renderer.h"
#include "AnyIterators.h"
#include "Policy.h"
#include <typeinfo>
#include <cstring>

RegisterModule(RolesModule);

RegCacheImpl(Role);
RegisterRole(Role);

const char* Role::gpcCategory = "Role";
const char* Role::gpcConfigPath = "Roles";

bool RolesModule::Init(const ROAnything config) {
	ROAnything roaConfig;
	if (config.LookupPath(roaConfig, Role::gpcConfigPath)) {
		HierarchyInstaller ai(Role::gpcCategory);
		return RegisterableObject::Install(roaConfig, Role::gpcCategory, &ai);
	}
	return false;
}

bool RolesModule::ResetFinis(const ROAnything config) {
	AliasTerminator at(Role::gpcCategory);
	return RegisterableObject::ResetTerminate(Role::gpcCategory, &at);
}

bool RolesModule::Finis() {
	return StdFinis(Role::gpcCategory, Role::gpcConfigPath);
}

bool Role::Init(Context &) {
	StatTrace(Role.Init, fName << ": abstract - nothing to init, returning true", coast::storage::Current());
	return true;
}

void Role::Finis(Session &, Role *) {
	StatTrace(Role.Finis, fName << ": abstract - nothing to do", coast::storage::Current());
}

bool Role::Synchronize(Context &) const {
	StatTrace(Role.Synchronize, fName << ": abstract returning true", coast::storage::Current());
	return true;
}

bool Role::CheckLevel(const String &queryRoleName) const {
	StartTrace1(Role.CheckLevel, "my role name [" << fName << "], query-role to check for [" << queryRoleName << "]");
	bool bLevelOk = false;
	if (!(bLevelOk = fName.IsEqual(queryRoleName))) {
		if (TriggerEnabled(Role.CheckLevel)) {
			// the following code would be nice to see role relations
			// -> but we need to re-authenticate as soon as the role names are different
			// names are not equal, check for their relation
			long lThisLevel = GetRoleLevel(this);
			Role *pQRole = Role::FindRole(queryRoleName);
			if (pQRole) {
				long lQRoleLevel = GetRoleLevel(pQRole);
				Trace("my role level:" << lThisLevel << " query-role level:" << lQRoleLevel);
				// if the roles are on the same level, they cannot be related
				if (lThisLevel != lQRoleLevel) {
					if (lQRoleLevel > lThisLevel) {
						// check if current role is a parent of the query-role
						Role *pRole = pQRole;
						String strRoleName;
						while (!bLevelOk && pRole && (pRole = (Role*) pRole->GetSuper()) && pRole) {
							pRole->GetName(strRoleName);
							bLevelOk = strRoleName.IsEqual(fName);
							Trace("role [" << strRoleName << "]" << (bLevelOk ? " is parent" : ""));
						}
					}
				}
			}
			// reset to false because it is not intended to be used like this at the moment
			bLevelOk = false;
		}
	}
	return bLevelOk;
}

long Role::GetRoleLevel(const Role *pRole) {
	StartTrace(Role.GetRoleLevel);
	long lLevel = -1;
	String strRoleName;
	if (pRole) {
		lLevel = 0L;
		pRole->GetName(strRoleName);
		try {
			while (pRole && (pRole = dynamic_cast<const Role *> (pRole->GetSuper()))) {
				++lLevel;
			}
		} catch (std::bad_cast& bc) {
			SYSINFO("bad_cast caught: " << bc.what());
		}
	}
	Trace("Role <" << strRoleName << "> has Level " << lLevel);
	return lLevel;
}

long Role::GetTimeout() const {
	return Lookup("SessionTimeout", 60L);
}

void Role::PrepareTmpStore(Context &c) {
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

				if (!stateAlreadyDefined) {
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

bool Role::GetNewPageName(Context &c, String &transition, String &pagename) const {
	// this method implements the default page resolving mechanism
	// it searches a new page through a lookup in the action/page map
	// table, defined in the role's *.any file
	StartTrace1(Role.GetNewPageName, "Rolename <" << fName << "> currentpage= <" << pagename << ">, transition= <" << transition << ">");
	if (IsStayOnSamePageToken(transition)) {
		return true;
	}
	ROAnything entry;
	if (GetNextActionConfig(entry, transition, pagename)) {
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

bool Role::GetNextActionConfig(ROAnything &entry, String &transition, String &pagename) const {
	StartTrace(Role.GetNextActionConfig);
	// get the action page map from the role's configuration file
	// map inheritance is not needed explicitly but realized using hierarchic configured objects
	String index("Map.");
	index << pagename << "." << transition;
	Trace("trying [" << index << "]");
	if (!Lookup(index, entry)) {
		String defaultindex("Map.Default.");
		defaultindex << transition;
		Trace("trying [" << defaultindex << "]");
		Lookup(defaultindex, entry);
	}
	return !entry.IsNull();
}

bool Role::IsStayOnSamePageToken(String &transition) const {
	StartTrace1(Role.IsStayOnSamePageToken, "checking token <" << transition << ">");
	ROAnything tokens;
	bool bIsStayToken(false);
	if (transition == "StayOnSamePage" || (Lookup("StayOnSamePageTokens", tokens) && tokens.Contains(transition))) {
		transition = "PreprocessAction";
		bIsStayToken = true;
	}
	Trace("resulting token <" << transition << "> is " << (bIsStayToken?"":"not ") << "to StayOnSamePage");
	return bIsStayToken;
}

void Role::CollectLinkState(Anything &stateIn, Context &c) {
	StartTrace(Role.CollectLinkState);
	// copy selected fields from TmpStore into Link
	// the symmetric operation is in GetNewPageName
	ROAnything stateFullList, roaStatefulEntry;
	if (Lookup("StateFull", stateFullList)) {
		Anything tmpStore = c.GetTmpStore();
		AnyExtensions::Iterator<ROAnything> statefulIter(stateFullList);
		String strStateName(32L);
		while (statefulIter.Next(roaStatefulEntry)) {
			strStateName = roaStatefulEntry.AsString();
			if (strStateName.Length()) {
				if (!stateIn.IsDefined(strStateName) && tmpStore.IsDefined(strStateName)) {
					Trace("copying content of TmpStore[\"" << strStateName << "\"]");
					stateIn[strStateName] = tmpStore[strStateName];
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

bool Role::TransitionAlwaysOK(const String &transition) const {
	return (transition == "Logout");
}

// this method verifies the authentication level of role
// if everything is ok it let's the subclass verify the
// detailed parameters of the query in DoVerify
bool Role::Verify(Context &ctx, String &transition, String &pagename) const {
	StartTrace1(Role.Verify, "Rolename <" << fName << "> currentpage= <" << pagename << ">, transition= <" << transition << ">");
	// if the action is always possible (e.g. logout) no further checking is necessary
	if (TransitionAlwaysOK(transition)) {
		return true;
	}
	// we check the role level by role name
	// if no role is defined in the query we use the default defined in slot DefaultRole or as last resort, use the Role base class
	String name = GetRequestRoleName(ctx, transition);
	// check the level of the role it is defined in the config
	// assuming some levels of roles (e.g. Guest < Customer < PaymentCustomer)
	if (CheckLevel(name)) {
		// We have the right role level
		// let's check the query parameters
		Trace("role level is OK");
		return DoVerify(ctx, transition, pagename);
	}
	// We have a level which is too low for this
	// query, we can't proceed without authentication
	Trace("role level not equal");
	return false;
}

bool Role::DoVerify(Context &, String &, String &) const {
	StatTrace(Role.DoVerify, "not overridden for <" << fName << ">, returning true", coast::storage::Current());
	return true;
}

String Role::GetDefaultRoleName(Context &ctx) {
	StartTrace(Role.GetDefaultRoleName);
	String role;
	ROAnything rspec;
	if (ctx.Lookup("DefaultRole", rspec)) {
		Renderer::RenderOnString(role, ctx, rspec);
		Trace("rendered DefaultRole [" << role << "]");
	} else {
		role = "Role";
		Trace("using default [" << role << "]");
	}
	return role;
}

Role *Role::FindRoleWithDefault(const char *role_name, Context &ctx, const char *dflt) {
	Role *ret = Role::FindRole(role_name);
	if (ret == 0) {
		String msg;
		msg << "<" << ctx.GetSessionId() << "> " << "no valid role <" << role_name << "> found; using <" << dflt << ">";
		SystemLog::Info(msg);
		ret = Role::FindRole(dflt);
		if (ret == 0) {
			msg.Trim(0);
			msg << "<" << ctx.GetSessionId() << "> " << "Role <" << role_name << "> and Role <" << dflt << "> not registered -- Danger";
			SYSERROR(msg);
			// fake a new role...as a last resort before crash
			Assert(ret != 0); // fail in Debug mode
			::abort();
		}
	}
	return ret;
}

String Role::DoGetRequestRoleName(Context & ctx, String const &transition) const {
	String name;
	Anything query = ctx.GetQuery();
	if (query.IsDefined("role")) {
		name = query["role"].AsString(name);
		StatTrace(Role.DoGetRequestRoleName, "got query role <" << name << ">", coast::storage::Current());
	} else {
		name = GetDefaultRoleName(ctx);
		StatTrace(Role.DoGetRequestRoleName, "got default role <" << name << ">", coast::storage::Current());
	}
	return name;
}
