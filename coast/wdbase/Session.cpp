/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//#define TRACE_LOCKS

//--- interface include --------------------------------------------------------
#include "Session.h"

//--- standard modules used ----------------------------------------------------
#include "TraceLocks.h"
#include "SystemLog.h"
#include "Role.h"
#include "Page.h"
#include "RequestProcessor.h"
#include "Renderer.h"
#include "LocalizationUtils.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

class EXPORTDECL_WDBASE AccessTimer
{
public:
	Session *fSession;
	AccessTimer(Session *session);
	~AccessTimer();

	void Use() { }
};

AccessTimer::AccessTimer(Session *session) : fSession(session)
{
}

AccessTimer::~AccessTimer()
{
	fSession->ResetAccessTime();
}

//---- Session -----------------------------------------------------------
Session::Session(const char *name, Context &ctx)
	: NotCloned(name)
	, fMutex("Session")
	, fTerminated(false)
	, fStore(Storage::Global())
	, fId(Storage::Global())
	, fAddress(Storage::Global())
	, fAccessCounter(1)
	, fAccessTime(time(NULL))
	, fRemoteAddr(Storage::Global())
	, fBrowser(Storage::Global())
	, fRefCount(0L)
{
	StartTrace(Session.Session);
	fStore["RoleStore"] = MetaThing(fStore.GetAllocator());
}

Session::~Session()
{
	StartTrace1(Session.~Session, "Session id: <" << fId << ">");
	String logMsg("deleted");
	logMsg << " " << fId;
	SystemLog::Info(logMsg);
	{
		LockUnlockEntry me(fMutex);
		if (fRefCount != 0) {
			logMsg = "";
			logMsg << "deleted Session id: <" << fId << "> and Refcount WAS NOT ZERO but: <" << fRefCount << ">\n";

			SystemLog::WriteToStderr(logMsg);
			SYSERROR(logMsg);
		}
	}
}

void Session::Init(const char *id, Context &ctx)
{
	StartTrace1(Session.Init, "id <" << NotNull(id) << ">");
	// is entrypoint but doesn't need Mutex, since it is similar to a constructor
	fId = id;
	PutInStore("SessionId", fId);
	GetRole(ctx);

	// extract some information from environment
	// for verification purposes
	Anything env(ctx.GetEnvStore());
	fRemoteAddr = env["REMOTE_ADDR"].AsCharPtr("");
	fBrowser = env["HTTP_USER_AGENT"].AsCharPtr("");

	// extract some information from query arguments
	Anything query(ctx.GetQuery());
	fAddress = 0;
	fPort = -1;

	if ( query.IsDefined("adr") ) { // PS: keep care of duplicate entries!
		fAddress = query["adr"][0L].AsCharPtr("");
	}
	if ( query.IsDefined("port") ) {
		fPort = query["port"][0L].AsLong(-1);
	}
	String logMsg("created: ");
	logMsg << fId;
	SystemLog::Info(logMsg);
}

//---- Getters, Setters

const char *Session::GetId() const
{
	return (const char *)fId;
}

long Session::GetAccessTime() const
{
	// assumption fMutex is already set by caller
	return fAccessTime;
}

void Session::ResetAccessTime()
{
	// assumption fMutex is already set by caller
	fAccessTime = time(0);
}

void Session::ResetTimeAndFlags()
{
	TRACE_LOCK_START("ResetTimeAndFlags");
	if (fMutex.TryLock()) {
		fTerminated = false;
		ResetAccessTime();
		fMutex.Unlock();
	}
}

long Session::GetAccessCounter() const
{
	// assumption fMutex is already set by caller
	return fAccessCounter;
}

bool Session::GetSessionInfo(Anything &sessionListInfo, Context &ctx, const char *slotName)
{
	StartTrace(Session.GetSessionInfo);
	bool ret = false;
	if ( !IsBusy() ) {
		Trace("Session not busy");
		ctx.Push(this);
		ROAnything roaConfig;
		// if someone wants to override the default he can specify either another action name
		// or an action script to collect specific session information
		ctx.Lookup("SessionInfoAction", roaConfig);
		String action;
		// this is just the fallback action when no other action and/or config is defined
		action = "SessionInfo";
		ret = Action::ExecAction(action, ctx, roaConfig);
	} else {
		Trace("Session busy");
		sessionListInfo["List"][slotName]["info"] = "< Session busy >";
	}
	return ret;
}

long Session::GetTimeout(Context &ctx)
{
	Role *role = GetRole(ctx);
	// assumption fMutex is already set by caller
	if (role) {
		return role->GetTimeout();
	}
	return 60;	// one minute
}

void Session::SetRole(Role *newRole, Context &ctx)
{
	StartTrace(Session.SetRole);
	// assumption fMutex is already set by caller
	Role *oldRole = GetRole(ctx);
	if (newRole != oldRole || !fStore.IsDefined("RoleName") ) {
		String oldRoleName("none");
		String newRoleName("none");
		if (newRole) {
			newRole->GetName(newRoleName);
		}
		if (oldRole) {
			oldRole->GetName(oldRoleName);
			oldRole->Finis(*this, newRole); // clean up rolestore if required, here we know the new newRole
		}
		String msg("switching newRole from <");
		msg << oldRoleName << "> to <" << newRoleName << ">";
		Trace(msg);
		SystemLog::Info(msg);
		PutInStore("RoleName", newRoleName);
		// Needed only when context used with copy of session store
		ctx.GetSessionStore()["RoleName"] = newRoleName;
	}
}

Role *Session::GetRole(Context &ctx)
{
	StartTrace(Session.GetRole);
	// assumption fMutex is already set by caller
	Role *role = 0;
	String role_name = ((ROAnything)fStore)["RoleName"].AsString("");
	if (role_name.Length() > 0) {
		role = Role::FindRole(role_name);
		Trace("RoleName of fStore: [" << role_name << "]");
	} else {
		role_name = Role::GetDefaultRoleName(ctx);
		role = Role::FindRoleWithDefault(role_name, ctx);
		Trace("Default RoleName: [" << role_name << "]");
		if (role) {
			PutInStore("RoleName", role_name);
		}
	}
	Trace("RoleName: [" << role_name << "], addr &" << (long)role);
	return role;
}

String Session::GetRoleName(Context &ctx)
{
	// assumption fMutex is already set by caller
	if ( fStore.IsDefined("RoleName") ) {
		return fStore["RoleName"].AsCharPtr("Role");
	}
	return Role::GetDefaultRoleName(ctx);
}

bool Session::IsBusy()
{
	TRACE_LOCK_START("IsBusy");
	bool isBusy = true;

	if (fMutex.TryLock()) {
		isBusy = false;
		fMutex.Unlock();
	}
	return isBusy;
}

void Session::Notify(ESessionEvt evt, Context &ctx)
{
	StartTrace1(Session.Notify, "trying to get session lock");
	TRACE_LOCK_START("Notify");
	LockUnlockEntry me(fMutex);
	Trace("got session lock");
	IntNotify(evt, ctx);
}

void Session::IntNotify(ESessionEvt evt, Context &ctx)
{
	StartTrace1(Session.IntNotify, "virtual function not overridden!");
}

//--- immutable context
bool Session::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(Session.Lookup, "key: <" << NotNull(key) << ">");
	return ROAnything(fStore).LookupPath(result, key, delim, indexdelim);
}

//---- Session Store Mgmt

TrickyThing &Session::GetRoleStoreGlobal()
{
	return (TrickyThing &)fStore["RoleStore"];
}

TrickyThing &Session::GetStoreGlobal()
{
	return fStore;
}

void Session::PutInStore(const char *key, const Anything &a)
{
	fStore[key] = a;
}

void Session::HTMLDebugStore(ostream &reply)
{
#ifdef COAST_TRACE
	if ( Tracer::CheckWDDebug( "Session.HTMLSessionStore", Storage::Current() ) ) {
		reply << "Session Store:\n" << fStore << "\n";
	}
#endif
}

void Session::RemoveFromStore(const char *key)
{
	fStore.Remove(key);
}

//---- entry points
bool Session::MakeInvalid(Context &ctx)
{
	TRACE_LOCK_START("MakeInvalid");
	if ( fMutex.TryLock() ) {
		StartTrace1(Session.MakeInvalid, "Session id: <" << fId << ">");
		String logMsg("Session: <");
		logMsg << fId << "> invalidation request";
		SystemLog::Info(logMsg);

		// invalidate session by setting last access time before timeout period
		fAccessTime = time(0) - GetTimeout(ctx) - 1;
		if (ctx.GetSession() == this) {
			ctx.Push((Session *)0);
		}
		fMutex.Unlock();
		return true;
	}
	return false;
}

bool Session::IsTerminated()
{
	StartTrace(Session.IsTerminated);
	// assumption if s can't aqcuire its internal lock
	// the session is not terminated
	TRACE_LOCK_START("IsTerminated");
	bool isTerminated = false;
	if ( fMutex.TryLock() ) {
		isTerminated = fTerminated;
		fMutex.Unlock();
	}
	Trace("IsTerminated: " << (isTerminated ? "true" : "false"));
	return isTerminated;
}

bool Session::IsDeletable(long secs, Context &ctx, bool roleNotRelevant)
{
	StartTrace1(Session.IsDeletable, "secs: " << secs << " roleNotRelevant: " << (roleNotRelevant ? "true" : "false"));
	// assumption if Session can't acquire its internal lock
	// the session is not terminated
	TRACE_LOCK_START("CheckTimeout");
	String msg;
	bool isDeletable = false;
	bool isUnRefed = false;
	bool isTimeout = false;
	bool isBusy = true;
	long delta = 0;
	long timeout = 0;
	// keep locked code as short as possible
	if ( fMutex.TryLock() ) {
		isBusy = false;
		// First check whether this session is referenced somewhere
		if (fRefCount == 0) {
			isUnRefed = true;
		}
		// If we are operating on the disabled session store, the timeout doesn't matter
		// We do this to save memory by freeing the disabled sessions.
		if ( roleNotRelevant == false ) {
			delta = (secs - fAccessTime);
			timeout = GetTimeout(ctx);
			isTimeout = (delta >= timeout);
			if (isTimeout) {
				fTerminated = true;
			}
		} else {
			fTerminated = true;
			isTimeout = true;
		}
		fMutex.Unlock();
	}
	if (isBusy) {
		msg << "SessionId <" << fId << ">  isBusy " << isBusy << " isUnRefed " << isUnRefed <<
			" isTimeout " << isTimeout << " [" << delta << "," << timeout << "] isDeletable 0";
		SystemLog::Info(msg);
		return false;
	}

	if (isUnRefed && isTimeout) {
		isDeletable = true;
	}
	msg << "SessionId <" << fId << ">  isBusy " << isBusy << " isUnRefed " << isUnRefed <<
		" isTimeout " << isTimeout << " [" << delta << "," << timeout << "] isDeletable " << isDeletable;
	SystemLog::Info(msg);
	return isDeletable;
}

bool CheckHeader(Context &ctx, const String &hdrSlot, const String &expValue, const String &turnOffSlot, String &reason)
{
	reason.Trim(0);
	ROAnything env(ctx.GetEnvStore());
	String currValue(env[hdrSlot].AsString(""));
	if ((expValue != currValue) && !(ctx.Lookup(turnOffSlot, 0L))) {
		if (currValue.Length() == 0) {
			reason << " Requests " << hdrSlot << " info is null";
		} else {
			reason << hdrSlot << " doesn't match last info";
		}
		return false;
	}
	return true;
}

bool Session::Verify(Context &ctx)
{
	TRACE_LOCK_START("Verify");
	LockUnlockEntry mutex(fMutex);
	Anything env(ctx.GetEnvStore());
	String logMsg(fId);

	// check for termination flag
	if (fTerminated) {
		logMsg << " Session already terminated";
		SystemLog::Info(logMsg);
		return false;
	}

	ResetAccessTime();

	String reason;
	if (!CheckHeader(ctx, "REMOTE_ADDR", fRemoteAddr, "InstableRemoteAdress", reason)) {
		SystemLog::Info(reason);
		return false;
	}
	if (!CheckHeader(ctx, "HTTP_USER_AGENT", fBrowser, "InstableUserAgent", reason)) {
		SystemLog::Info(reason);
		return false;
	}

	// so far everything seems to be fine
	return true;
}

void Session::RenderNextPage(ostream &reply, Context &ctx, const ROAnything &roaConfig)
{
	StartTrace(Session.RenderNextPage);
	if (fMutex.TryLock()) {
		TRACE_LOCK_START("RenderNextPage");
		AccessTimer at(this);
		at.Use();
		++fAccessCounter;
		DoRenderNextPage(reply, ctx);
		fMutex.Unlock();
	} else {
		DoRenderBusyPage(reply, ctx);
	}
}

void Session::DoRenderNextPage(ostream &reply, Context &context)
{
	StartTrace(Session.DoRenderNextPage);

	context.SetRole(GetRole(context));

	String transition, currentpage;
	SetupContext(context, transition, currentpage);

	DoFindNextPage(context, transition, currentpage);

	Page *newPage = Page::FindPage(currentpage);
	// lets hope we got a new page
	if (newPage) {
		// Now Start a new page...
		newPage->Start(reply, context);
	} else {
		String msg("Session::RenderNextPage: Page [");
		msg << currentpage << "] not found.";
		RequestProcessor::Error(reply, msg, context);
		String logMsg(fId);
		logMsg << " Session::RenderNextPage: newPage == 0";
		SYSWARNING(logMsg);
	}
}

void Session::DoRenderBusyPage(ostream &reply, Context &ctx)
{
	StartTrace(Session.RenderBusyPage);

	Anything env(ctx.GetEnvStore());			// get environment
	Anything query(ctx.GetQuery());		// get query
	TraceAny(query, "query");

	// this is a trimmed down context object
	// since you can't use this session and its role

	Page *p = Page::FindPage("BusyPage");
	if ( p ) {
		p->Start(reply, ctx);
	}
}

//--- auxiliary methods and hooks for RenderNextPage
void Session::DoFindNextPage(Context &context, String &transition, String &currentpage)
{
	StartTrace(Session.DoFindNextPage);
	Trace( "before transition =<" << transition << "> page = <" << currentpage << ">" << " role of session = <" << GetRoleName(context) << ">");
	bool done = false;
	long lLoopCount = 0;
	String strLastTransition(transition), strLastPage(currentpage), strInitialPage(currentpage);
	GetRole(context)->PrepareTmpStore(context);
	do {
	starttransition:
		if (!NeedsPageInsert(context, transition, currentpage)) {
			if (!FinishPage(context, transition, currentpage)) {
			}
			if (AfterPageInsert(context, transition, currentpage) && RetrieveFromDelayed(context, transition, currentpage)) {
				done = false;
				goto starttransition;
			}
		} else {
			SaveToDelayed(context, transition, currentpage);
		}
		if (!GetRole(context)->Synchronize(context)) {
			PrepareLogout(context, transition, currentpage);
			done = true;
		}
		if (!GetRole(context)->GetNewPageName(context, transition, currentpage)) {
			currentpage = context.Lookup("ErrorPage", "ErrorPage");
		}
		done = PreparePage(context, transition, currentpage) || done;
		if ( !done ) {
			if ( strLastTransition == transition && strLastPage == currentpage ) {
				// increment loop counter to be able to break when we detect no change in transition and page over time
				++lLoopCount;
				Trace("Detected same transition [" << transition << "] and page [" << currentpage << "] as before!, LoopCount:" << lLoopCount);
			} else {
				strLastTransition = transition;
				strLastPage = currentpage;
				lLoopCount = 0;
			}
			Trace("last transition =<" << strLastTransition << "> page = <" << strLastPage << ">");
			Trace("intermediate transition =<" << transition << "> page = <" << currentpage << "> role of session = <" << GetRoleName(context) << ">");
		}
	} while ( !done && ( lLoopCount < 2L ) );
	if ( !done ) {
		SYSERROR("Cancelled DoFindNextPage due to potential endless loop on Page [" << strLastPage << "] and Token [" << strLastTransition << "], InitialPage was [" << strInitialPage << "]");
	}
	Trace("after transition =<" << transition << "> page = <" << currentpage << "> role of session = <" << NotNullStr(GetRoleName(context)) << ">");
}

namespace {
	void getDefaultAction(Context& context, String& transition) {
		StartTrace(Session.getDefaultAction);
		ROAnything roTransition;
		if (context.Lookup("DefaultAction", roTransition) ) {
			TraceAny(roTransition, "transition/action evaluation config");
			transition.Trim(0);
			Renderer::RenderOnString( transition, context, roTransition );
		} else {
			transition = "Home";
		}
		Trace("resulting transition <" << transition << ">");
	}
}

void Session::SetupContext(Context &context, String &transition, String &pagename)
{
	StartTrace(Session.SetupContext);
	Anything env(context.GetEnvStore());	// get environment
	Anything query(context.GetQuery());		// get query

	Normalize(query);	// remove "fld_" first implicit dependency to FieldRenderers
	// FIXME: 	this dependency should be removed
	//			the prefix mechanism should be standardized
	//			with several types e.g. buttons

	TraceAny(query, "query");

	context.SetLanguage(LocalizationUtils::FindLanguageKey(context, context.Lookup("Language", "E")));

	// the action to be executed
	getDefaultAction(context, transition);
	if (query.IsDefined("action")) {
		String theToken = query["action"].AsCharPtr(transition);
		if (theToken != "") {
			transition = theToken;
		}
	}

	// we have to finish the last page (so as to exec Action)
	// so lets try to extract a page from the query
	pagename = context.Lookup("StartPage", "HomePage"); // use default
	if (query.IsDefined("page")) {
		pagename = query["page"].AsCharPtr(pagename);	// use AsCharPtr() as to catch NULL
	}

	if ( query.IsDefined("delayedIndex") ) {
		// save delayed environment to keep lookup semantics consistent
		Anything tmpStore(context.GetTmpStore());
		long delayedIndex = query["delayedIndex"].AsLong(0L);
		if ( delayedIndex >= 0 ) {
			tmpStore["delayed"] = fStore["delayed"][delayedIndex];
		}
	}
}

void Session::SetInReauthenticate(Context &context)
{
	Anything tmpStore(context.GetTmpStore());
	Role *r = context.GetRole();
	String roleName("Role");
	if (r) {
		r->GetName(roleName);
	}
	StatTrace(Session.SetInReauthenticate, "Role [" << roleName << "]", Storage::Current());
	tmpStore["InReauthenticate"] = roleName;
}

void Session::ForcedLogin(Context &context, String &transition, String &currentpage)
{
	// we always go the transition "Login" for wrong roles
	transition = "Login";
	// and start this transition from the /StartPage or HomePage
	currentpage = context.Lookup("StartPage", "HomePage");
	StatTrace(Session.ForcedLogin, "transition <" << transition << "> startpage <" << currentpage << ">", Storage::Current());
}

bool Session::NeedsPageInsert(Context &context, String &transition, String &currentpage)
{
	StartTrace(Session.NeedsPageInsert);
	// we only check the role for non-empty queries
	if (context.GetQuery().IsNull() || GetRole(context)->Verify(context, transition, currentpage)) {
		// hook
		return RequirePageInsert(context, transition, currentpage);
	}
	// the role is insufficient
	ForcedLogin(context, transition, currentpage);
	return true;
}

bool Session::RequirePageInsert(Context &context, String &transition, String &currentpage)
{
	// just a hook might use a lookup for easier configurability
	// invent a config key based on currentpage
	// use /InsertPage { /page transitiontotake }
//    ROAnything inserttransitions = context.Lookup("InsertAction");
//
//    if (inserttransitions.IsDefined(currentpage)){
//        String newtrans = inserttransitions[currentpage].AsCharPtr(0);
//        if (newtrans.Length()>0){
//            transition = newtrans;
//            return true;
//        }
//    }
	StatTrace(Session.RequirePageInsert,"returning false", Storage::Current());
	return false;
}

void Session::SaveToDelayed(Context &context, String &transition, String &pagename)
{
	StartTrace(Session.SaveToDelayed);
	if (!InReAuthenticate(GetRole(context), context)) {
		// to prevent endless loops
		Trace("normal <" << transition << "> pg:<" << pagename << ">");
		// remember last current query for later processing
		Anything query;
		Anything delayed = context.GetQuery().DeepClone(fStore.GetAllocator());
		Anything tmpStore(context.GetTmpStore());
		TraceAny(delayed, "Session: <" << fId << "> original query content");

		if (context.GetEnvStore()["header"].IsDefined("HTTPS")) {
			delayed["HTTPS"] = context.GetEnvStore()["header"]["HTTPS"].AsBool(0L);
		}
		if (delayed.IsDefined("SessionIsNew")) {
			delayed.Remove("SessionIsNew");
		}
		// preserve selected entries of the environment
		ROAnything preserve;

		if (context.Lookup("DelayedEnvironment", preserve)) {
			Anything env(context.GetEnvStore());
			Anything delayedEnv;
			for (long i = 0, sz = preserve.GetSize(); i < sz; ++i) {
				const char *slot = preserve[i].AsCharPtr("");
				delayedEnv[slot] = env[slot];
			}
			delayed["delayedEnv"] = delayedEnv;
		}
		//FIXME allow only one delayed query, quick fix for no logon of frontdoor stresser
		fStore["delayed"][0L] = delayed;
		query["delayedIndex"] = 0L;
		tmpStore["delayed"] = delayed; // to make it accessible by Lookups for delayed
		query["action"] = transition;
		query["page"] = pagename;
		// Collect the roles state full information
		GetRole(context)->CollectLinkState(query, context);
		// go to login page for the required role
		TraceAny(query, "new query content");
		context.SetQuery(query);
		SetInReauthenticate(context);
	} else {
		// unprivileged user tries to invoke a secure action on this role
		// but reauthenticate was already called so
		// don't call reauthenticate again
		String logMsg(fId);
		logMsg << " Trying to reauthenticate " << GetRoleName(context) << " twice";
		Trace(logMsg);
		SYSERROR(logMsg);
	}
}

bool Session::FinishPage(Context &context, String &action, String &pagename)
{
	StartTrace1(Session.FinishPage, "Action: <" << action << ">, PageName: <" << pagename << ">");
	// is a role change necessary?
	if (pagename.Length() > 0) {
		Page *oldPage = Page::FindPage(pagename);
		if ( oldPage && !oldPage->Finish(action, context) ) {
			// stay on page because we fail to successfully postprocess it
			String msg;
			msg << "Finalizing page <" << pagename << "> failed";
			Trace(msg);
			return false; // stuck user on page, because we failed
		}
	} // nothing to do
	return true;
}

bool Session::AfterPageInsert(Context &context, String &action, String &pagename)
{
	StartTrace(Session.AfterPageInsert);
	Role *newrole = CheckRoleExchange(action, context);
	if (newrole) {
		if (newrole->Init(context)) {
			// yes we really got it
			String roleName;
			newrole->GetName(roleName);
			Trace("after Role Init succeed:<" << NotNullStr(roleName) << "> action:<" << action << ">");
			SetRole(newrole, context);

			// update context
			context.SetRole(newrole);
			return true;
		} else {
			// something nasty happened
			PrepareLogout(context, action, pagename);
		}
	}
	return CameFromPageInsert(context, action, pagename);
}

void Session::PrepareLogout(Context &context, String &transition, String &currentpage)
{
	StartTrace(Session.PrepareLogout);
	transition = "Logout";
	currentpage = context.Lookup("StartPage", "HomePage");
}

bool Session::CameFromPageInsert(Context &context, String &action, String &pagename)
{
	StartTrace(Session.CameFromPageInsert);
	// just a hook for page inserts aside from role changes
	// invent a configuration item that looks if pagename and action match
	// e.g. /InsertedPages { /action { "Page1" "Page2" } }
//    ROAnything insertedpages=context.Lookup("InsertedPages");
//    if (insertedpages.IsDefined(action)){
//        ROAnything pages = insertedpages[action];
//        return pages.Contains(pagename);
//    }
//
	return false;
}

bool Session::RetrieveFromDelayed(Context &context, String &action, String &currentpage)
{
	StartTrace1(Session.RetrieveFromDelayed, "Session: <" << fId << "> Action: <" << action << "> CurrentPage: <" << currentpage << ">");
	// but first we have to check whether the current action triggers a role exchange has already happened
	ROAnything roaQuery(context.GetQuery());
	TraceAny(((ROAnything)fStore)["delayed"], "delayed content in session");
	if (fStore.IsDefined("delayed") && roaQuery.IsDefined("delayedIndex")) {
		long index = roaQuery["delayedIndex"].AsLong(0);
		if ( index >= 0 ) {
			Anything delayed(fStore["delayed"][index], fStore.GetAllocator());
			// restore old environment
			Anything previousEnv(fStore.GetAllocator());
			if (delayed.LookupPath(previousEnv, "delayedEnv")) {
				delayed.Remove("delayedEnv");
				Anything env(context.GetEnvStore());
				TraceAny(env, "env before modification");
				for (long i = 0, sz = previousEnv.GetSize(); i < sz; ++i) {
					const char *slot = previousEnv.SlotName(i);
					env[slot] = previousEnv[i]; // this will copy
				}
				TraceAny(env, "modified env");
			}
			context.SetQuery(fStore["delayed"][index]);
			fStore["delayed"].Remove(index);
			if (fStore["delayed"].GetSize() == 0) {
				fStore.Remove("delayed"); // clean up
			}

			String transition;
			getDefaultAction(context, transition);
			Trace("transition [" << transition << "]");
			String lastAction(delayed["action"].AsCharPtr(transition));
			const char *lastpage = delayed["page"].AsCharPtr(context.Lookup("StartPage", "HomePage"));
			Trace("lastpage [" << lastpage << "] lastAction [" << lastAction << "]");
			if ( lastAction.Length() > 0 ) {
				// yes there was really something to do
				action = lastAction;
				currentpage = lastpage;
				Trace("final action [" << action << "] on page [" << currentpage << "]");
				return true; // re-iterate in RenderNextPage
			}
		}
	}
	return false;
}

bool Session::PreparePage(Context &context, String &transition, String &currentpage)
{
	StartTrace1(Session.PreparePage, "Transition: <" << transition << ">, Page: <" << currentpage << ">");

	Page *p = Page::FindPage(currentpage);
	if (p) {
		bool ret = p->Prepare(transition, context);
		Trace("after: Transition: <" << transition << ">, Page: <" << currentpage << ">");
		return ret;
	}
	String errorpage = context.Lookup("ErrorPage", "ErrorPage");
	if (currentpage == errorpage) {
		// we have already tried to find the error page
		String msg(fId);
		msg << " Session::PreparePage: " << errorpage << " not found";
		SYSWARNING(msg);
		return true; // force break of loop in RenderNextPage
	}
	// we need to render an errorpage
	currentpage = errorpage;
	transition = "Error";
	return false; // try one more time for ErrorPage
}

bool Session::InReAuthenticate(Role *r, Context &context)
{
	// check the reauthenticate flag to prevent
	// endless recursive reauthentication
	// of roles because of wrong configuration
	// or malicious implementation of Role::Verify
	Anything tmpStore(context.GetTmpStore());
	if ( r && tmpStore.IsDefined("InReauthenticate")) {
		String rolename;
		if ( r->GetName(rolename) && rolename == tmpStore["InReauthenticate"].AsCharPtr(0)) {
			return true; // yes we are trying to reauthenticate
		}
	}
	return false;
}

//------------------------------------------------------------------------------------
Role *Session::CheckRoleExchange(const char *action, Context &c)
{
	StartTrace1(Session.CheckRoleExchange, "action: <" << NotNull(action) << ">");

	// a role change is only possible if we can find RoleChanges in the context
	String strActionToRoleEntry("RoleChanges.");
	strActionToRoleEntry.Append(action);
	ROAnything actionRoleChanges = c.Lookup(String("RoleChanges.").Append(action));

	Role *contextRole = GetRole(c);
	if ( contextRole ) {
		String oldrole;
		contextRole->GetName(oldrole);
		if ( oldrole.Length() ) {
			strActionToRoleEntry.Append('.').Append(oldrole);
		}
	}
	String roleName = c.Lookup(strActionToRoleEntry).AsString("");
	Trace("action to role entry to lookup in context [" << strActionToRoleEntry << "] resulted in rolename [" << roleName << "]");
	// now lets try to find this role
	return Role::FindRole(roleName);
}

// URL state management

void Session::CollectLinkState(Anything &a, Context &c) const
{
	if (fAddress.Length() > 0) {
		a["adr"] = fAddress;
	}
	if (fPort != -1) {
		a["port"] = fPort;
	}
	a["sessionId"] = fId;
	Page *page = c.GetPage();
	if (page) {
		String pName;
		page->GetName(pName);
		a["page"] = pName;
	}
	Anything query = c.GetQuery();
	if (query.IsDefined("SessionIsNew")) {
		query.Remove("SessionIsNew");
	}
	if (query.IsDefined("delayedIndex")) {
		a["delayedIndex"] = query["delayedIndex"];
	}
}

bool Session::Info(Anything &info, Context &ctx)
{
	StartTrace(Session.Info);
	info["Role"] = GetRoleName(ctx);
	info["Accessed"] = GetAccessCounter();
	info["Last"] = GetAccessTime();
	info["Timeout"] = GetTimeout(ctx);
	info["Referenced"] = GetRefCount();
	// Don't allow unintentional alteration of session store
	info["SessionStore"] = GetStoreGlobal().DeepClone();
	return true;
}

bool Session::Ref()
{
	// Assumption mutex is set by caller (method is protected)
	StartTrace(Session.Ref);
	++fRefCount;
	Trace("Session: <" << fId << "> ++RefCount is: <" << fRefCount << ">");
	return true;
}

bool Session::UnRef()
{
	// Assumption mutex is set by caller (method is protected)
	StartTrace(Session.UnRef);
	String msg;
	--fRefCount;
	Trace("Session: <" << fId << "> --RefCount is: <" << fRefCount << ">");
	if (fRefCount < 0) {
		msg << "Session: <" << fId << "> --RefCount is NEGATIVE: <" << fRefCount << ">";
		SYSERROR(msg);
		Trace(msg);
		return false;
	} else {
		return true;
	}
}

long Session::GetRefCount() const
{
	// Assumption mutex is set by caller (method is protected)
	StartTrace(Session.GetRefCount);
	return fRefCount;
}

const String Session::FR_FIELDPREFIX = "fld_";
const String Session::FR_BUTTONPREFIX = "b_";
const String Session::FR_IMAGEPREFIX = "i_";

void Session::Normalize(Anything &query)
{
	Anything fields(query["fields"]);
	Anything koord(query["Click"]);			// Stores Image.x and Image.y koordinates

	//Separate fields
	long s = query.GetSize();
	for (long i = 0; i < s; ++i) {
		String slotname = query.SlotName(i);
		if (! slotname.IsEqual("")) {
			if (slotname.SubString(0, FR_FIELDPREFIX.Length()).IsEqual(FR_FIELDPREFIX)) {
				// "pure" field
				slotname = slotname.SubString(FR_FIELDPREFIX.Length());
				fields[slotname] = query.At(i);
			} else if (slotname.SubString(0, FR_BUTTONPREFIX.Length()).IsEqual(FR_BUTTONPREFIX)) {
				//button
				slotname = slotname.SubString(FR_BUTTONPREFIX.Length());
				if (!query.IsDefined("Button")) {
					query["Button"] = slotname;
				}
				fields[slotname] = query.At(i);
			} else if (slotname.SubString(0, FR_IMAGEPREFIX.Length()).IsEqual(FR_IMAGEPREFIX)) {
				// image button
				slotname = slotname.SubString(FR_IMAGEPREFIX.Length());
				long len = slotname.Length();
				// Save the click point koordinate
				String xy = slotname.SubString(len - 1);
				koord[xy] = query.At(i);
				// remove the .x or .y at the end
				query["Button"] = slotname.SubString(0, len - 2);
			}
		}
	}
	if (!fields.IsNull()) {
		query["fields"] = fields;
	}
	if (!koord.IsNull()) {
		query["Click"] = koord;
	}
}

//--- SessionInfo ---
RegisterAction(SessionInfo);

SessionInfo::SessionInfo(const char *name) : Action(name) { }

SessionInfo::~SessionInfo() { }

bool SessionInfo::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(SessionInfo.DoExecAction);
	Session *s = ctx.GetSession();
	if (s) {
		Anything sessionInfo(ctx.GetTmpStore()["SessionInfo"]);
		s->Info(sessionInfo[s->GetId()], ctx);
		DoGetInfo(s, sessionInfo[s->GetId()], ctx);
		return true;
	}
	return false;
}

bool SessionInfo::DoGetInfo(Session *s, Anything &info, Context &ctx)
{
	StartTrace(SessionInfo.DoGetInfo);
	return false;
}
