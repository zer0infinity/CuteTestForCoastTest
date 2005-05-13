/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTnewDAImpl.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "SybCTnewDA.h"
#include "Timers.h"
#include "TimeStamp.h"
#include "StringStream.h"
#include "Action.h"
#include "PeriodicAction.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

Mutex SybCTnewDAImpl::fgStructureMutex("StructureMutex", Storage::Global());
Anything SybCTnewDAImpl::fgListOfSybCT;
Anything SybCTnewDAImpl::fgContextMessages;
CS_CONTEXT *SybCTnewDAImpl::fg_cs_context;
bool SybCTnewDAImpl::fgInitialized = false;
PeriodicAction *SybCTnewDAImpl::fgpPeriodicAction = NULL;
Semaphore *SybCTnewDAImpl::fgpResourcesSema = NULL;

//---- SybCTnewDAImpl ----------------------------------------------------------------
RegisterDataAccessImpl(SybCTnewDAImpl);

SybCTnewDAImpl::SybCTnewDAImpl(const char *name)
	: DataAccessImpl(name)
{
	StartTrace(SybCTnewDAImpl.SybCTnewDAImpl);
}

SybCTnewDAImpl::~SybCTnewDAImpl()
{
	StartTrace(SybCTnewDAImpl.~SybCTnewDAImpl);
}

IFAObject *SybCTnewDAImpl::Clone() const
{
	StartTrace(SybCTnewDAImpl.Clone);
	return new SybCTnewDAImpl(fName);
}

bool SybCTnewDAImpl::Init(ROAnything config)
{
	StartTrace(SybCTnewDAImpl.Init);
	if ( !fgInitialized ) {
		ROAnything myCfg;
		String strInterfacesPathName;
		long nrOfSybCTs = 5L;
		long lCloseConnectionTimeout = 60L;
		// check if the number of SybCTs is specified in Config
		if (config.LookupPath(myCfg, "SybaseModule.SybCTnewDAImpl")) {
			nrOfSybCTs = myCfg["ParallelQueries"].AsLong(nrOfSybCTs);
			lCloseConnectionTimeout = myCfg["CloseConnectionTimeout"].AsLong(lCloseConnectionTimeout);
			strInterfacesPathName = myCfg["InterfacesPathName"].AsString();
		}

		MutexEntry me(fgStructureMutex);
		me.Use();
		fgContextMessages.SetAllocator(Storage::Global());
		fgContextMessages = Anything();
		fgListOfSybCT.SetAllocator(Storage::Global());
		fgListOfSybCT = Anything();
		// SybCTnewDA::Init initializes the cs_context.  It must be done only once
		if ( SybCTnewDA::Init(&fg_cs_context, &fgContextMessages, strInterfacesPathName, nrOfSybCTs) == CS_SUCCEED ) {
			// use the semaphore to block when no more resources are available
			fgListOfSybCT["Size"] = nrOfSybCTs;
			fgpResourcesSema = new Semaphore(nrOfSybCTs);
			String server, user;
			for ( long i = 0; i < nrOfSybCTs; i++ ) {
				SybCTnewDA *pCT = new SybCTnewDA(fg_cs_context);
				DoPutbackConnection(pCT, false, server, user);
			}
			if ( !fgpPeriodicAction ) {
				fgpPeriodicAction = new PeriodicAction("SybCheckCloseOpenedConnectionsAction", lCloseConnectionTimeout);
				fgpPeriodicAction->Start();
			}
			fgInitialized = true;
		}
	}
	return fgInitialized;
}

bool SybCTnewDAImpl::Finis()
{
	StartTrace(SybCTnewDAImpl.Finis);
	if ( fgpPeriodicAction ) {
		fgpPeriodicAction->Terminate();
		delete fgpPeriodicAction;
		fgpPeriodicAction = NULL;
	}
	bool bInitialized;
	{
		MutexEntry me(fgStructureMutex);
		me.Use();
		bInitialized = fgInitialized;
		// force pending/upcoming Exec calls to fail
		fgInitialized = false;
	}
	if ( bInitialized ) {
		for (long lCount = 0L; lCount < fgListOfSybCT["Size"].AsLong(0L) && fgpResourcesSema->Acquire(); lCount++) {
			SybCTnewDA *pSyb = NULL;
			bool bIsOpen = false;
			String strServer, strUser;
			if ( DoGetConnection(pSyb, bIsOpen, strServer, strUser) ) {
				if ( bIsOpen ) {
					pSyb->Close(true);
				}
				delete pSyb;
			}
		}
		delete fgpResourcesSema;
		fgpResourcesSema = NULL;
		SybCTnewDA::Finis(fg_cs_context);
		// trace messages which occurred without a connection
		while ( fgContextMessages.GetSize() ) {
			SysLog::Warning(fgContextMessages[0L].AsString());
			fgContextMessages.Remove(0L);
		}
	}
	return !fgInitialized;
}

bool SybCTnewDAImpl::IntGetOpen(SybCTnewDA *&pSyb, bool &bIsOpen, const String &server, const String &user)
{
	StartTrace1(SybCTnewDAImpl.IntGetOpen, "server [" << server << "] user [" << user << "]");
	pSyb = NULL;
	bIsOpen = false;
	Anything anyTimeStamp(Storage::Global()), anyEntry(Storage::Global());
	TraceAny(fgListOfSybCT, "current list of connections");
	if ( fgListOfSybCT.LookupPath(anyTimeStamp, "Open") && anyTimeStamp.GetSize() ) {
		String strToLookup(server);
		if ( strToLookup.Length() && user.Length() ) {
			strToLookup << '.' << user;
		}
		Trace("Lookup name [" << strToLookup << "]");
		for (long lIdx = 0; lIdx < anyTimeStamp.GetSize(); lIdx++) {
			Anything anyTS(Storage::Global());
			anyTS = anyTimeStamp[lIdx];
			for (long lTimeSub = 0L; lTimeSub < anyTS.GetSize(); lTimeSub++) {
				if ( ( strToLookup.Length() <= 0 ) || strToLookup == anyTS[lTimeSub][1L].AsString() ) {
					Trace("removing subentry :" << lIdx << ":" << lTimeSub);
					anyEntry = anyTS[lTimeSub];
					anyTS.Remove(lTimeSub);
					// we do not have to close the connection before using because the SybCTnewDA is for the same server and user
					bIsOpen = ( strToLookup.Length() > 0 );
					break;
				}
			}
			if ( !anyEntry.IsNull() ) {
				pSyb = SafeCast(anyEntry[0L].AsIFAObject(), SybCTnewDA);
				if ( anyTS.GetSize() == 0L ) {
					anyTimeStamp.Remove(lIdx);
				}
				break;
			}
		}
	}
	return !anyEntry.IsNull();
}

bool SybCTnewDAImpl::DoGetConnection(SybCTnewDA *&pSyb, bool &bIsOpen, const String &server, const String &user)
{
	StartTrace(SybCTnewDAImpl.DoGetConnection);
	pSyb = NULL;
	bIsOpen = false;
	MutexEntry me(fgStructureMutex);
	me.Use();
	if ( !server.Length() || !user.Length() || !IntGetOpen(pSyb, bIsOpen, server, user) ) {
		// favour unused connection against open connection of different server/user
		if ( fgListOfSybCT["Unused"].GetSize() ) {
			Trace("removing first unused element");
			pSyb = SafeCast(fgListOfSybCT["Unused"][0L].AsIFAObject(), SybCTnewDA);
			fgListOfSybCT["Unused"].Remove(0L);
		} else {
			String strEmpty;
			if ( IntGetOpen(pSyb, bIsOpen, strEmpty, strEmpty) ) {
				if ( !bIsOpen ) {
					Trace("connection of different server or user");
					// if this call would return false we could possibly delete and recreate an object
					pSyb->Close();
				}
			}
		}
	}
	Trace("returning &" << (long)(IFAObject *)pSyb);
	if ( pSyb == NULL ) {
		SYSERROR("could not get a valid SybCTnewDA");
	}
	return (pSyb != NULL);
}

void SybCTnewDAImpl::DoPutbackConnection(SybCTnewDA *&pSyb, bool bIsOpen, const String &server, const String &user)
{
	StartTrace1(SybCTnewDAImpl.DoPutbackConnection, "putting &" << (long)(IFAObject *)pSyb);
	MutexEntry me(fgStructureMutex);
	me.Use();
	if ( bIsOpen ) {
		String strToStore(server);
		strToStore << '.' << user;
		TimeStamp aStamp;
		Anything anyTimeStamp(Storage::Global());
		if ( !fgListOfSybCT.LookupPath(anyTimeStamp, "Open") ) {
			anyTimeStamp = MetaThing(Storage::Global());
			fgListOfSybCT["Open"] = anyTimeStamp;
		}
		Anything anyToStore(Storage::Global());
		anyToStore[0L] = (IFAObject *)pSyb;
		anyToStore[1L] = strToStore;
		anyTimeStamp[aStamp.AsString()].Append(anyToStore);
	} else {
		fgListOfSybCT["Unused"].Append((IFAObject *)pSyb);
	}
	TraceAny(fgListOfSybCT, "current list of connections");
}

bool SybCTnewDAImpl::Exec( Context &ctx, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(SybCTnewDAImpl.Exec);
	bool bRet = false;
	DAAccessTimer(SybCTnewDAImpl.Exec, fName, ctx);
	// check if we are initialized
	bool bInitialized = false;
	if ( fgInitialized ) {
		MutexEntry me(fgStructureMutex);
		me.Use();
		bInitialized = fgInitialized;
	}
	if ( bInitialized ) {
		SemaphoreEntry aEntry(*fgpResourcesSema);
		// we need the server and user to see if there is an existing and Open SybCTnewDA
		String user, server;
		in->Get( "SybDBUser", user, ctx);
		in->Get( "SybDBHost", server, ctx);
		Trace ("USER IS:" << user );
		Trace ("Host is:" << server );
		out->Put("QuerySource", server, ctx);

		SybCTnewDA *pSyb = NULL;
		bool bIsOpen = false;
		// we slipped through and are ready to get a SybCT and execute a query
		// find a free SybCTnewDA, we should always get a valid SybCTnewDA here!
		if ( DoGetConnection(pSyb, bIsOpen, server, user) ) {
			SybCTnewDA::DaParams daParams(&ctx, in, out, &fName);
			// test if the connection is still valid, eg. we are able to get/set connection params
			if ( bIsOpen ) {
				SybCTnewDA::DaParams outParams;
				if ( pSyb->GetConProps(CS_USERDATA, (CS_VOID **)&outParams, CS_SIZEOF(SybCTnewDA::DaParams)) != CS_SUCCEED ) {
					// try to close and reopen connection
					pSyb->Close();
					bIsOpen = false;
				}
			}
			if ( !bIsOpen ) {
				String passwd, app;
				in->Get( "SybDBPW", passwd, ctx);
				in->Get( "SybDBApp", app, ctx);
				// open new connection
				if ( !( bIsOpen = pSyb->Open( daParams, user, passwd, server, app) ) ) {
					SYSWARNING("Could not open connection to server [" << server << "] with user [" << user << "]");
				}
			}
			if ( bIsOpen ) {
				String command;
				if ( DoPrepareSQL(command, ctx, in) ) {
					String resultformat, resultsize;
					in->Get( "SybDBResultFormat", resultformat, ctx);
					in->Get( "SybDBMaxResultSize", resultsize, ctx);
					if ( !(bRet = pSyb->SqlExec( daParams, command, resultformat, resultsize.AsLong(0L) ) ) ) {
						SYSWARNING("could not execute the sql command or it was aborted");
						//					// maybe a close is better here to reduce the risk of further failures
						//					pSyb->Close();
					}
				} else {
					out->Put("Messages", "Rendered slot SQL resulted in an empty string", ctx);
				}
			}
			DoPutbackConnection(pSyb, bIsOpen, server, user);
		} else {
			SYSERROR("unable to get SybCTnewDA");
		}
	} else {
		SysLog::Error("Tried to access SybCTnewDAImpl when SybaseModule was not initialized!\n Try to add a slot SybaseModule to Modules slot and /SybaseModule { /SybCTnewDAImpl { <config> } } into Config.any");
	}
	return bRet;
}

bool SybCTnewDAImpl::DoPrepareSQL( String &command, Context &ctx, ParameterMapper *in)
{
	StartTrace(SybCTnewDAImpl.DoPrepareSQL);
	DAAccessTimer(SybCTnewDAImpl.DoPrepareSQL, fName, ctx);
	OStringStream os(command);
	in->Get("SQL", os, ctx);
	os.flush();
	SubTrace (Query, "QUERY IS [" << command << "]");
	return (command.Length() > 0L);
}

bool SybCTnewDAImpl::CheckCloseOpenedConnections(long lTimeout)
{
	StartTrace(SybCTnewDAImpl.CheckCloseOpenedConnections);
	bool bRet = false;
	Anything anyTimeStamp(Storage::Global());
	TimeStamp aStamp;
	aStamp -= lTimeout;
	Trace("current timeout " << lTimeout << "s, resulting time [" << aStamp.AsString() << "]");
	MutexEntry me(fgStructureMutex);
	me.Use();
	if ( fgInitialized ) {
		TraceAny(fgListOfSybCT, "current list of connections");
		if ( fgListOfSybCT.LookupPath(anyTimeStamp, "Open") && anyTimeStamp.GetSize() ) {
			SybCTnewDA *pSyb = NULL;
			long lTS = 0L;
			// if we still have open connections and the last access is older than lTimeout seconds
			while ( anyTimeStamp.GetSize() && ( aStamp > TimeStamp(anyTimeStamp.SlotName(lTS)) ) ) {
				Anything anyTS(Storage::Global());
				anyTS = anyTimeStamp[lTS];
				TraceAny(anyTS, "stamp of connections to close [" << anyTimeStamp.SlotName(0L) << "]");
				while ( anyTS.GetSize() ) {
					pSyb = SafeCast(anyTS[0L][0L].AsIFAObject(), SybCTnewDA);
					anyTS.Remove(0L);
					if ( pSyb != NULL ) {
						Trace("closing timeouted connection");
						if ( pSyb->Close() ) {
							bRet = true;
						}
					} else {
						SYSWARNING("Sybase connection with address " << (long)pSyb << " not valid anymore!");
					}
					fgListOfSybCT["Unused"].Append((IFAObject *)pSyb);
				}
				anyTimeStamp.Remove(lTS);
			}
		}
	} else {
		SYSERROR("SybCTnewDAImpl not initialized!");
	}
	return bRet;
}

//---- SybCheckCloseOpenedConnectionsAction ----------------------------------------------------------
//: triggers cleanup of open but unused sybase connections
class EXPORTDECL_SYBASECT SybCheckCloseOpenedConnectionsAction : public Action
{
public:
	//--- constructors
	SybCheckCloseOpenedConnectionsAction(const char *name) : Action(name) { }
	~SybCheckCloseOpenedConnectionsAction() {}

	//:cleans the session list from timeouted sessions
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
		StartTrace(SybCheckCloseOpenedConnectionsAction.DoExecAction);
		return SybCTnewDAImpl::CheckCloseOpenedConnections(ctx.Lookup("PeriodicActionTimeout", 60L));
	}
};

//--- SybCheckCloseOpenedConnectionsAction ---
RegisterAction(SybCheckCloseOpenedConnectionsAction);
