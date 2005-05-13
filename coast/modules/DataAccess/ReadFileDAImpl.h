/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ReadFileDAImpl_H
#define _ReadFileDAImpl_H

//--- superclass interface ---
#include "config_dataaccess.h"
#include "FileDAImpl.h"

//---- ReadFileDAImpl ----------------------------------------------------------
//! <B>DataAccess for reading a file from disk</B>
/*!
<B>Inputmapper-Configuration:</B><PRE>
{
	/DocumentRoot	Mapperspec		optional, if specified, this path will be prepended to the given filename
	/Filename		Mapperspec		mandatory, path and name of file
	/Extension		Mapperspec		optional, extension of the file if not already specified in Filename slot
	/Mode			Mapperspec		optional, [text|binary] (all lowercase!), default text, mode to open file
}
</PRE>
<B>Outputmapper-Configuration:</B><PRE>
{
	/FileContent	Mapperspec		optional, store the content of the file in this slot
}
</PRE>
*/
class EXPORTDECL_DATAACCESS ReadFileDAImpl: public FileDAImpl
{
public:
	ReadFileDAImpl(const char *name);
	~ReadFileDAImpl();

	//! returns a new TRX object
	IFAObject *Clone() const;

	//! executes the transaction
	//! \param c The context of the transaction
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

protected:
	virtual int DoGetMode(ROAnything roaModes) {
		return (FileDAImpl::DoGetMode(roaModes) | ios::in);
	}

private:
	// constructor
	ReadFileDAImpl();
	ReadFileDAImpl(const ReadFileDAImpl &);
	// assignement
	ReadFileDAImpl &operator=(const ReadFileDAImpl &);

	friend class ReadFileDAImplTest;
};

#endif		//not defined _ReadFileDAImpl_H
