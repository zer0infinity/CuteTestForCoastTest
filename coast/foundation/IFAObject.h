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
#include "foundation.h"
#include "AllocatorNewDelete.h"

class String;

//---- IFAObject ----------------------------------------------------------
//! This is the abstract root class for all IFAObjects
/*! It defines the cloning API IFAObject.Clone(), that is needed for the prototype pattern.
 */
class EXPORTDECL_FOUNDATION IFAObject : public Coast::AllocatorNewDelete {
public:
	/*! virtual destructor to nicely cleanup derived types */
	virtual ~IFAObject() {
	}

	/*! Support for prototype pattern
	 * @return Baseclass pointer to cloned instance of a derived type
	 */
	IFAObject *Clone() const {
		return Clone(Storage::Current());
	}

	/*! Support for prototype pattern
	 * @param a Allocator passed in to allocate new objects with if required
	 * @return Baseclass pointer to cloned instance of a derived type
	 */
	virtual IFAObject *Clone(Allocator *a) const = 0;
};

//---- NamedObject ----------------------------------------------------------
//! Defines abstract named object api
/*! This object serves as a base class for \em logically \em named objects of same type.
 * The name could be used to distinguish different instances of an object of the same base.
 */
class EXPORTDECL_FOUNDATION NamedObject: public IFAObject {
public:
	/*! named object default constructor */
	NamedObject() : IFAObject() {}

	/*! Naming support interface to set the objects name
	 * @param name
	 */
	virtual void SetName(const char *name) = 0;
	/*! Naming support interface for getting this objects name as String
	 * @param name String representation of this objects name
	 * @return true in case retrieval of name was successful
	 * @note The default implementation here will return false to force overriding in derived classes.
	 */
	virtual bool GetName(String &name) const = 0;
	/*! Naming support interface for getting this objects name as plain old char array
	 * @return char array representation of this objects name
	 */
	virtual const char *GetName() const  = 0;

private:
	//!not allowed
	NamedObject(const NamedObject &);
	//!not allowed
	NamedObject &operator=(const NamedObject &);
};

#endif
