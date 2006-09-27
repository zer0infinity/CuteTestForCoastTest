/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _IFAOBJECT_H
#define _IFAOBJECT_H

#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "config.h"

class String;

//---- IFAObject ----------------------------------------------------------
//! This is the abstract root class for all IFAObjects
//! It defines cloning API Clone, that is needed for the prototype pattern
class EXPORTDECL_FOUNDATION IFAObject
{
public:
	IFAObject() {}
	virtual ~IFAObject() {}

	//! support for prototypes is required
	virtual IFAObject *Clone() const = 0;
};

//---- NamedObject ----------------------------------------------------------
//!defines named object api
class EXPORTDECL_FOUNDATION NamedObject : public IFAObject
{
public:
	//!named object constructor
	NamedObject(const char *name);

	//!naming support setting name
	virtual void SetName(const char *) = 0;
	//!naming support getting name
	virtual bool GetName(String &str) const;
	//!naming support getting name
	virtual const char *GetName() const {
		return "anonymous";
	}

private:
	//!not allowed
	NamedObject();
	//!not allowed
	NamedObject(const NamedObject &);
	//!not allowed
	NamedObject &operator=(const NamedObject &);
};

#endif
