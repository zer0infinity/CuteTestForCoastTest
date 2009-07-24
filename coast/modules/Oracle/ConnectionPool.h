/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef CONNECTIONPOOL_H_
#define CONNECTIONPOOL_H_

//---- ConnectionPool include -------------------------------------------------
#include "config_coastoracle.h"
#include "DataAccessImpl.h"
#include "Threads.h"

//---- forward declaration -----------------------------------------------
class OraclePooledConnection;
class PeriodicAction;

namespace Coast
{
	namespace Oracle
	{

//---- ConnectionPool ----------------------------------------------------------
//! <b>Oracle specific class to handle backend connection pooling</b>
		/*!
		 * @par Description
		 * A ConnectionPool is used to keep track of a predefined number of Connections to a back end of some type.
		 * This pool will keep track of a number of connections against Oracle database servers. It is not limited to any number of connections
		 * or servers. The Connections tracked should allow a connect to any similar server/user combination as this is the key to keep
		 * Connections open for some time to reduce overhead of opening/closing connections every time a request is made.
		 * To achieve this, the connection to a specific server/user will stay open after usage and if a borrow request is made for a connection
		 * having the same server/user combination, the previously release connection will be returned.
		 * In the case there is no open connection to with a specific server/user connection and no unused/unopened connection around,
		 * an existing and already opened connection will be selected, closed and returned to use with different credentials.
		 \par Configuration
		 \code
		 {
			 /ParallelQueries			long	defines number of parallel queries/sp-calls which can be issued, default 5
			 /CloseConnectionTimeout	long	timeout [s] after which to close open connections, default 60
		 }
		 \endcode
		 */
		class EXPORTDECL_COASTORACLE ConnectionPool
		{
			//! lock to protect internal bookkeeping structure and list of connections
			SimpleMutex fStructureMutex;
			/*! keeps a list of connections
			 * It is important that the connection class is derived from IFAObject to allow for dynamic cast when dealing with the spcific connection.
			 */
			Anything fListOfConnections;
			//! track if the pool is initialized - if not, every access will fail
			bool fInitialized;
			//! pointer to action which periodically checks for timed out connections to close
			PeriodicAction *fpPeriodicAction;
			//! counting semaphore to keep track of the available connections
			Semaphore *fpResourcesSema;
		public:
			/*! construct the connection pool
			 * @param name used to distinguish the internal structure mutex from others
			 */
			ConnectionPool( const char *name );
			/*! cleanup the pool - close connections and delete the connections */
			~ConnectionPool();

			/*! initialize the pool using config as configuration
			 * @param config configuration parameters as described in class details section
			 * @return true in case the pool connections could be allocated and initialization was successful
			 */
			bool Init( ROAnything config );
			/*! uninitialize the pool by closing and destructing the preallocated connections
			 * @return true if it was successful, false otherwise
			 */
			bool Finis();

			/*! external interface function to periodically check if unused connections need to be closed
			 * \param lTimeout time [s] after which to close an unused and still open connection
			 * \return true in case at least one connection was closed this time
			 */
			bool CheckCloseOpenedConnections( long lTimeout );

			/*! Borrow a connection against server using the given user name
			 * If all connections are currently in use, the caller will get blocked until a connections gets released.
			 * @param pConnection connection to borrow
			 * @param bIsOpen flag which tells if the given connection is already open
			 * @param server name of server to connect to - in oracle it is the same as the connection string
			 * @param user name of the user to connect with
			 * @return true in case of success, false signals internal bookkeeping failure
			 * @note This method is thread safe and blocks until a connection is available.
			 */
			bool BorrowConnection( OraclePooledConnection *&pConnection, bool &bIsOpen, const String &server,
								   const String &user );
			/*! put back a connection not used anymore by the borrower
			 * @param pConnection connection to release
			 * @param bIsOpen flag to tell if the given connection is still open or if it had to be closed due to errors
			 * @param server name of server to connect to - in oracle it is the same as the connection string
			 * @param user name of the user which was used to connect
			 */
			void ReleaseConnection( OraclePooledConnection *&pConnection, bool bIsOpen, const String &server,
									const String &user );
			/*! Helper class to ensure requester lock out when all connections are currently in use
			 * Makes use of automatic Semaphore.Acquire in ctor and Semaphore.Release in dtor using a SemaphoreEntry
			 */
			class ConnectionLock
			{
				SemaphoreEntry fSemaEntry;
				ConnectionLock();
				ConnectionLock(const ConnectionLock &);
			public:
				/*! Supply surrounding ConnectionPool object to allow access to Semaphore resource
				 * @param rConnPool surrounding ConnectionPool object
				 */
				ConnectionLock(ConnectionPool &rConnPool) : fSemaEntry(*rConnPool.fpResourcesSema) {}
				~ConnectionLock() {}
			};
		private:
			/*! try to find a matching and open connection using the given credentials
			 * @param pConnection connection returned
			 * @param bIsOpen flag which tells if the given connection is already open
			 * @param server name of server to connect to - in oracle it is the same as the connection string
			 * @param user name of the user to connect with
			 * @return true in case we found a matching and open connection, false otherwise
			 * @note This method is not protected against concurrent access. The caller must ensure mutexed access!
			 */
			bool IntGetOpen( OraclePooledConnection *&pConnection, bool &bIsOpen, const String &server, const String &user );

			/*! internal method to borro a connection to use against server using the given user name
			 * @param pConnection connection to borrow
			 * @param bIsOpen flag which tells if the given connection is already open
			 * @param server name of server to connect to - in oracle it is the same as the connection string
			 * @param user name of the user to connect with
			 * @return true in case of success, false signals internal bookkeeping failure
			 * @note This method is not protected against concurrent access. The caller must ensure mutexed access!
			 */
			bool IntBorrowConnection( OraclePooledConnection *&pConnection, bool &bIsOpen, const String &server,
									  const String &user );

			/*! internal method to put back a connection not used anymore by the borrower
			 * @param pConnection connection to release
			 * @param bIsOpen flag to tell if the given connection is still open or if it had to be closed due to errors
			 * @param server name of server to connect to - in oracle it is the same as the connection string
			 * @param user name of the user which was used to connect
			 * @note This method is not protected against concurrent access. The caller must ensure mutexed access!
			 */
			void IntReleaseConnection( OraclePooledConnection *&pConnection, bool bIsOpen, const String &server,
									   const String &user );

			//! forbid default ctor
			ConnectionPool();
			//! forbid copy ctor
			ConnectionPool( const ConnectionPool & );
			//! forbid assignement operator
			ConnectionPool &operator=( const ConnectionPool & );
		};

	} // end namespace Oracle
} // end namespace Coast

#endif /* CONNECTIONPOOL_H_ */
