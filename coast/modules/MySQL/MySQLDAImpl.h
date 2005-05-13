/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MySQLDAImpl_H
#define _MySQLDAImpl_H

#include "config_mysql.h"
//--- superclass interface ---
#include "DataAccessImpl.h"

//--- modules used in the interface ---
#include <mysql.h>

//! DataAccess for
class EXPORTDECL_MYSQL MySQLDAImpl: public DataAccessImpl
{
public:
	MySQLDAImpl(const char *name);
	~MySQLDAImpl();

	// returns a new TRX object
	IFAObject *Clone() const;

	//! executes the transaction
	//! \param c The context of the transaction
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

protected:
	//! Helper method to report errors
	void SetErrorMsg(const char *msg, MYSQL *mysql, Context &context, ResultMapper *out );

private:
	//constructor
	MySQLDAImpl();
	MySQLDAImpl(const MySQLDAImpl &);
	//assignement
	MySQLDAImpl &operator=(const MySQLDAImpl &);
};

/* Don't add stuff after this #endif */
#endif		//not defined _MySQLDAImpl_H
