/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "config_wdbase.h"
#include "LookupInterface.h"
#include "Anything.h"

class Session;
class Role;
class Page;
class Server;
class ConfNamedObject;
class Request;
class Socket;

//---- Context -------------------------------------------------------------------
//!Request Context object that contains read only configuration information and read writeable stores
class EXPORTDECL_WDBASE Context : public virtual LookupInterface
{
public:
	//!empty constructor useful for testcases and to add elements later on
	Context();

	//!Socket * constructor needed by HandleRequest
	Context(Socket *);

	//!stream for mocking up socket in tests
	Context(iostream *ios);

	//!constructor with request anything that contains all information of the request
	Context(Anything &request);

	//!push a new lookupinterface onto the composite stack of lookup interfaces
	//! \param li the new top lookupinterface
	void Push(LookupInterface *li);

	//!push a registerable object by name; so you can reference it by name
	//! \param key the name of the object eg. "Page"
	//! \param ro the object
	void Push(const char *key, LookupInterface *ro);

	//!push the session object
	//! \param s the session object
	void Push(Session *s);

	//!push the Request object
	//! \param s the Request anything
	void PushRequest(const Anything &request);

	//!push a store object
	//! \param s the Request anything
	void PushStore(const char *key, Anything &store);

	//!pop a store -> remove it completly
	//! \param key fills in the stores key if stack is not empty
	//! \return returns true if found; otherwise false
	bool PopStore(String &key);

	//!pop the top interface
	//! \return returns true if not empty; otherwise false
	bool Pop();

	//!find a configurable object by name; always use SafeCast Macro (dynamic_cast if available)
	//! \param key the key only a simple key can match
	LookupInterface *Find(const char *key) const;

	//!replace the first registerable object named by <I>key</I> with <I>ro</I>
	//! \param key only a simple key can match
	//! \param ro the new object
	//! If there is no object named by key in the Context ro is pushed with key.
	void Replace(const char *key, LookupInterface *ro);

	//!remove the first registerable object named by <I>key</I>
	//! \param key only a simple key can match
	//! \return returns the index of the match or -1
	//! If there is no object named by key in the Context it is removed
	long Remove(const char *key);

	//!access Session object if set
	//! \return returns Session if set; otherwise null
	Session *GetSession() const;

	//!access to session id if set
	//! \return returns session id string if set; otherwise null
	const char *GetSessionId() const;

	//!access to the request object of this request
	//! \return returns Request any object if set; otherwise null
	Anything &GetRequest();

	//!access to socket object; returns null if not there
	Socket *GetSocket();

	//!access to iostream based on socket
	iostream *GetStream();

	//!returns byte-length of last read reply of socket, if no socket active result is zero
	long GetReadCount();
	//!returns byte-length of last written request on socket, if no socket active result is zero
	long GetWriteCount();

	//!access the stores by name
	//! \param key the path to the store
	//! \param result the anything representing the store
	//! \return bool if the store exists else false
	bool GetStore(const char *key, Anything &result);

	//!process action token
	bool Process(String &token);

//--- legacy api starts here ---//
#ifdef DEPRECATED_CTX
#endif
	//!legacy constructor with env and query anything and some, but not all important objects
	Context(const Anything &env, const Anything &query, Server *srv, Session *s, Role *g, Page *p = 0);
	~Context();

	//!setting server in context
	void SetServer(Server *);
	//!access the server
	Server *GetServer() const;

	//!set the role
	void SetRole(Role *g);
	//!access the role
	Role *GetRole() const;

	//!set the page
	void SetPage(Page *p);
	//!access the page
	Page *GetPage() const;

	//!set the query anything
	void SetQuery(const Anything &query);
	//!access the query anything; it contains the query arguments after parsing the uri string
	Anything &GetQuery();

	//!access the env store which contains information about the request
	Anything &GetEnvStore();

	//!access the temporary request store
	Anything &GetTmpStore();

	//!access the role store of the session; caution reference semantics will not hold when using thread local store
	//! <b>CAUTION</b>
	//! The Anything returned by GetRoleStoreGlobal() is one of the few exceptions
	//! where the usual reference semantics of Anything arrays may not apply!
	//! Reason: The result is allocated within a global memory allocator rather than the
	//! pool allocator that is usually used when processing requests.
	//! Be aware that mixing differently allocated Anythings will always lead to the
	//! automatic copying into the allocator of the respective target Anything.
	//! Do use a globally allocated Anything to store the result of this method, e.g.
	//! "TrickyThing roleStore(GetRoleStoreGlobal());".
	Anything &GetRoleStoreGlobal();

	//!access the session store; caution reference semantics will not hold when using thread local store
	//! <b>CAUTION</b>
	//! The Anything returned by GetRoleStoreGlobal() is one of the few exceptions
	//! where the usual reference semantics of Anything arrays may not apply!
	//! Reason: The result is allocated within a global memory allocator rather than the
	//! pool allocator that is usually used when processing requests.
	//! Be aware that mixing differently allocated Anythings will always lead to the
	//! automatic copying into the allocator of the respective target Anything.
	//! Do use a globally allocated Anything to store the result of this method, e.g.
	//! "TrickyThing roleStore(GetRoleStoreGlobal());".
	Anything &GetSessionStore();

	//!assemble state into a which will be used in link
	void CollectLinkState(Anything &a);

	//! contains the language set
	const char *Language() const	{
		return fLanguage;
	}

	//! sets the language key values can be D E F I etc
	//! \param lang value of language key
	void SetLanguage(const char *lang) 	{
		fLanguage = lang;
	}

	//! returns the user id if any of this request
	const char *User() const	{
		return fUser;
	}

	//! sets userid for this request
	//! \param user an identifying string for the user of this request
	void SetUser(const char *user) 	{
		fUser = user;
	}

	//!generate debug output onto reply;
	//! \param msg the msg to mark this output
	//! \param reply the stream to generate output to
	//! \param printAny flag if true Anythings containing the store and request are dumped
	void DebugStores(const char *msg, ostream &reply, bool printAny = false);

	//!generates output into the reply stream as HTML if the flag in Dbg.any are set accordingly
	void HTMLDebugStores(ostream &reply);

	static const String DebugStoreSeparator;

protected:
	//!the central hook for looking up read only context information;  policy is stores before configuration before request information (query and environment)
	//! \param key the key we are looking for
	//! \param result the ROAnything that provides the result
	//! \param delim the delimiter for structured keys
	//! \return returns true if key is found otherwise false
	bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;

	//!lookup read only information in the requests temporary and session stores
	//! \param key the key we are looking for
	//! \param result the ROAnything that provides the result
	//! \param delim the delimiter for structured keys
	//! \return returns true if key is found otherwise false
	bool LookupStores(const char *key, ROAnything &result, char delim, char indexdelim) const;

	//!lookup read only information from the contexts lookup interface objects
	//! \param key the key we are looking for
	//! \param result the ROAnything that provides the result
	//! \param delim the delimiter for structured keys
	//! \return returns true if key is found otherwise false
	bool LookupObjects(const char *key, ROAnything &result, char delim, char indexdelim) const;

	//!lookup read only information from the contexts request
	//! \param key the key we are looking for
	//! \param result the ROAnything that provides the result
	//! \param delim the delimiter for structured keys
	//! \return returns true if key is found otherwise false
	bool LookupRequest(const char *key, ROAnything &result, char delim, char indexdelim) const;

	//!lookup read only information from the localized strings file
	//! \param key the key we are looking for
	//! \param result the ROAnything that provides the result
	//! \param delim the delimiter for structured keys
	//! \return returns true if key is found otherwise false
	bool LookupLocalized(const char *key, ROAnything &result, char delim, char indexdelim) const;

	//!factor out initialization of tmp store
	void InitTmpStore();

	//!factor out initialization of session and sessionstore
	//! \param s session
	void InitSession(Session *s);

	//!access builtin stores directly by simple key to support legacy api
	//! \param key a simple key, no subpart resolution is made
	//! \return returns the top element of the store stack with key
	Anything &GetBuiltinStore(const char *key);

	//!factors out push functionality
	//! \param key storing object by name
	//! \param li the object providing configuration information
	void DoPush(const char *key, LookupInterface *li);

	//!finds the index of the confnamedobject by key
	//! \param key name of the object to search
	//! \return index >= 0 if the object was found, -1 otherwise
	long FindIndex(const char *key) const;

	//!insures that a pushed store is always an array
	void InsureArrayStore(Anything &store);

	//!relese session lock, if configured to do so
	//!returns true, if we really held the session lock
	bool UnlockSession();
	//!reacquire session lock if configuration told me to release it
	void LockSession();

	//! the session of this context
	Session *fSession;

	//! the reference to the session store
	Anything fSessionStore;

	//! cached language setting
	String fLanguage;

	//! cached user setting
	String fUser;

	//!the stack containing the lookup interfaces
	Anything fLookupStack;

	//!cache of the stack size
	long fStackSz;

	//!information about the store stack
	Anything fStoreHistory;

	//!the contexts store
	MetaThing fStore;

	//!the empty return value
	Anything fEmpty;
	//!cache of the store size
	long fStoreSz;

	//!the request read
	Anything fRequest;

	//!the requests socket if any
	Socket *fSocket;

	//!mock of fSocket stream
	iostream *fMockStream;

	//!leave session locked or unlock it (in that case, U are responsible
	//! what U do with the Sessionstore.
	bool fUnlockSession;

private:
	Context(const Context &);
	Context &operator=(const Context &);
	// due to its changed semantics GetRoleStore() has been
	// replaced by GetRoleStoreGlobal() .. read the associated comments carefully
	Anything &GetRoleStore() {
		return GetRoleStoreGlobal();
	}
	friend class ContextTest;
	friend class SessionReleaser;
};
class EXPORTDECL_WDBASE SessionReleaser
{
public:
	SessionReleaser(Context &c): fContext(c) {
		fWasLockedByMe = fContext.UnlockSession();
	}
	~SessionReleaser() {
		if (fWasLockedByMe) {
			fContext.LockSession();
		}
	}
	void Use() const {
		/* dummy */
	}
private:
	Context &fContext;
	bool	fWasLockedByMe;

	SessionReleaser();
	SessionReleaser(const SessionReleaser &);
	SessionReleaser operator=(const SessionReleaser &);
};

#endif
