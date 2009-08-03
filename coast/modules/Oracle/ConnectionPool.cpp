/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ConnectionPool.h"

#include "OraclePooledConnection.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Timers.h"
#include "TimeStamp.h"
#include "StringStream.h"
#include "PeriodicAction.h"
#include "AnyIterators.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

namespace Coast
{
	namespace Oracle
	{

//---- ConnectionPool ----------------------------------------------------------------
		ConnectionPool::ConnectionPool( const char *name ) :
			fStructureMutex( String( name ).Append( "StructureMutex" ), Storage::Global() ), fListOfConnections(
				Storage::Global() ), fInitialized( false ), fpPeriodicAction( NULL ), fpResourcesSema( NULL )
		{
			StartTrace(ConnectionPool.ConnectionPool);
		}

		ConnectionPool::~ConnectionPool()
		{
			StartTrace(ConnectionPool.~ConnectionPool);
		}

		bool ConnectionPool::Init( ROAnything myCfg )
		{
			StartTrace(ConnectionPool.Init);
			if ( !fInitialized ) {
				long nrOfConnections( myCfg["ParallelQueries"].AsLong( 5L ) );
				long lCloseConnectionTimeout( myCfg["CloseConnectionTimeout"].AsLong( 60L ) );

				LockUnlockEntry me( fStructureMutex );
				{
					// use the semaphore to block when no more resources are available
					fListOfConnections["Size"] = nrOfConnections;
					fpResourcesSema = new Semaphore( nrOfConnections );
					String server, user;
					for ( long i = 0; i < nrOfConnections; ++i ) {
						OraclePooledConnection *pConnection = new ( Storage::Global() ) OraclePooledConnection( i,
								myCfg["MemPoolSize"].AsLong( 2048L ), myCfg["MemPoolBuckets"].AsLong( 16L ) );
						IntReleaseConnection( pConnection, server, user );
					}
					if ( !fpPeriodicAction ) {
						fpPeriodicAction = new PeriodicAction( "OracleCheckCloseOpenedConnectionsAction",
															   lCloseConnectionTimeout );
						fpPeriodicAction->Start();
					}
					fInitialized = true;
				}
			}
			return fInitialized;
		}

		bool ConnectionPool::Finis()
		{
			StartTrace(ConnectionPool.Finis);
			if ( fpPeriodicAction ) {
				fpPeriodicAction->Terminate();
				delete fpPeriodicAction;
				fpPeriodicAction = NULL;
			}
			bool bInitialized;
			{
				LockUnlockEntry me( fStructureMutex );
				bInitialized = fInitialized;
				// force pending/upcoming Exec calls to fail
				fInitialized = false;
			}
			if ( bInitialized ) {
				for ( long lCount = 0L; lCount < fListOfConnections["Size"].AsLong( 0L ) && fpResourcesSema->Acquire(); ++lCount ) {
					OraclePooledConnection *pConnection( NULL );
					String strServer, strUser;
					if ( BorrowConnection( pConnection, strServer, strUser ) ) {
						if ( pConnection->isOpen() ) {
							pConnection->Close( true );
						}
						delete pConnection;
					}
				}
				delete fpResourcesSema;
				fpResourcesSema = NULL;
			}
			return !fInitialized;
		}

		bool ConnectionPool::BorrowConnection( OraclePooledConnection *&pConnection, const String &server, const String &user )
		{
			StartTrace(ConnectionPool.BorrowConnection);
			LockUnlockEntry me( fStructureMutex );
			return IntBorrowConnection( pConnection, server, user );
		}

		void ConnectionPool::ReleaseConnection( OraclePooledConnection *&pConnection, const String &server, const String &user )
		{
			StartTrace(ConnectionPool.ReleaseConnection);
			LockUnlockEntry me( fStructureMutex );
			IntReleaseConnection( pConnection, server, user );
		}

		bool ConnectionPool::IntGetOpen( OraclePooledConnection *&pConnection, const String &server, const String &user )
		{
			StartTrace1(ConnectionPool.IntGetOpen, "server [" << server << "] user [" << user << "]");
			pConnection = NULL;
			ROAnything roaTimeStamp, roaTSEntry, roaConn;
			TraceAny(fListOfConnections, "current list of connections");
			if ( ROAnything( fListOfConnections ).LookupPath( roaTimeStamp, "Open" ) && roaTimeStamp.GetSize() ) {
				String strToLookup( server );
				if ( strToLookup.Length() && user.Length() ) {
					strToLookup << '.' << user;
				}
				Trace("Lookup name [" << strToLookup << "]");
				bool bDone( false );
				AnyExtensions::Iterator<ROAnything> aTSIter( roaTimeStamp );
				while ( !bDone && aTSIter.Next( roaTSEntry ) ) {
					AnyExtensions::Iterator<ROAnything> aConnIter( roaTSEntry );
					while ( !bDone && aConnIter.Next( roaConn ) ) {
						if ( ( strToLookup.Length() <= 0 ) || strToLookup.IsEqual( roaConn[1L].AsCharPtr("") ) ) {
							String strTSEntry( "Open" );
							strTSEntry << ':' << aTSIter.Index();
							String strConnEntry( strTSEntry );
							strConnEntry << ':' << aConnIter.Index();
							Trace("removing conn entry :" << strConnEntry );
							pConnection = SafeCast(roaConn[0L].AsIFAObject(), OraclePooledConnection);
							SlotCleaner::Operate( fListOfConnections, strConnEntry );
							if ( roaTSEntry.GetSize() <= 0 ) {
								Trace("removing timestamp entry :" << strTSEntry );
								SlotCleaner::Operate( fListOfConnections, strTSEntry );
							}
							TraceAny(fListOfConnections, "after cleaning");
							// we do not have to close the connection before using because the OraclePooledConnection is for the same server and user
							bDone = true;
						}
					}
				}
			}
			return pConnection;
		}

		bool ConnectionPool::IntBorrowConnection( OraclePooledConnection *&pConnection, const String &server,
				const String &user )
		{
			StartTrace(ConnectionPool.IntBorrowConnection);
			pConnection = NULL;
			if ( !server.Length() || !user.Length() || !IntGetOpen( pConnection, server, user ) ) {
				Trace("favour unused connection against open connection of different server/user");
				if ( fListOfConnections["Unused"].GetSize() ) {
					Trace("removing first unused element");
					pConnection = SafeCast(fListOfConnections["Unused"][0L].AsIFAObject(), OraclePooledConnection);
					fListOfConnections["Unused"].Remove( 0L );
				} else {
					Trace("no unused connection found, try to look for any open connection not in use");
					String strEmpty;
					if ( IntGetOpen( pConnection, strEmpty, strEmpty ) ) {
						Trace("need to close connection of different server or user before using connection");
						// if this call would return false we could possibly delete and recreate an object
						pConnection->Close();
					}
				}
			}
			Trace("returning &" << (long)(IFAObject *)pConnection);
			if ( pConnection == NULL ) {
				SYSERROR("could not get a valid OracleConnection");
			}
			return ( pConnection != NULL );
		}

		void ConnectionPool::IntReleaseConnection( OraclePooledConnection *&pConnection, const String &server,
				const String &user )
		{
			StartTrace1(ConnectionPool.IntReleaseConnection, "putting &" << (long)(IFAObject *)pConnection);
			if ( pConnection->isOpen() ) {
				String strToStore( server );
				strToStore << '.' << user;
				TimeStamp aStamp;
				Anything anyTimeStamp( Storage::Global() );
				if ( !fListOfConnections.LookupPath( anyTimeStamp, "Open" ) ) {
					anyTimeStamp = MetaThing( Storage::Global() );
					fListOfConnections["Open"] = anyTimeStamp;
				}
				Anything anyToStore( Storage::Global() );
				anyToStore[0L] = (IFAObject *) pConnection;
				anyToStore[1L] = strToStore;
				anyTimeStamp[aStamp.AsString()].Append( anyToStore );
			} else {
				fListOfConnections["Unused"].Append( (IFAObject *) pConnection );
			}
			TraceAny(fListOfConnections, "current list of connections");
		}

		bool ConnectionPool::CheckCloseOpenedConnections( long lTimeout )
		{
			StartTrace(ConnectionPool.CheckCloseOpenedConnections);
			bool bRet = false;
			Anything anyTimeStamp( Storage::Global() );
			TimeStamp aStamp;
			aStamp -= lTimeout;
			Trace("current timeout " << lTimeout << "s, resulting time [" << aStamp.AsString() << "]");
			LockUnlockEntry me( fStructureMutex );
			if ( fInitialized ) {
				TraceAny(fListOfConnections, "current list of connections");
				if ( fListOfConnections.LookupPath( anyTimeStamp, "Open" ) && anyTimeStamp.GetSize() ) {
					OraclePooledConnection *pConnection = NULL;
					long lTS = 0L;
					// if we still have open connections and the last access is older than lTimeout seconds
					while ( anyTimeStamp.GetSize() && ( aStamp > TimeStamp( anyTimeStamp.SlotName( lTS ) ) ) ) {
						Anything anyTS( Storage::Global() );
						anyTS = anyTimeStamp[lTS];
						TraceAny(anyTS, "stamp of connections to close [" << anyTimeStamp.SlotName(0L) << "]");
						while ( anyTS.GetSize() ) {
							pConnection = SafeCast(anyTS[0L][0L].AsIFAObject(), OraclePooledConnection);
							anyTS.Remove( 0L );
							if ( pConnection != NULL ) {
								Trace("closing timeouted connection");
								if ( pConnection->Close() ) {
									bRet = true;
								}
							} else {
								SYSWARNING("connection with address " << (long)pConnection << " not valid anymore!");
							}
							fListOfConnections["Unused"].Append( (IFAObject *) pConnection );
						}
						anyTimeStamp.Remove( lTS );
					}
				}
			} else {
				SYSERROR("ConnectionPool not initialized!");
			}
			return bRet;
		}

	} // end namespace Oracle
} // end namespace Coast
