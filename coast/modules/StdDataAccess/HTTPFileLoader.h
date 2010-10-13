/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPFileLoader_H
#define _HTTPFileLoader_H

//--- superclass interface ---
#include "config_stddataaccess.h"
#include "ReadFileDAImpl.h"

//! FIXME: DataAccess to be refactored soon, do not stray HTTP knowledge around everywhere...
class EXPORTDECL_STDDATAACCESS HTTPFileLoader: public ReadFileDAImpl
{
public:
	HTTPFileLoader(const char *name);
	~HTTPFileLoader();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	//! executes the transaction
	//! \param c The context of the transaction
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

protected:
	//! loads the file
	//! \param filename full pathname of the file to be loaded
	//! \param c The context of the transaction
	//! \param input the input mapper, assumes functionality of CgiParams
	//! \param output the output mapper, assumes functionality of StreamTransferMapper
	virtual bool ProcessFile(const String &filename, Context &context, ParameterMapper *in, ResultMapper *out);

	//! produces an error reply according to HTTPError in context
	//! \param filename full pathname of the file
	virtual void ProduceErrorReply(const String &filename, Context &context, ParameterMapper *in, ResultMapper *out);
	virtual bool GenReplyStatus(Context &context, ParameterMapper *in, ResultMapper *out);
	virtual bool GenReplyHeader(Context &context, ParameterMapper *in, ResultMapper *out);

private:
	//constructor
	HTTPFileLoader();
	HTTPFileLoader(const HTTPFileLoader &);
	//assignement
	HTTPFileLoader &operator=(const HTTPFileLoader &);

	friend class HTTPFileLoaderTest;
};

#endif		//not defined _HTTPFileLoader_H
