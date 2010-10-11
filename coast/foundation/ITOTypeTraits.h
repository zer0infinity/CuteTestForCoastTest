/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ITOTypeTraits_H
#define _ITOTypeTraits_H

//---- standard-module include ----------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION

#if defined(USE_TR1)
#include <tr1/type_traits>
namespace boost_or_tr1 = std::tr1;
#elif defined(USE_STD0X)
#include <type_traits>
namespace boost_or_tr1 = std;
#else
#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>
namespace boost_or_tr1 = boost;
#endif


namespace Coast
{
	namespace TypeTraits
	{
		struct NullType {};

	    template <int v>
	    struct Int2Type {
	        enum { value = v };
	    };

		template <typename T>
		class TypeTraits
		{
		private:
			template <class U> struct PointerTraits {
				enum { result = false };
				typedef NullType PointeeType;
			};

			template <class U> struct PointerTraits<U *> {
				enum { result = true };
				typedef U PointeeType;
			};

			template <class U> struct PointerTraits<U* &> {
				enum { result = true };
				typedef U PointeeType;
			};

			template <class U> struct UnConst {
				typedef U Result;
				enum { isConst = 0 };
			};

			template <class U> struct UnConst<const U &> {
				typedef U &Result;
				enum { isConst = 1 };
			};

			template <class U> struct UnConst<const U> {
				typedef U Result;
				enum { isConst = 1 };
			};

		public:
			typedef typename UnConst<T>::Result NonConstType;
			enum { isConst  = UnConst<T>::isConst };
			typedef typename PointerTraits<T>::PointeeType PointeeType;
			enum { isPointer = PointerTraits<T>::result };
		};

		template <typename T>
		class fooTypeTraits
		{
		private:
			template <class U> struct PlainTypeGetter {
				typedef U Result;
			};

			template <class U> struct PlainTypeGetter<U *> {
				typedef U Result;
			};

			template <class U> struct PlainTypeGetter<U &> {
				typedef U Result;
			};

			template <class U> struct PlainTypeGetter<const U> {
				typedef U Result;
			};

			template <class U> struct PlainTypeGetter<const U *> {
				typedef U Result;
			};

			template <class U> struct PlainTypeGetter<const U &> {
				typedef U Result;
			};

			template <class U> struct ConstCorrectRef2PtrGetter {
				typedef U *Result;
			};

			template <class U> struct ConstCorrectRef2PtrGetter<U &> {
				typedef U *Result;
			};

			template <class U> struct ConstCorrectRef2PtrGetter<U *> {
				typedef NullType Result;
			};

			template <class U> struct ConstCorrectPtr2RefGetter {
				typedef U &Result;
			};

			template <class U> struct ConstCorrectPtr2RefGetter<U *> {
				typedef U &Result;
			};

			template <class U> struct ConstCorrectPtr2RefGetter<U &> {
				typedef NullType Result;
			};

		public:
			typedef typename PlainTypeGetter< typename boost_or_tr1::remove_const<T>::type>::Result PlainType;
			typedef const typename PlainTypeGetter< typename boost_or_tr1::remove_const<T>::type>::Result ConstPlainType;
			typedef typename PlainTypeGetter< typename boost_or_tr1::remove_const<T>::type>::Result &PlainTypeRef;
			typedef const typename PlainTypeGetter< typename boost_or_tr1::remove_const<T>::type>::Result &ConstPlainTypeRef;
			typedef typename PlainTypeGetter< typename boost_or_tr1::remove_const<T>::type>::Result *PlainTypePtr;
			typedef const typename PlainTypeGetter< typename boost_or_tr1::remove_const<T>::type>::Result *ConstPlainTypePtr;
			typedef typename ConstCorrectPtr2RefGetter< typename boost_or_tr1::remove_const<T>::type >::Result ConstCorrectPtr2RefType;
			typedef typename ConstCorrectRef2PtrGetter< typename boost_or_tr1::remove_const<T>::type >::Result ConstCorrectRef2PtrType;
		};
	}
};

#endif
