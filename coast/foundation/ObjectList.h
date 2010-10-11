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
#include "ITOTypeTraits.h"
#include "STLStorage.h"

//---- c-module include -----------------------------------------------------
#include <algorithm>
#include <list>

//---- class ObjectList ----------------------------------------------------------------
#define DefaultListType std::list

template <
typename Tp,
		 template < typename, typename > class tListType = DefaultListType,
		 template < class > class STLAlloc = DefaultAllocatorGlobalType
		 >
class ObjectList : public tListType< Tp, STLAlloc< Tp > >
{
public:
	typedef STLAlloc< Tp > STLAllocatorType;
	typedef tListType< Tp, STLAllocatorType > ListType;
	typedef ObjectList<Tp, tListType, STLAlloc > ThisType;
	typedef typename ListType::reference ListTypeReference;
	typedef typename ListType::value_type ListTypeValueType;
	typedef typename ListType::const_iterator ListIterator;

private:
	//! we have to delete elements if they were pointers, let an appropriate function do this work
	struct DeleteWrapper {
		// tricky section to determine if given type is a pointer and deletable
		enum DeleteFuncSelector { Reftype, Pointertype };
		enum { delAlgo = (boost_or_tr1::is_pointer<Tp>::value) ? Pointertype : Reftype };

		void DoDeleteObject(const ListTypeValueType &newObjPtr, Coast::TypeTraits::Int2Type<Reftype> ) {};
		void DoDeleteObject(const ListTypeValueType &newObjPtr, Coast::TypeTraits::Int2Type<Pointertype> ) {
			StatTrace(ObjectList.DoDeleteObject, "deleting element:" << (long)newObjPtr, Storage::Current());
			delete newObjPtr;
		}
		void operator() (Tp pElement) {
			DoDeleteObject(pElement, Coast::TypeTraits::Int2Type<delAlgo>() );
		}
	};

	ObjectList();

	//!standard assignement operator prohibited
	ThisType &operator=(const ThisType &);
	ObjectList(const ThisType &);

public:
	ObjectList(const char *name, Allocator *a = Storage::Global())
		: ListType()
		, fpAlloc(a)
		, fName(name, -1, fpAlloc)
		, fShutdown(false)
		, fDestructiveShutdown(false) {
		StatTrace(ObjectList.ObjectList, "param ctor this:" << (long)this, Storage::Current());
	}

	virtual ~ObjectList() {
		StartTrace1(ObjectList.~ObjectList, (fDestructiveShutdown ? "destructive" : ""));
		if ( fDestructiveShutdown ) {
			// no more workers waiting and capable of emptying the list
			std::for_each(this->listptr()->begin(), this->listptr()->end(), DeleteWrapper());
			this->listptr()->clear();
		}
	}

	bool InsertTail(const ListTypeValueType &newObjPtr) {
		return DoInsertTail(newObjPtr);
	}

	/*! removes the head element of the list
		\param aElement reference to a receiving element, depending on the type (pointer, element) an assignment operator of the element is required!
		\param lSecs unused
		\param lNanosecs unused
		\return true only when an element could be get, false in case the list was empty or we are in shutdown mode */
	bool RemoveHead(ListTypeReference aElement, long lSecs = 0L, long lNanosecs = 0L) {
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
		return this->fShutdown;
	}

	void ResetAfterShutdown() {
		this->fShutdown = false;
		this->fDestructiveShutdown = false;
	}

protected:
	/*! removes the head element of the list
		\param aElement reference to a receiving element, depending on the type (pointer, element) an assignment operator of the element is required!
		\param lSecs unused
		\param lNanosecs unused
		\return true only when an element could be get, false in case the list was empty or we are in shutdown mode */
	virtual bool DoRemoveHead(ListTypeReference aElement, long lSecs = 0L, long lNanosecs = 0L) {
		StartTrace(ObjectList.DoRemoveHead);
		if ( !IsShuttingDown() && !IntIsEmpty() ) {
			IntRemoveHead(aElement);
			Trace("element removed");
			return true;
		}
		Trace("in shutdown or empty list");
		return false;
	}

	virtual bool DoInsertTail(const ListTypeValueType &newObjPtr) {
		StartTrace(ObjectList.DoInsertTail);
		if ( !IsShuttingDown() ) {
			this->listptr()->push_back(newObjPtr);
			Trace("pushing object, new size:" << (long)this->constlistptr()->size());
			return true;
		}
		Trace("in shutdown");
		return false;
	}

	/*! Method to be called to signal that we want to destroy the list. If the elements should still be consumed by the workers reading from the list false should be passed as argument. If a 'destructive' shutdown is wanted true should be passed.
		\param bDestructive set to true if the list should not be emptied by the workers and not accessed further */
	virtual void DoSignalShutdown(bool bDestructive = false) {
		StartTrace1(ObjectList.DoSignalShutdown, (bDestructive ? "destructive" : ""));
		this->fDestructiveShutdown = bDestructive;
		this->fShutdown = true;
	}

	virtual size_t DoGetSize() const {
		StatTrace(ObjectList.DoGetSize, "this:" << (long)this, Storage::Current());
		return IntGetSize();
	}
	virtual bool DoIsEmpty() const {
		return IntIsEmpty();
	}

private:
	size_t IntGetSize() const {
		StatTrace(ObjectList.IntGetSize, "size:" << (long)this->constlistptr()->size(), Storage::Current());
		return this->constlistptr()->size();
	}
	bool IntIsEmpty() const {
		return this->constlistptr()->empty();
	}

	void IntRemoveHead(ListTypeReference aElement) {
		aElement = this->listptr()->front();
		this->listptr()->pop_front();
	}

	ListType *listptr() {
		return this;
	}
	const ListType *constlistptr() const {
		return this;
	}

	Allocator *fpAlloc;
	String fName;

protected:
	bool fShutdown;
	bool fDestructiveShutdown;
};

#endif
