/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPMessageEntry_H
#define _LDAPMessageEntry_H

//---- baseclass include -------------------------------------------------
#include "config_LDAPDA.h"
#include "LDAPAPI.h"

//---- LDAPMessageEntry ---------------------------------------------------
//! Automatic destruction of LDAPMessage struct
class EXPORTDECL_LDAPDA LDAPMessageEntry
{
public:
	//!Sets the LDAPMessage pointer to pointer in the constructor
	LDAPMessageEntry(LDAPMessage **LDAPMessagePtrIn);

	//!releases the LDAPMessage in destructor
	~LDAPMessageEntry();

	//!dummy method to prevent optimizing compilers from optimizing away unused variables
	void Use() const;

private:
	//!reference to LDAPMessage pointer (pointer to pointer) used by this object
	LDAPMessage **fLDAPMessagePtr;
};

#endif
