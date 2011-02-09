/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _STL_fast_pool_allocator_H
#define _STL_fast_pool_allocator_H

//---- STL_fast_pool_allocator ----------------------------------------------------------
namespace STLStorage
{
	// use fast_pool_allocator type for std::vector container types which allocate a bunch of elements at once
	template <
	typename T,
			 typename UserAllocator = BoostPoolUserAllocatorGlobal,
			 unsigned NextSize = 32 >
	class fast_pool_allocator
	{
	public:
		typedef T value_type;
		typedef UserAllocator user_allocator;
		BOOST_STATIC_CONSTANT(unsigned, next_size = NextSize);

		typedef pool_refcounted<UserAllocator> pool_refcounted_type;
		typedef typename pool_refcounted_type::pool_refcount_storer int_pool_instance_type;

		typedef value_type *pointer;
		typedef const value_type *const_pointer;
		typedef value_type &reference;
		typedef const value_type &const_reference;
		typedef typename pool_refcounted_type::int_pool_type pool_type;
		typedef typename pool_type::size_type size_type;
		typedef typename pool_type::difference_type difference_type;
		typedef fast_pool_allocator<T, UserAllocator, NextSize> ThisType;

		template <typename U>
		struct rebind {
			typedef fast_pool_allocator<U, UserAllocator, NextSize> other;
		};

	private:
		ThisType &operator=(const ThisType &);

	public:
		/* constructors and destructor
		*/
		fast_pool_allocator()
			: fpIntPool() {
			_StartTrace1(fast_pool_allocator.fast_pool_allocator, "this:" << (long)this << " sizeof(T):" << (long)sizeof(T));
			void *pMem = (void *)UserAllocator::malloc(sizeof(pool_refcounted_type));
			fpIntPool = int_pool_instance_type( new (pMem) pool_refcounted_type(sizeof(T), NextSize) );
			_Trace("IntPool @" << (long)GetImplRef(fpIntPool));
		}

		fast_pool_allocator(const ThisType &aAllocator)
			: fpIntPool(aAllocator.fpIntPool) {
			_StartTrace1(fast_pool_allocator.fast_pool_allocator, "copy this:" << (long)this << " sizeof(T):" << (long)sizeof(T) << " other:" << (long)&aAllocator << " sizeof(other):" << (long)sizeof(typename ThisType::value_type));
			_Trace("IntPool @" << (long)GetImplRef(fpIntPool));
		}

		template <typename U>
		fast_pool_allocator(const fast_pool_allocator<U, UserAllocator, NextSize>& aAllocator)
			: fpIntPool() {
			_StartTrace1(fast_pool_allocator.fast_pool_allocator, "copy other type this:" << (long)this << " sizeof(T):" << (long)sizeof(T) << " other:" << (long)&aAllocator << " sizeof(U):" << (long)sizeof(typename pool_allocator<U, UserAllocator, NextSize>::value_type));
			if ( GetImplRef(aAllocator.fpIntPool) ) {
				fpIntPool = aAllocator.fpIntPool->Clone(sizeof(T), NextSize);
			} else {
				_Trace("no pool created for other type ??");
				void *pMem = (void *)UserAllocator::malloc(sizeof(pool_refcounted_type));
				fpIntPool = int_pool_instance_type( new (pMem) pool_refcounted_type(sizeof(T), NextSize) );
			}
			_Trace("IntPool @" << (long)GetImplRef(fpIntPool));
		}

		~fast_pool_allocator() {
			_StatTrace(fast_pool_allocator.~fast_pool_allocator, "this:" << (long)this << " IntPool:" << (long)GetImplRef(fpIntPool), Coast::Storage::Current());
		}

		static pointer address(reference r) {
			return &r;
		}
		static const_pointer address(const_reference s) {
			return &s;
		}
		static size_type max_size() {
			return (std::numeric_limits<size_type>::max)();
		}
		void construct(const pointer ptr, const value_type &t) {
			new (ptr) T(t);
		}
		void destroy(const pointer ptr) {
			ptr->~T();
			(void) ptr; // avoid unused variable warning
		}

		bool operator==(const fast_pool_allocator &) const {
			return true;
		}
		bool operator!=(const fast_pool_allocator &) const {
			return false;
		}

		pointer allocate(const size_type n) throw( std::bad_alloc ) {
			const pointer ptr =
				(n == 1) ?
				static_cast<pointer>( fpIntPool->malloc() ) :
				static_cast<pointer>( fpIntPool->ordered_malloc(n) );
			_StatTrace(fast_pool_allocator.allocate_n, "this:" << (long)this << " ptr:" << (long)ptr << " size:" << (long)n << '*' << (long)sizeof(T) << " calling " << ( (n == 1) ? "" : "ordered_" ) << "malloc" << " fpIntPool:" << (long)GetImplRef(fpIntPool), Coast::Storage::Current());
			if (ptr == 0) {
				throw std::bad_alloc();
			}
			return ptr;
		}
		pointer allocate(const size_type n, const void *const) throw( std::bad_alloc ) {
			_StatTrace(fast_pool_allocator.allocate_n_void, "this:" << (long)this << " size:" << (long)n << '*' << (long)sizeof(T) << " void*", Coast::Storage::Current());
			return allocate(n);
		}
		pointer allocate() throw( std::bad_alloc ) {
			const pointer ptr = static_cast<pointer>( fpIntPool->malloc() );
			_StatTrace(fast_pool_allocator.allocate, "this:" << (long)this << " ptr:" << (long)ptr << " default size:1*" << (long)sizeof(T) << " fpIntPool:" << (long)fpIntPool.operator->(), Coast::Storage::Current());
			if (ptr == 0) {
				throw std::bad_alloc();
			}
			return ptr;
		}
		void deallocate(const pointer ptr, const size_type n) {
			_StatTrace(fast_pool_allocator.deallocate_ptr_n, "this:" << (long)this << " ptr:" << (long)ptr << " size:" << (long)n << '*' << (long)sizeof(T) << " calling free " << ( (n == 1) ? "" : "_n" ) << " fpIntPool:" << (long)GetImplRef(fpIntPool), Coast::Storage::Current());
#ifdef BOOST_NO_PROPER_STL_DEALLOCATE
			if (ptr == 0 || n == 0) {
				return;
			}
#endif
			if (n == 1) {
				fpIntPool->free(ptr);
			} else {
				fpIntPool->free(ptr, n);
			}
		}
		void deallocate(const pointer ptr) {
			_StatTrace(fast_pool_allocator.deallocate_ptr, "this:" << (long)this << " ptr:" << (long)ptr << " size:1*" << (long)sizeof(T) << " calling free" << " fpIntPool:" << (long)GetImplRef(fpIntPool), Coast::Storage::Current());
			fpIntPool->free(ptr);
		}
		int_pool_instance_type fpIntPool;
	};

	template < typename T > struct fast_pool_allocator_global : public STLStorage::fast_pool_allocator<T, BoostPoolUserAllocatorGlobal, 32 > {};
	template < typename T > struct fast_pool_allocator_current : public STLStorage::fast_pool_allocator<T, BoostPoolUserAllocatorCurrent, 32 > {};
}

#endif
