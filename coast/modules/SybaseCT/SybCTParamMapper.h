/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTParamMapper_H
#define _SybCTParamMapper_H

//---- Mapper include -------------------------------------------------
#include "config_sybasect.h"
#include "Mapper.h"

//---- SybCTParamMapper ----------------------------------------------------------
//: comment ParameterMapper
class EXPORTDECL_SYBASECT SybCTParamMapper : public ParameterMapper
{
public:
	//--- constructors
	SybCTParamMapper(const char *name);

	IFAObject *Clone() const;			// support for prototype

	//: pulls out an String according to key
	// Clients use this method to pull out an String value with name key from the Mapper
	//!param: key - the name defines kind of value to get
	//!param: value - the value to be mapped
	//!param: ctx - the thread context of the invocation
	//!retv: returns true if the mapping was successful otherwise false
	virtual bool Get(const char *key, String &value, Context &ctx);

private:
	SybCTParamMapper();
	SybCTParamMapper(const SybCTParamMapper &);
	SybCTParamMapper &operator=(const SybCTParamMapper &);
};

#endif
