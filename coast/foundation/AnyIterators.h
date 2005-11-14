/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnyIterators_H
#define _AnyIterators_H

//---- baseclass include -------------------------------------------------
#include "config_foundation.h"
#include "Anything.h"
#include "Dbg.h"

//---- forward declaration -----------------------------------------------

namespace AnyExtensions
{

//---- Iterator -----------------------------------------------------------
	/*! Iterates simply over the (RO)Anythings slots, staying always on the uppermost level
	 */
	template < class XThing >
	class Iterator
	{
	public:
		typedef XThing PlainType;
		typedef XThing &PlainTypeRef;
		typedef long PositionType;
		typedef long &PositionTypeRef;
		typedef const long &ConstPositionTypeRef;
		typedef long SizeType;
		typedef long &SizeTypeRef;
		typedef const long &ConstSizeTypeRef;

		/*! Constructor
			\param a the Anything to iterate on */
		Iterator(PlainTypeRef a)
			: fAny(a)
			, fPosition(-1)
			, fSize(a.GetSize()) {
			StartTrace1(Iterator.Ctor, "size:" << fSize);
		}
		virtual ~Iterator() {
			StartTrace(Iterator.Dtor);
		}

		/*! Gets the next Anything
			\param a out - reference to the next element
			\return true, if there was a next element, false if the iteration has finished */
		bool Next(PlainTypeRef a) {
			return DoGetNext(a);
		}

		/*! Gets the next Anything
			\param a out - reference to the next element
			\return true, if there was a next element, false if the iteration has finished */
		bool operator()(PlainTypeRef a) {
			return Next(a);
		}

		/*! Get current index into base (RO)Anything
			\return current iterator index */
		ConstPositionTypeRef Index() const {
			return fPosition;
		}

	protected:
		PlainTypeRef GetAny() {
			return fAny;
		}

		/*! Get current index into base (RO)Anything for modification
			\return current iterator index */
		PositionTypeRef IndexRef() {
			return fPosition;
		}

		bool SetIndex(PositionType lPos) {
			if ( lPos >= 0L && lPos < MaxIndex() ) {
				IndexRef() = lPos;
				return true;
			}
			return false;
		}

		virtual bool NextIndex() {
			if ( ++IndexRef() < MaxIndex() ) {
				return true;
			}
			--IndexRef();
			return false;
		}

		ConstSizeTypeRef MaxIndex() {
			return fSize;
		}

		/*! Get the next element based on some criteria, subclasses could implement special behavior
			\param a reference to the next Anything
			\return true if a matching next element was found, false otherwise */
		virtual bool DoGetNext(PlainTypeRef a) {
			StartTrace(Iterator.DoGetNext);
			if ( NextIndex() ) {
				a = GetAny()[Index()];
				return true;
			}
			return false;
		}

	private:
		PlainType		fAny;
		PositionType	fPosition;
		SizeType		fSize;

		Iterator();
		Iterator(const Iterator &);
		Iterator &operator=(const Iterator &);
	};

//---- LeafIterator -----------------------------------------------------------
	/*! Iterates through the whole Anything structure, and returns all Leafs (i.e. Type!=eArray)
	 */
	template < class XThing >
	class LeafIterator: public Iterator<XThing>
	{
	public:
		typedef Iterator<XThing> BaseIterator;
		typedef LeafIterator<XThing> ThisIterator;
		typedef typename BaseIterator::PlainType PlainType;
		typedef typename BaseIterator::PlainTypeRef PlainTypeRef;

		/*! Constructor
			\param a the Anything to iterate on */
		LeafIterator(PlainTypeRef a)
			: BaseIterator(a)
			, subIter(0) {
			StartTrace(LeafIterator.Ctor);
		}

		virtual ~LeafIterator() {
			StartTrace(LeafIterator.Dtor);
			delete subIter;
		}

	private:
		/*! Get the next element based on some criteria, subclasses could implement special behavior
			\param a reference to the next Anything
			\return true if a matching next element was found, false otherwise */
		virtual bool DoGetNext(PlainTypeRef a) {
			StartTrace(LeafIterator.DoGetNext);
			if ( subIter ) {
				// We are already descended into the Anything
				if ( subIter->Next(a) ) {
					// He has found the next
					return true;
				} else {
					// He has done his Job
					delete subIter;
					subIter = 0;
				}
			}
			// Move to the Next slot that holds something different from an empty Array
			bool found = false;
			PlainType next;
			while ( !found && BaseIterator::DoGetNext(next) ) {
				if ( next.GetType() == AnyArrayType ) {
					if ( next.GetSize() > 0 ) {
						// its an Array and not empty, we have to descend
						subIter = new ThisIterator(next);
						found = subIter->Next(a);
					}
				} else {
					// a leaf
					a = next;
					found = true;
				}
			}
			return found;
		}

		BaseIterator *subIter;
	};

}
#endif
