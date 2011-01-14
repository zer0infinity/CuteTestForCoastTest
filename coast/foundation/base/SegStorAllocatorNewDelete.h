/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SegStorAllocatorNewDelete_H
#define _SegStorAllocatorNewDelete_H

#include "ITOStorage.h"
#include "ITOTypeTraits.h"
#include <new>
#include <cassert>
#include "boost/pool/pool.hpp"
#include "boost/pool/detail/singleton.hpp"
#include "SystemLog.h"
#include <boost/function.hpp>

namespace Coast {

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
};

}
#endif /* _SegStorAllocatorNewDelete_H */
