/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPFileLoader_H
#define _HTTPFileLoader_H

#include "ReadFileDAImpl.h"

//! FIXME: DataAccess to be refactored soon, do not stray HTTP knowledge around everywhere...
class HTTPFileLoader: public ReadFileDAImpl {
public:
	HTTPFileLoader(const char *name) :
		ReadFileDAImpl(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) HTTPFileLoader(fName);
	}

	//! executes the transaction
	//! \param c The context of the transaction
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

protected:
	//! loads the file
	/*! @param filename full pathname of the file to be loaded
	 * @param ctx The context in which the transaction takes place
	 * @param input ParameterMapper object that is mapping data from the client space to the data access object on request
	 * @param output ResultMapper object that maps the result of the access back into client space */
	virtual bool ProcessFile(const String &filename, Context &ctx, ParameterMapper *input, ResultMapper *output);

	//! produces an error reply according to HTTPError in context
	/*! @param filename of the file which was requested
	 * @param ctx The context in which the transaction takes place
	 * @param input ParameterMapper object that is mapping data from the client space to the data access object on request
	 * @param output ResultMapper object that maps the result of the access back into client space */
	virtual void ProduceErrorReply(const String &filename, Context &ctx, ParameterMapper *input, ResultMapper *output);

	//! generate HTTP Status line
	/*! @param ctx The context in which the transaction takes place
	 * @param input ParameterMapper object that is mapping data from the client space to the data access object on request
	 * @param output ResultMapper object that maps the result of the access back into client space */
	virtual bool GenReplyStatus(Context &ctx, ParameterMapper *input, ResultMapper *output);

	//! generate HTTP header
	/*! @param ctx The context in which the transaction takes place
	 * @param input ParameterMapper object that is mapping data from the client space to the data access object on request
	 * @param output ResultMapper object that maps the result of the access back into client space */
	virtual bool GenReplyHeader(Context &ctx, ParameterMapper *input, ResultMapper *output);

private:
	friend class HTTPFileLoaderTest;
};

#endif		//not defined _HTTPFileLoader_H
