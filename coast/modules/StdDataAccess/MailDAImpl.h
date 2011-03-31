/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MailDAImpl_H
#define _MailDAImpl_H

#define DEF_UNKNOWN "No-domain-provided"

#include "DataAccessImpl.h"

class Context;

//! DataAccess for
class MailDAImpl: public DataAccessImpl {
	MailDAImpl();
	MailDAImpl(const MailDAImpl &);
	MailDAImpl &operator=(const MailDAImpl &);
public:
	MailDAImpl(const char *name) :
		DataAccessImpl(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) MailDAImpl(fName);
	}

	//! executes the transaction
	//! \param c The context of the transaction
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);
};

#endif		//not defined _MailDAImpl_H
