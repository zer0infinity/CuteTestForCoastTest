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
		: fpList( NULL )
		, fLock( pName, Storage::Global() )
	{}
	~set_wrapper() {
		LockUnlockEntry aGuard( fLock );
		delete fpList;
		fpList = NULL;
	}

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

	/*! Find range of items within given boundaries
		\param aItemMin minimum item to find
		\param aItemMax maximum item to find
		\param rItemsBegin iterator position pointing to first element in range
		\param rItemsEnd iterator position pointing to element after last element in range
		\return true in case the range is not empty */
	bool FindItemRange(const ItemType &aItemMin, const ItemType &aItemMax, ConstIteratorType &rItemsBegin, ConstIteratorType &rItemsEnd) const {
		StartTrace1(set_wrapper.FindItemRange, "find items for range [" << aItemMin.AsString() << "," << aItemMax.AsString() << "]");
		bool bFound(false);
		if ( HasList() ) {
			LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
			bFound = IntFindItemRange( aItemMin, aItemMax, rItemsBegin, rItemsEnd );
		}
		return bFound;
	}

	/*! Reduce list of items to given boundaries
		\param aItemMin minimum item to find
		\param aItemMax maximum item to find
		\return true in case we reduced the list */
	bool ReduceToRange(const ItemType &aItemMin, const ItemType &aItemMax) {
		StartTrace1(set_wrapper.ReduceToRange, "reduce items to range [" << aItemMin.AsString() << "," << aItemMax.AsString() << "]");
		bool bModified(false);
		if ( HasList() ) {
			LockUnlockEntry aGuard( fLock );
			bModified = IntReduceToRange( aItemMin, aItemMax );
		}
		return bModified;
	}

	/*! Return iterators spanning all elements
		\param rItemsBegin beginning position of range
		\param rItemsEnd end position of range
		\return true in case the range is not empty */
	bool FullRange(ConstIteratorType &rItemsBegin, ConstIteratorType &rItemsEnd) const {
		StartTrace(set_wrapper.FullRange);
		bool bFound(false);
		if ( HasList() ) {
			LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
			rItemsBegin = IntGetConstListPtr()->begin();
			rItemsEnd = IntGetConstListPtr()->end();
			bFound = ( rItemsBegin != rItemsEnd );
		}
		return bFound;
	}

	bool DeleteItemRange(const IteratorType &rItemsBegin, const IteratorType &rItemsEnd) {
		StartTrace1(set_wrapper.DeleteItemRange, "[" << ( ( rItemsBegin != IntGetConstListPtr()->end() ) ? (*rItemsBegin).AsString() : "<end>" ) << "," << ( ( rItemsEnd != IntGetConstListPtr()->end() ) ? (*rItemsEnd).AsString() : "<end>" ) << ")");
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
		if ( IntHasList() ) {
			IntGetListPtr()->clear();
		}
	}

	size_type Size() const {
		LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
		size_type sz(0);
		if ( IntHasList() ) {
			sz = IntGetConstListPtr()->size();
		}
		return sz;
	}

	bool HasList() const {
		LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
		return IntHasList();
	}

	bool IsEnd(const IteratorType &aIter) const {
		LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
		return ( IntHasList() ? ( aIter == IntGetConstListPtr()->end() ) : true );
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
		const Type *pList( fpList );
		StatTrace(set_wrapper.IntHasList, "ptr is:" << (long)pList, Storage::Current());
		return ( pList != NULL );
	}

	Type *IntGetListPtr() {
		Type *pList( fpList );
		StatTrace(set_wrapper.IntGetListPtr, "ptr is:" << (long)pList, Storage::Current());
		return pList;
	}

	const Type *IntGetConstListPtr() const {
		const Type *pList( fpList );
		StatTrace(set_wrapper.IntGetConstListPtr, "ptr is:" << (long)pList, Storage::Current());
		return pList;
	}

	Type *IntGetCreateListPtr() {
		StartTrace(set_wrapper.IntGetCreateListPtr);
		if ( !IntHasList() ) {
			fpList = new Type();
		}
		Type *pList( fpList );
		Trace("ptr is:" << (long)pList);
		return pList;
	}

	/*! Find range of items within given boundaries
		\param aItemMin minimum item to find
		\param aItemMax maximum item to find
		\param rItemsBegin iterator position pointing to first element in range
		\param rItemsEnd iterator position pointing to element after last element in range
		\return true in case the range is not empty */
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

	/*! Reduce list of items to given boundaries
		\param aItemMin minimum item to keep
		\param aItemMax maximum item to keep
		\return true in case we reduced the list */
	bool IntReduceToRange(const ItemType &aItemMin, const ItemType &aItemMax) {
		StartTrace1(set_wrapper.IntReduceToRange, "reduce items to range [" << aItemMin.AsString() << "," << aItemMax.AsString() << "]");
		bool bModified( false );
		if ( IntHasList() ) {
			IteratorType aEnd( IntGetListPtr()->end() ), aBoundPos;
			aBoundPos = IntGetListPtr()->upper_bound( aItemMin );
			if ( ( aBoundPos != aEnd ) && ( aBoundPos != IntGetListPtr()->begin() ) ) {
				Trace("removing [" << IntGetListPtr()->begin()->AsString() << "," << ( ( aBoundPos != aEnd ) ? aBoundPos->AsString() : "<end>" ) << ")");
				IntGetListPtr()->erase( IntGetListPtr()->begin(), aBoundPos );
				bModified = true;
			}
			aBoundPos = IntGetListPtr()->upper_bound( aItemMax );
			if ( aBoundPos != aEnd ) {
				Trace("removing [" << aBoundPos->AsString() << ",<end>)");
				IntGetListPtr()->erase( aBoundPos, IntGetListPtr()->end() );
				bModified = true;
			}
		}
		return bModified;
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
	Type *fpList;
	MutexPolicy fLock;
};

template < class AL, class AR >
inline void CopyItems(AL *pThisList, const AR *pOtherList)
{
//	pThisList->reserve(pOtherList->size());
	std::copy(pOtherList->begin(), pOtherList->end(), std::inserter(*pThisList, pThisList->begin()));
}

#endif
