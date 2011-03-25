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
	void *AllocatorNewDelete::operator new(size_t sz, Allocator *a) throw()
	{
		if (a) {
			void *ptr = a->Calloc(1, sz + Memory::AlignedSize<Allocator *>::value);//lint !e1550
			Memory::allocatorFor(ptr) = a; // remember address of responsible Allocator
			return reinterpret_cast<char *>(ptr) + Memory::AlignedSize<Allocator *>::value; // needs cast because of pointer arithmetic
		}
		return a;
	}

	void *AllocatorNewDelete::operator new[](size_t sz, Allocator *a) throw()
	{
		if (a) {
			return operator new(sz, a);
		} else {
			sz += Memory::AlignedSize<Allocator *>::value;
			return reinterpret_cast<char *>(calloc(sz, 1)) + Memory::AlignedSize<Allocator *>::value;//lint !e613
		}
	}

	void *AllocatorNewDelete::operator new[](std::size_t sz) throw()
	{
		return operator new[](sz, static_cast<Allocator*>(0L));
	}

	void AllocatorNewDelete::operator delete(void *ptr) throw()
	{
		void *realPtr = Memory::realPtrFor(ptr);
		Allocator *a = Memory::allocatorFor(realPtr);
		if (a) {
			Memory::safeFree(a, realPtr);
		} else {
			free(realPtr);
		}
	}

	void AllocatorNewDelete::operator delete(void *ptr, Allocator *a) throw()
	{
		Memory::safeFree(a, ptr);
	}

	void AllocatorNewDelete::operator delete[](void *ptr) throw()
	{
		operator delete(ptr);//lint !e1023
	}

	AllocatorNewDelete::~AllocatorNewDelete() {}
}
