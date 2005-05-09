/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Context.h"

//--- project modules used -----------------------------------------------------
#include "Session.h"
#include "Server.h"
#include "Role.h"
#include "Page.h"
#include "LocalizationUtils.h"
#include "LocalizedStrings.h"
#include "AnythingUtils.h"

//--- standard modules used ----------------------------------------------------
#include "Socket.h"
#include "StringStream.h"
#include "Dbg.h"

#ifdef DEBUG
#include "SysLog.h"
#endif

//--- c-library modules used ---------------------------------------------------

const String Context::DebugStoreSeparator("<!--separator 54353021345321784456 -->");

//---- Context ------------------------------------------------------------------
Context::Context() :
	fSession(0),
	fSessionStoreGlobal(MetaThing(Storage::Global()), Storage::Global()),
	fSessionStoreCurrent(MetaThing(Storage::Current()), Storage::Current()),
	fStackSz(0),
	fStoreSz(0),
	fRequest(),
	fSocket(0),
	fMockStream(0),
	fUnlockSession(false),
	fCopySessionStore(false)
{
	InitTmpStore();
}
Context::Context(Anything &request) :
	fSession(0),
	fSessionStoreGlobal(MetaThing(Storage::Global()), Storage::Global()),
	fSessionStoreCurrent(MetaThing(Storage::Current()), Storage::Current()),
	fStackSz(0),
	fStoreSz(0),
	fRequest(request),
	fSocket(0),
	fMockStream(0),
	fUnlockSession(false),
	fCopySessionStore(false)

{
	InitTmpStore();
	fLanguage = LocalizationUtils::FindLanguageKey(*this, Lookup("Language", "D"));
}
Context::Context(Socket *socket)  :
	fSession(0),
	fSessionStoreGlobal(MetaThing(Storage::Global()), Storage::Global()),
	fSessionStoreCurrent(MetaThing(Storage::Current()), Storage::Current()),
	fStackSz(0),
	fStoreSz(0),
	fSocket(socket),
	fMockStream(0),
	fUnlockSession(false),
	fCopySessionStore(false)
{
	// the arguments we get for this request
	if ( fSocket ) {
		fRequest["ClientInfo"] = fSocket->ClientInfo();
	}
	InitTmpStore();
	fLanguage = LocalizationUtils::FindLanguageKey(*this, Lookup("Language", "D"));
}
Context::Context(iostream *stream)  :
	fSession(0),
	fSessionStoreGlobal(MetaThing(Storage::Global()), Storage::Global()),
	fSessionStoreCurrent(MetaThing(Storage::Current()), Storage::Current()),
	fStackSz(0),
	fStoreSz(0),
	fSocket(0),
	fMockStream(stream),
	fUnlockSession(false),
	fCopySessionStore(false)
{
	// the arguments we get for this request
	InitTmpStore();
	fLanguage = LocalizationUtils::FindLanguageKey(*this, Lookup("Language", "D"));
}
Context::Context(const Anything &env, const Anything &query, Server *server, Session *s, Role *role, Page *page)
	:	fSession(0), // don't initialize because InitSession would interpret it as same session and not increment
		// session's ref count while the destructor decrements it. Init(s) does the needed intitialization
		// while InitSession handels the refcounting correctly.
		fSessionStoreGlobal(MetaThing(Storage::Global()), Storage::Global()),
		fSessionStoreCurrent(MetaThing(Storage::Current()), Storage::Current()),
		fStackSz(0),
		fStoreSz(0),
		fSocket(0),
		fMockStream(0),
		fUnlockSession(false),
		fCopySessionStore(false)
{

	InitSession(s);
	InitTmpStore();
	fRequest["env"] = env;
	fRequest["query"] = query;
	Push("Server", server);
	Push("Role", role);
	Push("Page", page);
	fLanguage = LocalizationUtils::FindLanguageKey(*this, Lookup("Language", "D"));
}

Context::~Context()
{
	if (fSession) {
		LockSession();
		// SOP: should we resynch store again, or should PutInStore take care?
		fSession->UnRef();
		fSession->fMutex.Unlock();
	}
}

void Context::InitSession(Session *s)
{
	// Make a copy of the session store if fCopySessionStore is on. Reference the session to
	// inhibit premature destruction of session object.
	StartTrace1(Context.InitSession, String() << (long)(void *)this);

	bool sessionIsDifferent =  (s != fSession);
	ROAnything contextAny;
	if (Lookup("Context", contextAny)) {
		fUnlockSession = contextAny["UnlockSession"].AsBool(false);
		fCopySessionStore = contextAny["CopySessionStore"].AsBool(false);
	}

	Trace("CopySessionStore: " << (fCopySessionStore ? "true" : "false"));
	Trace("UnlockSession: " << (fUnlockSession ? "true" : "false"));
	Trace("s = " << (long)(void *)s << " fSession = " << (long)(void *)fSession );
	Trace("session is " << (sessionIsDifferent ? "" : "not ") << "different");

	Session *saveSession = fSession;
	if (sessionIsDifferent || fCopySessionStore) {
		// first handle pushed session because it might get deleted underway
		fSession = s;
		if (fSession) {
			Trace("InitSession new s: About to   lock <" << fSession->GetId() << ">");
			fSession->fMutex.Lock();
			if (sessionIsDifferent) {
				fSession->Ref();
			}
			if (fCopySessionStore) {
				fSessionStoreCurrent = fSession->GetStoreGlobal().DeepClone(fSessionStoreCurrent.GetAllocator());
				Trace("InitSession new s: About to unlock <" << fSession->GetId() << ">");
			} else {
				fSessionStoreGlobal = fSession->GetStoreGlobal();
			}
			UnlockSession();
		} else {
			if (fCopySessionStore) {
				fSessionStoreCurrent = MetaThing(fSessionStoreCurrent.GetAllocator());
			} else {
				fSessionStoreGlobal = MetaThing(fSessionStoreGlobal.GetAllocator());
			}
		}
		if (saveSession) {
			if (sessionIsDifferent) {
				if (fUnlockSession) {
					Trace("InitSession old s: About to lock <" << saveSession->GetId() << ">");
					saveSession->fMutex.Lock();
				}
				saveSession->UnRef();
				if (fUnlockSession) {
					Trace("InitSession old s: About to unlock <" << saveSession->GetId() << ">");
					saveSession->fMutex.Unlock();
				}
			}
		}
		// for test cases with no session given, the session store does not survive
	}
}

void Context::InitTmpStore()
{
	MetaThing tmp;
	PushStore("tmp", tmp);
}

Session *Context::GetSession() const
{
	return fSession;
}

const char *Context::GetSessionId() const
{
	return (fSession) ? fSession->GetId() : 0;
}

void Context::SetServer(Server *server)
{
	Replace("Server", server);
}

Server *Context::GetServer() const
{
	return SafeCast(Find("Server"), Server);
}

void Context::SetRole(Role *role)
{
	Replace("Role", role);
}

Role *Context::GetRole() const
{
	return SafeCast(Find("Role"), Role);
}

void Context::SetPage(Page *p)
{
	StartTrace1(Context.SetPage, "New Page &" << (long)p );
	Replace("Page", p);
}

Page *Context::GetPage() const
{
	return SafeCast(Find("Page"), Page);
}

void Context::SetQuery(const Anything &query)
{
	fRequest["query"] = query;
}

Anything &Context::GetQuery()
{
	return fRequest["query"];
}

Anything &Context::GetEnvStore()
{
	return fRequest["env"];
}

Anything &Context::GetRoleStoreGlobal()
{
	return GetSessionStore()["RoleStore"];
}

Anything &Context::GetSessionStore()
{
	return fCopySessionStore ? fSessionStoreCurrent : fSessionStoreGlobal;
}

Anything &Context::GetTmpStore()
{
	return GetBuiltinStore("tmp");
}

Anything &Context::GetBuiltinStore(const char *key)
{
	long top = fStore[key].GetSize() - 1;
	return fStore[key][top];
}

//--- lookup immutable objects ----------------
/*
original sacred sequence of lookups
bool Context::DoLookup(const char *key, ROAnything &result, char delim) const
{
	StartTrace1(Context.Lookup, "key: <" << NotNull(key) << ">");

	ROAnything tmpStore(fTmpStore);
	//test for path expression
	Assert(key);
	if ( tmpStore.LookupPath(result, key, delim) )
	{
		Trace(key << " found in TmpStore");
		return true;
	}
	ROAnything roleStore(fRoleStore);
	if ( roleStore.LookupPath(result, key, delim) )
	{
		Trace(key << " found in roleStore");
		return true;
	}
	if (fRole && fRole->Lookup(key, result, delim))
	{
		Trace(key << " found in fRole");
		return true;
	}
	if (fSession && fSession->Lookup(key, result, delim))
	{
		Trace(key << " found in fSession");
		return true;
	}

	LocalizedStrings *ls= LocalizedStrings::LocStr();
	if (ls && ls->Lookup(key, result))
	{
		Trace(key << " found in LocalizedStrings");
		return true;
	}

	if (fPage && fPage->Lookup(key, result, delim))
	{
		Trace(key << " found in fPage");
		return true;
	}

	if (fServer && fServer->Lookup(key, result, delim))		// look in server first because of
															// string lookup
	{
		Trace(key << " found in fServer");
		return true;
	}

	ROAnything env(fEnvStore);
 	if ( env.LookupPath(result, key) )
 		return true;

	ROAnything query(fQueryStore);
 	if ( query.LookupPath(result, key, delim) )
 		return true;

	Trace(key << " not found");
	return false;
}

bool Context::DoLookup(const char *key, ROAnything &result, char delim) const
{
	StartTrace1(Context.Lookup, "key: <" << NotNull(key) << ">");

	ROAnything tmpStore(fTmpStore);
	//test for path expression
	Assert(key);

	// policy: stores before configs
	if ( tmpStore.LookupPath(result, key, delim) )
	{
		Trace(key << " found in TmpStore");
		return true;
	}
	ROAnything roleStore(fRoleStore);
	if ( roleStore.LookupPath(result, key, delim) )
	{
		Trace(key << " found in roleStore");
		return true;
	}

	if (fRole && fRole->Lookup(key, result, delim))
	{
		Trace(key << " found in fRole");
		return true;
	}
	if (fSession && fSession->Lookup(key, result, delim))
	{
		Trace(key << " found in fSession");
		return true;
	}
	LocalizedStrings *ls= LocalizedStrings::LocStr();
	if (ls && ls->Lookup(key, result))
	{
		Trace(key << " found in LocalizedStrings");
		return true;
	}

	if (fPage && fPage->Lookup(key, result, delim))
	{
		Trace(key << " found in fPage");
		return true;
	}

	if (fServer && fServer->Lookup(key, result, delim))		// look in server first because of
															// string lookup
	{
		Trace(key << " found in fServer");
		return true;
	}

	// legacy
	ROAnything env(fEnvStore);
 	if ( env.LookupPath(result, key) )
 		return true;

	ROAnything query(fQueryStore);
 	if ( query.LookupPath(result, key, delim) )
 		return true;

	Trace(key << " not found");
	return false;
}
*/
void Context::CollectLinkState(Anything &a)
{
	Role *r = GetRole();
	if (r) {
		r->CollectLinkState(a, *this);
	}
}

void Context::DebugStores(const char *msg, ostream &reply, bool printAny)
{
#ifdef DEBUG
	SysLog::WriteToStderr(String("+++++++++++++++++++") << NotNull(msg) << "+++++++++++++++++++++++++\n");
	SysLog::WriteToStderr(String("store(") << fStore.RefCount() << "): \n");
	if (printAny) {
		String strbuf;
		StringStream stream(strbuf);
		fStore.PrintOn(stream) << "\n";
		stream.flush();
		SysLog::WriteToStderr(strbuf);
	}
	SysLog::WriteToStderr(String("Request(") << fRequest.RefCount() << "): \n");
	if (printAny) {
		String strbuf;
		StringStream stream(strbuf);
		fRequest.PrintOn(stream) << "\n";
		stream.flush();
		SysLog::WriteToStderr(strbuf);
	}
	SysLog::WriteToStderr(String("-------------------") << NotNull(msg) << "-------------------------\n");
#endif
}

void Context::HTMLDebugStores(ostream &reply)
{
#ifdef DEBUG
	if ( Tracer::CheckWDDebug("Context.HTMLWDDebug", Storage::Current()) ) {
		reply << DebugStoreSeparator;
		reply << "<p><hr>\n<xmp>\n";
		Session *s = fSession;
		if (s) {
			reply << "Session-Nummer: " << s->GetId() << "\n";
			reply << "Access-Counter: " << s->GetAccessCounter() << "\n";
			reply << "Access-Time:    " << s->GetAccessTime() << "\n";
			reply << "Ref-Count:      " << s->GetRefCount() << "\n";
		}
		Page *page = GetPage();
		if (page) {
			String pName;
			page->GetName(pName);
			reply << "Page:           " << pName << "\n";
		}
		String rName("None");
		Role *r = GetRole();
		if (r) {
			r->GetName(rName);
		}
		reply << "Role:           " << rName << "\n\n";

		// show Lookup stack on html page
		if ( Tracer::CheckWDDebug("Context.HTMLWDDebug.LookupStack", Storage::Current()) ) {
			reply << "</xmp><hr><xmp>Lookup stack:\n" << fLookupStack << "\n";
		}

		// show tmp store on html page
		if ( Tracer::CheckWDDebug("Context.HTMLWDDebug.TmpStore", Storage::Current()) ) {
			reply << "</xmp><hr><xmp>Tmp store:\n" << fStore << "\n";
		}

		// show session store on html page
		if ( fSession  ) {
			fSession->HTMLDebugStore(reply);
		}

		// show request store on html page
		if ( Tracer::CheckWDDebug("Context.HTMLWDDebug.EnvStore", Storage::Current()) ) {
			reply << "</xmp><hr><xmp>Request:\n" << fRequest << "\n";
		}

		reply << "</xmp>\n";
	}
#endif
}

//---- new implementation ---------------------------------------------

void Context::Push(LookupInterface *li)
{
	StartTrace(Context.Push);
	DoPush("??anonymous??", li);
}

bool Context::Pop()
{
	StartTrace(Context.Pop);

	if ( fStackSz > 0 ) {
		fLookupStack.Remove(fStackSz);
		fStackSz--;
		fLookupStack[0L].Remove(fStackSz);
		Trace("Stack[" << fStackSz << "]");
		return true;
	}

	Trace("Stack[" << fStackSz << "]");
	return false;
}

void Context::DoPush(const char *key, LookupInterface *li)
{
	StartTrace1(Context.DoPush, "key:<" << NotNull(key) << ">");
	if ( key && li ) {
		fLookupStack[0L].Append(key);
		fLookupStack.Append((IFAObject *)li);
		Trace("Stack[" << fStackSz << "]");
		fStackSz++;
	}
}

void Context::Push(const char *key, LookupInterface *ro)
{
	StartTrace1(Context.Push, "key<" << NotNull("key") << ">");

	DoPush(key, ro);
}

void Context::Push(Session *s)
{
	StartTrace1(Context.Push, "session");

	InitSession(s);

	Trace("Stack[" << fStackSz << "]");
}

void Context::PushRequest(const Anything &request)
{
	StartTrace1(Context.Push, "session");

	fRequest = request;
	TraceAny(fRequest, "Request: ");
}

void Context::InsureArrayStore(Anything &store)
{
	if ( store.GetType() != Anything::eArray ) {
		MetaThing newStore;
		if ( !store.IsNull() ) {
			newStore.Append(store);
		}
		store = newStore;
	}
}

void Context::PushStore(const char *key, Anything &store)
{
	//FIXME: push store operations might be generalizable like lookup interface
	StartTrace1(Context.PushStore, "key:<" << NotNull(key) << ">");
	TraceAny(store, "Store to put:");

	if ( key ) {
		InsureArrayStore(store);
		fStoreHistory.Append(key);
		fStore[key].Append(store);
		fStoreSz++;
	}
	TraceAny(fStore, "fStore:");
}

bool Context::PopStore(String &key)
{
	StartTrace(Context.PopStore);

	if ( fStoreSz > 1 ) { // never pop the tmp store
		long at = fStoreHistory.GetSize() - 1;
		key = fStoreHistory[at].AsCharPtr("");
		fStoreHistory.Remove(at);

		long stack = fStore[key].GetSize() - 1;
		if ( stack >= 0 ) {
			fStore[key].Remove(stack);
			fStoreSz--;
			return true;
		}
	}
	TraceAny(fStore, "fStore:");
	return false;
}

LookupInterface *Context::Find(const char *key) const
{
	StartTrace1(Context.Find, "key:<" << NotNull(key) << ">");

	TraceAny(fLookupStack, "Lookup Stack before");

	long index = FindIndex(key);
	if (index >= 0) {
		Trace("found at fLookupStack[" << index << "]<" << NotNull(key) << ">");
		// no Safecast here, because we do a LookupInterface is not an IFAObject
		LookupInterface *li = (LookupInterface *)fLookupStack[index+1].AsIFAObject(0);
		return li;
	}
	Trace("<" << NotNull(key) << "> not found");
	return 0;
}

long Context::FindIndex(const char *key) const
{
	StartTrace1(Context.FindIndex, "key:<" << NotNull(key) << ">");

	long result = -1;

	if ( key ) {
		for (long i = fStackSz - 1; i >= 0; i--) {
			if ( fLookupStack[0L][i] == key ) {
				result = i;
				break;
			}
		}
	}

	return result;
}

long Context::Remove(const char *key)
{
	StartTrace(Context.Remove);
	if ( !key ) {
		return -1;
	}
	TraceAny(fLookupStack, "Lookup Stack before");

	long index = FindIndex(key);
	if ( index >= 0 ) {
		fLookupStack.Remove(index + 1);
		fLookupStack[0L].Remove(index);
		fStackSz--;
	}
	TraceAny(fLookupStack, "Lookup Stack after");
	return index;
}

void Context::Replace(const char *key, LookupInterface *ro)
{
	StartTrace(Context.Replace);
	if ( !key || !ro ) {
		return;
	}

	TraceAny(fLookupStack, "Lookup Stack before");

	long index = FindIndex(key);
	if ( index >= 0 ) {
		fLookupStack[index+1] = (IFAObject *)ro;
	} else {
		Push(key, ro);
	}
	TraceAny(fLookupStack, "Lookup Stack after");
}

bool Context::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(Context.DoLookup, "key:<" << NotNull(key) << ">");

	if (LookupStores(key, result, delim, indexdelim) 	||
		LookupLocalized(key, result, delim, indexdelim)	||
		LookupObjects(key, result, delim, indexdelim)	||
		LookupRequest(key, result, delim, indexdelim) ) {
		Trace("found");
		return true;
	}
	Trace("failed");
	return false;
}

bool Context::LookupStores(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(Context.LookupStores, "key:<" << NotNull(key) << ">");
	ROAnything store;
	long last = fStore.GetSize() - 1;
	long stack;
	for (long i = last; i >= 0; i--) {
		stack = fStore[i].GetSize() - 1;

		if ((stack >= 0) && ROAnything(fStore)[i][stack].LookupPath(result, key, delim, indexdelim)) {
			// stores are arranged as stacks
			// pushing the same name several times means hiding older stores
			// we use always the top element for lookups
			Trace("found");
			return true;
		}
	}

	if ( fCopySessionStore ) {
		if (ROAnything(fSessionStoreCurrent)["RoleStore"].LookupPath(result, key, delim, indexdelim)) {
			Trace("found in RoleStore [Current]");
			return true;
		}
		if (ROAnything(fSessionStoreCurrent).LookupPath(result, key, delim, indexdelim)) {
			Trace("found in SessionStore [Current]");
			return true;
		}
	} else {
		if (ROAnything(fSessionStoreGlobal)["RoleStore"].LookupPath(result, key, delim, indexdelim)) {
			Trace("found in RoleStore [Global]");
			return true;
		}
		if (ROAnything(fSessionStoreGlobal).LookupPath(result, key, delim, indexdelim)) {
			Trace("found in SessionStore [Global]");
			return true;
		}
	}
	Trace("failed");
	return false;
}

bool Context::LookupObjects(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(Context.LookupObjects, "key:<" << NotNull(key) << ">");

	// iterate over objects backwards; sequence of push defines lookup sequence in reverse -> first pushed last searched
	for (long i = fStackSz; i >= 1; i--) {
		String searchObject(fLookupStack[0L][i-1].AsCharPtr("anon"));
		Trace("searching[" << i - 1L << "]<" << searchObject << ">");
		LookupInterface *li = (LookupInterface *)fLookupStack[i].AsIFAObject(0);
		if (li) {
			if (li->Lookup(key, result, delim, indexdelim)) {
				Trace("found");
				return true;
			}
			Trace("not found");
		}
	}
	Trace("failed");
	return false;
}

bool Context::LookupRequest(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	ROAnything env = ROAnything(fRequest)["env"];
	ROAnything query = ROAnything(fRequest)["query"];

	if ( env.LookupPath(result, key, delim, indexdelim) ) {
		return true;
	}
	if ( query.LookupPath(result, key, delim, indexdelim) ) {
		return true;
	}
	return ROAnything(fRequest).LookupPath(result, key, delim, indexdelim);
}

bool Context::LookupLocalized(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(Context.LookupLocalized, "key:<" << key << ">");
	LocalizedStrings *ls = LocalizedStrings::LocStr();
	if (ls && ls->Lookup(key, result, delim, indexdelim)) {
		Trace(key << " found in LocalizedStrings");
		return true;
	}
	Trace("failed");
	return false;
}

Anything &Context::GetRequest()
{
	return fRequest;
}

Socket *Context::GetSocket()
{
	return fSocket;
}

iostream *Context::GetStream()
{
	return (fSocket) ? fSocket->GetStream() : fMockStream;
}

long Context::GetReadCount()
{
	return (fSocket) ? fSocket->GetReadCount() : 0;
}

long Context::GetWriteCount()
{
	return (fSocket) ? fSocket->GetWriteCount() : 0;
}

bool Context::GetStore(const char *key, Anything &store)
{
	StartTrace1(Context.GetStore, "key:<" << NotNull(key) << ">");
	Anything storeStack;

	if (key && fStore.LookupPath(storeStack, key)) {
		long stackindex = storeStack.GetSize() - 1;
		if ( stackindex >= 0 ) {
			store = storeStack[stackindex];
			return true;
		}
	}

	if (key && strcmp(key, "Session") == 0) {
		store = GetSessionStore();
		return true;
	}

	if (key && strcmp(key, "Role") == 0) {
		store = GetRoleStoreGlobal();
		return true;
	}

	Trace("failed");
	return false;
}

bool Context::Process(String &token)
{
	return Action::ExecAction(token, *this, Lookup(token));
}

bool Context::UnlockSession()
{
	if (fSession && fUnlockSession && fSession->IsLockedByMe()) {
		fSession->fMutex.Unlock();
		return true;
	}
	return false;
}
void Context::LockSession()
{
	if (fSession && fUnlockSession) {
		fSession->fMutex.Lock();
	}
}
