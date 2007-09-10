/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _STLStorage_H
#define _STLStorage_H

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Dbg.h"

#include <exception>

// std::numeric_limits
#include <boost/limits.hpp>
// new, std::bad_alloc
#include <new>

// boost::singleton_pool
#include <boost/pool/pool.hpp>
#include <boost/detail/workaround.hpp>

#include <boost/shared_ptr.hpp>

// The following code will be put into Boost.Config in a later revision
#if defined(_RWSTD_VER) || defined(__SGI_STL_PORT) || \
    BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
#define BOOST_NO_PROPER_STL_DEALLOCATE
#endif

//---- STLStorage ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
namespace STLStorage
{

	template <class T>
	class STLAllocator
	{
	public:
		// type definitions
		typedef T        value_type;
		typedef T       *pointer;
		typedef const T *const_pointer;
		typedef T       &reference;
		typedef const T &const_reference;
		typedef std::size_t    size_type;
		typedef std::ptrdiff_t difference_type;

		// rebind allocator to type U
		template <class U>
		struct rebind {
			typedef STLAllocator<U> other;
			rebind() {
				StartTrace(STLAllocator.rebind);
			}
			~rebind() {
				StartTrace(STLAllocator.~rebind);
			}
		};

		class fAllocatorNotInitialized : public std::exception
		{
		public:
			virtual const char *what() const throw() {
				static const char pMsg[] = "FATAL: fAllocator passed is NULL!!";
				SYSERROR(pMsg);
				return pMsg;
			}
		};

		// return address of values
		pointer address (reference value) const {
			StartTrace1(STLAllocator.address, "reference value");
			return &value;
		}

		const_pointer address (const_reference value) const {
			StartTrace1(STLAllocator.address, "const_reference value");
			return &value;
		}

		/* constructors and destructor
		* - initialize fAllocator to requested storage
		*/
		STLAllocator(Allocator *pAlloc = Storage::Current()) throw(fAllocatorNotInitialized)
			: fAllocator(pAlloc) {
			StartTrace(STLAllocator.STLAllocator);
			if ( !fAllocator ) {
				throw fAllocatorNotInitialized();
			}
		}

		STLAllocator(const STLAllocator &aAllocator) throw(fAllocatorNotInitialized)
			: fAllocator(aAllocator.fAllocator) {
			StartTrace1(STLAllocator.STLAllocator, "copy");
			if ( !fAllocator ) {
				throw fAllocatorNotInitialized();
			}
		}

		template <class U>
		STLAllocator (const STLAllocator<U>& aAllocator) throw(fAllocatorNotInitialized)
			: fAllocator(aAllocator.fAllocator) {
			StartTrace1(STLAllocator.STLAllocator, "copy other type");
			if ( !fAllocator ) {
				throw fAllocatorNotInitialized();
			}
		}

		~STLAllocator() {
			StartTrace(STLAllocator.~STLAllocator);
		}

		STLAllocator &operator=(const STLAllocator &aAllocator) {
			fAllocator = aAllocator.fAllocator;
			return (*this);
		}

		// return maximum number of elements that can be allocated
		size_type max_size () const throw() {
			//XXX in case of PoolAllocators, we could determine the max number somewhow
			size_type tSz = std::numeric_limits<std::size_t>::max() / sizeof(T);
			StatTrace(STLAllocator.max_size, "maximal size:" << (long)tSz, Storage::Current());
			return tSz;
		}

		// allocate but don't initialize num elements of type T
		pointer allocate (size_type num, const void* = 0) {
			// print message and allocate memory with global new
			StartTrace1(STLAllocator.allocate, "num:" << (long)num << " of size:" << (long)sizeof(T) << " but no initialization");
			pointer ret = (pointer)fAllocator->Calloc(num, sizeof(T));
			return ret;
		}

		// initialize elements of allocated storage p with value value
		void construct (pointer p, const T &value) {
			StartTrace1(STLAllocator.construct, "placement new");
			// initialize memory with placement new
			new((void *)p)T(value);
		}

		// destroy elements of initialized storage p
		void destroy (pointer p) {
			// destroy objects by calling their destructor
			p->~T();
		}

		// deallocate storage p of deleted elements
		void deallocate (pointer p, size_type num) {
			// print message and deallocate memory with global delete
			StartTrace1(STLAllocator.deallocate, "num:" << (long)num << " of size:" << (long)sizeof(T));
			fAllocator->Free((void *)p);
		}

		Allocator *fAllocator;
	};

	// return that all specializations of this allocator are interchangeable
	template <class T1, class T2>
	bool operator== (const STLAllocator<T1>& left, const STLAllocator<T2>& right) throw()
	{
		StartTrace1(STLAllocator.operator == , "equal comparing two types");
		// compare internal fAllocator member
		return left.fAllocator == right.fAllocator;
	}
	template <class T1, class T2>
	bool operator!= (const STLAllocator<T1>& left, const STLAllocator<T2>& right) throw()
	{
		StartTrace1(STLAllocator.operator != , "unequal comparing two types");
		// compare internal fAllocator member
		return left.fAllocator != right.fAllocator;
	}

	// adaption of Storage::Global / Storage::Current for boost::poolXXX usage
	struct BoostPoolUserAllocatorGlobal {
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		static char *malloc(const size_type bytes);
		static void free(char *const block);
	};

	struct BoostPoolUserAllocatorCurrent {
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		static char *malloc(const size_type bytes);
		static void free(char *const block);
	};
}

#include "STL_pool_allocator.h"
#include "STL_fast_pool_allocator.h"

#endif
