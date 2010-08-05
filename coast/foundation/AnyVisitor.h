/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnyVisitor_H
#define _AnyVisitor_H

//---- baseclass include -------------------------------------------------
#include "config_foundation.h"
#include "Anything.h"

//---- AnyVisitor ----------------------------------------------------------
//! apply Visitor pattern to Anything structures
/*!
apply the visitor pattern and externalize algorithms traversing anythings.
means to get rid of manually dispatching using GetType()
*/
class EXPORTDECL_FOUNDATION AnyVisitor
{
public:
//!provide virtual destructor for subclasses with data
	virtual ~AnyVisitor() {};
//!visit an empty Anything of type eNull
	virtual void	VisitNull(long index, const char *slotname) {}
//!visit an Anything representing a String, for historical reasons type called eCharPtr
	virtual void	VisitCharPtr(const String &value, const AnyImpl *id, long index, const char *slotname) {}
//!visit an Anything representing a Sequence/Dictionary, type called eArray
	/*
	use ROAnything for refering the internal implementation to
	avoid leaking AnyArrayImpl class to the outside world (it is hidden in Anything.cpp
	<BR>
	Template Method implementation provides 4 auxiliary hook methods
	in default implementation to avoid re-coding the loop
	in every visitor.
	*/
	virtual void	VisitArray(const ROAnything value, const AnyImpl *id, long index, const char *slotname);
//!visit an Anything representing an integral number, type called eLong
	virtual void	VisitLong(long value, const AnyImpl *id, long index, const char *slotname) {}
//!visit an Anything representing a real number, type called eDouble
	virtual void	VisitDouble(double value, const AnyImpl *id, long index, const char *slotname) {}
//!visit an Anything representing a binary buffer, type called eVoidBuf
	virtual void	VisitVoidBuf(const String &value, const AnyImpl *id, long index, const char *slotname) {}
//!visit an Anything representing an object reference/pointer, type called eObject
	virtual void	VisitObject(IFAObject *value, const AnyImpl *id, long index, const char *slotname) {}
protected:
//!auxiliary hook method for general VisitArray implementation
	virtual void ArrayBefore(const ROAnything value, const AnyImpl *id, long index, const char *slotname) {}
//!auxiliary hook method for general VisitArray implementation
	virtual void ArrayBeforeElement(long index, const String &key) {}
//!auxiliary hook method for general VisitArray implementation
	virtual void ArrayAfterElement(long index, const String &key) {}
//!auxiliary hook method for general VisitArray implementation
	virtual void ArrayAfter(const ROAnything value, const AnyImpl *id, long index, const char *slotname) {}
};
#endif
