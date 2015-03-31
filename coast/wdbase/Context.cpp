/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Context.h"
#include "Session.h"
#include "Server.h"
#include "Role.h"
#include "Page.h"
#include "LocalizationUtils.h"
#include "LocalizedStrings.h"
#include "Socket.h"
#include <typeinfo>
#include <cstring>

const String Context::DebugStoreSeparator("<!-- separator 54353021345321784456 -->");

Context::Context() :
	fSession(0), fSessionStoreGlobal(Anything::ArrayMarker(), coast::storage::Global()), fSessionStoreCurrent(Anything::ArrayMarker(),
			coast::storage::Current()), fStackSz(0), fStoreSz(0), fStore(Anything::ArrayMarker()), fRequest(Anything::ArrayMarker()), fSocket(0),
			fCopySessionStore(false) {
	InitTmpStore();
}

Context::Context(Anything &request) :
	fSession(0), fSessionStoreGlobal(Anything::ArrayMarker(), coast::storage::Global()), fSessionStoreCurrent(Anything::ArrayMarker(),
			coast::storage::Current()), fStackSz(0), fStoreSz(0), fStore(Anything::ArrayMarker()), fRequest(request), fSocket(0),
			fCopySessionStore(false) {
	InitTmpStore();
	fLanguage = LocalizationUtils::FindLanguageKey(*this, Lookup("Language", "E"));
}

Context::Context(Socket *socket) :
	fSession(0), fSessionStoreGlobal(Anything::ArrayMarker(), coast::storage::Global()), fSessionStoreCurrent(Anything::ArrayMarker(),
			coast::storage::Current()), fStackSz(0), fStoreSz(0), fStore(Anything::ArrayMarker()), fRequest(Anything::ArrayMarker()), fSocket(
			socket), fCopySessionStore(false) {
	// the arguments we get for this request
	if (fSocket) {
		fRequest["ClientInfo"] = fSocket->ClientInfo();
	}
	InitTmpStore();
	fLanguage = LocalizationUtils::FindLanguageKey(*this, Lookup("Language", "E"));
}

Context::Context(const Anything &env, const Anything &query, Server *server, Session *s, Role *role, Page *page) :
			fSession(0), // don't initialize because InitSession would interpret it as same session and not increment
			// session's ref count while the destructor decrements it. Init(s) does the needed intitialization
			// while InitSession handles the refcounting correctly.
			fSessionStoreGlobal(Anything::ArrayMarker(), coast::storage::Global()), fSessionStoreCurrent(Anything::ArrayMarker(),
					coast::storage::Current()), fStackSz(0), fStoreSz(0), fStore(Anything::ArrayMarker()), fSocket(0),
			fCopySessionStore(false) {
	InitSession(s);
	InitTmpStore();
	fRequest["env"] = env;
	fRequest["query"] = query;
	Push("Server", server);
	Push("Role", role);
	Push("Page", page);
	fLanguage = LocalizationUtils::FindLanguageKey(*this, Lookup("Language", "E"));
}

Context::~Context() {
	if (fSession) {
		LockSession();
		// SOP: should we resynch store again, or should PutInStore take care?
		fSession->UnRef();
		fSession->fMutex.Unlock();
	}
}

void Context::InitSession(Session *s) {
	// Make a copy of the session store if fCopySessionStore is on. Reference the session to
	// inhibit premature destruction of session object.
	StartTrace1(Context.InitSession, String() << (long)(void *)this);

	bool sessionIsDifferent = (s != fSession);
	ROAnything contextAny;
	if (Lookup("Context", contextAny)) {
		fCopySessionStore = contextAny["CopySessionStore"].AsBool(false);
	}
	Trace("CopySessionStore: " << (fCopySessionStore ? "true" : "false"));
	Trace("s = " << (long)(void *)s << " fSession = " << (long)(void *)fSession );
	Trace("session is " << (sessionIsDifferent ? "" : "not ") << "different");

	Session *saveSession = fSession;
	if (sessionIsDifferent || fCopySessionStore) {
		// first handle pushed session because it might get deleted underway
		fSession = s;
		if (fSession) {
			Trace("new s: About to   lock <" << fSession->GetId() << ">");
			fSession->fMutex.Lock();
			if (sessionIsDifferent) {
				fSession->Ref();
				Trace("After fSession->Ref() id: [" << fSession->GetId() <<
						"] refCount: [" << fSession->GetRefCount() << "]");
			}
			if (fCopySessionStore) {
				fSessionStoreCurrent = fSession->GetStoreGlobal().DeepClone(fSessionStoreCurrent.GetAllocator());
				Trace("new s: About to unlock <" << fSession->GetId() << ">");
			} else {
				fSessionStoreGlobal = fSession->GetStoreGlobal();
			}
			UnlockSession();
		} else {
			if (fCopySessionStore) {
				fSessionStoreCurrent = Anything(Anything::ArrayMarker(),fSessionStoreCurrent.GetAllocator());
			} else {
				fSessionStoreGlobal = Anything(Anything::ArrayMarker(),fSessionStoreGlobal.GetAllocator());
			}
		}
		if (saveSession) {
			if (sessionIsDifferent) {
				// in case the session was used in UnlockSession 'mode', we need to protect the call to UnRef
				if (fCopySessionStore) {
					Trace("old s: About to lock <" << saveSession->GetId() << ">");
					saveSession->fMutex.Lock();
				}
				saveSession->UnRef();
				Trace("After saveSession->UnRef() id: [" << saveSession->GetId() << "] refCount: [" << saveSession->GetRefCount() << "]");

				// we need to unlock independently of fUnlockSession value
				Trace("old s: About to unlock <" << saveSession->GetId() << ">");
				saveSession->fMutex.Unlock();
			}
		}
		// for test cases with no session given, the session store does not survive
	}
}

void Context::InitTmpStore() {
	Anything tmp = Anything(Anything::ArrayMarker());
	Push("tmp", tmp);
}

Session *Context::GetSession() const {
	return fSession;
}

const char *Context::GetSessionId() const {
	return (fSession) ? fSession->GetId() : 0;
}

void Context::SetServer(Server *server) {
	Replace("Server", server);
}

Server *Context::GetServer() const {
	return SafeCast(Find("Server"), Server);
}

void Context::SetRole(Role *role) {
	Replace("Role", role);
}

Role *Context::GetRole() const {
	return SafeCast(Find("Role"), Role);
}

void Context::SetPage(Page *p) {
	StatTrace(Context.SetPage, "New Page [" << (p?p->GetName():"null") << "]", coast::storage::Current());
	Replace("Page", p);
}

Page *Context::GetPage() const {
	return SafeCast(Find("Page"), Page);
}

void Context::SetQuery(const Anything &query) {
	fRequest["query"] = query;
}

Anything &Context::GetQuery() {
	return fRequest["query"];
}

Anything &Context::GetEnvStore() {
	return fRequest["env"];
}

Anything &Context::GetRoleStoreGlobal() {
	return GetSessionStore()["RoleStore"];
}

Anything &Context::GetSessionStore() {
	StartTrace1(Context.GetSessionStore, "fCopySessionStore: " << ( fCopySessionStore ? "true" : "false") );
	return fCopySessionStore ? fSessionStoreCurrent : fSessionStoreGlobal;
}

Anything &Context::GetTmpStore() {
	StartTrace(Context.GetTmpStore);
	const char *key = "tmp";
	long index = -1L;
	return IntGetStore(key, index);
}

void Context::CollectLinkState(Anything &a) {
	Role *r = GetRole();
	if (r) {
		r->CollectLinkState(a, *this);
	}
}

void Context::DebugStores(const char *msg, std::ostream &reply, bool printAny) {
	if (msg) {
		reply << "+++++++++++++++++++" << NotNull(msg) << "+++++++++++++++++++++++++\n";
	}
	Session *s = fSession;
	if (s) {
		reply << "Session-Nummer: " << s->GetId() << '\n';
		reply << "Access-Counter: " << s->GetAccessCounter() << '\n';
		reply << "Access-Time:    " << s->GetAccessTime() << '\n';
		reply << "Ref-Count:      " << s->GetRefCount() << '\n';
	}
	Page *page = GetPage();
	if (page) {
		String pName;
		page->GetName(pName);
		reply << "Page:           " << pName << '\n';
	}
	String rName("None");
	Role *r = GetRole();
	if (r) {
		r->GetName(rName);
	}
	reply << "Role:           " << rName << "\n\n";

	// show Lookup stack on html page
	if (TriggerEnabled(Context.HTMLWDDebug.LookupStack) || printAny) {
		reply << "Lookup stack #refs:" << fLookupStack.RefCount() << '\n' << fLookupStack << '\n';
	}

	// show tmp store on html page
	if (TriggerEnabled(Context.HTMLWDDebug.TmpStore) || printAny) {
		reply << "Tmp store #refs:" << fStore.RefCount() << '\n' << fStore << '\n';
	}

	// show session store on html page
	if (fSession) {
		fSession->HTMLDebugStore(reply);
	}

	// show request store on html page
	if (TriggerEnabled(Context.HTMLWDDebug.EnvStore) || printAny) {
		reply << "Request #refs:" << fRequest.RefCount() << '\n' << fRequest << '\n';
	}
	if (msg) {
		reply << "-------------------" << NotNull(msg) << "-------------------------\n";
	}
	reply.flush();
}

void Context::HTMLDebugStores(std::ostream &reply) {
	if (TriggerEnabled(Context.HTMLWDDebug)) {
		reply << DebugStoreSeparator;
		reply << "<hr>\n<pre>\n";
		DebugStores(0, reply);
		reply << "</pre>\n";
	}
}

Anything &Context::IntGetStore(const char *key, long &index) {
	StartTrace1(Context.IntGetStore, "key:<" << NotNull(key) << ">");
	TraceAny(fStore, "fStore and size:" << fStoreSz);
	index = fStoreSz;
	while (index >= 0) {
		index = FindIndex(fStore, key, index);
		if (index >= 0) {
			// matching entry found, check if it is of correct type
			if (fStore["Stack"][index].GetType() != AnyObjectType) {
				TraceAny(fStore["Stack"][index], "found top element of key [" << key << "] at index:" << index);
				return fStore["Stack"][index];
			} else {
				SYSWARNING("IntGetStore entry at [" << key << "] is not of expected Anything-type!");
			}
		}
		--index;
	}
	return fEmpty;
}

bool Context::GetStore(const char *key, Anything &result) {
	StartTrace1(Context.GetStore, "key:<" << NotNull(key) << ">");

	if (key) {
		long index = -1;
		result = IntGetStore(key, index);
		if (index >= 0) {
			return true;
		}
		if (strcmp(key, "Session") == 0) {
			result = GetSessionStore();
			return true;
		}
		if (strcmp(key, "Role") == 0) {
			result = GetRoleStoreGlobal();
			return true;
		}
	}
	Trace("failed");
	return false;
}

bool Context::Push(const char *key, LookupInterface *li) {
	StartTrace1(Context.Push, "key:<" << NotNull(key) << "> li:&" << (long)li);
	if (key && li) {
		Trace( "TypeId of given LookupInterface:" << typeid(*li).name());
		bool bIsLookupAdapter = ((typeid(*li) == typeid(AnyLookupInterfaceAdapter<Anything> )) || (typeid(*li)
								== typeid(AnyLookupInterfaceAdapter<ROAnything> )));
		if (bIsLookupAdapter) {
			fStore["Keys"].Append(key);
			fStore["Stack"].Append((IFAObject *) li);
			++fStoreSz;
			TraceAny(fStore, "fStore and size:" << fStoreSz);
		} else {
			fLookupStack["Keys"].Append(key);
			fLookupStack["Stack"].Append((IFAObject *) li);
			++fStackSz;
			TraceAny(fLookupStack, "fLookupStack and size:" << fStackSz);
		}
		return true;
	}
	return false;
}

bool Context::Pop(String &key) {
	StartTrace(Context.Pop);
	if (fStackSz > 0) {
		--fStackSz;
		key = fLookupStack["Keys"][fStackSz].AsString();
		fLookupStack["Stack"].Remove(fStackSz);
		fLookupStack["Keys"].Remove(fStackSz);
		TraceAny(fLookupStack, "fLookupStack and size:" << fStackSz);
		return true;
	}
	return false;
}

bool Context::Push(const char *key, Anything &store) {
	StartTrace1(Context.Push, "key:<" << NotNull(key) << ">");
	TraceAny(store, "Store to put:");

	if (key && (store.GetType() != AnyNullType)) {
		// EnsureArrayImpl is needed to be able to extend existing stack entries by reference
		// without this conversion, a problem would arise when a simple value was pushed which got extended by other values
		//  -> only the simple value would persist
		Anything::EnsureArrayImpl(store);
		fStore["Keys"].Append(key);
		fStore["Stack"].Append(store);
		++fStoreSz;
		TraceAny(fStore, "fStore and size:" << fStoreSz);
		return true;
	}
	return false;
}

bool Context::PopStore(String &key) {
	StartTrace(Context.PopStore);
	if (fStoreSz > 1) { // never pop the tmp store at "fStore.tmp:0"
		--fStoreSz;
		key = fStore["Keys"][fStoreSz].AsString();
		fStore["Stack"].Remove(fStoreSz);
		fStore["Keys"].Remove(fStoreSz);
		TraceAny(fStore, "fStore and size:" << fStoreSz);
		return true;
	}
	return false;
}

void Context::Push(Session *s) {
	StartTrace1(Context.Push, "session");
	InitSession(s);
}

void Context::PushRequest(const Anything &request) {
	StartTrace1(Context.PushRequest, "request");
	fRequest = request;
	TraceAny(fRequest, "Request: ");
}

LookupInterface *Context::Find(const char *key) const {
	StartTrace1(Context.Find, "key:<" << NotNull(key) << ">");

	TraceAny(fLookupStack, "fLookupStack and size:" << fStackSz);

	long index = FindIndex(fLookupStack, key);
	if (index >= 0) {
		Trace("found at fLookupStack[Stack][" << index << "]<" << NotNull(key) << ">");
		// no Safecast here, because a LookupInterface is not an IFAObject
		LookupInterface *li = (LookupInterface *) fLookupStack["Stack"][index].AsIFAObject(0);
		return li;
	}
	Trace("<" << NotNull(key) << "> not found");
	return 0;
}

long Context::FindIndex(const Anything &anyStack, const char *key, long lStartIdx) const {
	StartTrace1(Context.FindIndex, "key:<" << NotNull(key) << ">");

	long result = -1;

	if (key) {
		long sz = anyStack["Keys"].GetSize();
		if (lStartIdx < 0 || lStartIdx > sz) {
			lStartIdx = sz;
		}
		for (long i = lStartIdx; --i >= 0;) {
			// if ( anyStack["Keys"][i].AsString().IsEqual(key) )
			// another unnice workaround to find some microseconds...
			if (strcmp(anyStack["Keys"][i].AsCharPtr(), key) == 0) {
				result = i;
				break;
			}
		}
	}
	return result;
}

long Context::Remove(const char *key) {
	StartTrace(Context.Remove);
	if (!key) {
		return -1;
	}
	TraceAny(fLookupStack, "fLookupStack and size before:" << fStackSz);

	long index = FindIndex(fLookupStack, key);
	if (index >= 0) {
		fLookupStack["Stack"].Remove(index);
		fLookupStack["Keys"].Remove(index);
		--fStackSz;
	}
	TraceAny(fLookupStack, "fLookupStack and size after:" << fStackSz);
	return index;
}

void Context::Replace(const char *key, LookupInterface *li) {
	StartTrace(Context.Replace);
	if (!key || !li) {
		return;
	}
	TraceAny(fLookupStack, "fLookupStack and size before:" << fStackSz);

	long index = FindIndex(fLookupStack, key);
	if (index >= 0) {
		fLookupStack["Stack"][index] = (IFAObject *) li;
	} else {
		Push(key, li);
	}
	TraceAny(fLookupStack, "fLookupStack and size after:" << fStackSz);
}

bool Context::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
	StartTrace1(Context.DoLookup, "key:<" << NotNull(key) << ">");

	if (LookupStack(key, result, delim, indexdelim) || LookupStores(key, result, delim, indexdelim) || LookupLocalized(key, result, delim,
			indexdelim) || LookupObjects(key, result, delim, indexdelim) || LookupRequest(key, result, delim, indexdelim)) {
		Trace("found");
		return true;
	}
	Trace("failed");
	return false;
}

bool Context::LookupStack(const char *key, ROAnything &result, char delim, char indexdelim) const {
	StartTrace1(Context.LookupStack, "key:<" << NotNull(key) << ">");

	TraceAny(fStore, "fStore and size:" << fStoreSz);
	for (long i = ((ROAnything) fStore)["Stack"].GetSize(); --i >= 0;) {
		if (fStore["Stack"][i].GetType() == AnyObjectType) {
			LookupInterface *li = (LookupInterface *) fStore["Stack"][i].AsIFAObject(0);
			if (li && li->Lookup(key, result, delim, indexdelim)) {
				TraceAny(result, "found through LookupInterface at " << fStore["Keys"][i].AsString() << ':' << i << '.' << key );
				return true;
			}
		} else {
			if (((ROAnything) fStore)["Stack"][i].LookupPath(result, key, delim, indexdelim)) {
				TraceAny(result, "found at " << fStore["Keys"][i].AsString() << ':' << i << '.' << key );
				return true;
			}
		}
	}
	return false;
}

bool Context::LookupStores(const char *key, ROAnything &result, char delim, char indexdelim) const {
	StartTrace1(Context.LookupStores, "key:<" << NotNull(key) << ">");
	if (fCopySessionStore) {
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

bool Context::LookupObjects(const char *key, ROAnything &result, char delim, char indexdelim) const {
	StartTrace1(Context.LookupObjects, "key:<" << NotNull(key) << ">");
	TraceAny(fLookupStack, "fLookupStack and size:" << fStackSz);
	for (long i = ((ROAnything) fLookupStack)["Stack"].GetSize(); --i >= 0;) {
		if (fLookupStack["Stack"][i].GetType() == AnyObjectType) {
			Trace("checking LookupInterface (&" << (long)fLookupStack["Stack"][i].AsIFAObject(0) << ") at " <<
					fLookupStack["Keys"][i].AsString() << ':' << i);
			LookupInterface *li = (LookupInterface *) fLookupStack["Stack"][i].AsIFAObject(0);
			if (li->Lookup(key, result, delim, indexdelim)) {
				Trace("value found");
				return true;
			}
			Trace("value not found");
		}
	}
	return false;
}

bool Context::LookupRequest(const char *key, ROAnything &result, char delim, char indexdelim) const {
	bool bRet(false);
	if (!(bRet = ROAnything(fRequest)["env"].LookupPath(result, key, delim, indexdelim))) {
		if (!(bRet = ROAnything(fRequest)["query"].LookupPath(result, key, delim, indexdelim))) {
			bRet = ROAnything(fRequest).LookupPath(result, key, delim, indexdelim);
		}
	} StatTrace(Context.LookupRequest, "key:<" << NotNull(key) << "> " << (bRet ? "" : "not ") << "found", coast::storage::Current());
	return bRet;
}

bool Context::LookupLocalized(const char *key, ROAnything &result, char delim, char indexdelim) const {
	StartTrace1(Context.LookupLocalized, "key:<" << NotNull(key) << ">");
	LocalizedStrings *ls = LocalizedStrings::LocStr();
	if (ls && ls->Lookup(key, result, delim, indexdelim)) {
		Trace(key << " found in LocalizedStrings");
		return true;
	}
	Trace("failed");
	return false;
}

Anything &Context::GetRequest() {
	return fRequest;
}

Socket *Context::GetSocket() {
	return fSocket;
}

std::iostream *Context::GetStream() {
	return fSocket ? fSocket->GetStream() : 0;
}

long Context::GetReadCount() {
	return (fSocket) ? fSocket->GetReadCount() : 0;
}

long Context::GetWriteCount() {
	return (fSocket) ? fSocket->GetWriteCount() : 0;
}

bool Context::Process(String &token) {
	return Action::ExecAction(token, *this, Lookup(token));
}

bool Context::UnlockSession() {
	if (fSession && fCopySessionStore && fSession->IsLockedByMe()) {
		fSession->fMutex.Unlock();
		return true;
	}
	return false;
}

void Context::LockSession() {
	if (fSession && fCopySessionStore) {
		fSession->fMutex.Lock();
	}
}
