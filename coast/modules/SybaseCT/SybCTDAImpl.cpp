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

Mutex SybCTDAImpl::fgStartMutex("StartMutex");
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
		MutexEntry me(fgStartMutex);
		me.Use();
		Anything myCfg;
		String strInterfacesPathName;
		long nrOfSybCTs = 5L;
		// check if the number of SybCTs is specified in Config
		if (config.LookupPath(myCfg, "SybaseModule")) {
			if (myCfg.IsDefined("ParallelSybCTDAImpls")) {
				nrOfSybCTs = myCfg["ParallelSybCTDAImpls"].AsLong(5L);
			}
			if (myCfg.IsDefined("InterfacesPathName")) {
				strInterfacesPathName = myCfg["InterfacesPathName"].AsCharPtr();
			}
		}
		// use the semaphore to block when no more resources are available
		fgCountingSema = new Semaphore(nrOfSybCTs);

		// SybCT::Init initializes the cs_context.  It must be done only once
		if ( SybCT::Init(&fg_cs_context, &fgContextMessages, strInterfacesPathName) == CS_SUCCEED ) {
			for ( long i = 0; i < nrOfSybCTs; i++ ) {
				Anything *pMessages = new Anything();
				SybCT *aCT = new SybCT(fg_cs_context);
				Mutex *aMutex = new Mutex( String() << "Mutex-" << i, Storage::Global() );
				fgListOfSybCT[i]["InUse"] = (IFAObject *)aMutex;
				fgListOfSybCT[i]["SybCT"] = (IFAObject *)aCT;
				fgListOfSybCT[i]["Messages"] = (IFAObject *)pMessages;
			}
		}
		fgInitialized = true;
	}
	return fgInitialized;
}

bool SybCTDAImpl::Finis()
{
	StartTrace(SybCTDAImpl.Finis);
	MutexEntry me(fgStartMutex);
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
			delete (Anything *)(fgListOfSybCT[0L]["Messages"].AsIFAObject());
			delete (Mutex *)(fgListOfSybCT[0L]["InUse"].AsIFAObject());
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

bool SybCTDAImpl::Exec( Context &context, InputMapper *in, OutputMapper *out)
{
	StartTrace(SybCTDAImpl.Exec);

	// check if we are initialized
	if (fgInitialized) {
		// Check if the structure for DBqueries is available
		if ( !fgStartMutex.TryLock() ) {
			//SimpleMutexEntry aME(SybCT::fgMessagesMutex); aME.Use();
			String mainMsgErr;
			GetName(mainMsgErr);
			mainMsgErr << ": SybCTDAImpl not yet started!";
			out->Put("MainMsgErr", mainMsgErr, context);
			out->Put("MainMsgErrNumber", String("34004"), context);
			Trace("SybCTDAImpl not yet started!");
			return false;
		}
		SemaphoreEntry se(*fgCountingSema);
		fgStartMutex.Unlock();

		// If the structure for DBqueries is available, find a free SybCT
		long nrOfSybCTs = fgListOfSybCT.GetSize();
		bool retCode   = false;

		// Try to get a free SybCT from the list
		for ( long idx = 0; idx < nrOfSybCTs; idx++ ) {
			Mutex *aMutex = (Mutex *)fgListOfSybCT[idx]["InUse"].AsIFAObject();
			if ( aMutex ) {
				if ( aMutex->TryLock() ) {
					SybCT *aCT = (SybCT *)fgListOfSybCT[idx]["SybCT"].AsIFAObject();
					Trace( "=== SybCT = " << (long)aCT );

					Anything *pMessages = (Anything *)fgListOfSybCT[idx]["Messages"].AsIFAObject();
					Anything &aMsgAny = *pMessages, queryParams;
					aMsgAny = Anything();

					FillParameters(context, in, out, queryParams);
					if ( !aCT->Open( pMessages, queryParams["user"].AsString(), queryParams["password"].AsString(), queryParams["server"].AsString(), queryParams["app"].AsString() ) ) {
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
					Trace("returning " << (retCode ? "true" : "false"));
					aMutex->Unlock();
					return retCode;
				}
			}
		}

		// Every aCT has its own messages
		String mainMsgErr;
		GetName(mainMsgErr);
		mainMsgErr << ": SybCTDAImpl no free SybDB could be found!";
		out->Put("MainMsgErr", mainMsgErr, context);
		out->Put("MainMsgErrNumber", String("34006"), context);
		Trace("no free SybDB could be found!");
	} else {
		SysLog::Error("Tried to access SybCTDAImpl when SybaseModule was not initialized!\n Try to add a slot SybaseModule to Modules slot and /SybaseModule {...} into Config.any");
	}
	return false;	// Break the never-end while-loop
}
