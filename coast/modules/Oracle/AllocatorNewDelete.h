/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ALLOCATORNEWDELETE_H_
#define ALLOCATORNEWDELETE_H_

#include "config_coastoracle.h"
#include "ITOStorage.h"
#include <new>

namespace Coast
{

	class EXPORTDECL_COASTORACLE AllocatorNewDelete
	{
	public:
		//! force creation of vtable to allow for dynamic_cast
		virtual ~AllocatorNewDelete() = 0;

		static void *operator new(size_t sz, Allocator *a);
		static void operator delete(void *ptr);
	private:
		//! disallow unintended creation of non-Allocator instances
		static void *operator new(size_t sz);
	};

}

#endif /* ALLOCATORNEWDELETE_H_ */
