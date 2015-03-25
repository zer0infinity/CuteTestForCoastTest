/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CgiCaller_H
#define _CgiCaller_H

#include "HTTPFileLoader.h"

//! DataAccess for calling programs via CGI (common gateway interface)
//! expects the input mapper to provide the following keys
//! "program"
class CgiCaller: public HTTPFileLoader {
public:
	CgiCaller(const char *name) :
		HTTPFileLoader(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) CgiCaller(fName);
	}

	static void SplitPath(const String &fullPath, String &path, String &file);
protected:
	//! do not use parents header processing
	/*! @copydoc HTTPFileLoader::GenReplyHeader() */
	virtual bool GenReplyHeader(Context &ctx, ParameterMapper *input, ResultMapper *output);
	//! executes the file
	/*! @copydoc HTTPFileLoader::ProcessFile() */
	virtual bool ProcessFile(const String &filename, Context &ctx, ParameterMapper *input, ResultMapper *output);
};

#endif
