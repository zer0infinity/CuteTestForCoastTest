/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnyImplTypes_H
#define _AnyImplTypes_H

//---- AnyImplTypes ----------------------------------------------------------
//! Definition of AnyImpl type values
/*!
This is done in this separate file to break include dependencies of Anything.h to AnyImpl.h
*/
namespace AnyImplHelper
{
	enum AnyTypes {
		eNull,
		eCharPtr,
		eArray,
		eLong,
		eDouble,
		eVoidBuf,
		eObject
	};
};

typedef AnyImplHelper::AnyTypes AnyImplType;

// convenience definitions
#define AnyNullType AnyImplHelper::eNull
#define AnyCharPtrType AnyImplHelper::eCharPtr
#define AnyArrayType AnyImplHelper::eArray
#define AnyLongType AnyImplHelper::eLong
#define AnyDoubleType AnyImplHelper::eDouble
#define AnyVoidBufType AnyImplHelper::eVoidBuf
#define AnyObjectType AnyImplHelper::eObject

#endif
