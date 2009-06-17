/*
 * OracleCheckCloseOpenedConnectionsAction.cpp
 *
 *  Created on: Jun 17, 2009
 *      Author: m1huber
 */

#include "OracleCheckCloseOpenedConnectionsAction.h"

#include "OracleModule.h"
#include "ConnectionPool.h"

//--- OracleCheckCloseOpenedConnectionsAction ---
RegisterAction(OracleCheckCloseOpenedConnectionsAction);

bool OracleCheckCloseOpenedConnectionsAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(OracleCheckCloseOpenedConnectionsAction.DoExecAction);
	OracleModule *pModule = SafeCast(WDModule::FindWDModule("OracleModule"), OracleModule);
	ConnectionPool *pConnectionPool(0);
	if ( pModule && ( pConnectionPool = pModule->GetConnectionPool()) ) {
		return pConnectionPool->CheckCloseOpenedConnections(ctx.Lookup("PeriodicActionTimeout", 60L));
	}
	return false;
}
