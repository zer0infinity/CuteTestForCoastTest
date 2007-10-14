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

#include "Anything.h"
#include "STLStorage.h"
#include <set>

//---- forward declaration -----------------------------------------------

//---- set_wrapper ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
template <
typename T,
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

	set_wrapper()
		: fpList()
	{}

	bool AddItem(const ItemType &aItem) {
		InsertResultType aRetCode( GetCreateListPtr()->insert(aItem) );
		StatTrace(set_wrapper.AddItem, "Item [" << aItem.AsString() << "] was " << (aRetCode.second ? "" : "not " ) << "successful", Storage::Current());
		// success means it was a new entry, otherwise there is already an item present
		return aRetCode.second;
	}

	bool DeleteItem(const ItemType &aItem) {
		StartTrace(set_wrapper.DeleteItem);
		bool bRet(false);
		if ( HasList() ) {
			size_type aRetCode( GetCreateListPtr()->erase(aItem) );
			// number of elements == 1 means success
			bRet = ( aRetCode == 1 );
		}
		return bRet;
	}

	bool ReplaceItem(const ItemType &aItem) {
		StartTrace(set_wrapper.ReplaceItem);
		DeleteItem(aItem);
		return AddItem(aItem);
	}

	/*! this is the exact match version of find
		\param
		\return true in case we found the item, rItemPos will then be the iterator pointing to the item */
	bool FindItem(const ItemType &aItem, ConstIteratorType &rItemPos) const {
		StartTrace(set_wrapper.FindItem);
		bool bFound(false);
		if ( HasList() ) {
			rItemPos = GetListPtr()->find( aItem );
			bFound = ( rItemPos != GetListPtr()->end() );
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
			ConstIteratorType aEnd( GetListPtr()->end() );
			rItemsBegin = GetListPtr()->lower_bound( aItemMin );
			rItemsEnd = GetListPtr()->upper_bound( aItemMax );
			Trace("[" << ( ( rItemsBegin != aEnd ) ? (*rItemsBegin).AsString() : "<end>" ) << "," << ( ( rItemsEnd != aEnd ) ? (*rItemsEnd).AsString() : "<end>" ) << ")");
			if ( ( rItemsBegin != aEnd || rItemsEnd != aEnd ) ) {
				// check if values are within range
				// -> begin value must at least be lower or equal the given aItemMax
				bFound = ( *rItemsBegin < aItemMax || !( aItemMax < *rItemsBegin ) );
			}
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
			rItemsBegin = GetCreateListPtr()->begin();
			rItemsEnd = GetCreateListPtr()->end();
			bFound = ( rItemsBegin != rItemsEnd );
		}
		return bFound;
	}

	bool DeleteItemRange(const IteratorType &rItemsBegin, const IteratorType &rItemsEnd) {
		StartTrace1(set_wrapper.DeleteItemRange, "[" << ( ( rItemsBegin != fpList->end() ) ? (*rItemsBegin).AsString() : "<end>" ) << "," << ( ( rItemsEnd != fpList->end() ) ? (*rItemsEnd).AsString() : "<end>" ) << ")");
		bool bRet(false);
		if ( HasList() ) {
			GetCreateListPtr()->erase(rItemsBegin, rItemsEnd);
			bRet = true;
		}
		return bRet;
	}

	bool DeleteItemRange(const ItemType &aItemMin, const ItemType &aItemMax) {
		StartTrace1(set_wrapper.DeleteItemRange, "deleting items for range [" << aItemMin.AsString() << "," << aItemMax.AsString() << "]");
		bool bRet(false);
		if ( HasList() ) {
			IteratorType rItemsBegin, rItemsEnd;
			if ( FindItemRange(aItemMin, aItemMax, rItemsBegin, rItemsEnd) ) {
				GetCreateListPtr()->erase(rItemsBegin, rItemsEnd);
				bRet = true;
			}
		}
		return bRet;
	}

	size_type Size() const {
		if ( this->HasList() ) {
			return this->GetListPtr()->size();
		}
		return size_type(0);
	}

	bool HasList() const {
		return ( fpList.get() != NULL );
	}

	bool IsEnd(const IteratorType &aIter) const {
		return ( HasList() ? ( aIter == fpList->end() ) : true );
	}

	const Type *GetListPtr() const {
		return fpList.get();
	}

	template < template < class > class A >
	set_wrapper &operator=(const set_wrapper<T, A>& other) {
		if ( other.HasList() ) {
			CopyItems(GetCreateListPtr(), other.GetListPtr());
		}
		return (*this);
	}

	Type *GetCreateListPtr() {
		if ( !HasList() ) {
			fpList = std::auto_ptr< Type >( new Type() );
		}
		return fpList.get();
	}

private:
	std::auto_ptr< Type > fpList;
};

template < class AL, class AR >
inline void CopyItems(AL *pThisList, const AR *pOtherList)
{
//	pThisList->reserve(pOtherList->size());
	std::copy(pOtherList->begin(), pOtherList->end(), inserter(*pThisList, pThisList->begin()));
}

#endif
