/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AllocatorNewDelete.h"
#include "Dbg.h"

namespace Coast
{

	static void *AllocatorNewDelete::operator new(size_t sz, Allocator *a) throw()
	{
		if (a) {
			//!FIXME: correct alignment could improve performance
			void *mem = a->Calloc(1, sz + sizeof(Allocator *));
			((Allocator **) mem)[0L] = a; // remember address of responsible Allocator
			char *ptr = (char *)mem + sizeof(Allocator *); // needs cast because of pointer arithmetic
			StatTrace(AllocatorNewDelete.new, "sz:" << (long)sz << " a:" << (long)a << " ptr:" << (long)ptr, Storage::Current());
			return ptr;
		}
		return a;
	}
	static void AllocatorNewDelete::operator delete(void *ptr) throw()
	{
		if (ptr) {
			//!FIXME: correct alignment could improve performance
			void *realPtr = (char *) ptr - sizeof(Allocator *);
			Allocator *a = ((Allocator **) realPtr)[0L]; // retrieve Allocator
			size_t sz(a->Free(realPtr));
			StatTrace(AllocatorNewDelete.delete, " a:" << (long)a << " ptr:" << (long)ptr << " sz:" << (long)sz, Storage::Current());
		}
	}
	static void AllocatorNewDelete::operator delete(void *ptr, Allocator *a) throw()
	{
		if (ptr && a) {
			void *realPtr = (char *) ptr - sizeof(Allocator *);
			assert(((Allocator **) realPtr)[0L] == a);
			size_t sz(a->Free(realPtr));
			StatTrace(AllocatorNewDelete.delete, " a:" << (long)a << " ptr:" << (long)ptr << " sz:" << (long)sz, Storage::Current());
		}
	}

	AllocatorNewDelete::~AllocatorNewDelete() {}

}
