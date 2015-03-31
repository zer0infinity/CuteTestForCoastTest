/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ObserverIf_H
#define _ObserverIf_H

#include <algorithm>
#include <list>

//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
template
<
typename TObservedType,
		 typename TArgs
		 >
class Observable
{
	friend class ThreadPoolTest;
	SimpleMutex fObserversMutex;
public:
	typedef TObservedType *tObservedPtr;
	typedef TArgs tArgsRef;

	class Observer
	{
	public:
		virtual ~Observer() {}
		/*! interface which must be implemented if you want to be notified when objects's state changes
			\param pObserved the object to be observed
			\param aUpdateArgs arguments that describe the change */
		virtual void Update(tObservedPtr pObserved, tArgsRef aUpdateArgs) = 0;
	};

	typedef Observer *tObserverPtr;
	typedef std::list<tObserverPtr> tObserverList;
	typedef typename tObserverList::iterator tObserverListIterator;
	typedef typename tObserverList::value_type tObserverListValueType;

	Observable(const char *name, Allocator *a = coast::storage::Global())
		: fObserversMutex(name, a)
		, fObserversList()
	{}
	virtual ~Observable() {}

	//! adds Observer that gets notified
	bool AddObserver(tObserverPtr pObserver) {
		LockUnlockEntry aEntry(fObserversMutex);
		return DoAddObserver(pObserver);
	}

protected:
	//! hook to let the observers know that something has changed
	void NotifyAll(tArgsRef aUpdateArgs) {
		LockUnlockEntry aEntry(fObserversMutex);
		std::for_each(fObserversList.begin(), fObserversList.end(), UpdateWrapper(static_cast<tObservedPtr>(this), aUpdateArgs));
	}

	virtual bool DoAddObserver(tObserverPtr pObserver) {
		fObserversList.push_back(pObserver);
		return true;
	}

private:
	struct UpdateWrapper {
		UpdateWrapper(tObservedPtr pObserved, tArgsRef aUpdateArgs)
			: fpObserved(pObserved)
			, fUpdateArgs(aUpdateArgs)
		{}
		void operator() (tObserverPtr pElement) {
			pElement->Update(fpObserved, fUpdateArgs);
		}
		tObservedPtr fpObserved;
		tArgsRef fUpdateArgs;
	};
	tObserverList fObserversList;
};

#endif
