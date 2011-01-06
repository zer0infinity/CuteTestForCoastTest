/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AllocatorNewDelete.h"

namespace Coast
{
	AllocatorNewDelete::~AllocatorNewDelete() {}

	void *AllocatorNewDelete::operator new(size_t sz, Allocator *a)
	{
		if (a) {
			void *mem = a->Calloc(1, sz + Memory::AlignedSize<Allocator *>::value);
			(reinterpret_cast<Allocator **>( mem))[0L] = a; // remember address of responsible Allocator
			char *ptr = reinterpret_cast<char *>(mem) + Memory::AlignedSize<Allocator *>::value; // needs cast because of pointer arithmetic
			return ptr;
		}
		return a;
	}
	//TODO: refactor to DRY, check if alignedSize is an issue with pointers (might be with 32bit pointers)
	void AllocatorNewDelete::operator delete(void *ptr)
	{
		if (ptr) {
			void *realPtr = reinterpret_cast<char *>( ptr) - Memory::AlignedSize<Allocator *>::value;
			Allocator *a = (reinterpret_cast<Allocator **>(realPtr))[0L]; // retrieve Allocator
			a->Free(realPtr);
		}
	}

	void AllocatorNewDelete::operator delete(void *ptr, Allocator *a)
	{
		if (ptr && a) {
			void *realPtr = reinterpret_cast<char *>( ptr) - Memory::AlignedSize<Allocator *>::value;
			Allocator *aStored = (reinterpret_cast<Allocator **>( realPtr))[0L]; // retrieve Allocator
			assert(aStored == a);
			aStored->Free(realPtr);
		}
	}
}
