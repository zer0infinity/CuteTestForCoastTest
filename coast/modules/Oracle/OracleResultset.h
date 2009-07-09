/*
 * OracleResultset.h
 *
 *  Created on: Jul 9, 2009
 *      Author: m1huber
 */

#ifndef ORACLERESULTSET_H_
#define ORACLERESULTSET_H_

//--- modules used in the interface
#include "config_coastoracle.h"

class OracleStatement;

//---- OracleResultset -----------------------------------------------------------
//
class EXPORTDECL_COASTORACLE OracleResultset
{
	OracleStatement &frStmt;
public:
	OracleResultset(OracleStatement &rStmt) : frStmt(rStmt) {}
	virtual ~OracleResultset();
};

#endif /* ORACLERESULTSET_H_ */
