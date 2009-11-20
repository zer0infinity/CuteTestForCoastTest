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
#include <cassert>

namespace Coast
{

	class EXPORTDECL_COASTORACLE AllocatorNewDelete
	{
	public:
		//! force creation of vtable to allow for dynamic_cast
		virtual ~AllocatorNewDelete() = 0;

		/*! operator used when placement new gets called
		 * @param sz size of memory block to allocate
		 * @param a Allocator used for allocating memory
		 */
		static void *operator new(std::size_t sz, Allocator *a) throw();
		/*! operator used when delete() gets called
		 * @param ptr memory block to delete
		 */
		static void operator delete(void *ptr) throw();
		/*! operator used when delete gets called within ctor of allocated class
		 * @param ptr memory block to delete
		 * @param a Allocator used for freeing memory
		 */
		static void operator delete(void *ptr, Allocator *a) throw();

	private:
		//! disallow unintended creation of non-Allocator instances
		static void *operator new(std::size_t sz) throw (std::bad_alloc);
		static void *operator new (std::size_t size, const std::nothrow_t &nothrow_constant) throw();
		static void operator delete (void *ptr, const std::nothrow_t &nothrow_constant) throw();
	};

}
#endif /* ALLOCATORNEWDELETE_H_ */
