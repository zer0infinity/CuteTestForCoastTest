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
		typedef XThing	&RefType;
		typedef long PositionType;
		typedef long &PositionRefType;
		typedef const long &PositionRefTypeConst;

		/*! Constructor
			\param a the Anything to iterate on */
		Iterator(RefType a)
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
		bool Next(RefType a) {
			return DoGetNext(a);
		}

		/*! Gets the next Anything
			\param a out - reference to the next element
			\return true, if there was a next element, false if the iteration has finished */
		bool operator()(RefType a) {
			return Next(a);
		}

		/*! Get current index into base (RO)Anything
			\return current iterator index */
		PositionRefTypeConst Index() const {
			return fPosition;
		}

	protected:
		RefType GetAny() {
			return fAny;
		}
		PositionRefType GetPosition() {
			return fPosition;
		}
		long GetSize() {
			return fSize;
		}

		/*! Get the next element based on some criteria, subclasses could implement special behavior
			\param a reference to the next Anything
			\return true if a matching next element was found, false otherwise */
		virtual bool DoGetNext(RefType a) {
			StartTrace(Iterator.DoGetNext);
			if ( ++GetPosition() < GetSize() ) {
				a = GetAny()[Index()];
				return true;
			}
			return false;
		}

	private:
		PlainType		fAny;
		PositionType	fPosition;
		long			fSize;

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
		typedef typename BaseIterator::RefType RefType;

		/*! Constructor
			\param a the Anything to iterate on */
		LeafIterator(RefType a)
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
		virtual bool DoGetNext(RefType a) {
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
				if ( next.GetType() == Anything::eArray ) {
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
				next = PlainType();
			}
			return found;
		}

		BaseIterator *subIter;
	};

}
#endif
