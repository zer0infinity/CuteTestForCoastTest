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
#include "SystemLog.h"
#include "Dbg.h"
#include <cstring>
#include <cstdio>
#include <exception>

// std::numeric_limits
#include <boost/limits.hpp>
// new, std::bad_alloc
#include <new>

// boost::singleton_pool
#include <boost/pool/pool.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/intrusive_ptr.hpp>

// The following code will be put into Boost.Config in a later revision
#if defined(_RWSTD_VER) || defined(__SGI_STL_PORT) || \
    BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
#define BOOST_NO_PROPER_STL_DEALLOCATE
#endif

// disable tracing if requested, even if in COAST_TRACE mode, eg. performance tests
#if defined(COAST_TRACE) && 0
#define _StatTrace(trigger, msg, allocator) 	StatTrace(trigger, msg, allocator)
#define _StartTrace(trigger)					StartTrace(trigger)
#define _StartTrace1(trigger, msg)				StartTrace1(trigger, msg)
#define _Trace(msg)								Trace(msg);
#else
#define _StatTrace(trigger, msg, allocator)
#define _StartTrace(trigger)
#define _StartTrace1(trigger, msg)
#define _Trace(msg);
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
				_StartTrace(STLAllocator.rebind);
			}
			~rebind() {
				_StartTrace(STLAllocator.~rebind);
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
			_StartTrace1(STLAllocator.address, "reference value");
			return &value;
		}

		const_pointer address (const_reference value) const {
			_StartTrace1(STLAllocator.address, "const_reference value");
			return &value;
		}

		/* constructors and destructor
		* - initialize fAllocator to requested storage
		*/
		STLAllocator(Allocator *pAlloc = Coast::Storage::Current()) throw(fAllocatorNotInitialized)
			: fAllocator(pAlloc) {
			_StartTrace(STLAllocator.STLAllocator);
			if ( !fAllocator ) {
				throw fAllocatorNotInitialized();
			}
		}

		STLAllocator(const STLAllocator &aAllocator) throw(fAllocatorNotInitialized)
			: fAllocator(aAllocator.fAllocator) {
			_StartTrace1(STLAllocator.STLAllocator, "copy");
			if ( !fAllocator ) {
				throw fAllocatorNotInitialized();
			}
		}

		template <class U>
		STLAllocator (const STLAllocator<U>& aAllocator) throw(fAllocatorNotInitialized)
			: fAllocator(aAllocator.fAllocator) {
			_StartTrace1(STLAllocator.STLAllocator, "copy other type");
			if ( !fAllocator ) {
				throw fAllocatorNotInitialized();
			}
		}

		~STLAllocator() {
			_StartTrace(STLAllocator.~STLAllocator);
		}

		STLAllocator &operator=(const STLAllocator &aAllocator) {
			fAllocator = aAllocator.fAllocator;
			return (*this);
		}

		// return maximum number of elements that can be allocated
		size_type max_size () const throw() {
			//XXX in case of PoolAllocators, we could determine the max number somewhow
			size_type tSz = std::numeric_limits<std::size_t>::max() / sizeof(T);
			_StatTrace(STLAllocator.max_size, "maximal size:" << (long)tSz, Coast::Storage::Current());
			return tSz;
		}

		// allocate but don't initialize num elements of type T
		pointer allocate (size_type num, const void* = 0) {
			// print message and allocate memory with global new
			_StartTrace1(STLAllocator.allocate, "num:" << (long)num << " of size:" << (long)sizeof(T) << " but no initialization");
			pointer ret = (pointer)fAllocator->Calloc(num, sizeof(T));
			if ( ret == NULL ) {
				static char pMsg[255] = { 0 };
				snprintf(pMsg, 254, "STLAllocator::allocate failed to allocate memory of size %lub!\n", (unsigned long)num);
				SystemLog::WriteToStderr(pMsg, strlen(pMsg));
			}
			return ret;
		}

		// initialize elements of allocated storage p with value value
		void construct (pointer p, const T &value) {
			_StartTrace1(STLAllocator.construct, "placement new");
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
			fAllocator->Free((void *)p);
		}

		Allocator *fAllocator;
	};

	// return that all specializations of this allocator are interchangeable
	template <class T1, class T2>
	bool operator== (const STLAllocator<T1>& left, const STLAllocator<T2>& right) throw()
	{
		_StartTrace1(STLAllocator.operator == , "equal comparing two types");
		// compare internal fAllocator member
		return left.fAllocator == right.fAllocator;
	}
	template <class T1, class T2>
	bool operator!= (const STLAllocator<T1>& left, const STLAllocator<T2>& right) throw()
	{
		_StartTrace1(STLAllocator.operator != , "unequal comparing two types");
		// compare internal fAllocator member
		return left.fAllocator != right.fAllocator;
	}

	template < typename UserAllocator >
	class pool_refcounted
	{
		friend class STLStorageTest;
		typedef typename UserAllocator::size_type sz_type;
		typedef pool_refcounted<UserAllocator> ThisType;

		pool_refcounted();
		ThisType &operator=(const ThisType &);

		enum {
			nOthers = 4
		};
	public:
		typedef boost::intrusive_ptr<ThisType> pool_refcount_storer;
		typedef typename boost::pool<UserAllocator> int_pool_type;
		typedef int_pool_type *PointerType;
		typedef int_pool_type &ReferenceType;

		explicit pool_refcounted(const sz_type nrequested_size, const sz_type nnext_size)
			: fRefcount(0)
			, fReqSz(nrequested_size)
			, fNextSz(nnext_size)
			, fpPool(0)
			, fOtherPools() {
			_StatTrace(pool_refcounted.pool_refcounted, "sizeof:" << (long)sizeof(int_pool_type) << " @" << (long)this, Coast::Storage::Current());
			void *pMem = (void *)UserAllocator::malloc(sizeof(int_pool_type));
			fpPool = new (pMem) int_pool_type(nrequested_size, nnext_size);
		}

		~pool_refcounted() {
			_StartTrace1(pool_refcounted.~pool_refcounted, "refcnt:" << fRefcount << " fpPool:" << (long)fpPool << " @" << (long)this);
			if ( fpPool != NULL && fRefcount <= 0) {
				_Trace("calling release_memory");
				fpPool->release_memory();
				fpPool->~int_pool_type();
				UserAllocator::free((char *)fpPool);
			}
		}

		template < class P >
		P Clone() {
			_StartTrace1(pool_refcounted.Clone, "Clone<P> for size:" << (long)fReqSz << " @" << (long)this);
			AddRef();
			return this;
		}

		pool_refcount_storer Clone(sz_type nrequested_size, sz_type nnext_size) {
			_StartTrace1(pool_refcounted.Clone, "Clone for size:" << (long)nrequested_size << " @" << (long)this);
			if ( nrequested_size == fReqSz ) {
				_Trace("equal size, returning this");
				return pool_refcount_storer(this);
			} else {
				_Trace("find stored instance");
				bool bFound(false);
				int i(0), iFree(-1);
				for (; i < nOthers; ++i) {
					if ( ( fOtherPools[i].get() != NULL ) && ( fOtherPools[i]->PoolSize() == nrequested_size ) ) {
						bFound = true;
						break;
					}
					if ( iFree == -1 && ( fOtherPools[i].get() == 0 ) ) {
						iFree = i;
					}
				}
				if ( bFound ) {
					_Trace("found pool for size:"  << (long)nrequested_size << " @" << (long)fOtherPools[i].get());
					return fOtherPools[i];
				} else {
					_Trace("creating new pool for size:"  << (long)nrequested_size << " this->size:" << (long)fReqSz);
					void *pMem = (void *)UserAllocator::malloc(sizeof(ThisType));
					ThisType *pRet = new (pMem) ThisType(nrequested_size, nnext_size);
					if ( iFree >= 0 && iFree < nOthers ) {
						_Trace("storing new pool at idx:" << iFree << " @" << (long)pRet);
						fOtherPools[iFree] = pool_refcount_storer(pRet);
						return fOtherPools[iFree];
					}
				}
			}
			return pool_refcount_storer();
		}

		sz_type PoolSize() const {
			return fReqSz;
		}
		sz_type NextSize() const {
			return fNextSz;
		}

		void AddRef() {
			++fRefcount;
			_StatTrace(pool_refcounted.AddRef, "new refcnt:" << fRefcount << " @" << (long)this, Coast::Storage::Current());
		}

		bool Release() {
			--fRefcount;
			_StatTrace(pool_refcounted.Release, "new refcnt:" << fRefcount << " @" << (long)this, Coast::Storage::Current());
			return ( fRefcount <= 0 );
		}

		void *malloc() {
			// AddRef(); not needed, pool_refcounted are able to manage themselves without destroying pools in use
			return fpPool->malloc();
		}
		void *ordered_malloc(const sz_type n) {
			// AddRef(); not needed, pool_refcounted are able to manage themselves without destroying pools in use
			return fpPool->ordered_malloc(n);
		}
		void free(void *const chunk) {
			// Release(); not needed, pool_refcounted are able to manage themselves without destroying pools in use
			fpPool->free(chunk);
		}
		void free(void *const chunks, const sz_type n) {
			// Release(); not needed, pool_refcounted are able to manage themselves without destroying pools in use
			fpPool->free(chunks, n);
		}
		void ordered_free(void *const chunk) {
			// Release(); not needed, pool_refcounted are able to manage themselves without destroying pools in use
			fpPool->ordered_free(chunk);
		}
		void ordered_free(void *const chunks, const sz_type n) {
			// Release(); not needed, pool_refcounted are able to manage themselves without destroying pools in use
			fpPool->ordered_free(chunks, n);
		}

	private:
		int fRefcount;
		sz_type fReqSz, fNextSz;
		PointerType fpPool;
		pool_refcount_storer fOtherPools[nOthers];
	};
}

namespace boost
{
	template <typename T>
	inline void intrusive_ptr_add_ref(STLStorage::pool_refcounted<T>* p)
	{
		p->AddRef();
	}

	template <typename T>
	inline void intrusive_ptr_release(STLStorage::pool_refcounted<T>* p)
	{
		if (p->Release()) {
			p->STLStorage::pool_refcounted<T>::~pool_refcounted();
			T::free( reinterpret_cast<char *>(p) );
		}
	}
} // namespace boost

#include "STL_pool_allocator.h"
#include "STL_fast_pool_allocator.h"

#if defined(__GNUG__)  && ( __GNUC__ >= 4 )
#define DefaultAllocatorGlobalType STLStorage::fast_pool_allocator_global
#define DefaultAllocatorCurrentType STLStorage::fast_pool_allocator_current
#else
#define DefaultAllocatorGlobalType std::allocator
#define DefaultAllocatorCurrentType std::allocator
#endif

#endif
