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
//--- superclass interface ---
#include "config_stddataaccess.h"
#include "DataAccessImpl.h"

//! DataAccess for
class EXPORTDECL_STDDATAACCESS MailDAImpl: public DataAccessImpl
{

public:
	MailDAImpl(const char *name);
	~MailDAImpl();

// returns a new TRX object
	IFAObject *Clone() const;

//! executes the transaction
//! \param c The context of the transaction
	virtual bool Exec(Context &c, InputMapper *, OutputMapper *);

private:

	//constructor
	MailDAImpl();
	MailDAImpl(const MailDAImpl &);
	//assignement
	MailDAImpl &operator=(const MailDAImpl &);

//friend class MailDAImplTest;

};

/* Don't add stuff after this #endif */
#endif		//not defined _MailDAImpl_H
