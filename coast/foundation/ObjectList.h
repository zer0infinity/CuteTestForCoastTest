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
#include "ITOString.h"

//---- c-module include -----------------------------------------------------
#include <list>

//---- const ----------------------------------------------------------------
using namespace std;

class NullType {};

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
	typedef typename PointerTraits<T>::PointeeType PointeeType;
};

template<typename Tp>
class ObjectList : public list<Tp>
{
public:
	ObjectList(const char *name, Allocator *a = Storage::Global())
		: fShutdown(false)
		, fDestructiveShutdown(false)
		, fName(name, -1, a)
		, fpAlloc(a)
	{}

	virtual ~ObjectList() {
		if ( fDestructiveShutdown ) {
			// no more workers waiting and capable of emptying the list
			// we have to delete elements if they were pointers, let an appropriate callback function do this work... or have a memory leak
			if ( TypeTraits<Tp>::isPointer ) {
				while ( IntGetSize() > 0 ) {
					Tp pElement;
					IntRemoveHead(pElement);
					delete pElement;
				}
			}
		}
	}

	virtual bool InsertTail(const typename list<Tp>::value_type &newObjPtr) {
		if ( !IsShuttingDown() ) {
			push_back(newObjPtr);
			return true;
		}
		return false;
	}

	virtual bool RemoveHead(typename list<Tp>::reference aElement) {
		if ( !IsShuttingDown() && !this->empty() ) {
			IntRemoveHead(aElement);
			return true;
		}
		return false;
	}

	inline virtual size_t GetSize() {
		return IntGetSize();
	}

	/*! Method to be called to signal that we want to destroy the list. If the elements should still be consumed by the workers reading from the list false should be passed as argument. If a 'destructive' shutdown is wanted true should be passed.
		\param bDestructive set to true if the list should not be emptied by the workers and not accessed further */
	inline virtual void SignalShutdown(bool bDestructive = false) {
		fDestructiveShutdown = bDestructive;
		fShutdown = true;
	}

	inline virtual bool IsShuttingDown() const {
		return fShutdown;
	}

protected:
	virtual void IntRemoveHead(typename list<Tp>::reference aElement) {
		aElement = this->front();
		this->pop_front();
	}

	inline virtual size_t IntGetSize() const {
		return this->size();
	}

	bool			fShutdown;
	bool			fDestructiveShutdown;

private:
	//!standard copy constructor prohibited
	ObjectList(const ObjectList<Tp> &);
	//!standard assignement operator prohibited
	void operator=(const ObjectList<Tp> &);

	String			fName;
	Allocator		*fpAlloc;
};

#endif
