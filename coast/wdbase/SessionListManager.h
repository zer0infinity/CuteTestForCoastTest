/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SessionListManager_H
#define _SessionListManager_H

#include "WDModule.h"
#include "DiffTimer.h"
#include "Threads.h"

//---- forward declaration -----------------------------------------------
class Session;
class Context;
class PeriodicAction;
class URLFilter;

//---- SessionFactoriesModule -----------------------------------------------------------
class SessionFactoriesModule : public WDModule
{
public:
	SessionFactoriesModule(const char *name);
	virtual ~SessionFactoriesModule();

	virtual bool Init(const ROAnything config);
	virtual bool ResetFinis(const ROAnything );
	virtual bool Finis();
};

//---- SessionFactory -----------------------------------------------------------
//!factory for session creation
class SessionFactory : public HierarchConfNamed
{
public:
	SessionFactory(const char *SessionFactoryName);
	virtual ~SessionFactory();

	//! registry interface
	RegCacheDef(SessionFactory);	// FindSessionFactory()
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) SessionFactory(fName);
	}
	//!allocation protocol for session allocation
	virtual Session *DoMakeSession(Context &ctx);
	//!custom steps to prepare a session
	virtual Session *DoPrepareSession(Context &ctx, Session *session, bool &isBusy);

protected:
	//!loads configuration as entry from main config
	bool DoLoadConfig(const char *category);
	//!sets config name to Config -> reads configuration from Config.any
	bool DoGetConfigName(const char *category, const char *objName, String &configFileName) const;

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	SessionFactory();
	SessionFactory(const SessionFactory &);
	SessionFactory &operator=(const SessionFactory &);
};

#define RegisterSessionFactory(name) RegisterObject(name, SessionFactory)

//---- SessionListManager ----------------------------------------------------------
//! manages list of sessions
/*! this is the one and only manager of the session list; it is a singleton\n
\b creation: sessions are created by factory objects; they have a unique session id\n
\b retrieval: sessions are retrieved through the session key\n
\b deletion: sessions can be actively disabled (but not deleted);\n
Sessions will be deleted by a SessionCleanerThread running periodically. */
class SessionListManager: public WDModule
{
public:
	//!it exists only one since it is a not cloned
	SessionListManager(const char *name);
	//!does nothing since everything should be done in Finis
	virtual ~SessionListManager();

	static SessionListManager *SLM();
	static void SetFinalize(bool finalize);

	//--- module initialization termination ---
	//!initialize SessionList during initialization phase
	virtual bool Init(const ROAnything config);
	//!finalize SessionList (delete all Sessions) during termination phase
	virtual bool Finis();
	//!terminate SessionListManager for reset
	virtual bool ResetFinis(const ROAnything );
	//!reinitializes max values and restarts session cleaner thread
	virtual bool ResetInit(const ROAnything config);

	//!create a session if possible; try to cleanup if fMaxSessionsAllowed is reached
	virtual Session *CreateSession(String &id, Context &ctx);
	//!find a session using a sessionId as key defined in args; the args list must first be prepared by FilterQuery
	virtual Session *LookupSession(const String &id, Context &ctx);
	//!prepares the session by checking its state and verifying it; tries to create a new session if not verified
	virtual Session *PrepareSession(Session *, bool &isBusy, Context &ctx);	// prepare or create a session
	//!disable a session using a sessionId as key; removes it from the active sessions list but does not delete it
	virtual void DisableSession(const String &id, Context &ctx);
	//! clean up sessions that have had a timeout; locks fSessionsMutex
	virtual long CleanupSessions(Context &ctx, bool forceLock = false);
	//! Retrieves information about the session list and stores the information in slot \b SessionInfo in the TmpStore, locks fSessionsMutex
	/*! \param sessionListInfo the resulting anything containing all session information grouped by session id
		\param ctx Context used
		\param config Configuration used when only a subset of all available Sessions should get listed, valid names are \c Start and \c PageSize
		\return true if lock could be obtained
		\return false otherwise */
	virtual bool SessionListInfo(Anything &sessionListInfo, Context &ctx, const ROAnything &config);
	//! retrieves information about a specific session; locks fSessionsMutex
	virtual bool GetASessionsInfo(Anything &sessionInfo, const String &sessionId, Context &ctx, const ROAnything &config);

	//!support reinit by locking out session cleaner and other session list activity
	virtual void EnterReInit();
	//!support reinit
	virtual void LeaveReInit();

	//! filters query and looks up sessionId
	virtual String FilterQueryAndGetId(Context &ctx);

	//!ID that's unique across multiple hosts and/or processes.
	//!May depend on IP address (through gethostid()).
	virtual	String GetUniqueInstanceId();

protected:
	friend class SessionListManagerTest;

	//--- creation and intialization ---
	//!tries to create a new session if possible
	virtual Session *MakeSession(Context &ctx);
	//! hook for subclasses if they want to create their own sessions; preferred way use factories
	virtual Session *DoMakeSession(Context &ctx);
	//!initialize the newly created session
	virtual String &InitSession(String &id, Session *session, Context &ctx);
	//!store the session in the fSessions list with the key id
	virtual void AddSession(const String &id, Session *session, Context &ctx);

	//--- url filtering ---
	//! Filters and unscrambles query content by using an URLFilter object
	/*! This method tries to find a registered URLFilter by looking up its name in the Context at \b URLFilterName. If no name is set,
	 * the default URLFilter is used.
	 * The URLFilter configuration must be defined in the Context as it gets looked up using the name \b URLFilterSpec
	 * Check @ref URLFilterHandleQueryConfiguration to see how to configure.
	 *
	 * Default filtering applies for the tags if they appear in the unscrambled part:
	 * \li \c sessionId
	 * \li \c role
	 * \li \c page
	 *
	 * \c action is also filtered if it is a rolechange action
	 *
	 * Default unscrambling applies for the tags X (when not using baseurls) or X1 and X2 (when using baseurls)
	 *
	 * @param ctx Context containing the request e.g. query and environment as well as all configurable objects
	 * @return true if URLFilter could be found and processing was possible
	 * @return false otherwise */
	virtual bool FilterQuery(Context &ctx);

	//! generate standard filter to be backward compatible if nothing specified
	virtual Anything StdFilterTags(Context &ctx);

	//! find url filter defined in context
	virtual URLFilter *FindURLFilter(Context &ctx);

	//--- retrieval -----
	//! retrieves a session object from the sessions list
	//!accesses the fSessions list of sessions; therefore it locks the fSessionsListMutex
	//! it returns the session found if it is not terminated or busy
	//! \param sessionId the session id
	//! \param ctx the request context
	virtual Session *IntLookupSession(const String &sessionId, Context &ctx);
	//!check if session is in use i.e. the sessions mutex is locked
	//! \param ctx the request context
	//! \param session actual session in this request
	//! \param isBusy out param session is locked if true
	//! \return returns true if busy
	virtual bool SessionIsBusy(Session *session, bool &isBusy, Context &ctx);
	//!verifies the session
	//! \param ctx the request context
	//! \param session actual session in this request
	//! \param sessionId sessionId in this request, is reset if verification fails
	//! \return returns true if verification is ok; else false
	bool VerifySession(Session *session, String &sessionId, Context &ctx);
	//!generation of unique id's (usually for sessions)
	virtual void GetNextId(String &s, Context &ctx);
	//--- deletion
	//!removes sessions from sessionList, that have had a timeout; uses no lock
	virtual Anything DoCleanup(Anything &sessionList, Anything &sessions2Delete, Context &ctx, bool roleNotRelevant = false);
	//!deletes sessions contained in session2Delete; sets no locks since nothing is shared without locks
	virtual void DoDeleteSessions(const Anything &sessions2Delete, Context &ctx);
	//!forces removal of sessions
	virtual void ForcedSessionCleanUp(Context &ctx);

	//--- statistics ---------
	//!returns number of sessions that are activ
	virtual long GetNumberOfSessions();

protected:
	//--- subclass api
	//!singleton cache
	static Mutex fgSessionListManagerMutex;
	static SessionListManager *fgSessionListManager;
	static bool fgFinalize;

	//!cleaner thread that periodically deletes sessions if they have had a timeout
	PeriodicAction *fSessionCleaner;

	//!guard for sessions list a very central lock; be careful using it
	Mutex fSessionsMutex;
	//!list of active sessions in the system
	Anything fSessions;
	//!list of disabled sessions they will be deleted when they timeout
	Anything fDisabledSessions;

	//!guard for generation of unique ids (usually used for session keys)
	Mutex fNextIdMutex;
	//!id of last generated session key
	HRTIME fNextId;

	//!max number of sessions allowed
	long fMaxSessionsAllowed;
	//!pointer to session factory
	SessionFactory *fSessionFactory;
	//!additional loggin of session info to cerr (not only SystemLog)
	long fLogToCerr;
	//!ID that's unique across multiple hosts and/or processes.
	//!May depend on IP address (through gethostid()).
	String fUniqueInstanceId;
};

#endif
