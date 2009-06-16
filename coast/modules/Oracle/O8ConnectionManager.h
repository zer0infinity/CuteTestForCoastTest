/*
 * O8ConnectionManager.h
 *
 *  Created on: Jun 15, 2009
 *      Author: m1huber
 */

#ifndef O8CONNECTIONMANAGER_H_
#define O8CONNECTIONMANAGER_H_

//--- modules used in the interface
#include "config_coastoracle.h"
#include "Threads.h"

#include "oci.h"

class O8Connection;

//---- O8ConnectionManager -----------------------------------------------------------
// manages a pool of available user specific connections..
// (pool is created initially)

class EXPORTDECL_COASTORACLE O8ConnectionManager: public IFAObject
{
public:
	O8ConnectionManager(long poolsize = 10);
	~O8ConnectionManager();

	O8Connection *BorrowConnection(text *username, text *password); // for exclusive use
	void ReleaseConnection(O8Connection *c);
protected:
	// returns nothing, object not cloneable
	IFAObject *Clone() const {
		return NULL;
	};

	bool CheckPool(text *username, text *password);
	bool CreateConnectionPool(text *username, text *password, Anything &pool);
protected:
	long fPoolSize;
	MetaThing fTable;
	Mutex fMutex;
};

#endif /* O8CONNECTIONMANAGER_H_ */
