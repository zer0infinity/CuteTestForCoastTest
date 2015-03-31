/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//#define TRACE_LOCKS

#include "SessionListManager.h"
#include "SystemBase.h"
#include "URLFilter.h"
#include "TraceLocks.h"
#include "Session.h"
#include "PeriodicAction.h"
#include "TimeStamp.h"
#include "StringStream.h"
#include "Context.h"
#include "AnyIterators.h"
#include "Policy.h"
#include <iomanip>

//: triggers cleanup of sessions
class CleanSessions: public Action {
public:
	CleanSessions(const char *name) :
		Action(name) {
	}
	//:cleans the session list from timeouted sessions
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

bool SessionListManager::fgFinalize = false;
void SessionListManager::SetFinalize(bool finalize) {
	fgFinalize = finalize;
}
SessionListManager *SessionListManager::fgSessionListManager = 0;
Mutex SessionListManager::fgSessionListManagerMutex("SessionListManager");

void stopcleaner() {
	SessionListManager::SetFinalize(true);
	SessionListManager *pSLM = SessionListManager::SLM();
	if (pSLM) {
		pSLM->Finis();
	}
}

SessionListManager *SessionListManager::SLM() {
	if (!fgSessionListManager && !fgFinalize) {
		LockUnlockEntry me(fgSessionListManagerMutex);
		if (!fgSessionListManager && !fgFinalize) {
			fgSessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
#if !defined(WIN32)
			// atexit on Unix (solaris, linux) is executed before deletion of static objects
			// on WIN32 this is the other way round... (should use DLL_PROCESS_DETACH instead)
			atexit(stopcleaner);
#endif
		}
	}
	return fgSessionListManager;
}

RegisterModule(SessionListManager);
SessionListManager::SessionListManager(const char *name)
	: WDModule(name)
	, fSessionCleaner(0)
	, fSessionsMutex("Sessions")
	, fSessions(Anything::ArrayMarker())
	, fNextIdMutex("SessionId")
	, fNextId(-9999)
	, fMaxSessionsAllowed(-8888)
	, fSessionFactory(0)
	, fLogToCerr(0)
{
	StartTrace1(SessionListManager.SessionListManager, "Name:<" << NotNull(name) << ">");
	StringStream ss(fUniqueInstanceId);
#if defined(WIN32)
	long hostid = 112233L;
	long pid = GetCurrentProcessId();
#else
	long hostid = ::gethostid();
	long pid = coast::system::getpid();
#endif
	ss << std::hex << hostid << "!" << std::dec << (unsigned long) abs(pid);
}

bool SessionListManager::Init(const ROAnything config)
{
	StartTrace(SessionListManager.Init);
	TraceAny(config, "Config: ");
	fNextId = 0L;
	// acquire lock because it gets unlocked from within ResetInit()
	EnterReInit();
	return ResetInit(config);
}

bool SessionListManager::Finis()
{
	StartTrace(SessionListManager.Finis);
	Anything config;
	ResetFinis(config);
	Context ctx;
	ForcedSessionCleanUp(ctx);
	// release lock because it got locked from within ResetFinis()
	LeaveReInit();
	return true;
}

bool SessionListManager::ResetFinis(const ROAnything )
{
	StartTrace(SessionListManager.ResetFinis);
	EnterReInit();
	if (fSessionCleaner) {
		fSessionCleaner->Terminate();
		delete fSessionCleaner;
		fSessionCleaner = 0;
	}
	fMaxSessionsAllowed = 0L;

	if (fSessionFactory) {
		fSessionFactory = 0;
	}

	return true;
}

bool SessionListManager::ResetInit(const ROAnything config)
{
	StartTrace(SessionListManager.ResetInit);
	TraceAny(config, "Config: ");
	ROAnything moduleConfig;
	bool ret = ((ROAnything)config).LookupPath(moduleConfig, "SessionListManager");
	long cleanerWait = moduleConfig["CleanerTimeout"].AsLong(10L);
	String cleanerAction = moduleConfig["CleanerAction"].AsCharPtr("CleanSessions");
	fMaxSessionsAllowed = moduleConfig["MaxSessionsAllowed"].AsLong(10000L);
	fLogToCerr = moduleConfig["Log2Cerr"].AsLong(0L);
	fSessionFactory = SessionFactory::FindSessionFactory(moduleConfig["SessionFactory"].AsCharPtr("SessionFactory"));

	if ( !fSessionCleaner ) {
		String m;
		m << "\tLaunching new session cleaner";
		SystemLog::WriteToStderr(m);
		// periodically clean up sessions
		fSessionCleaner = new (coast::storage::Global()) PeriodicAction(cleanerAction, cleanerWait);
		fSessionCleaner->Start();
		m = "";
		m << " done" << "\n";
		SystemLog::WriteToStderr(m);
	}
	Trace("Init " << ((ret) ? "succeeded" : "failed"));
	LeaveReInit();
	return ret;
}

Session *SessionListManager::DoMakeSession(Context &ctx) {
	StartTrace(SessionListManager.DoMakeSession);
	if (fSessionFactory) {
		return fSessionFactory->DoMakeSession(ctx);
	}
	return new (coast::storage::Global()) Session("Session");
}

Session *SessionListManager::MakeSession(Context &ctx) {
	StartTrace(SessionListManager.MakeSession);
	TRACE_LOCK_START("MakeSession");
	Session *session = 0;
	long sessionsCount = GetNumberOfSessions();
	Trace("fSessionsCount: " << sessionsCount << " fMaxSessionsAllowed: " << fMaxSessionsAllowed);
	if (sessionsCount >= fMaxSessionsAllowed) {
		// try to make room for new session
		sessionsCount = CleanupSessions(ctx, true);
	}
	if (sessionsCount < fMaxSessionsAllowed) {
		session = DoMakeSession(ctx);
		ctx.Push(session);
	}
	return session;
}

Session *SessionListManager::LookupSession(const String &id, Context &ctx) {
	StartTrace(SessionListManager.LookupSession);
	Session *session = 0;
	Trace("SessionId:<" << id << ">");
	if (id.Length() > 0) {
		session = IntLookupSession(id, ctx);
	}
	return session;
}

void SessionListManager::DisableSession(const String &sessionId, Context &ctx)
{
	StartTrace(SessionListManager.DisableSession);
	Trace("Session to remove: " << sessionId);
	TRACE_LOCK_START("DisableSession");
	Session *s = 0;
	Anything session;
	{
		// remove session from sessions list so it is no longer accessible
		LockUnlockEntry mutex(fSessionsMutex);
		Trace("Size:[" << fSessions.GetSize() << "]");
		TraceAny(fSessions, "Sessions active");
		if ( fSessions.LookupPath(session, sessionId) ) {
			s = SafeCast(session.AsIFAObject(0), Session);
			if ( s ) {
				fDisabledSessions.Append(session);
				fSessions.Remove(sessionId);
			}
		} else {
			Trace("Session not found");
		}
		Trace("Size:[" << fSessions.GetSize() << "]");
		TraceAny(fSessions, "Sessions active after");
	}
	if (s) {
		s->Notify(Session::eRemoved, ctx);
	}
	if ( ctx.GetSession() == s ) { // not intended to disable one's own session.
		ctx.Push((Session *)0);    // disable session also in context
	}
	return;						 // Disable session is ok if session is removed or not there.
}

Session *SessionListManager::CreateSession(String &sessionId, Context &ctx) {
	StartTrace(SessionListManager.CreateSession);
	Session *session = MakeSession(ctx);
	if (session) {
		Trace("Adding session");
		AddSession(InitSession(sessionId, session, ctx), session, ctx);
		Trace("sessionId added:<" << sessionId << ">");
	} else {
		Trace("Session allocation failed");
		SystemLog::Error("can't create session");
	}
	return session;
}

String &SessionListManager::InitSession(String &id, Session *session, Context &ctx) {
	StartTrace(SessionListManager.InitSession);
	Anything any(ctx.GetRequest());
	Anything query;
	if (any.LookupPath(query, "query")) { // look for session id in query
		// modify query, so its obvious, that this query created a new session
		query["SessionIsNew"] = Anything(true);
		// set session id  if defined
		id = ROAnything(query)["sessionId"].AsCharPtr("");
	}
	if (id.Length() == 0) { // session id not set, generate one
		GetNextId(id, ctx);
	}
	session->Init(id, ctx);
	return id;
}

void SessionListManager::AddSession(const String &id, Session *session, Context &ctx)
{
	StartTrace(SessionListManager.AddSession);
	TRACE_LOCK_START("AddSession");
	{
		LockUnlockEntry mutex(fSessionsMutex);

		if (fSessions.IsDefined(id)) {
			fDisabledSessions.Append(fSessions[id]);
		}
		fSessions[id] = Anything(session, coast::storage::Global());
		String msg("Session created; Sessions in use: ");
		msg << fSessions.GetSize();
		SystemLog::Info(msg);
	}
	session->Notify(Session::eAdded, ctx);
}

Session *SessionListManager::IntLookupSession(const String &id, Context &ctx)
{
	StartTrace1(SessionListManager.IntLookupSession, "id <" << id << ">");
	TRACE_LOCK_START("IntLookupSession");
	Session *s = 0;
	{
		LockUnlockEntry mutex(fSessionsMutex);
		Anything session;
		// make sure no new entries are created in fSessions
		// if id is not really there
		if ( fSessions.LookupPath(session, id) ) {
			s = SafeCast(session.AsIFAObject(0), Session);

			// make sure this session won't get deleted
			// while processing this request
			if (s) {
				if (!s->IsBusy() && s->IsTerminated()) {
					s = 0;
				} else {
					s->ResetTimeAndFlags();
				}
			}
		}
	}
	if (s) {
		ctx.Push(s);
	}
	return s;
}

void SessionListManager::GetNextId(String &s, Context &ctx)
{
	TRACE_LOCK_START("GetNextId");
	LockUnlockEntry mutex(fNextIdMutex);
	// take timestamp as session key
	HRTIME lastId = fNextId;
	fNextId = GetHRTIME();
//  On solaris:
//	Although the units  of  hi-res  time  are  always  the  same
//	(nanoseconds),  the actual resolution is hardware dependent.
//	Hi-res time is guaranteed to be monotonic (it won't go back-
//	ward, it won't periodically wrap) and linear (it won't occa-
//	sionally speed up or slow down for adjustment, like the time
//	of  day  can),  but not necessarily unique: two sufficiently
//	proximate calls may return the same value.

	if ( fNextId <= lastId ) {
		if ( (lastId - fNextId) > 1 ) {
			SystemLog::Warning("Session Id generation instable");
		}
		fNextId = lastId + 1;
	}
	s.Append(GetUniqueInstanceId());
	s.Append("_");
	s.Append((long)time(0)); // mark it with a timestamp
	s.Append("_").Append(fNextId);
}

String SessionListManager::GetUniqueInstanceId() {
	return String(fUniqueInstanceId, coast::storage::Current());
}

bool SessionListManager::SessionIsBusy(Session *session, bool &isBusy, Context &ctx)
{
	StartTrace(SessionListManager.SessionIsBusy);
	long start( time(0) );
	long now( start );
	while ( ( isBusy = session->IsBusy() ) && ( ( now - start ) < 60 ) ) {
		Thread::Wait(1);
		now = time(0);
	}
	return isBusy;
}

bool SessionListManager::VerifySession(Session *session, String &sessionId, Context &ctx)
{
	StartTrace(SessionListManager.VerifySession);
	if (!session->Verify(ctx)) {
		String logMsg(session->GetId());
		logMsg << " Can't verify session";
		SystemLog::Info(logMsg);
		String newId;
		GetNextId(newId, ctx);
		ctx.GetQuery()["sessionId"] = newId;
		Trace("leaving with false");
		return false;
	}
	Trace("leaving with true");
	return true;
}

Session *SessionListManager::PrepareSession(Session *session, bool &isBusy, Context &ctx)
{
	StartTrace(SessionListManager.PrepareSession);

	Trace("Session to prepare:[" << (long)session << "]");
	String sessionId = ctx.Lookup("SessionId", "");
	if (session) {
		if (SessionIsBusy(session, isBusy, ctx)) {
			return session;
		}
	}

	if (session == 0 || !VerifySession(session, sessionId, ctx)) {
		session = CreateSession(sessionId, ctx);
	}

	if (session != 0) {
		// From now on we have the session available in context
		ctx.Push(session);
		if ( fSessionFactory ) {
			session = fSessionFactory->DoPrepareSession(ctx, session, isBusy);
		}
		Trace("Session prepared:[" << (long)session << "]");
	} else {
		Trace("No session available. Request is not processed. Adjust MaxSessionsAllowed.");
	}
	return session;
}

long SessionListManager::CleanupSessions(Context &ctx, bool forceLock)
{
	StartTrace(SessionListManager.CleanupSessions);
	TRACE_LOCK_START("CleanupSessions");
	long szActiveBefore = 0;
	long szActiveAfter = 0;
	long szDisabled = 0;
	Anything sessions2Delete;
	bool wasLocked = false;
	if (fLogToCerr) {
		String m("SLM::CleanupSessions entering.\n");
		SystemLog::WriteToStderr(m);
	}
	{
		wasLocked = fSessionsMutex.TryLock();
		if (!wasLocked && forceLock) {
			fSessionsMutex.Lock();
			wasLocked = true;
		}
		if (wasLocked) {
			if (fLogToCerr) {
				String m;
				m << "SLM::CleanupSessions got mutex (Start cleaning). Time: [" << TimeStamp::Now().AsString() << "]\n";
				SystemLog::WriteToStderr(m);
			}
			wasLocked = true;
			szActiveBefore = fSessions.GetSize();
			Trace("sz active before: " << szActiveBefore);
			Trace("sz disabled before: " << fDisabledSessions.GetSize());
			sessions2Delete = DoCleanup(fSessions, sessions2Delete, ctx);
			sessions2Delete = DoCleanup(fDisabledSessions, sessions2Delete, ctx, true);
			szActiveAfter = fSessions.GetSize();
			szDisabled =  fDisabledSessions.GetSize();
			Trace("sz active after: " << szActiveAfter);
			Trace("sz disabled after: " << szDisabled);
			fSessionsMutex.Unlock();
		}
	}
	if (wasLocked) {
		DoDeleteSessions(sessions2Delete, ctx);

		if (fLogToCerr) {
			// Active:   sessions currently in use, accessible through SLM
			// Deleted:  These sessions was sent the eRemoved event and then they where destructed
			// Disabled: Kept under SLM control, but no more accesible (avoid destruction of session
			//           objects still in use by requests)
			OStringStream os;
			os	<< "Sessions now Active   : <" << std::setw(7) << szActiveAfter << ">  " <<
				"Deleted  : <" << std::setw(7) << (szActiveBefore - szActiveAfter) << ">  " <<
				"Disabled : <" << std::setw(9) << szDisabled << ">" << std::endl;
			SystemLog::WriteToStderr(os.str());
		}
	}
	if (fLogToCerr) {
		String m;
		m << "SLM::CleanupSessions leave.    (End   cleaning). Time: [" << TimeStamp::Now().AsString() << "]\n";
		SystemLog::WriteToStderr(m);
	}
	return szActiveAfter;
}

Anything SessionListManager::DoCleanup(Anything &sessionList, Anything &sessions2Delete, Context &ctx, bool roleNotRelevant)
{
	StartTrace(SessionListManager.DoCleanup);

	long szBefore = sessionList.GetSize();
	Trace("szBefore: " << szBefore);
	long secs = time(0);
	for (long i = szBefore - 1; i >= 0; --i) {
		Session *s = SafeCast(sessionList[i].AsIFAObject(0), Session);

		if (s && s->IsDeletable(secs, ctx, roleNotRelevant)) {
			// remove terminated session from sessions list
			sessions2Delete.Append(sessionList[i]);
			sessionList.Remove(i);
		}
		if (s == ctx.GetSession()) {
			ctx.Push((Session *)0);
		}
	}
	Trace("szAfter: " << sessionList.GetSize());
	Trace("szDelete: " << sessions2Delete.GetSize());
	return sessions2Delete;
}

void SessionListManager::DoDeleteSessions(const Anything &sessions2Delete, Context &ctx)
{
	StartTrace(SessionListManager.DoDeleteSessions);

	long sz = sessions2Delete.GetSize();
	Trace("have to delete " << sz << " sessions");
	for (long i = 0; i < sz; ++i) {
		Session *s = SafeCast(sessions2Delete[i].AsIFAObject(0), Session);
		if (s) {
			Trace("notifying (Session::eRemoved) session &" << (long)s);
			s->Notify(Session::eRemoved, ctx);
			delete s;
		}
	}
}

void SessionListManager::ForcedSessionCleanUp(Context &ctx)
{
	StartTrace(SessionListManager.ForcedSessionCleanUp);
	TRACE_LOCK_START("ForcedSessionCleanUp");
	long szNumberOfSessions = 0;
	{
		LockUnlockEntry me(fSessionsMutex);
		szNumberOfSessions = fSessions.GetSize();
		String logMsg("Force deleting ");
		logMsg << szNumberOfSessions << " Sessions";
		Trace(logMsg);
		SystemLog::Info(logMsg);
		DoDeleteSessions(fSessions, ctx);
		fSessions = Anything(Anything::ArrayMarker(), fSessions.GetAllocator());

		logMsg = "Force deleting ";
		logMsg << fDisabledSessions.GetSize() << " disabled Sessions";
		Trace(logMsg);
		SystemLog::Info(logMsg);
		DoDeleteSessions(fDisabledSessions, ctx);
		fDisabledSessions = Anything(Anything::ArrayMarker(), fDisabledSessions.GetAllocator());
	}
}

long SessionListManager::GetNumberOfSessions()
{
	StartTrace(SessionListManager.GetNumberOfSessions);
	TRACE_LOCK_START("GetNumberOfSessions");
	LockUnlockEntry me(fSessionsMutex);
	return fSessions.GetSize();
}

URLFilter *SessionListManager::FindURLFilter(Context &ctx)
{
	StartTrace(SessionListManager.FindURLFilter);
	// select an URLFilter if defined in configuration and available
	const char *ufName = "URLFilter";
	ROAnything ufn;
	if (ctx.Lookup("URLFilterName", ufn)) {
		ufName = ufn.AsCharPtr(ufName);
	}
	URLFilter *uf = URLFilter::FindURLFilter(ufName);

	// use default URLFilter if defined filter is not available
	if (!uf) {
		String logMsg("URLFilter <");
		logMsg << ufName << "> not found";
		SystemLog::Error(logMsg);
		uf = URLFilter::FindURLFilter("URLFilter");
	} else {
		Trace("Using Filter <" << ufName << ">");
	}
	return uf;
}

Anything SessionListManager::StdFilterTags(Context &ctx)
{
	StartTrace(SessionListManager.StdFilterTags);
	Anything tmpFilter;

	// setup filter manually to be backward compatible
	tmpFilter["Tags2Suppress"].Append("sessionId");
	tmpFilter["Tags2Suppress"].Append("role");
	tmpFilter["Tags2Suppress"].Append("page");

	ROAnything roleChangeActions, roaEntry;
	if (ctx.Lookup("RoleChanges", roleChangeActions) ) {
		TraceAny(roleChangeActions,"role changes");
		AnyExtensions::Iterator<ROAnything> slotIter(roleChangeActions);
		String strAction;
		while (slotIter.Next(roaEntry)) {
			if ( slotIter.SlotName(strAction)) {
				Trace("adding action [" << strAction << "]");
				tmpFilter["Tags2Suppress"]["action"].Append(strAction);
			}
		}
	}

	long useBaseURL = 0;
	if (ctx.Lookup("UseBaseURL", useBaseURL)) {
		tmpFilter["Tags2Unscramble"].Append("X1");
		tmpFilter["Tags2Unscramble"].Append("X2");
	} else {
		tmpFilter["Tags2Unscramble"].Append("X");
	}
	TraceAny(tmpFilter, "default filter tags");
	return tmpFilter;
}

bool SessionListManager::FilterQuery(Context &ctx)
{
	StartTrace1(SessionListManager.FilterQuery, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");

	Anything env(ctx.GetEnvStore());
	Anything query(ctx.GetQuery());
	URLFilter *uf = FindURLFilter(ctx);

	if (uf) {
		// handle cookie copying if filter defined
		ROAnything filterCookieConf(ctx.Lookup("CookieFilterSpec"));
		if (!filterCookieConf.IsNull()) {
			if (!uf->HandleCookie(query, env, filterCookieConf, ctx)) {
				SystemLog::Warning("Environment contained no cookies");
			}
		}

		// handle query filtering
		Anything tmpFilter;
		ROAnything filterQueryConf(ctx.Lookup("URLFilterSpec"));
		if ( filterQueryConf.IsNull() ) {
			tmpFilter = StdFilterTags(ctx); // necessary because of lifetime issues
			filterQueryConf = tmpFilter;
		}

		uf->HandleQuery(query, filterQueryConf, ctx);
	} else {
		return false;
	}
	TraceAny(query, "query: ");
	return true;
}

String SessionListManager::FilterQueryAndGetId(Context &ctx) {
	StartTrace(SessionListManager.FilterQueryAndGetId);
	// now we try to find a session id
	String sessionId;
	Anything query(ctx.GetQuery());
	Anything sid;
	if (FilterQuery(ctx) && query.LookupPath(sid, "sessionId")) {
		TraceAny(query, "query: ");
		sessionId = sid.AsCharPtr(0);
	}
	return sessionId;
}

//:support reinit by locking out session cleaner and other session list activity
void SessionListManager::EnterReInit()
{
	StartTrace(SessionListManager.EnterReInit);
	TRACE_LOCK_START("EnterReInit");
	fSessionsMutex.Lock();
}

void SessionListManager::LeaveReInit()
{
	StartTrace(SessionListManager.LeaveReInit);
	TRACE_LOCK_START("LeaverReInit");
	fSessionsMutex.Unlock();
}

bool SessionListManager::SessionListInfo(Anything &sessionListInfo, Context &ctx, const ROAnything &config)
{
	StartTrace(SessionListManager.SessionListInfo);
	TRACE_LOCK_START("SessionListInfo");
	if (fSessionsMutex.TryLock()) {
		sessionListInfo["List"] = Anything(Anything::ArrayMarker());
		ctx.GetTmpStore()["SessionInfo"] = sessionListInfo["List"];
		Session *originalSession = ctx.GetSession();
		long szSessionListSize = fSessions.GetSize();
		Trace("sz active: " << szSessionListSize);
		sessionListInfo["Size"] = szSessionListSize;

		long start = config["Start"].AsLong(0L);
		long pageSize = config["PageSize"].AsLong((szSessionListSize > 10L) ? 10L : szSessionListSize);
		pageSize = (szSessionListSize < start + pageSize) ? szSessionListSize - start : pageSize;

		for (long i = start; i < start + pageSize; ++i) {
			Session *s = SafeCast(fSessions[i].AsIFAObject(0), Session);
			if ( s ) {
				const char *slotName = fSessions.SlotName(i);
				Assert(slotName);
				sessionListInfo["List"][slotName]["id"] = slotName;
				s->GetSessionInfo(sessionListInfo, ctx, slotName);
			}
		}
		sessionListInfo["Pages"] = szSessionListSize / pageSize + (szSessionListSize % pageSize) ? 1 : 0;
		sessionListInfo["PageSize"] = pageSize;
		ctx.Push(originalSession);
		fSessionsMutex.Unlock();
		return true;
	}
	return false;
}

bool SessionListManager::GetASessionsInfo(Anything &sessionInfo, const String &sessionId, Context &ctx, const ROAnything &config)
{
	StartTrace(SessionListManager.GetASessionsInfo);
	TRACE_LOCK_START("GetASessionsInfo");
	Session *originalSession = ctx.GetSession();
	Session *s = (Session *) NULL;
	LockUnlockEntry mutex(fSessionsMutex);
	{
		Anything session;
		if ( fSessions.LookupPath(session, sessionId) ) {
			s = SafeCast(session.AsIFAObject(0), Session);
		}
	}
	if (!s) {
		return false;
	}
	// If we query our own session, Context.InitSession will not decrement refcounts
	// because old and new sessions are the same.
	bool tmpIncrementRefCount = (originalSession != s);
	if (tmpIncrementRefCount) {
		originalSession->fMutex.Lock();
		originalSession->Ref();
		originalSession->fMutex.Unlock();
	}
	sessionInfo = Anything(Anything::ArrayMarker());
	ctx.GetTmpStore()["SessionInfo"] = sessionInfo;
	bool ret = s->GetSessionInfo(sessionInfo, ctx, "SessionInfo");
	ctx.Push(originalSession);
	if (tmpIncrementRefCount) {
		originalSession->fMutex.Lock();
		originalSession->UnRef();
		originalSession->fMutex.Unlock();
	}
	return ret;
}

Session *SessionFactory::DoMakeSession(Context &ctx) {
	StartTrace(SessionFactory.DoMakeSession);
	Trace("fName: " << fName);
	return new (coast::storage::Global()) Session(fName);
}

Session *SessionFactory::DoPrepareSession(Context &ctx, Session *session, bool &isBusy) {
	StartTrace(SessionFactory.DoPrepareSession);
	return session;
}

bool SessionFactory::DoLoadConfig(const char *category) {
	StartTrace1(SessionFactory.DoLoadConfig, "category: <" << NotNull(category) << ">");
	if (HierarchConfNamed::DoLoadConfig(category) && fConfig.IsDefined(fName)) {
		// AcceptorFactories use only a subset of the whole configuration file
		fConfig = fConfig[fName];
		Assert(!fConfig.IsNull());
		TraceAny(fConfig, "fConfig: ");
		return (!fConfig.IsNull());
	}
	Trace("fConfig is null, returning false");
	fConfig = Anything();
	return false;
}

bool SessionFactory::DoGetConfigName(const char *category, const char *objName, String &configFileName) const {
	StartTrace1(SessionFactory.DoGetConfigName, "category: <" << NotNull(category) << "> object: <" << NotNull(objName) << ">");
	configFileName = "Config";
	Trace("returning <" << configFileName << "> true");
	return true;
}

RegCacheImpl(SessionFactory);
RegisterSessionFactory(SessionFactory);

RegisterAction(CleanSessions);

bool CleanSessions::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace(CleanSessions.DoExecAction);
	SessionListManager *slm = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	if (slm) {
		slm->CleanupSessions(ctx);
		return true;
	}
	return false;
}

RegisterModule(SessionFactoriesModule);

bool SessionFactoriesModule::Init(const ROAnything config) {
	StartTrace(SessionFactoriesModule.Init);
	TraceAny(config, "config");
	if (config.IsDefined("SessionFactories")) {
		HierarchyInstaller hi("SessionFactory");
		return RegisterableObject::Install(config["SessionFactories"], "SessionFactory", &hi);
	}
	return false;
}

bool SessionFactoriesModule::ResetFinis(const ROAnything config) {
	StartTrace(SessionFactoriesModule.ResetFinis);
	AliasTerminator at("SessionFactory");
	return RegisterableObject::ResetTerminate("SessionFactory", &at);
}

bool SessionFactoriesModule::Finis() {
	StartTrace(SessionFactoriesModule.Finis);
	return StdFinis("SessionFactory", "SessionFactories");
}
