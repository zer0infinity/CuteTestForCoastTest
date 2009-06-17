/*
 * OracleCheckCloseOpenedConnectionsAction.h
 *
 *  Created on: Jun 17, 2009
 *      Author: m1huber
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
