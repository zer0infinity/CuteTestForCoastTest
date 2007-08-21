/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ObjectList_r_H
#define _ObjectList_r_H

//---- baseclass include -----------------------------------------------------
#include "config_mtfoundation.h"
#include "ObjectList.h"

//---- standard-module include ----------------------------------------------
#include "SysLog.h"
#include "Threads.h"

//---- c-module include -----------------------------------------------------

//---- class ObjectList_r ----------------------------------------------------------------

template<typename Tp>
class EXPORTDECL_MTFOUNDATION ObjectList_r : public ObjectList<Tp>
{
public:
	typedef ObjectList<Tp> BaseClass;
	typedef typename BaseClass::ListType ListType;
	typedef typename BaseClass::ListTypeReference ListTypeReference;
	typedef typename BaseClass::ListTypeValueType ListTypeValueType;

	ObjectList_r(const char *name, Allocator *a = Storage::Global())
		: BaseClass(name, a)
		, fMutex(name, a)
	{}

	virtual ~ObjectList_r() {
		StartTrace1(ObjectList_r.~ObjectList_r, (BaseClass::fDestructiveShutdown ? "destructive" : ""));
		LockUnlockEntry me(fMutex);
		BaseClass::fShutdown = true;
		if ( BaseClass::fDestructiveShutdown == false ) {
			// let the workers empty the list and destruct its elements
			while ( !BaseClass::DoIsEmpty() ) {
				fCondEmpty.Wait(fMutex);
			}
		}
		// if the list was in destructive shutdown, let the baseclass remove and destruct the elements
	}

private:
	/*! removes the head element of the list
		\param aElement reference to a receiving element, depending on the type (pointer, element) an assignment operator of the element is required!
		\param lSecs time in seconds after which the wait will be aborted, specify 0 for endless wait
		\param lNanosecs time in nanoseconds (10e-9) after which the wait will be aborted, specify 0 for endless wait
		\return true only when an element could be get, false in case the list was empty, we are in shutdown mode or a timeout occured */
	virtual bool DoRemoveHead(ListTypeReference aElement, long lSecs = 0L, long lNanosecs = 0L) {
		StartTrace1(ObjectList_r.DoRemoveHead, "sec:" << lSecs << " nano:" << lNanosecs);
		LockUnlockEntry me(fMutex);
		// wait on new element
		while ( !BaseClass::IsShuttingDown() && BaseClass::DoIsEmpty() ) {
			if ( fCondFull.TimedWait(fMutex, lSecs, lNanosecs) == TIMEOUTCODE ) {
				Trace("premature exit because of a timeout and empty list");
				return false;
			}
		}
		if ( BaseClass::DoRemoveHead(aElement) ) {
			fCondEmpty.Signal();
			Trace("got an element");
			return true;
		}
		Trace("in shutdown");
		return false;
	}

	/*! Method to be called to signal that we want to destroy the list. If the elements should still be consumed by the workers reading from the list false should be passed as argument. If a 'destructive' shutdown is wanted true should be passed.
		\param bDestructive set to true if the list should not be emptied by the workers, otherways the call blocks until the list empty */
	virtual void DoSignalShutdown(bool bDestructive = false) {
		StartTrace1(ObjectList_r.DoSignalShutdown, (bDestructive ? "destructive" : ""));
		{
			LockUnlockEntry me(fMutex);
			if ( bDestructive == false ) {
				Trace("waiting on workerThreads to drain the list");
				while ( !BaseClass::DoIsEmpty() ) {
					fCondEmpty.Wait(fMutex);
				}
			}
			BaseClass::DoSignalShutdown(bDestructive);
		}
		// wake up the workerThreads such that they recheck the shutdown flag
		fCondFull.BroadCast();
	}

	/*! returns the current number of elements in the list
		/return number of elements */
	virtual size_t DoGetSize() const {
		StartTrace(ObjectList_r.DoGetSize);
		size_t tmpSz = 0;
		{
			LockUnlockEntry me(const_cast<SimpleMutex &>(fMutex));
			tmpSz = BaseClass::DoGetSize();
		}
		Trace("current size:" << (long)tmpSz);
		return tmpSz;
	}

	virtual bool DoInsertTail(const ListTypeValueType &newObjPtr) {
		StartTrace(ObjectList_r.DoInsertTail);
		LockUnlockEntry me(fMutex);
		if ( BaseClass::DoInsertTail(newObjPtr) ) {
			fCondFull.BroadCast();
			Trace("success");
			return true;
		}
		Trace("failure");
		return false;
	}

	//!standard copy constructor prohibited
	ObjectList_r(const ObjectList_r<Tp> &);
	//!standard assignement operator prohibited
	void operator=(const ObjectList_r<Tp> &);

	SimpleMutex fMutex;
	SimpleMutex::ConditionType fCondEmpty;
	SimpleMutex::ConditionType fCondFull;
};

#endif
