/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DataAccess_H
#define _DataAccess_H

#include "config_dataaccess.h"
#include "DataAccessImpl.h"

//---- DataAccess --------------------------------------------------------------------
//! Monomorph DataAccess entry point, minimal easy to use client api
//! this class supports a flexible data access mechanism through composition of its
//! parameter objects inputMapper, outputMapper and dataAccessImpl
class EXPORTDECL_DATAACCESS DataAccess
{
public:
	//! daName configures the data access that should take place
	DataAccess(const char *daName);
	~DataAccess();

	//! simplified data access api with context
	//! this api supports preconfigured data accesses that are linked by the fName of the
	//! this Object
	//! \param context the context of this data access call
	bool StdExec(Context &context);

	//! generic data access api with input and output mappers
	//! \param input mapper that generates the needed input for the data access
	//! \param output mapper that handles the output produced by the data access
	//! \param context the context of this data access call
	bool Exec(ParameterMapper *input, ResultMapper *output, Context &context);

protected:
	friend class DataAccessTest;

	//! find the DataAccessImpl by Name
	//! \param daName the name of the mapper sought
	//! \param context the context of this data access call
	DataAccessImpl *GetImpl(const char *daName, Context &context);

	//! name identifying the data access
	String fName;

private:
	bool GetMyParameterMapper(Context &c, ParameterMapper *&pm);
	bool GetMyResultMapper(Context &c, ResultMapper *&rm);
	void HandleError(Context &c, String name, const char *file, long line, String msg);
};

#endif		//not defined _DataAccess_H
