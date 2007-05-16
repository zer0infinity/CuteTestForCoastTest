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
#include "SysLog.h"
#include "Dbg.h"
#include "AnyLookupInterfaceAdapter.h"

class Session;
class Role;
class Page;
class Server;
class Socket;

//---- Context -------------------------------------------------------------------
//!Request Context object that contains read only configuration information and read writeable stores
class EXPORTDECL_WDBASE Context : public virtual LookupInterface
{
public:
	//! empty constructor useful for testcases and to add elements later on
	Context();

	//! Socket * constructor needed by HandleRequest
	Context(Socket *);

	/*! Constructor of Context used for TestCases
		\param ios stream for mocking up socket in tests */
	Context(iostream *ios);

	//! constructor with request anything that contains all information of the request
	Context(Anything &request);

	/*! Push a LookupInterface derived object onto the object stack
		\param key Name of passed object.This name does not have to be unique, it is only used for information purpose. This means a Push with the same name does not overwrite an already existing stack element with the same name.
		\return true when key is not null and li is not null
		\param li pointer to a LookupInterface derived object */
	bool Push(const char *key, LookupInterface *li);

	/*! Push the session object and initialize appropriate stores
		\param s the session object */
	void Push(Session *s);

	/*! Push the Request object
		\param s the Request anything */
	void PushRequest(const Anything &request);

	/*! Pop the top pushed stack element.
		\return true if not empty; otherwise false
		\note Do only Pop elements pushed with Push() interfaces here. Otherways the name history of Push elements will get screwed up! */
	bool Pop(String &key);

	/*! Convenience inner class to manage Push and PopStore calls. Less error prone than doing it manually. */
	template < class ContainerType >
	class PushPopEntry
	{
	public:
		typedef AnyLookupInterfaceAdapter<ContainerType> LookupAdapterType;
		typedef const ContainerType &ConstContainerTypeRef;

		/* Constructor of PushPopEntry. An object of this class can be used where some content must be temporarily pushed onto the Contexts lookup stack. The current implementation uses AnyLookupInterfaceAdapter to wrap the given [RO]Anything.
			\param ctx Context to operate on
			\param pcStoreName Name used to identify the pushed store for informational purposes. The name is not used as a unique key so duplicates are allowed.
			\param store [RO]Anything to use as underlying data container
			\param pcBaseKey optional param which specifies the segment used to emulate nested content in a Lookup. If the lookup-key starts with this name we cut it away before doing a concrete lookup.*/
		PushPopEntry(Context &ctx, const char *pcStoreName, ConstContainerTypeRef store, const char *pcBaseKey = NULL)
			: fCtx(ctx)
			, fStoreName(pcStoreName)
			, fbPushSuccess(false)
			, fAdapter(store, pcBaseKey)
			, fpLookupInterface(&fAdapter) {
			fbPushSuccess = fCtx.Push(fStoreName, fpLookupInterface);
		}

		~PushPopEntry() {
			if ( fbPushSuccess ) {
				String strPoppedStore;
				bool bPopCode =  fCtx.PopStore(strPoppedStore);
				if ( !bPopCode ) {
					SYSWARNING("tried to pop Store when no more Store to pop available");
				} else if ( fStoreName != strPoppedStore ) {
					SYSWARNING("names of pushed [" << fStoreName << "] and popped [" << strPoppedStore << "] Stores were not identical!");
				}
			}
		}
	private:
		Context &fCtx;
		String fStoreName;
		bool fbPushSuccess;
		LookupAdapterType fAdapter;
		LookupInterface *fpLookupInterface;
	};

	/*! Replace the first registerable object named by <I>key</I> with <I>li</I>. If there is no object named by key in the Context li is pushed with key.
		\param key only a simple key can match
		\param li the new object */
	void Replace(const char *key, LookupInterface *li);

	/*! Remove the first registerable object named by <I>key</I>. If there is no object named by key in the Context it is removed
		\param key only a simple key can match
		\return returns the index of the match or -1 */
	long Remove(const char *key);

	/*! Access Session object if set
		\return returns Session if set; otherwise null */
	Session *GetSession() const;

	/*! Access to session id if set
		\return returns session id string if set; otherwise null */
	const char *GetSessionId() const;

	/*! Access to the request object of this request
		\return returns Request any object if set; otherwise fEmpty any */
	Anything &GetRequest();

	/*! Access to socket object
		\return Socket object if set; otherwise null */
	Socket *GetSocket();

	/*! Access to iostream based on socket or fMockStream for tests
		\return iostream pointer if available or fMockStream */
	iostream *GetStream();

	/*! Get read count on internal socket if any
		\return byte-length of last read reply of socket; otherwise zero */
	long GetReadCount();

	/*! Get write count on internal socket if any
		\return byte-length of last written request on socket; otherwise zero */
	long GetWriteCount();

	/*! access the stores by name
		\param key the path to the store
		\param result the anything representing the store in case of success
		\return true if the store exists, false otherwise */
	bool GetStore(const char *key, Anything &result);

	//! process action token
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

	/*! Access the role store of the session; caution reference semantics will not hold when using thread local store
		\note The Anything returned by GetRoleStoreGlobal() is one of the few exceptions
		where the usual reference semantics of Anything arrays may not apply!
		Reason: The result is allocated within a global memory allocator rather than the
		pool allocator that is usually used when processing requests.
		Be aware that mixing differently allocated Anythings will always lead to the
		automatic copying into the allocator of the respective target Anything.
		Do use a globally allocated Anything to store the result of this method, e.g.
		"TrickyThing roleStore(GetRoleStoreGlobal());". */
	Anything &GetRoleStoreGlobal();

	/*! Access the session store; caution reference semantics will not hold when using thread local store
		\note The Anything returned by GetRoleStoreGlobal() is one of the few exceptions
		where the usual reference semantics of Anything arrays may not apply!
		Reason: The result is allocated within a global memory allocator rather than the
		pool allocator that is usually used when processing requests.
		Be aware that mixing differently allocated Anythings will always lead to the
		automatic copying into the allocator of the respective target Anything.
		Do use a globally allocated Anything to store the result of this method, e.g.
		"TrickyThing roleStore(GetRoleStoreGlobal());". */
	Anything &GetSessionStore();

	//!assemble state into a which will be used in link
	void CollectLinkState(Anything &a);

	//! contains the language set
	const char *Language() const	{
		return fLanguage;
	}

	/*! Aets the language key values can be D E F I etc
		\param lang value of language key */
	void SetLanguage(const char *lang) 	{
		fLanguage = lang;
	}

	//! returns the user id if any of this request
	const char *User() const	{
		return fUser;
	}

	/*! Sets userid for this request
		\param user an identifying string for the user of this request */
	void SetUser(const char *user) 	{
		fUser = user;
	}

	/*! Generate debug output onto reply;
		\param msg the msg to mark this output
		\param reply the stream to generate output to
		\param printAny flag if true Anythings containing the store and request are dumped */
	void DebugStores(const char *msg, ostream &reply, bool printAny = false);

	//!generates output into the reply stream as HTML if the flag in Dbg.any are set accordingly
	void HTMLDebugStores(ostream &reply);

	static const String DebugStoreSeparator;

protected:
	/*! the central hook for looking up read only context information;  policy is stores before configuration before request information (query and environment)
		\param key the key we are looking for
		\param result the ROAnything that provides the result
		\param delim the delimiter for structured keys
		\return returns true if key is found otherwise false */
	bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;

	/*! Lookup information in the contexts store stack.
		\param key the key we are looking for
		\param result the ROAnything that provides the result
		\param delim the delimiter for structured keys
		\return returns true if key is found otherwise false
		\note This method does not include the tmp store while processing the store stack! */
	bool LookupStack(const char *key, ROAnything &result, char delim, char indexdelim) const;

	/*! Lookup information in the requests temporary and session stores but not in the store stack.
		\param key the key we are looking for
		\param result the ROAnything that provides the result
		\param delim the delimiter for structured keys
		\return returns true if key is found otherwise false */
	bool LookupStores(const char *key, ROAnything &result, char delim, char indexdelim) const;

	/*! Lookup information from the contexts lookup interface objects
		\param key the key we are looking for
		\param result the ROAnything that provides the result
		\param delim the delimiter for structured keys
		\return returns true if key is found otherwise false */
	bool LookupObjects(const char *key, ROAnything &result, char delim, char indexdelim) const;

	/*! Lookup information from the contexts request
		\param key the key we are looking for
		\param result the ROAnything that provides the result
		\param delim the delimiter for structured keys
		\return returns true if key is found otherwise false */
	bool LookupRequest(const char *key, ROAnything &result, char delim, char indexdelim) const;

	/*! Lookup information from the localized strings file
		\param key the key we are looking for
		\param result the ROAnything that provides the result
		\param delim the delimiter for structured keys
		\return returns true if key is found otherwise false */
	bool LookupLocalized(const char *key, ROAnything &result, char delim, char indexdelim) const;

	/*! factor out initialization of tmp store */
	void InitTmpStore();

	/*! factor out initialization of session and sessionstore
		\param s session to push */
	void InitSession(Session *s);

	/*! Push the given Anything on top of the lookup stack. This is a very convenient way to pass arguments to subsequent context based operations.
		\param key Name of passed object. This name does not have to be unique, it is only used for information purpose. This means a Push with the same name does not overwrite an already existing stack element with the same name.
		\param store An Anything containing specific params/infos.
		\return true when key is not null, and store was not a Null-Any
		\note Do not forget to pop the pushed Anything after operation! You should use a PushPopEntry whenever possible instead. */
	bool Push(const char *key, Anything &store);

	/*! Pop the previously pushed store from the lookup stack.
		\param key Name of the popped element. Can be used to check if the correct element was popped off the stack.
		\return true if a store could be popped
		\note The internal tmp store, stack element 0, can not be popped and remains always on the stack! */
	bool PopStore(String &key);

	/*! find a configurable object by name
		\param key Name of the object when it was pushed */
	LookupInterface *Find(const char *key) const;

	/*! finds the index of the confnamedobject by key
		\param anyStack internal stack to search, either fStore or fLookupStack
		\param key name of the object to search
		\param lStartIdx where to start the search at, useful when doing subsequent calls
		\return index >= 0 if the object was found, -1 otherwise */
	long FindIndex(const Anything &anyStack, const char *key, long lStartIdx = -1L) const;

	/*! access a stacked store by name
		\param key the path to the store
		\param result the anything representing the store in case of success
		\param bFullStore set to true if the whole stack should be returned, otherwise the top element will be returned only
		\return true if the store exists, false otherwise */
	Anything &IntGetStore(const char *key, long &index);

	/*! relese session lock, if configured to do so
		\return true, if we really held the session lock */
	bool UnlockSession();

	/*! reacquire session lock if configuration told me to release it */
	void LockSession();

	//! the session of this context
	Session *fSession;

	//! the reference to the session store (Storage::Global and Storage::Current)
	Anything fSessionStoreGlobal;
	Anything fSessionStoreCurrent;

	//! cached language setting
	String fLanguage;

	//! cached user setting
	String fUser;

	//! the stack containing the lookup interfaces
	Anything fLookupStack;

	//! cache of the stack size
	long fStackSz;

	//! the contexts store
	MetaThing fStore;

	//! the empty return value
	Anything fEmpty;

	//! cache of the store size
	long fStoreSz;

	//! the request read
	Anything fRequest;

	//! the requests socket if any
	Socket *fSocket;

	//! mock of fSocket stream
	iostream *fMockStream;

	/*! leave session locked or unlock it - in that case, U are responsible what U do with the Sessionstore! */
	bool fUnlockSession;

	/*! if set, make a copy of the session store. This allows concurrent requests using the same session because lookups targeting the session store don't need a lock. */
	bool fCopySessionStore;

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
	friend class LoopbackProcessor;
	friend class HttpFlowController;
	friend class TestSequence;
	// gcc <= 2.95.[34] needs an explicit friend declaration
	template< class ContainerType > friend class PushPopEntry;
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
