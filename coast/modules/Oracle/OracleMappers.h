/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _OracleMappers_H
#define _OracleMappers_H

//---- ParameterMapper include -------------------------------------------------
#include "config_coastoracle.h"
#include "Mapper.h"

//---- OracleParameterMapper ----------------------------------------------------------
//! <B>Mapper able to handle Oracle specific stored procedure/function parameters</B>
/*!
 * This mapper can be used for any OracleDAImpl specific query but it is required for PL/SQL queries. To separate
 * between Parameter values and others, all Parameter values need to be placed below slot /Params in the parameter
 * mapper configuration.
 * If a requested parameter key is not found using the full Params.key it will be searched using key only. This was
 * implemented to simplify usage under some circumstances. Please note that all slot names in the Params section need
 * to be in upper case letter as oracle is internally only uses upper case names.
 * @par Configuration
\code
{
	/Name				MapperSpec		mandatory, name of stored procedure/function to execute
	/Params {							mandatory if IN or INOUT parameter values are used in call
		/<PARAMNAME>	MapperSpec		mandatory, IN or INOUT parameter name and its value. The name must match with
											the parameter name of the declared stored procedure/function and must be
											written in all upper case letters!
		...
	}
}
\endcode
\note Parameter names need to be written in all upper case letters, otherwise the implemented mapping will not work -
 an appropriate message will be written into the Messages slot.
*/
class EXPORTDECL_COASTORACLE OracleParameterMapper : public ParameterMapper
{
public:
	/*! Default registering ctor using a unique name to register mapper with
	 * @param name Mapper gets registered using this name
	 */
	OracleParameterMapper(const char *name);

protected:
	/*! major hook method for subclasses, default does script interpretation
	 * @param key Used to lookup a value in the Context
	 * @param value Returned value
	 * @param ctx The context used to perform input mapping
	 * @param script to be interpreted if any, for subclasses this is the config to use
	 * @return returns true if the mapping was successful otherwise false
	 */
	virtual bool DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script);

	/*! Clone interface implementation
	 * @return Pointer to IFAObject base class
	 */
	IFAObject *Clone() const;
private:
	OracleParameterMapper();
	OracleParameterMapper(const OracleParameterMapper &);
	OracleParameterMapper &operator=(const OracleParameterMapper &);
};

//---- OracleResultMapper ----------------------------------------------------------
//! <b>Mapper used to process Oracle specific output mappings</b>
/*!
 * This mapper should be used for any OracleDAImpl. It's main feature is to provide further mappings for stored
 * procedures/functions parameters. If just simple INOUT or OUT parameters are present, it will just put the value of
 * the parameter using its name. But if the parameter is a REF_CURSOR, the whole structure consisting of QueryResult,
 * QueryTitles and QueryCount will be put below the parameters name. This is the only way to keep results of different
 * parameters separate without confusing too much. Further it is possible to define mappings specific to subslots of
 * such structured results.
 * If the feature of specific parameter sub slot mapping is not needed, one can define a single top level mapping rule
 * which is then applied if no specific rule is found. This might be a way to provide reasonable defaults.
 * @par Configuration
\code
{
	/QueryResult 		MapperSpec			optional, specify default behavior when results get put,
												when a stored procedure parameter gets put - usually as PARAMNAME.QueryResult -
												and it is not specialized like below, this MapperSpec will get used
	/QueryTitles		MapperSpec			optional, as above
	/QueryCount	 		MapperSpec			optional, as above

	/<PARAMNAME> {							optional, specialize MapperSpec for named stored procedure/function parameter
		/QueryResult 	MapperSpec			optional, MapperSpec to use when PARAMNAME.QueryResult gets put
		/QueryTitles	MapperSpec			optional, as above
		/QueryCount	 	MapperSpec			optional, as above
	}
}
\endcode
\note Parameter names need to be written in all upper case letters, otherwise the implemented mapping will not work.
*/
class EXPORTDECL_COASTORACLE OracleResultMapper : public ResultMapper
{
public:
	/*! Default registering ctor using a unique name to register mapper with
	 * @param name Mapper gets registered using this name
	 */
	OracleResultMapper(const char *name);

protected:
	/*! Major hook for subclasses that want to do something with their config passed as script. The default is to interpret the script and put a value for every script item used. Recursion will be stopped by DoFinalPutAny which places its value under slot key below given DoGetDestinationSlot()
	 * @param key the key usually defines the associated kind of output-value
	 * @param value the value to be mapped
	 * @param ctx The context to put values into
	 * @param script current mapper configuration as ROAnything
	 * @return returns true if the mapping was successful otherwise false */
	virtual bool DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script);

	/*! Clone interface implementation
	 * @return Pointer to IFAObject base class
	 */
	IFAObject *Clone() const;

private:
	OracleResultMapper();
	OracleResultMapper(const OracleResultMapper &);
	OracleResultMapper &operator=(const OracleResultMapper &);
};

#endif
