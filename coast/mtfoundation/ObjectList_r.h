/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ObjectList_r_H
#define _ObjectList_r_H

//---- standard-module include ----------------------------------------------
#include "SysLog.h"
#include "Threads.h"

//---- baseclass include -----------------------------------------------------
#include "ObjectList.h"

//---- c-module include -----------------------------------------------------

//---- class ObjectList_r ----------------------------------------------------------------

template<typename Tp>
class ObjectList_r : public ObjectList<Tp>
{
public:
	ObjectList_r(const char *name, Allocator *a = Storage::Global())
		: ObjectList<Tp>(name, a)
		, fMutex(name, a)
	{}

	virtual ~ObjectList_r() {
		SimpleMutexEntry me(fMutex);
		fShutdown = true;
		if ( fDestructiveShutdown == false ) {
			// let the workers empty the list and destruct its elements
			while ( !this->empty() ) {
				fCondEmpty.Wait(fMutex);
			}
		}
		// if the list was in destructive shutdown, let the baseclass remove and destruct the elements
	}

	virtual bool InsertTail(const typename list<Tp>::value_type &newObjPtr) {
		SimpleMutexEntry me(fMutex);
		if ( !IsShuttingDown() && ObjectList<Tp>::InsertTail(newObjPtr) ) {
			fCondFull.BroadCast();
			return true;
		}
		return false;
	}

	/*! removes the head element of the list
		\param aElement reference to a receiving element, depending on the type (pointer, element) an assignment operator of the element is required!
		\param lSecs time in seconds after which the wait will be aborted, specify 0 for endless wait
		\param lNanosecs time in nanoseconds (10e-9) after which the wait will be aborted, specify 0 for endless wait
		\return true only when an element could be get, false in case the list was empty, we are in shutdown mode or a timeout occured */
	virtual bool RemoveHead(typename list<Tp>::reference aElement, long lSecs = 0L, long lNanosecs = 0L) {
		SimpleMutexEntry me(fMutex);
		while ( !IsShuttingDown() && this->empty() ) {
			if ( fCondFull.TimedWait(fMutex, lSecs, lNanosecs) == TIMEOUTCODE ) {
				// prematurely exit in case of a timeout
				return false;
			}
		}
		if ( !IsShuttingDown() && ObjectList<Tp>::RemoveHead(aElement) ) {
			fCondEmpty.Signal();
			return true;
		}
		return false;
	}

	/*! returns the current number of elements in the list
		/return number of elements */
	virtual size_t GetSize() {
		size_t tmpSz = 0;
		{
			SimpleMutexEntry me(fMutex);
			tmpSz = IntGetSize();
		}
		return tmpSz;
	}

	/*! Method to be called to signal that we want to destroy the list. If the elements should still be consumed by the workers reading from the list false should be passed as argument. If a 'destructive' shutdown is wanted true should be passed.
		\param bDestructive set to true if the list should not be emptied by the workers, otherways the call blocks until the list empty */
	virtual void SignalShutdown(bool bDestructive = false) {
		{
			SimpleMutexEntry me(fMutex);
			if ( bDestructive == false ) {
				// wait for workerThreads to drain the list
				while (this->size() > 0) {
					fCondEmpty.Wait(fMutex);
				}
			}
			ObjectList<Tp>::SignalShutdown(bDestructive);
		}
		// wake up the workerThreads such that they recheck the shutdown flag
		fCondFull.BroadCast();
	}

private:
	//!standard copy constructor prohibited
	ObjectList_r(const ObjectList_r<Tp> &);
	//!standard assignement operator prohibited
	void operator=(const ObjectList_r<Tp> &);

	SimpleMutex		fMutex;
	SimpleCondition	fCondEmpty;
	SimpleCondition	fCondFull;
};

#endif
