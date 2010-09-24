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
#include "ITOTypeTraits.h"

//---- forward declaration -----------------------------------------------

namespace AnyExtensions
{

//---- Iterator -----------------------------------------------------------
	/*! Iterates simply over the (RO)Anythings slots, staying always on the uppermost level
	 */
	template
	<
	class XThing,
		  class XRetThing = XThing,
		  class SlotNameType = String
		  >
	class Iterator
	{
	public:
		typedef XThing PlainType;
		typedef XThing &PlainTypeRef;
		typedef XRetThing &PlainRetTypeRef;
		typedef typename boost_or_tr1::mpl::if_< boost_or_tr1::is_same<PlainType, ROAnything> ,
				ROAnything,
				PlainTypeRef>::type StoredType;

		typedef long PositionType;
		typedef long &PositionTypeRef;
		typedef const long &ConstPositionTypeRef;
		typedef long SizeType;
		typedef long &SizeTypeRef;
		typedef const long &ConstSizeTypeRef;
		typedef SlotNameType &SlotNameTypeRef;

		/*! Constructor
			\param a the Anything to iterate on */
		explicit Iterator(StoredType a)
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
		bool Next(PlainRetTypeRef a) {
			return DoGetNext(a);
		}

		/*! Gets the next Anything
			\param a out - reference to the next element
			\return true, if there was a next element, false if the iteration has finished */
		bool operator()(PlainRetTypeRef a) {
			return Next(a);
		}

		/*! Get current index into base (RO)Anything
			\return current iterator index */
		ConstPositionTypeRef Index() const {
			return fPosition;
		}

		/*! Get slotname of current position if any
			\param strSlotName name of slot to check for
			\return true if it is a named slot and slotname could be retrieved */
		bool SlotName(SlotNameTypeRef strSlotName) const {
			const char *pcSN = fAny.SlotName(fPosition);
			if ( pcSN != NULL ) {
				strSlotName = pcSN;
			}
			return ( pcSN != NULL );
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
		virtual bool DoGetNext(PlainRetTypeRef a) {
			StartTrace(Iterator.DoGetNext);
			if ( NextIndex() ) {
				a = GetAny()[Index()];
				return true;
			}
			return false;
		}

	private:
		StoredType		fAny;
		PositionType	fPosition;
		SizeType		fSize;

		Iterator();
		Iterator(const Iterator &);
		Iterator &operator=(const Iterator &);
	};

//---- LeafIterator -----------------------------------------------------------
	/*! Iterates through the whole Anything structure, and returns all Leafs (i.e. Type!=eArray)
	 */
	template <
	class XThing,
		  class XRetThing = XThing,
		  class SlotNameType = String
		  >
	class LeafIterator: public Iterator<XThing, XRetThing, SlotNameType>
	{
	public:
		typedef Iterator<XThing, XRetThing, SlotNameType> BaseIterator;
		typedef LeafIterator<XThing, XRetThing, SlotNameType> ThisIterator;
		typedef typename BaseIterator::StoredType StoredType;
		typedef typename BaseIterator::PlainType PlainType;
		typedef typename BaseIterator::PlainTypeRef PlainTypeRef;

		/*! Constructor
			\param a the Anything to iterate on */
		LeafIterator(StoredType a)
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
			bool found(false);
			TraceAny(this->GetAny(), "current content");
			while ( !found && BaseIterator::DoGetNext(fanySubRef) ) {
				TraceAny(fanySubRef, "descendant");
				if ( fanySubRef.GetType() == AnyArrayType ) {
					if ( fanySubRef.GetSize() > 0 ) {
						// its an Array and not empty, we have to descend
						subIter = new ThisIterator(fanySubRef);
						found = subIter->Next(a);
					}
				} else {
					// a leaf
					a = fanySubRef;
					found = true;
				}
			}
			return found;
		}

		BaseIterator *subIter;
		PlainType	fanySubRef;
	};

}
#endif
