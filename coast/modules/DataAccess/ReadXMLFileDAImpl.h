/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ReadXMLFileDAImpl_H
#define _ReadXMLFileDAImpl_H

#include "ReadFileDAImpl.h"

//---- ReadXMLFileDAImpl ----------------------------------------------------------
//! DataAccess for reading a file from disk
/*!
<B>Inputmapper-Configuration:</B><PRE>
{
	/DocumentRoot	Mapperspec		optional, if specified, this path will be prepended to the given filename
	/Filename		Mapperspec		mandatory, path and name of file
	/Extension		Mapperspec		optional, extension of the file if not already specified in Filename slot
	/Mode			Mapperspec		optional, [text|binary] (all lowercase!), default text, mode to open file
	/ResultSlotName	Mapperspec		optional, store the parsed XML in this slot, default is ParsedXMLAsAny
}
</PRE>
*/
class ReadXMLFileDAImpl: public ReadFileDAImpl
{
public:
	ReadXMLFileDAImpl(const char *name);
	~ReadXMLFileDAImpl();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	//! executes the transaction
	//! \param c The context of the transaction
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

private:
	// constructor
	ReadXMLFileDAImpl();
	ReadXMLFileDAImpl(const ReadXMLFileDAImpl &);
	// assignement
	ReadXMLFileDAImpl &operator=(const ReadXMLFileDAImpl &);

	friend class ReadXMLFileDAImplTest;
};

#endif		//not defined _ReadXMLFileDAImpl_H
