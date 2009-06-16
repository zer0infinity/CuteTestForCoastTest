/*
 * O8ConnectionManager.cpp
 *
 *  Created on: Jun 15, 2009
 *      Author: m1huber
 */

#include "O8ConnectionManager.h"
#include "O8Connection.h"
#include "Dbg.h"

O8ConnectionManager::O8ConnectionManager(long s) :
	fPoolSize(s), fTable(Storage::Global()), fMutex("O8ConnectionManager", Storage::Global())
{
	StartTrace(O8ConnectionManager.O8ConnectionManager);
}

O8ConnectionManager::~O8ConnectionManager()
{
	StartTrace(O8ConnectionManager.~O8ConnectionManager);
	// FIXME: make sure the connections are not used anymore

	// close all connections
	LockUnlockEntry me(fMutex);
	for (long i = 0; i < fTable.GetSize(); i++) {
		Anything pool = fTable[i];
		// delete Semaphore
		Semaphore *cr = (Semaphore *) pool["Semaphore"].AsIFAObject();
		if (cr) {
			delete cr;
		}

		// delete Connections
		Anything conn = pool["Connections"];
		for (long j = 0; j < conn.GetSize(); j++) {
			O8Connection *c = (O8Connection *) conn[j].AsIFAObject();
			if (c) {
				delete c;
			}
		}
	}
	fTable = MetaThing();
}

void O8ConnectionManager::ReleaseConnection(O8Connection *c)
{
	StartTrace(O8ConnectionManager.ReleaseConnection);
	// client does not need this connection any longer

	LockUnlockEntry me(fMutex);
	const char *username = c->GetUserName();

	Anything pool = fTable[username]["Connections"];
	pool.Append(c);

	Semaphore *cr = (Semaphore *) fTable[username]["Semaphore"].AsIFAObject();
	cr->Release();
}

bool O8ConnectionManager::CreateConnectionPool(text *username, text *password, Anything &pool)
{
	StartTrace(O8ConnectionManager.CreateConnectionPool);
	// method runs under the protection of fMutex!
	// it tries to create 'fPoolSize' new connections to the database

	MetaThing connections;

	for (long i = 0; i < fPoolSize; i++) {
		//FIXME: do not connect immediately, just prepare
		O8Connection *c = new O8Connection(username, password);

		if (c && !c->SuccessfullyConnected()) {
			delete c;
		} else {
			if (c) {
				connections.Append(Anything(c));
			}
		}
	}
	if (connections.GetSize() == 0) {
		// could not establish any connection
		return FALSE;
	} else {
		// successfully established some connections
		// create cache structure to manage them

		pool["Connections"] = connections;

		// --- create critical region to control access to this pool

		Semaphore *cr = new Semaphore(connections.GetSize());
		pool["Semaphore"] = (IFAObject *) cr;
	}

	return TRUE;
}

bool O8ConnectionManager::CheckPool(text *username, text *password)
{
	StartTrace(O8ConnectionManager.CheckPool);
	// --- make sure that user specific connection pool exists

	LockUnlockEntry me(fMutex);
	if (!fTable.IsDefined((char *) username)) {
		// ---  pool needs to be created
		MetaThing pool;
		if (CreateConnectionPool(username, password, pool)) {
			// new pool available
			fTable[(char*) username] = pool;
		} else {
			// failure
			return FALSE;
		}
	}
	return TRUE;
}

O8Connection *O8ConnectionManager::BorrowConnection(text *username, text *password)
{
	StartTrace(O8ConnectionManager.BorrowConnection);
	// --- wait for / retrieve available connection from pool

	if (!CheckPool(username, password)) {
		return 0;
	}

	Semaphore *cr = 0;
	{
		LockUnlockEntry me(fMutex);
		cr = (Semaphore *) fTable[(char*) username]["Semaphore"].AsIFAObject();
	}

	// --- control access to connection pool (client is
	//     delayed until a connection becomes available)

	cr->Acquire();

	// --- get/remove connection from pool

	O8Connection *c = 0;
	{
		LockUnlockEntry me(fMutex);
		Anything pool = fTable[(char*) username]["Connections"];
		c = (O8Connection *) pool[0L].AsIFAObject();
		pool.Remove(0L);
	}

	return c;
}
