/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CgiCaller_H
#define _CgiCaller_H

//--- superclass interface ---
#include "config_stddataaccess.h"
#include "HTTPFileLoader.h"

//! DataAccess for calling programs via CGI (common gateway interface)
//! expects the input mapper to provide the following keys
//! "program"
class EXPORTDECL_STDDATAACCESS CgiCaller: public HTTPFileLoader
{
	friend class CgiCallerTest;

public:
	CgiCaller(const char *name);
	~CgiCaller();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

protected:
	virtual bool GenReplyHeader(Context &context, ParameterMapper *in, ResultMapper *out);
	//! executes the file
	//! \param filename full pathname of the file to be executed
	//! \param c The context of the transaction
	//! \param input the input mapper, assumes functionality of CgiParams
	//! \param output the output mapper, assumes functionality of StreamTransferMapper
	virtual bool ProcessFile(const String &filename, Context &context, ParameterMapper *in, ResultMapper *out);

private:
	//constructor
	CgiCaller();
	CgiCaller(const CgiCaller &);
	//assignement
	CgiCaller &operator=(const CgiCaller &);
	static void SplitPath(const String &fullPath, String &path, String &file);
	static void SplitRChar(const String &full, char sep, String &before, String &after);
	static String GetFileExtension(const String &file);
};

#endif		//not defined _CgiCaller_H
