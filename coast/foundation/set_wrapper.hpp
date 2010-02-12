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

#include "DummyMutex.h"

//---- forward declaration -----------------------------------------------
class Anything;
class LockUnlockEntry;

// disable tracing if requested, even if in COAST_TRACE mode, eg. performance tests
#define WD_DISABLE_TRACE
#if !defined(COAST_TRACE) || defined(WD_DISABLE_TRACE)
#define swStatTrace(trigger, msg, allocator)
#define swStartTrace(trigger)
#define swStartTrace1(trigger, msg)
#define swTrace(msg);
#else
#define swStatTrace(trigger, msg, allocator) 	StatTrace(trigger, msg, allocator)
#define swStartTrace(trigger)					StartTrace(trigger)
#define swStartTrace1(trigger, msg)				StartTrace1(trigger, msg)
#define swTrace(msg)							Trace(msg);
#endif

//---- set_wrapper ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
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
	typedef typename Type::const_reverse_iterator ConstReverseIteratorType;

	LOKI_DEFINE_VISITABLE()

	set_wrapper( const char *pName = "set_wrapper_lock" )
		: fpList( NULL )
		, fLock( pName, Storage::Global() )
	{}
	~set_wrapper() {
		LockUnlockEntry aGuard( fLock );
		if ( IntHasList() ) {
			IntGetListPtr()->clear();
		}
		delete fpList;
		fpList = NULL;
	}

	bool AddItem(const ItemType &aItem) {
		// success means it was a new entry, otherwise there is already an item present
		LockUnlockEntry aGuard( fLock );
		return IntAddItem( aItem );
	}

	bool DeleteItem(const ItemType &aItem) {
		swStartTrace(set_wrapper.DeleteItem);
		LockUnlockEntry aGuard( fLock );
		return IntDeleteItem( aItem );
	}

	bool DeleteItem( const ConstIteratorType &aItemPos ) {
		swStartTrace(set_wrapper.DeleteItem);
		LockUnlockEntry aGuard( fLock );
		return IntDeleteItem( aItemPos );
	}

	bool ReplaceItem(const ItemType &aItem) {
		swStartTrace(set_wrapper.ReplaceItem);
		LockUnlockEntry aGuard( fLock );
		IntDeleteItem(aItem);
		return IntAddItem(aItem);
	}

	/*! this is the exact match version of find
		\param
		\return true in case we found the item, rItemPos will then be the iterator pointing to the item */
	bool FindItem(const ItemType &aItem, ConstIteratorType &rItemPos) const {
		swStartTrace(set_wrapper.FindItem);
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
		swStartTrace1(set_wrapper.FindItemRange, "find items for range [" << aItemMin.AsString() << "," << aItemMax.AsString() << "]");
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
		swStartTrace1(set_wrapper.ReduceToRange, "reduce items to range [" << aItemMin.AsString() << "," << aItemMax.AsString() << "]");
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
		swStartTrace(set_wrapper.FullRange);
		bool bFound(false);
		if ( HasList() ) {
			LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
			rItemsBegin = IntGetConstListPtr()->begin();
			rItemsEnd = IntGetConstListPtr()->end();
			bFound = ( rItemsBegin != rItemsEnd );
		}
		return bFound;
	}

	/*! Return reverse_iterators spanning all elements
		\param rItemsBegin r-beginning position of range
		\param rItemsEnd r-end position of range
		\return true in case the range is not empty */
	bool FullReverseRange(ConstReverseIteratorType &rItemsBegin, ConstReverseIteratorType &rItemsEnd) const {
		swStartTrace(set_wrapper.FullReverseRange);
		bool bFound(false);
		if ( HasList() ) {
			LockUnlockEntry aGuard( *const_cast<MutexPolicy *>(&fLock) );
			rItemsBegin = IntGetConstListPtr()->rbegin();
			rItemsEnd = IntGetConstListPtr()->rend();
			bFound = ( rItemsBegin != rItemsEnd );
		}
		return bFound;
	}

	bool DeleteItemRange(const IteratorType &rItemsBegin, const IteratorType &rItemsEnd) {
		swStartTrace1(set_wrapper.DeleteItemRange, "[" << ( ( rItemsBegin != IntGetConstListPtr()->end() ) ? (*rItemsBegin).AsString() : "<end>" ) << "," << ( ( rItemsEnd != IntGetConstListPtr()->end() ) ? (*rItemsEnd).AsString() : "<end>" ) << ")");
		bool bRet(false);
		if ( HasList() ) {
			LockUnlockEntry aGuard( fLock );
			IntGetCreateListPtr()->erase(rItemsBegin, rItemsEnd);
			bRet = true;
		}
		return bRet;
	}

	bool DeleteItemRange(const ItemType &aItemMin, const ItemType &aItemMax) {
		swStartTrace1(set_wrapper.DeleteItemRange, "deleting items for range [" << aItemMin.AsString() << "," << aItemMax.AsString() << "]");
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
		swStatTrace(set_wrapper.IntHasList, "ptr is:" << (long)pList, Storage::Current());
		return ( pList != NULL );
	}

	Type *IntGetListPtr() {
		Type *pList( fpList );
		swStatTrace(set_wrapper.IntGetListPtr, "ptr is:" << (long)pList, Storage::Current());
		return pList;
	}

	const Type *IntGetConstListPtr() const {
		const Type *pList( fpList );
		swStatTrace(set_wrapper.IntGetConstListPtr, "ptr is:" << (long)pList, Storage::Current());
		return pList;
	}

	Type *IntGetCreateListPtr() {
		swStartTrace(set_wrapper.IntGetCreateListPtr);
		if ( !IntHasList() ) {
			fpList = new Type();
		}
		Type *pList( fpList );
		swTrace("ptr is:" << (long)pList);
		return pList;
	}

	/*! Find range of items within given boundaries
		\param aItemMin minimum item to find
		\param aItemMax maximum item to find
		\param rItemsBegin iterator position pointing to first element in range
		\param rItemsEnd iterator position pointing to element after last element in range
		\return true in case the range is not empty */
	bool IntFindItemRange(const ItemType &aItemMin, const ItemType &aItemMax, ConstIteratorType &rItemsBegin, ConstIteratorType &rItemsEnd) const {
		swStartTrace1(set_wrapper.IntFindItemRange, "find items for range [" << aItemMin.AsString() << "," << aItemMax.AsString() << "]");
		bool bFound(false);
		if ( IntHasList() ) {
			ConstIteratorType aEnd( IntGetConstListPtr()->end() );
			rItemsBegin = IntGetConstListPtr()->lower_bound( aItemMin );
			rItemsEnd = IntGetConstListPtr()->upper_bound( aItemMax );
			swTrace("[" << ( ( rItemsBegin != aEnd ) ? (*rItemsBegin).AsString() : "<end>" ) << "," << ( ( rItemsEnd != aEnd ) ? (*rItemsEnd).AsString() : "<end>" ) << ")");
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
		swStartTrace1(set_wrapper.IntReduceToRange, "reduce items to range [" << aItemMin.AsString() << "," << aItemMax.AsString() << "]");
		bool bModified( false );
		if ( IntHasList() ) {
			IteratorType aEnd( IntGetListPtr()->end() ), aBoundPos;
			aBoundPos = IntGetListPtr()->upper_bound( aItemMin );
			if ( ( aBoundPos != aEnd ) && ( aBoundPos != IntGetListPtr()->begin() ) ) {
				swTrace("removing [" << IntGetListPtr()->begin()->AsString() << "," << ( ( aBoundPos != aEnd ) ? aBoundPos->AsString() : "<end>" ) << ")");
				IntGetListPtr()->erase( IntGetListPtr()->begin(), aBoundPos );
				bModified = true;
			}
			aBoundPos = IntGetListPtr()->upper_bound( aItemMax );
			if ( aBoundPos != aEnd ) {
				swTrace("removing [" << aBoundPos->AsString() << ",<end>)");
				IntGetListPtr()->erase( aBoundPos, IntGetListPtr()->end() );
				bModified = true;
			}
		}
		return bModified;
	}

	bool IntAddItem(const ItemType &aItem) {
		InsertResultType aRetCode( IntGetCreateListPtr()->insert(aItem) );
		swStatTrace(set_wrapper.IntAddItem, "Item [" << aItem.AsString() << "] was " << (aRetCode.second ? "" : "not " ) << "successful", Storage::Current());
		// success means it was a new entry, otherwise there is already an item present
		return aRetCode.second;
	}

	bool IntDeleteItem(const ItemType &aItem) {
		swStartTrace(set_wrapper.IntDeleteItem);
		bool bRet(false);
		if ( IntHasList() ) {
			size_type aRetCode( IntGetCreateListPtr()->erase( aItem ) );
			// number of elements == 1 means success
			bRet = ( aRetCode == 1 );
		}
		return bRet;
	}

	bool IntDeleteItem(const ConstIteratorType &aItemPos) {
		swStartTrace(set_wrapper.IntDeleteItem);
		bool bRet(false);
		if ( IntHasList() ) {
			IntGetCreateListPtr()->erase( aItemPos );
			bRet = true;
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
