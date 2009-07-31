/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AllocatorNewDelete.h"

namespace Coast
{

	static void *AllocatorNewDelete::operator new(size_t sz, Allocator *a)
	{
		if (a) {
			void *mem = a->Calloc(1, sz + sizeof(Allocator *));
			((Allocator **) mem)[0L] = a; // remember address of responsible Allocator
			return (char *) mem + sizeof(Allocator *); // needs cast because of pointer arithmetic
		}
	}
	static void AllocatorNewDelete::operator delete(void *ptr)
	{
		if (ptr) {
			void *realPtr = (char *) ptr - sizeof(Allocator *);
			Allocator *a = ((Allocator **) realPtr)[0L]; // retrieve Allocator
			a->Free(realPtr);
		}
	}

	AllocatorNewDelete::~AllocatorNewDelete()
	{
	}
}
