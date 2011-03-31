/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OracleCheckCloseOpenedConnectionsAction.h"
#include "OracleModule.h"
#include "OracleEnvironment.h"
#include "Context.h"

//--- OracleCheckCloseOpenedConnectionsAction ---
RegisterAction(OracleCheckCloseOpenedConnectionsAction);

bool OracleCheckCloseOpenedConnectionsAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(OracleCheckCloseOpenedConnectionsAction.DoExecAction);
	OracleModule *pModule = SafeCast(WDModule::FindWDModule("OracleModule"), OracleModule);
	Coast::Oracle::ConnectionPool *pConnectionPool(0);
	if ( pModule && ( pConnectionPool = pModule->GetConnectionPool()) ) {
		return pConnectionPool->CheckCloseOpenedConnections(ctx.Lookup("PeriodicActionTimeout", 60L));
	}
	return false;
}
