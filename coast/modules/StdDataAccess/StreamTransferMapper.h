/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StreamTransferMapper_H
#define _StreamTransferMapper_H

//---- OutputMapper include -------------------------------------------------
#include "config_stddataaccess.h"
#include "Mapper.h"

//---- StreamTransferMapper ----------------------------------------------------------
//! transfer stream of DataAccessImpl to the stream in the context
//! future versions might incorporate post processing which we ignore
//! in the moment
class EXPORTDECL_STDDATAACCESS StreamTransferMapper : public ResultMapper
{
public:
	//--- constructors
	StreamTransferMapper(const char *name) : ResultMapper(name) {};
	IFAObject *Clone() const {
		return new StreamTransferMapper(fName);
	};

	//! \param key the name defines kind of input for the value
	//! \param is the stream to be mapped
	//! \param ctx the thread context of the invocation
	//! \param config ignored
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoPutStream(const char *key, istream &is, Context &ctx, ROAnything config);

protected:
	//!immediately output a valid HTTP response line and header
	//! synthesizes response line to OK if no indication
	//! is given in context
	void PutResponseLineAndHeader(ostream &os, Context &ctx);

private:
	StreamTransferMapper();
	StreamTransferMapper(const StreamTransferMapper &);
	StreamTransferMapper &operator=(const StreamTransferMapper &);

};

#endif
