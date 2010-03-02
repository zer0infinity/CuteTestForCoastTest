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

#include <boost/shared_ptr.hpp>

#include <loki/SmartPtr.h>
#include <loki/TypeTraits.h>

// The following code will be put into Boost.Config in a later revision
#if defined(_RWSTD_VER) || defined(__SGI_STL_PORT) || \
    BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
#define BOOST_NO_PROPER_STL_DEALLOCATE
#endif

// disable tracing if requested, even if in COAST_TRACE mode, eg. performance tests
#define WD_DISABLE_TRACE
#if !defined(COAST_TRACE) || defined(WD_DISABLE_TRACE)
#define _StatTrace(trigger, msg, allocator)
#define _StartTrace(trigger)
#define _StartTrace1(trigger, msg)
#define _Trace(msg);
#else
#define _StatTrace(trigger, msg, allocator) 	StatTrace(trigger, msg, allocator)
#define _StartTrace(trigger)					StartTrace(trigger)
#define _StartTrace1(trigger, msg)				StartTrace1(trigger, msg)
#define _Trace(msg)								Trace(msg);
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
		STLAllocator(Allocator *pAlloc = Storage::Current()) throw(fAllocatorNotInitialized)
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
			_StatTrace(STLAllocator.max_size, "maximal size:" << (long)tSz, Storage::Current());
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
			// print message and deallocate memory with global delete
			size_t sz(fAllocator->Free((void *)p));
			_StartTrace1(STLAllocator.deallocate, "num:" << (long)num << " of sizeof(T):" << (long)sizeof(T) << " sz:" << (long)sz);
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

////////////////////////////////////////////////////////////////////////////////
///  \class RefCountedWithFinalDestroy
///
///  \ingroup  SmartPointerOwnershipGroup
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Adapts intrusive reference counting to OwnershipPolicy-specific syntax
///  finally destroying owned object - Release returning true
////////////////////////////////////////////////////////////////////////////////

	template <class P>
	class RefCountedWithFinalDestroy
	{
	public:
		RefCountedWithFinalDestroy() {
			_StatTrace(RefCountedWithFinalDestroy.RefCountedWithFinalDestroy, "default sizeof(P):" << (long)sizeof(P), Storage::Current());
		}

		template <class U>
		RefCountedWithFinalDestroy(const RefCountedWithFinalDestroy<U>&) {
			_StatTrace(RefCountedWithFinalDestroy.RefCountedWithFinalDestroy, "copy other sizeof(P):" << (long)sizeof(P) << " sizeof(U):" << (long)sizeof(U), Storage::Current());
		}

		template <class U>
		static P Clone(U &val) {
			_StartTrace1(RefCountedWithFinalDestroy.Clone, "other sizeof(P):" << (long)sizeof(P) << " sizeof(U):" << (long)sizeof(U) << " @" << (long)val);
			P pRet(NULL);
			if (val != 0) {
				pRet = val->Clone<P>();
			}
			return pRet;
		}

		static P Clone(const P &val) {
			_StatTrace(RefCountedWithFinalDestroy.Clone, "@" << (long)val, Storage::Current());
			if (val != 0) {
				val->AddRef();
			}
			return val;
		}

		static bool Release(const P &val) {
			_StatTrace(RefCountedWithFinalDestroy.Release, "@" << (long)val, Storage::Current());
			if (val != 0) {
				return val->Release();
			}
			return false;
		}

		enum { destructiveCopy = false };

		//! nothing to swap because fRefcount is intrusively managed by pointee
		void Swap(RefCountedWithFinalDestroy &rhs) {
			_StatTrace(RefCountedWithFinalDestroy.Swap, "@" << (long)this << " <> " << (long)&rhs, Storage::Current());
		}
	};

////////////////////////////////////////////////////////////////////////////////
///  \class WDAllocatorStorage
///
///  \ingroup  SmartPointerStorageGroup
///  Implementation of the StoragePolicy used by SmartPtr.  Uses explicit call
///   to SPT's destructor followed by call to UserAllocator::free()
////////////////////////////////////////////////////////////////////////////////

	template <
	class SPT,
		  typename UserAllocator = STLStorage::BoostPoolUserAllocatorGlobal
		  >
	class WDAllocatorStorage
	{
	public:
		typedef SPT *StoredType;      /// the type of the pointee_ object
		typedef SPT *InitPointerType; /// type used to declare OwnershipPolicy type.
		typedef SPT *PointerType;     /// type returned by operator->
		typedef SPT &ReferenceType;   /// type returned by operator*

		WDAllocatorStorage() : pointee_(Default()) {
			_StatTrace(WDAllocatorStorage.WDAllocatorStorage, "default ctor", Storage::Current());
		}

		// The storage policy doesn't initialize the stored pointer
		//     which will be initialized by the OwnershipPolicy's Clone fn
		WDAllocatorStorage(const WDAllocatorStorage &) : pointee_(0) {
			_StatTrace(WDAllocatorStorage.WDAllocatorStorage, "copy ctor", Storage::Current());
		}

		template <class U>
		WDAllocatorStorage(const WDAllocatorStorage<U>&) : pointee_(0) {
			_StatTrace(WDAllocatorStorage.WDAllocatorStorage, "other type copy ctor", Storage::Current());
		}

		WDAllocatorStorage(const StoredType &p) : pointee_(p) {
			_StatTrace(WDAllocatorStorage.WDAllocatorStorage, "pointer ctor pointee:" << (long)p, Storage::Current());
		}

		PointerType operator->() const {
			return pointee_;
		}

		ReferenceType operator*() const {
			return *pointee_;
		}

		void Swap(WDAllocatorStorage &rhs) {
			std::swap(pointee_, rhs.pointee_);
		}

		// Accessors
		template <class F, class A>
		friend typename WDAllocatorStorage<F, A>::PointerType GetImpl(const WDAllocatorStorage<F, A>& sp);

		template <class F, class A>
		friend const typename WDAllocatorStorage<F, A>::StoredType &GetImplRef(const WDAllocatorStorage<F, A>& sp);

		template <class F, class A>
		friend typename WDAllocatorStorage<F, A>::StoredType &GetImplRef(WDAllocatorStorage<F, A>& sp);

	protected:
		// Destroys the data stored
		// (Destruction might be taken over by the OwnershipPolicy)
		void Destroy() {
			_StartTrace1(WDAllocatorStorage.Destroy, "pointee:" << (long)pointee_);
			if ( 0 != pointee_ ) {
				_Trace("destructing member");
				pointee_->~SPT();
				UserAllocator::free( (char *)pointee_ );
			}
		}

		// Default value to initialize the pointer
		static StoredType Default() {
			return 0;
		}

	private:
		// Data
		StoredType pointee_;
	};

	template <class F, class A>
	inline typename WDAllocatorStorage<F, A>::PointerType GetImpl(const WDAllocatorStorage<F, A>& sp)
	{
		return sp.pointee_;
	}

	template <class F, class A>
	inline const typename WDAllocatorStorage<F, A>::StoredType &GetImplRef(const WDAllocatorStorage<F, A>& sp)
	{
		return sp.pointee_;
	}

	template <class F, class A>
	inline typename WDAllocatorStorage<F, A>::StoredType &GetImplRef(WDAllocatorStorage<F, A>& sp)
	{
		return sp.pointee_;
	}

	template < typename T > struct WDAllocatorGlobalStorage : public WDAllocatorStorage<T, STLStorage::BoostPoolUserAllocatorGlobal > {
		typedef WDAllocatorStorage<T, STLStorage::BoostPoolUserAllocatorGlobal > BaseClassType;
		WDAllocatorGlobalStorage() : BaseClassType() {}
		WDAllocatorGlobalStorage(const WDAllocatorGlobalStorage &) : BaseClassType() {}
		template <class U>
		WDAllocatorGlobalStorage(const WDAllocatorGlobalStorage<U>&) : BaseClassType() {}
		WDAllocatorGlobalStorage(const typename BaseClassType::StoredType &p) : BaseClassType(p) {}
	};

	template < typename T > struct WDAllocatorCurrentStorage : public WDAllocatorStorage<T, STLStorage::BoostPoolUserAllocatorCurrent > {
		typedef WDAllocatorStorage<T, STLStorage::BoostPoolUserAllocatorCurrent > BaseClassType;
		WDAllocatorCurrentStorage() : BaseClassType() {}
		WDAllocatorCurrentStorage(const WDAllocatorCurrentStorage &) : BaseClassType() {}
		template <class U>
		WDAllocatorCurrentStorage(const WDAllocatorCurrentStorage<U>&) : BaseClassType() {}
		WDAllocatorCurrentStorage(const typename BaseClassType::StoredType &p) : BaseClassType(p) {}
	};

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
		typedef Loki::SmartPtr<ThisType, RefCountedWithFinalDestroy, Loki::DisallowConversion, Loki::AssertCheck, WDAllocatorCurrentStorage> pool_refcount_storer;
		typedef typename boost::pool<UserAllocator> int_pool_type;
		typedef int_pool_type *PointerType;
		typedef int_pool_type &ReferenceType;

		explicit pool_refcounted(const sz_type nrequested_size, const sz_type nnext_size)
			: fRefcount(1)
			, fReqSz(nrequested_size)
			, fNextSz(nnext_size)
			, fpPool(0)
			, fOtherPools() {
			_StatTrace(pool_refcounted.pool_refcounted, "sizeof:" << (long)sizeof(int_pool_type) << " @" << (long)this, Storage::Current());
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
				AddRef();
				return pool_refcount_storer(this);
			} else {
				_Trace("find stored instance");
				bool bFound(false);
				int i(0), iFree(-1);
				for (; i < nOthers; ++i) {
					if ( ( GetImplRef(fOtherPools[i]) != NULL ) && ( fOtherPools[i]->PoolSize() == nrequested_size ) ) {
						bFound = true;
						break;
					}
					if ( iFree == -1 && ( GetImplRef(fOtherPools[i]) == 0 ) ) {
						iFree = i;
					}
				}
				if ( bFound ) {
					_Trace("found pool for size:"  << (long)nrequested_size << " @" << (long)GetImplRef(fOtherPools[i]));
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
			_StatTrace(pool_refcounted.AddRef, "new refcnt:" << fRefcount << " @" << (long)this, Storage::Current());
		}

		bool Release() {
			--fRefcount;
			_StatTrace(pool_refcounted.Release, "new refcnt:" << fRefcount << " @" << (long)this, Storage::Current());
			return ( fRefcount <= 0 );
		}

		void *malloc() {
			// AddRef(); not needed, pool_refcounted are able to manage themselfes without destroying pools in use
			return fpPool->malloc();
		}
		void *ordered_malloc(const sz_type n) {
			// AddRef(); not needed, pool_refcounted are able to manage themselfes without destroying pools in use
			return fpPool->ordered_malloc(n);
		}
		void free(void *const chunk) {
			// Release(); not needed, pool_refcounted are able to manage themselfes without destroying pools in use
			fpPool->free(chunk);
		}
		void free(void *const chunks, const sz_type n) {
			// Release(); not needed, pool_refcounted are able to manage themselfes without destroying pools in use
			fpPool->free(chunks, n);
		}
		void ordered_free(void *const chunk) {
			// Release(); not needed, pool_refcounted are able to manage themselfes without destroying pools in use
			fpPool->ordered_free(chunk);
		}
		void ordered_free(void *const chunks, const sz_type n) {
			// Release(); not needed, pool_refcounted are able to manage themselfes without destroying pools in use
			fpPool->ordered_free(chunks, n);
		}

	private:
		int fRefcount;
		sz_type fReqSz, fNextSz;
		PointerType fpPool;
		pool_refcount_storer fOtherPools[nOthers];
	};
}

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
