/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _set_wrapper_H
#define _set_wrapper_H

//---- baseclass include -------------------------------------------------
#include <loki/Visitor.h>

#include "STLStorage.h"
#include <set>

//---- forward declaration -----------------------------------------------
class Anything;
class LockUnlockEntry;

//---- set_wrapper ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class DummyCondition;

class DummyMutex
{
	friend class DummyCondition;
public:
	typedef DummyCondition ConditionType;
	/*! create mutex with names to ease debugging of locking problems
		\param name a name to identify the mutex when tracing locking problems
		\param a allocator used to allocate the storage for the name */
	DummyMutex(const char *, Allocator *) {}
	~DummyMutex() {}

	/*! tries to acquire the mutex for this thread, blocks the caller if already locked (by another thread)
		acquires the mutex for the calling thread; this is NOT recursive, if you call lock twice for the same thread it deadlocks */
	void Lock() {}

	/*! releases the mutex */
	void Unlock() {}

	/*! tries to acquire the mutex if possible; bails out without locking if already locked
		\return false when already locked */
	bool TryLock() {
		return true;
	}

//	/*! returns native reprentation of mutex hidden by macro MUTEXPTR */
//	MUTEXPTR GetInternal() const {  return GETMUTEXPTR(fMutex); }

private:
	//!standard constructor prohibited
	DummyMutex();
	//!standard copy constructor prohibited
	DummyMutex(const DummyMutex &);
	//!standard assignement operator prohibited
	void operator=(const DummyMutex &);
};

class DummyCondition
{
public:
	typedef DummyMutex MutexType;
	//!creates a system dependent condition variable
	DummyCondition() {}
	//!destroys the system dependent condition variable
	~DummyCondition() {}

	/*! waits for the condition to be signaled atomicly releasing the mutex while waiting and reaquiring it when returning from the wait
		\param m the mutex which is used with the condition; it must be locked, since it will be unlocked
		\return system dependent return value; zero if call was ok */
	int Wait(MutexType &) {
		return 0;
	}

	/*! waits for the condition to be signaled at most secs seconds and nanosecs nanoseconds if available on the plattform. After this time we block on the SimpleMutex until we can lock it before leaving the function
		\param m the mutex which is used with the condition; it must be locked, since it will be unlocked
		\param secs timeout to wait in seconds
		\param nanosecs timeout to wait in nanoseconds (10e-9)
		\return system dependent return value; zero if call was ok */
	int TimedWait(MutexType &, long, long) {
		return 0;
	}

	//!access the system dependent handle to the condition variable
//	CONDPTR GetInternal() { return GETCONDPTR(fSimpleCondition); }
	//!signal the condition; do it while protected through the associated mutex, then unlock the mutex; zero or one waiting thread will be woken up
	int Signal() {
		return 0;
	}
	//!broadcasts the condition; do it while protected through the associated mutex, then unlock the mutex; all waiting thread will eventually be woken up
	int BroadCast() {
		return 0;
	}

private:
	//!get internal 'Id' mainly for Tracing
	long GetId() {
		return -1;
	}
};

template <
typename T,
		 class MutexPolicy = DummyMutex,
		 template < typename > class AllocType = DefaultAllocatorGlobalType
		 >
class set_wrapper : public Loki::BaseVisitable< Anything >
{
	set_wrapper(const set_wrapper &);

public:
	typedef std::less< T > CompareType;
	typedef T ItemType;
	typedef AllocType< ItemType > allocator_type;
	typedef std::set< T, CompareType, allocator_type > Type;
	typedef Type &TypeRef;
	typedef typename Type::size_type size_type;
	typedef std::pair< typename Type::iterator, bool > InsertResultType;
	typedef typename Type::iterator IteratorType;
	typedef typename Type::const_iterator ConstIteratorType;

	LOKI_DEFINE_VISITABLE()

	set_wrapper( const char *pName = "set_wrapper_lock" )
		: fpList()
		, fLock( pName, Storage::Global() )
	{}

	bool AddItem(const ItemType &aItem) {
		// success means it was a new entry, otherwise there is already an item present
		LockUnlockEntry aGuard( fLock );
		return IntAddItem( aItem );
	}

	bool DeleteItem(const ItemType &aItem) {
		StartTrace(set_wrapper.DeleteItem);
		LockUnlockEntry aGuard( fLock );
		return IntDeleteItem( aItem );
	}

	bool ReplaceItem(const ItemType &aItem) {
		StartTrace(set_wrapper.ReplaceItem);
		LockUnlockEntry aGuard( fLock );
		IntDeleteItem(aItem);
		return IntAddItem(aItem);
	}

	/*! this is the exact match version of find
		\param
		\return true in case we found the item, rItemPos will then be the iterator pointing to the item */
	bool FindItem(const ItemType &aItem, ConstIteratorType &rItemPos) const {
		StartTrace(set_wrapper.FindItem);
		bool bFound(false);
		if ( HasList() ) {
			LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
			rItemPos = IntGetConstListPtr()->find( aItem );
			bFound = ( rItemPos != IntGetConstListPtr()->end() );
		}
		return bFound;
	}

	/*! No compare version of find, do not compare VAT/STT/Value attributes of Item
		\param
		\return */
	bool FindItemRange(const ItemType &aItemMin, const ItemType &aItemMax, ConstIteratorType &rItemsBegin, ConstIteratorType &rItemsEnd) const {
		StartTrace1(set_wrapper.FindItemRange, "find items for range [" << aItemMin.AsString() << "," << aItemMax.AsString() << "]");
		bool bFound(false);
		if ( HasList() ) {
			LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
			bFound = IntFindItemRange( aItemMin, aItemMax, rItemsBegin, rItemsEnd );
		}
		return bFound;
	}

	/*! Return iterators spanning all elements
		\param
		\return */
	bool FullRange(IteratorType &rItemsBegin, IteratorType &rItemsEnd) {
		StartTrace(set_wrapper.FullRange);
		bool bFound(false);
		if ( HasList() ) {
			LockUnlockEntry aGuard( fLock );
			rItemsBegin = IntGetCreateListPtr()->begin();
			rItemsEnd = IntGetCreateListPtr()->end();
			bFound = ( rItemsBegin != rItemsEnd );
		}
		return bFound;
	}

	bool DeleteItemRange(const IteratorType &rItemsBegin, const IteratorType &rItemsEnd) {
		StartTrace1(set_wrapper.DeleteItemRange, "[" << ( ( rItemsBegin != fpList->end() ) ? (*rItemsBegin).AsString() : "<end>" ) << "," << ( ( rItemsEnd != fpList->end() ) ? (*rItemsEnd).AsString() : "<end>" ) << ")");
		bool bRet(false);
		if ( HasList() ) {
			LockUnlockEntry aGuard( fLock );
			IntGetCreateListPtr()->erase(rItemsBegin, rItemsEnd);
			bRet = true;
		}
		return bRet;
	}

	bool DeleteItemRange(const ItemType &aItemMin, const ItemType &aItemMax) {
		StartTrace1(set_wrapper.DeleteItemRange, "deleting items for range [" << aItemMin.AsString() << "," << aItemMax.AsString() << "]");
		bool bRet(false);
		if ( HasList() ) {
			LockUnlockEntry aGuard( fLock );
			IteratorType rItemsBegin, rItemsEnd;
			if ( IntFindItemRange(aItemMin, aItemMax, rItemsBegin, rItemsEnd) ) {
				IntGetCreateListPtr()->erase(rItemsBegin, rItemsEnd);
				bRet = true;
			}
		}
		return bRet;
	}

	void Clear() {
		LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
		if ( this->IntHasList() ) {
			this->IntGetListPtr()->clear();
		}
	}

	size_type Size() const {
		LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
		size_type sz(0);
		if ( this->IntHasList() ) {
			sz = this->IntGetConstListPtr()->size();
		}
		return sz;
	}

	bool HasList() const {
		LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
		return IntHasList();
	}

	bool IsEnd(const IteratorType &aIter) const {
		LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
		return ( IntHasList() ? ( aIter == fpList->end() ) : true );
	}

	const Type *GetConstListPtr() const {
		LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
		return IntGetConstListPtr();
	}

	set_wrapper &operator=(const set_wrapper &other) {
		return Assign( other );
	}
	template < class M, template < class > class A >
	set_wrapper &operator=(const set_wrapper<T, M, A>& other) {
		return Assign( other );
	}

	template < class M, template < class > class A >
	set_wrapper &Assign(const set_wrapper<T, M, A>& other) {
		if ( other.HasList() ) {
			LockUnlockEntry aGuard( fLock );
			CopyItems(IntGetCreateListPtr(), other.GetConstListPtr());
		}
		return (*this);
	}

	Type *GetCreateListPtr() {
		LockUnlockEntry aGuard( fLock );
		return IntGetCreateListPtr();
	}

protected:
	bool IntHasList() const {
		return ( fpList.get() != NULL );
	}

	Type *IntGetListPtr() {
		return fpList.get();
	}

	const Type *IntGetConstListPtr() const {
		return fpList.get();
	}

	Type *IntGetCreateListPtr() {
		if ( !IntHasList() ) {
			fpList = std::auto_ptr< Type >( new Type() );
		}
		return fpList.get();
	}

	/*! No compare version of find, do not compare VAT/STT/Value attributes of Item
		\param
		\return */
	bool IntFindItemRange(const ItemType &aItemMin, const ItemType &aItemMax, ConstIteratorType &rItemsBegin, ConstIteratorType &rItemsEnd) const {
		StartTrace1(set_wrapper.IntFindItemRange, "find items for range [" << aItemMin.AsString() << "," << aItemMax.AsString() << "]");
		bool bFound(false);
		if ( IntHasList() ) {
			ConstIteratorType aEnd( IntGetConstListPtr()->end() );
			rItemsBegin = IntGetConstListPtr()->lower_bound( aItemMin );
			rItemsEnd = IntGetConstListPtr()->upper_bound( aItemMax );
			Trace("[" << ( ( rItemsBegin != aEnd ) ? (*rItemsBegin).AsString() : "<end>" ) << "," << ( ( rItemsEnd != aEnd ) ? (*rItemsEnd).AsString() : "<end>" ) << ")");
			if ( ( rItemsBegin != aEnd || rItemsEnd != aEnd ) ) {
				// check if values are within range
				// -> begin value must at least be lower or equal the given aItemMax
				bFound = ( *rItemsBegin < aItemMax || !( aItemMax < *rItemsBegin ) );
			}
		}
		return bFound;
	}

	bool IntAddItem(const ItemType &aItem) {
		InsertResultType aRetCode( IntGetCreateListPtr()->insert(aItem) );
		StatTrace(set_wrapper.IntAddItem, "Item [" << aItem.AsString() << "] was " << (aRetCode.second ? "" : "not " ) << "successful", Storage::Current());
		// success means it was a new entry, otherwise there is already an item present
		return aRetCode.second;
	}

	bool IntDeleteItem(const ItemType &aItem) {
		StartTrace(set_wrapper.IntDeleteItem);
		bool bRet(false);
		if ( IntHasList() ) {
			size_type aRetCode( IntGetCreateListPtr()->erase(aItem) );
			// number of elements == 1 means success
			bRet = ( aRetCode == 1 );
		}
		return bRet;
	}

private:
	std::auto_ptr< Type > fpList;
	MutexPolicy fLock;
};

template < class AL, class AR >
inline void CopyItems(AL *pThisList, const AR *pOtherList)
{
//	pThisList->reserve(pOtherList->size());
	std::copy(pOtherList->begin(), pOtherList->end(), inserter(*pThisList, pThisList->begin()));
}

#endif
