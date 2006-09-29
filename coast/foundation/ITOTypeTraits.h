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

#if defined(WIN32)
#include "TypeTraits.h"
#else
class NullType {};

namespace Loki
{
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

		template <class U> struct PointerTraits<const U *> {
			enum { result = true };
			typedef U PointeeType;
		};

		template <class U> struct PointerTraits<U *> {
			enum { result = true };
			typedef U PointeeType;
		};

		template <class U> struct ReferenceTraits {
			enum { result = false };
			typedef NullType ReferredType;
		};

		template <class U> struct ReferenceTraits<const U &> {
			enum { result = true };
			typedef U ReferredType;
		};

		template <class U> struct ReferenceTraits<U &> {
			enum { result = true };
			typedef U ReferredType;
		};

		template <class U> struct UnConst {
			typedef U Result;
		};

		template <class U> struct UnConst<const U &> {
			typedef U &Result;
		};

		template <class U> struct UnConst<const U *> {
			typedef U *Result;
		};

		template <class U> struct UnConst<const U> {
			typedef U Result;
		};

		template <class U> struct PlainTypeGetter {
			enum { isConst = false };
			typedef U Result;
		};

		template <class U> struct PlainTypeGetter<U *> {
			enum { isConst = false };
			typedef U Result;
		};

		template <class U> struct PlainTypeGetter<U &> {
			enum { isConst = false };
			typedef U Result;
		};

		template <class U> struct PlainTypeGetter<const U> {
			enum { isConst = true };
			typedef U Result;
		};

		template <class U> struct PlainTypeGetter<const U *> {
			enum { isConst = true };
			typedef U Result;
		};

		template <class U> struct PlainTypeGetter<const U &> {
			enum { isConst = true };
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
		enum { isPointer = PointerTraits<T>::result };
		typedef typename PointerTraits<T>::PointeeType PointeeType;

		enum { isReference = ReferenceTraits<T>::result };
		typedef typename ReferenceTraits<T>::ReferredType ReferredType;

		enum { isConst = PlainTypeGetter<T>::isConst };
		typedef typename UnConst<T>::Result NonConstType;

		typedef typename PlainTypeGetter<T>::Result PlainType;
		typedef const typename PlainTypeGetter<T>::Result ConstPlainType;
		typedef typename PlainTypeGetter<T>::Result &PlainTypeRef;
		typedef const typename PlainTypeGetter<T>::Result &ConstPlainTypeRef;
		typedef typename PlainTypeGetter<T>::Result *PlainTypePtr;
		typedef const typename PlainTypeGetter<T>::Result *ConstPlainTypePtr;
		typedef typename ConstCorrectPtr2RefGetter<T>::Result ConstCorrectPtr2RefType;
		typedef typename ConstCorrectRef2PtrGetter<T>::Result ConstCorrectRef2PtrType;
	};
};
#endif

#endif
