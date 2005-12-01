/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ObjectList_H
#define _ObjectList_H

//---- standard-module include ----------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "ITOString.h"
#include "Dbg.h"
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

		template <class U> struct PointerTraits<U *> {
			enum { result = true };
			typedef U PointeeType;
		};

	public:
		enum { isPointer = PointerTraits<T>::result };
	};
};
#endif

//---- c-module include -----------------------------------------------------
#include <list>

//---- class ObjectList ----------------------------------------------------------------

template<typename Tp>
class EXPORTDECL_FOUNDATION ObjectList : public std::list<Tp>
{
	// tricky section to determine if given type is a pointer and deletable
	// ideas from Alexandrescu, loki-lib
	enum DeleteFuncSelector { Reftype, Pointertype };

	void DoDeleteObject(const typename std::list<Tp>::value_type &newObjPtr, Loki::Int2Type<Reftype> ) {};
	void DoDeleteObject(const typename std::list<Tp>::value_type &newObjPtr, Loki::Int2Type<Pointertype> ) {
		delete newObjPtr;
	};

public:
	ObjectList(const char *name, Allocator *a = Storage::Global())
		: fShutdown(false)
		, fDestructiveShutdown(false)
		, fpAlloc(a)
		, fName(name, -1, fpAlloc)
	{}

	ObjectList(const ObjectList<Tp> &aList)
		: std::list<Tp>(aList)
		, fShutdown(aList.fShutdown)
		, fDestructiveShutdown(false)	// set to false not to accidentally delete an element twice
		, fpAlloc(aList.fpAlloc ? aList.fpAlloc : Storage::Global())
		, fName(aList.fName, -1, fpAlloc) {
	}

	virtual ~ObjectList() {
		StartTrace1(ObjectList.~ObjectList, (fDestructiveShutdown ? "destructive" : ""));
		if ( fDestructiveShutdown ) {
			// no more workers waiting and capable of emptying the list
			// we have to delete elements if they were pointers, let an appropriate function do this work
			enum { delAlgo = (Loki::TypeTraits<Tp>::isPointer) ? Pointertype : Reftype };
			while ( IntGetSize() > 0 ) {
				Tp pElement;
				IntRemoveHead(pElement);
				DoDeleteObject(pElement, Loki::Int2Type<delAlgo>() );
			}
		}
	}

	bool InsertTail(const typename std::list<Tp>::value_type &newObjPtr) {
		return DoInsertTail(newObjPtr);
	}

	/*! removes the head element of the list
		\param aElement reference to a receiving element, depending on the type (pointer, element) an assignment operator of the element is required!
		\param lSecs unused
		\param lNanosecs unused
		\return true only when an element could be get, false in case the list was empty or we are in shutdown mode */
	bool RemoveHead(typename std::list<Tp>::reference aElement, long lSecs = 0L, long lNanosecs = 0L) {
		return DoRemoveHead(aElement, lSecs, lNanosecs);
	}

	size_t GetSize() const {
		return DoGetSize();
	}

	bool IsEmpty() const {
		return DoIsEmpty();
	}

	/*! Method to be called to signal that we want to destroy the list. If the elements should still be consumed by the workers reading from the list false should be passed as argument. If a 'destructive' shutdown is wanted true should be passed.
		\param bDestructive set to true if the list should not be emptied by the workers and not accessed further */
	void SignalShutdown(bool bDestructive = false) {
		DoSignalShutdown(bDestructive);
	}

	bool IsShuttingDown() const {
		return fShutdown;
	}

	void ResetAfterShutdown() {
		fShutdown = false;
		fDestructiveShutdown = false;
	}

protected:
	/*! removes the head element of the list
		\param aElement reference to a receiving element, depending on the type (pointer, element) an assignment operator of the element is required!
		\param lSecs unused
		\param lNanosecs unused
		\return true only when an element could be get, false in case the list was empty or we are in shutdown mode */
	virtual bool DoRemoveHead(typename std::list<Tp>::reference aElement, long lSecs = 0L, long lNanosecs = 0L) {
		StartTrace(ObjectList.DoRemoveHead);
		if ( !IsShuttingDown() && !IntIsEmpty() ) {
			IntRemoveHead(aElement);
			Trace("element removed");
			return true;
		}
		Trace("in shutdown or empty list");
		return false;
	}

	virtual bool DoInsertTail(const typename std::list<Tp>::value_type &newObjPtr) {
		StartTrace(ObjectList.DoInsertTail);
		if ( !IsShuttingDown() ) {
			push_back(newObjPtr);
			Trace("pushing object");
			return true;
		}
		Trace("in shutdown");
		return false;
	}

	/*! Method to be called to signal that we want to destroy the list. If the elements should still be consumed by the workers reading from the list false should be passed as argument. If a 'destructive' shutdown is wanted true should be passed.
		\param bDestructive set to true if the list should not be emptied by the workers and not accessed further */
	virtual void DoSignalShutdown(bool bDestructive = false) {
		StartTrace1(ObjectList.DoSignalShutdown, (bDestructive ? "destructive" : ""));
		fDestructiveShutdown = bDestructive;
		fShutdown = true;
	}

	virtual size_t DoGetSize() const {
		return IntGetSize();
	}
	virtual bool DoIsEmpty() const {
		return IntIsEmpty();
	}

	bool			fShutdown;
	bool			fDestructiveShutdown;

private:
	size_t IntGetSize() const {
		return this->size();
	}
	bool IntIsEmpty() const {
		return this->empty();
	}

	void IntRemoveHead(typename std::list<Tp>::reference aElement) {
		aElement = this->front();
		this->pop_front();
	}

	//!standard assignement operator prohibited
	void operator=(const ObjectList<Tp> &);

	Allocator		*fpAlloc;
	String			fName;
};

#endif
