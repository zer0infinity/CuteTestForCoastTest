/*
 * ConnectionPool.h
 *
 *  Created on: Jun 17, 2009
 *      Author: m1huber
 */

#ifndef CONNECTIONPOOL_H_
#define CONNECTIONPOOL_H_

//---- ConnectionPool include -------------------------------------------------
#include "config_coastoracle.h"
#include "DataAccessImpl.h"
#include "Threads.h"

//---- forward declaration -----------------------------------------------
class OracleConnection;
class PeriodicAction;

//---- ConnectionPool ----------------------------------------------------------
//:single line description of the class
// further explanation of the purpose of the class
// this may contain <B>HTML-Tags</B>
// ...
class EXPORTDECL_COASTORACLE ConnectionPool
{
public:
	//--- constructors
	ConnectionPool(const char *name);
	~ConnectionPool();

	bool Init(ROAnything config);
	//:implementers should terminate module expecting destruction
	bool Finis();

	bool CheckCloseOpenedConnections(long lTimeout);

	bool BorrowConnection(OracleConnection *&pConnection, bool &bIsOpen, const String &server, const String &user);
	void ReleaseConnection(OracleConnection *&pConnection, bool bIsOpen, const String &server, const String &user);

protected:
	bool IntGetOpen(OracleConnection *&pConnection, bool &bIsOpen, const String &server, const String &user);

	SimpleMutex fStructureMutex;
	Anything fListOfConnections;
	bool		fInitialized;
	PeriodicAction *fpPeriodicAction;
	Semaphore *fpResourcesSema;

private:
	bool IntBorrowConnection(OracleConnection *&pConnection, bool &bIsOpen, const String &server, const String &user);
	void IntReleaseConnection(OracleConnection *&pConnection, bool bIsOpen, const String &server, const String &user);

	//constructor
	ConnectionPool();
	ConnectionPool(const ConnectionPool &);
	//assignement
	ConnectionPool &operator=(const ConnectionPool &);
};

#endif /* CONNECTIONPOOL_H_ */
