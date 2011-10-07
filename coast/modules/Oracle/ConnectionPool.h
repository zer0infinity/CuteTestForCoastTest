/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef CONNECTIONPOOL_H_
#define CONNECTIONPOOL_H_

#include "DataAccessImpl.h"
#include "Threads.h"
#include "WPMStatHandler.h"

class OraclePooledConnection;
class PeriodicAction;

namespace Coast
{
	namespace Oracle
	{
		typedef std::auto_ptr<WPMStatHandler> StatEvtHandlerPtrType;

//! Oracle specific class to handle backend connection pooling
		/*!
		 * A ConnectionPool is used to keep track of a predefined number of Connections to a back end of some type.
		 * This pool will keep track of a number of connections against Oracle database servers. It is not limited to any number of connections
		 * or servers except a possible limit exists within oracle OCI API.
		 *
		 * The pooled Connections should allow connections to any server/user combination allowed by the back end. This combination is used
		 * as key to keep the Connections in an open list for some time to reduce overhead of opening/closing connections every
		 * time a request is made.
		 * When a request is made for a connection having the same server/user combination, the previously released connection
		 * will be returned.
		 *
		 * In case there is currently no open connection for the given server/user combination, and no unused/unopened connection around,
		 * the next already opened connection will be selected (note that it has a different server/user setting), closed and returned to
		 * the caller.
		 *
		 * @section cps1 Pool configuration
		 * @see Check @ref oms1 to find out where to place the following configuration
		 *
		\code
		{
			/ParallelQueries
			/CloseConnectionTimeout
			/MemPoolSize
			/MemPoolBuckets
		}
		\endcode
		 *
		 * @par \c ParallelQueries
		 * Optional, default 5\n
		 * Long	value to define number of concurrent back end calls. The pool will pre allocate the requested number of pooled
		 * connections. The connections will not get opened unless a explicitly opened by the caller.
		 *
		 * @par \c CloseConnectionTimeout
		 * Optional, default 60\n
		 * Long	value to define time period in [s] after which to check for open connections to be closed.
		 *
		 * @par \c MemPoolSize
		 * Optional, default 2048kB\n
		 * Long value defining the size of the PoolAllocator used by the memory functions used with the connections environment
		 *
		 * @par \c MemPoolBuckets
		 * Optional, default 16\n
		 * Long value defining the number of bucket sizes to allocate inside the PoolAllocator
		 */
		class ConnectionPool : public StatGatherer
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
			//! name of the Pool
			String fName;

			//! statistic event handler
			StatEvtHandlerPtrType fpStatEvtHandlerPool;

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
			 * @param server name of server to connect to - in oracle it is the same as the connection string
			 * @param user name of the user to connect with
			 * @return true in case of success, false signals internal bookkeeping failure
			 * @note This method is thread safe and blocks until a connection is available.
			 */
			bool BorrowConnection( OraclePooledConnection *&pConnection, const String &server,
								   const String &user, bool bUseTLS = false );
			/*! put back a connection not used anymore by the borrower
			 * @param pConnection connection to release
			 * @param server name of server to connect to - in oracle it is the same as the connection string
			 * @param user name of the user which was used to connect
			 */
			void ReleaseConnection( OraclePooledConnection *&pConnection, bool bUseTLS = false );

		protected:
			/*! implements the StatGatherer interface used by StatObserver
				\param statistics Anything to get statistics data */
			void DoGetStatistic(Anything &statistics);

			/*! get this instances given name
				\return name of this objects instance */
			const String &GetName() const {
				return fName;
			}

		private:
			/*! try to find a matching and open connection using the given credentials
			 * @param pConnection connection returned
			 * @param server name of server to connect to - in oracle it is the same as the connection string
			 * @param user name of the user to connect with
			 * @return true in case we found a matching and open connection, false otherwise
			 * @note This method is not protected against concurrent access. The caller must ensure mutexed access!
			 */
			bool IntGetOpen( OraclePooledConnection *&pConnection, const String &server, const String &user );

			/*! internal method to borro a connection to use against server using the given user name
			 * @param pConnection connection to borrow
			 * @param server name of server to connect to - in oracle it is the same as the connection string
			 * @param user name of the user to connect with
			 * @return true in case of success, false signals internal bookkeeping failure
			 * @note This method is not protected against concurrent access. The caller must ensure mutexed access!
			 */
			bool IntBorrowConnection( OraclePooledConnection *&pConnection, const String &server,
									  const String &user );

			/*! internal method to put back a connection not used anymore by the borrower
			 * @param pConnection connection to release
			 * @param server name of server to connect to - in oracle it is the same as the connection string
			 * @param user name of the user which was used to connect
			 * @note This method is not protected against concurrent access. The caller must ensure mutexed access!
			 */
			void IntReleaseConnection( OraclePooledConnection *&pConnection );

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
