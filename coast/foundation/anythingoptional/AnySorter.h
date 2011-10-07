/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ANYSORTER_H
#define _ANYSORTER_H

#include "AnyComparers.h"

//!---- Forward declarations ---------------------------------------------------------
class String;
class Anything;

//! Sorts Array-Anything
class AnySorter
{
public:
	enum EMode { asc, desc };
	/*! Sorts array entries having the slot defined by sortFieldName.
		Array entries which don't have such a slot and non-array entries are
		preserved in the sorted result by appending them in the order of their
		appearence in the data to be sorted at the end of the sorted data.
		\param sortFieldName slotname of slot containing the date to be used as sort key
		\param toSort data to be sorted in/out parameter
		\param mode sortorder, asc/desc
		\param sortCritIsNumber set to true if criteria is a numeric value */
	static void SortByKeyInArray(const String &sortFieldName, Anything &toSort, EMode mode = asc, bool sortCritIsNumber = false);
};

class SpecialLookupComparer : public AnyLookupValueComparer
{
public:
	SpecialLookupComparer(const char *lookuppath, const AnyComparer &theValueComparer);
	int Compare(const Anything &left, const Anything &right) const;
};

namespace AnyExtensions
{
//	struct ValueComparer
//	{
//		static int Compare(AnyArrayImpl &that, long leftInt, long rightInt) const
//		{
//			return 0;
//		}
//	};
//	struct KeyComparer
//	{
//		static int Compare(AnyArrayImpl &that, long leftInt, long rightInt) const
//		{
//			return that.IntKey(leftInt).Compare(that.IntKey(rightInt));
//		}
//	};
//
//	template
//	<
//		class Comparer
//	>
//	struct MergeSortAlgo : public Comparer
//1	{
//		static void Sort(AnyArrayImpl& aImpl)
//		{
//			MergeSortByComparer(0, aImpl.GetSize()-1);
//		}
//		static void MergeSort(AnyArrayImpl& aImpl, long low, long high)
//		{
//			if (low >= high) return;
//			long middle= (low+high)/2;
//			MergeSort(aImpl, low, middle);
//			MergeSort(aImpl, middle+1, high);
//			Merge(aImpl, low, high, middle);
//		}
//		static void Merge(AnyArrayImpl& aImpl, long lo, long hi, long m)
//		{
//			if (hi < m+1 || lo > m) return; // nothing to merge
//			long i, j, k;
//			const long sz=m-lo+1;
//			long a[sz];					// temporary array of lower half
//			for (k=0,i=lo; i <=m && k < sz;++i, ++k)
//				a[k]=aImpl.IntAt(i);
//			Assert(k == sz);
//			Assert(i > m);
//			j=m+1;
//			k=0; i=lo;
//			while (k<sz && j <=hi)
//			{
//				Assert(i<j);
//				if (Compare(aImpl, a[k], aImpl.IntAt(j)) <= 0)
//				{
//					fInd->SetIndex(i, a[k]);
//					++k;
//				}
//				else
//				{
//					fInd->SetIndex(i, aImpl.IntAt(j));
//					++j;
//				}
//				i++;
//			}
//			// copy the remainder
//			while ( k < sz && i < j )
//			{
//				Assert(j > hi);
//				fInd->SetIndex(i, a[k]);
//				++i;++k;
//			}
//			Assert(i == j);
//			Assert(k == sz);
//		}
//	};

	template
	<
	template <class> class ComparerPolicy /*= KeyComparer*/,
			 template <class> class SortPolicy /*= MergeSortAlgo*/
			 >
	struct Sorter
	{
		static void Sort(Anything &a) {
//			if ( a.GetType() == AnyArrayType )
			{
//				SortPolicy<ComparerPolicy>::Sort(*a.GetImpl());
//				RecreateKeyTable();
			}
		}
	};
};
#endif
