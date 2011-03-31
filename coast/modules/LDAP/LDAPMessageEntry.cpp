/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LDAPMessageEntry.h"

//---- LDAPMessageEntry ----------------------------------------------------------------
LDAPMessageEntry::LDAPMessageEntry(LDAPMessage **LDAPMessagePtrIn)
	: fLDAPMessagePtr(LDAPMessagePtrIn)
{
	*fLDAPMessagePtr = NULL;
}

LDAPMessageEntry::~LDAPMessageEntry()
{
	if ((*fLDAPMessagePtr) != NULL) {
		ldap_msgfree(*fLDAPMessagePtr);
	}
}

void LDAPMessageEntry::Use() const
{
}
