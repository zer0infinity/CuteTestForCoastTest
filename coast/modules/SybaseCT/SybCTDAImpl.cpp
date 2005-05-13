/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTDAImpl.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "SybCT.h"
#include "DiffTimer.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

Mutex SybCTDAImpl::fgStructureMutex("StartMutex");
Anything SybCTDAImpl::fgListOfSybCT;
Anything SybCTDAImpl::fgContextMessages;
CS_CONTEXT *SybCTDAImpl::fg_cs_context;
bool SybCTDAImpl::fgInitialized = false;
Semaphore *SybCTDAImpl::fgCountingSema = NULL;

//---- SybCTDAImpl ----------------------------------------------------------------
RegisterDataAccessImpl(SybCTDAImpl);

SybCTDAImpl::SybCTDAImpl(const char *name) : SybCTGenericDAImpl(name)
{
	StartTrace(SybCTDAImpl.SybCTDAImpl);
}

SybCTDAImpl::~SybCTDAImpl()
{
	StartTrace(SybCTDAImpl.~SybCTDAImpl);
}

bool SybCTDAImpl::Init(const Anything &config)
{
	StartTrace(SybCTDAImpl.Init);
	if (!fgInitialized) {
		MutexEntry me(fgStructureMutex);
		me.Use();
		Anything myCfg;
		String strInterfacesPathName;
		long nrOfSybCTs = 5L;
		// check if the number of SybCTs is specified in Config
		if (config.LookupPath(myCfg, "SybaseModule.SybCTDAImpl")) {
			if (myCfg.IsDefined("ParallelQueries")) {
				nrOfSybCTs = myCfg["ParallelQueries"].AsLong(5L);
			}
			if (myCfg.IsDefined("InterfacesPathName")) {
				strInterfacesPathName = myCfg["InterfacesPathName"].AsCharPtr();
			}
		}
		// use the semaphore to block when no more resources are available
		fgCountingSema = new Semaphore(nrOfSybCTs);
		fgListOfSybCT.SetAllocator(Storage::Global());
		fgListOfSybCT = Anything();
		fgContextMessages.SetAllocator(Storage::Global());
		fgContextMessages = Anything();
		// SybCT::Init initializes the cs_context.  It must be done only once
		if ( SybCT::Init(&fg_cs_context, &fgContextMessages, strInterfacesPathName) == CS_SUCCEED ) {
			for ( long i = 0; i < nrOfSybCTs; i++ ) {
				SybCT *aCT = new SybCT(fg_cs_context);
				SimpleMutex *aMutex = new SimpleMutex( String() << "SimpleMutex-" << i, Storage::Global() );
				fgListOfSybCT[i]["InUse"] = (IFAObject *)aMutex;
				fgListOfSybCT[i]["SybCT"] = (IFAObject *)aCT;
			}
		}
		fgInitialized = true;
	}
	return fgInitialized;
}

bool SybCTDAImpl::Finis()
{
	StartTrace(SybCTDAImpl.Finis);
	MutexEntry me(fgStructureMutex);
	me.Use();
	if (fgInitialized) {
		long l = 0L;
		for (l = 0L; l < fgListOfSybCT.GetSize(); l++) {
			fgCountingSema->Acquire();
		}

		SybCT::Finis(fg_cs_context);
		// trace messages which occurred without a connection
		while (fgContextMessages.GetSize()) {
			SysLog::Warning(String() << fgContextMessages[0L].AsCharPtr() << "\n");
			fgContextMessages.Remove(0L);
		}

		while ( fgListOfSybCT.GetSize() > 0L ) {
			delete (SybCT *)(fgListOfSybCT[0L]["SybCT"].AsIFAObject());
			delete (SimpleMutex *)(fgListOfSybCT[0L]["InUse"].AsIFAObject());
			fgListOfSybCT.Remove(0L);
		}

		while (l > 0L) {
			fgCountingSema->Release();
			l--;
		}
		delete( fgCountingSema );
		fgCountingSema = NULL;
		fgInitialized = false;
	}
	return !fgInitialized;
}

IFAObject *SybCTDAImpl::Clone() const
{
	StartTrace(SybCTDAImpl.Clone);
	return new SybCTDAImpl(fName);
}

bool SybCTDAImpl::Exec( Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(SybCTDAImpl.Exec);

	bool retCode = false;
	bool bInitialized = false;
	if ( fgInitialized ) {
		MutexEntry me(fgStructureMutex);
		me.Use();
		bInitialized = fgInitialized;
	}
	if ( bInitialized ) {
		long lToUseIndex = -1L;
		SybCT *aCT = NULL;
		SimpleMutex *pMutex = NULL;
		SemaphoreEntry se(*fgCountingSema);
		{
			MutexEntry me(fgStructureMutex);
			me.Use();
			// If the structure for DBqueries is available, find a free SybCT
			long nrOfSybCTs = fgListOfSybCT.GetSize();

			// Try to get a free SybCT from the list
			for ( lToUseIndex = 0L; lToUseIndex < nrOfSybCTs; lToUseIndex++ ) {
				pMutex = (SimpleMutex *)fgListOfSybCT[lToUseIndex]["InUse"].AsIFAObject();
				if ( pMutex && pMutex->TryLock() ) {
					break;
				}
			}
			aCT = (SybCT *)fgListOfSybCT[lToUseIndex]["SybCT"].AsIFAObject();
			Trace( "=== SybCT = " << (long)aCT );
		}
		if ( aCT != NULL ) {
			Anything aMsgAny, queryParams;
			FillParameters(context, in, out, queryParams);
			if ( !aCT->Open( &aMsgAny, queryParams["user"].AsString(), queryParams["password"].AsString(), queryParams["server"].AsString(), queryParams["app"].AsString() ) ) {
				Trace( "could not open SyBase");
			} else {
				Trace( "could open SyBase");
				DiffTimer aTimer;
				if ( !aCT->SqlExec( queryParams["query"].AsString(), queryParams["resultformat"].AsString(), queryParams["resultsize"].AsLong() ) ) {
					Trace( "could not execute the sql command");
				} else {
					Trace("sql used: " << aTimer.Diff() << "ms");
					Anything queryResults, queryTitles;
					if ( aCT->GetResult( queryResults, queryTitles ) ) {
						retCode = PutResults(context, in, out, queryParams, queryResults, queryTitles);
					} else {
						Trace( "could not fetch the result");
					}
				}
				aCT->Close(!retCode);
			}
			PutMessages(context, out, aMsgAny);
			{
				MutexEntry me(fgStructureMutex);
				me.Use();
				pMutex->Unlock();
			}
		} else {
			// Every aCT has its own messages
			String mainMsgErr;
			GetName(mainMsgErr);
			mainMsgErr << ": SybCTDAImpl no free SybDB could be found!";
			out->Put("MainMsgErr", mainMsgErr, context);
			out->Put("MainMsgErrNumber", String("34006"), context);
			Trace("no free SybDB could be found!");
		}
	} else {
		SysLog::Error("Tried to access SybCTDAImpl when SybaseModule was not initialized!\n Try to add a slot SybaseModule to Modules slot and /SybaseModule {...} into Config.any");
	}
	return retCode;	// Break the never-end while-loop
}
