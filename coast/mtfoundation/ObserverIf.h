/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ObserverIf_H
#define _ObserverIf_H

#include "config_mtfoundation.h"
//#include "lokiThreads.h"
#include "Threads.h"
#include <list>

//---- ObserverIf ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
template
<
typename TArgs
>
class EXPORTDECL_MTFOUNDATION Observable
{
	friend class ThreadPoolTest;
	SimpleMutex fObserversMutex;
public:
	typedef Observable *tObservedPtr;
	typedef TArgs tArgsRef;

	class EXPORTDECL_MTFOUNDATION Observer
	{
	public:
		//--- constructors
		Observer() {}
		virtual ~Observer() {}

		/*! interface which must be implemented if you want to be notified when objects's state changes
			\param aObserved the object to be observed
			\param aArgs arguments that describe the change */
		virtual void Update(tObservedPtr pObserved, tArgsRef aUpdateArgs) = 0;
	};

	typedef Observer *tObserverPtr;
	typedef std::list<tObserverPtr> tObserverList;
	typedef typename tObserverList::iterator tObserverListIterator;
	typedef typename tObserverList::value_type tObserverListValueType;

	//--- constructors
	Observable(const char *name, Allocator *a = Storage::Global())
		: fObserversMutex(name, a)
	{}
	virtual ~Observable() {}

	//! adds Observer that gets notified
	bool AddObserver(tObserverPtr pObserver) {
		SimpleMutexEntry aEntry(fObserversMutex);
		aEntry.Use();
		return DoAddObserver(pObserver);
	}

protected:
	//! hook to let the observers know that something has changed
	void NotifyAll(tArgsRef aUpdateArgs) {
		SimpleMutexEntry aEntry(fObserversMutex);
		aEntry.Use();
		tObserverListIterator myIter;
		for ( myIter = fObserversList.begin(); myIter != fObserversList.end(); ++myIter ) {
			tObserverPtr pObserver = *myIter;
			pObserver->Update(this, aUpdateArgs);
		}
	}

	virtual bool DoAddObserver(tObserverPtr pObserver) {
		fObserversList.push_back(pObserver);
		return true;
	}

private:
	tObserverList fObserversList;
};

#endif
