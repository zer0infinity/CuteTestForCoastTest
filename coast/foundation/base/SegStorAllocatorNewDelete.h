/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SegStorAllocatorNewDelete_H
#define _SegStorAllocatorNewDelete_H

#include "ITOStorage.h"//lint !e537

namespace Coast
{

//! Segregated Storage Allocator for operator new/delete
/*!
Caution: This class is only intended for AnyImpl's! Subclassing from one of the various AnyImpl
classes can lead to space leaks because of a possible size differences.
*/
	template<typename T>
	class SegStorAllocatorNewDelete {
	public:
		static void *operator new( size_t, Allocator *a ) {
			return a->Malloc<sizeof(T)>();
		}

		static void *operator new( size_t t ) {
			return operator new(t, Storage::Global());
		}

		static void operator delete( void *ptr, Allocator *a ) {
			a->Free<sizeof(T)>(ptr);
		}

		static void operator delete( void *ptr ) {
			operator delete(ptr, static_cast<T*>(ptr)->MyAllocator());
		}

		static void *operator new[]( size_t sz, Allocator *a ) {
			if (a) {
				void *ptr = a->Malloc(sz + Memory::AlignedSize<Allocator *>::value);
				Memory::allocatorFor(ptr) = a; // remember address of responsible Allocator
				return reinterpret_cast<char *>(ptr) + Memory::AlignedSize<Allocator *>::value; // needs cast because of pointer arithmetic
			}
			return a;
		}

		static void operator delete[]( void *ptr ) {
			void *realPtr = Memory::realPtrFor(ptr);
			Allocator *a = Memory::allocatorFor(realPtr);
			if (a) {
				Memory::safeFree(a, realPtr);
			} else {
				free(realPtr);
			}
		}
	};
}
#endif /* _SegStorAllocatorNewDelete_H */
