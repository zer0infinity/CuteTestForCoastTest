/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLECHECKCLOSEOPENEDCONNECTIONSACTION_H_
#define ORACLECHECKCLOSEOPENEDCONNECTIONSACTION_H_

#include "config_coastoracle.h"
#include "Action.h"

//---- OracleCheckCloseOpenedConnectionsAction ----------------------------------------------------------
//: triggers cleanup of open but unused connections
class EXPORTDECL_COASTORACLE OracleCheckCloseOpenedConnectionsAction : public Action
{
public:
	//--- constructors
	OracleCheckCloseOpenedConnectionsAction(const char *name) : Action(name) { }
	~OracleCheckCloseOpenedConnectionsAction() {}

	//:cleans the session list from timeouted sessions
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif /* ORACLECHECKCLOSEOPENEDCONNECTIONSACTION_H_ */
