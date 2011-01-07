/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnyImpls_H
#define _AnyImpls_H

//---- baseclass include -------------------------------------------------
#include "ITOString.h"
#include "AnyImplTypes.h"
#include "IFAObject.h"
#include "SegStorAllocatorNewDelete.h"

//---- forward declaration -----------------------------------------------
class Anything;
class AnyVisitor;

//---- AnyImpl --------------------------------------------------------------
class AnyImpl {
public:
	AnyImpl(Allocator *a) :
		fRefCount(1), fAllocator((a) ? a : Storage::Current()) {
	}
	virtual ~AnyImpl() {
		Assert(fRefCount <= 0);
	}

	virtual AnyImplType GetType() const = 0;

	virtual long GetSize() const {
		return 1;
	}
	virtual long Contains(const char *) const {
		return -1;
	}

	virtual long AsLong(long dflt) const = 0;

	virtual double AsDouble(double dflt) const = 0;

	virtual const char *AsCharPtr(const char *dflt) const = 0;

	virtual const char *AsCharPtr(const char *dflt, long &buflen) const = 0;

	virtual String AsString(const char *dflt) const = 0;

	virtual bool IsEqual(AnyImpl const *) const = 0;

	virtual AnyImpl *Clone(Allocator *a) const = 0;

	AnyImpl *DeepClone(Allocator *a, Anything &xreftable) const;

	virtual void Accept(AnyVisitor &v, long lIdx, const char *slotname) const = 0;

	void Ref() {
		++fRefCount;
	}

	void Unref() {
		if (--fRefCount <= 0) {
			delete this;
		}
	}
	const long &RefCount() const {
		return fRefCount;
	}

	String ThisToHex(Allocator *a = Storage::Current()) const;

	Allocator *MyAllocator() const {
		return fAllocator;
	}
protected:
	virtual AnyImpl *DoDeepClone(AnyImpl *res, Allocator *a, Anything &xreftable) const {
		return dynamic_cast<AnyImpl*> (res);
	}

private:
	AnyImpl(AnyImpl const &);
	AnyImpl& operator=(AnyImpl const &);
	long fRefCount;
	Allocator *fAllocator;
};

//---- AnyLongImpl -----------------------------------------------------------------
class AnyLongImpl: public Coast::SegStorAllocatorNewDelete<AnyLongImpl>, public AnyImpl {
	long fLong;
	String fBuf;

	AnyLongImpl(long l, const String &buf, Allocator *a) :
		AnyImpl(a), fLong(l), fBuf(buf, a) {
	}

public:
	AnyLongImpl(long l, Allocator *a);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	AnyImpl *Clone(Allocator *a) const;

	AnyImplType GetType() const {
		return AnyLongType;
	}

	long AsLong(long) const {
		return fLong;
	}

	double AsDouble(double) const {
		return static_cast<double> (fLong);
	}

	const char *AsCharPtr(const char *) const;

	const char *AsCharPtr(const char *, long &buflen) const;

	bool IsEqual(AnyImpl const *fAnyImp) const {
		return fLong == fAnyImp->AsLong(-1);
	}

	String AsString(const char *) const;

	void Accept(AnyVisitor &v, long lIdx, const char *slotname) const;
};

//---- AnyObjectImpl -----------------------------------------------------------------
class AnyObjectImpl: public Coast::SegStorAllocatorNewDelete<AnyObjectImpl>, public AnyImpl {
	IFAObject *fObject;

public:
	AnyObjectImpl(IFAObject *o, Allocator *a) :
		AnyImpl(a), fObject(o) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	AnyImpl *Clone(Allocator *a) const;

	AnyImplType GetType() const {
		return AnyObjectType;
	}

	long AsLong(long) const {
		return reinterpret_cast<long> (fObject);
	}

	IFAObject *AsIFAObject(IFAObject *) const {
		return fObject;
	}

	const char *AsCharPtr(const char *) const;

	const char *AsCharPtr(const char *, long &buflen) const;

	String AsString(const char *) const;

	bool IsEqual(AnyImpl const *) const {
		return false;
	}

	double AsDouble(double dft) const {
		return dft;
	}

private:
	void Accept(AnyVisitor &v, long lIdx, const char *slotname) const;
};

//---- AnyDoubleImpl -----------------------------------------------------------------
class AnyDoubleImpl: public Coast::SegStorAllocatorNewDelete<AnyDoubleImpl>, public AnyImpl {
	double fDouble;
	String fBuf;

	AnyDoubleImpl(double d, const String &buf, Allocator *a) :
		AnyImpl(a), fDouble(d), fBuf(buf, a) {
	}

public:
	AnyDoubleImpl(double d, Allocator *a);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	AnyImpl *Clone(Allocator *a) const;

	AnyImplType GetType() const {
		return AnyDoubleType;
	}

	long AsLong(long) const {
		return static_cast<long> (fDouble);
	}

	double AsDouble(double) const {
		return fDouble;
	}

	bool IsEqual(AnyImpl const *fAnyImp) const {
		return fDouble == fAnyImp->AsDouble(-1);
	}

	const char *AsCharPtr(const char *dflt) const;

	const char *AsCharPtr(const char *dflt, long &buflen) const;

	String AsString(const char *) const;

	void Accept(AnyVisitor &v, long lIdx, const char *slotname) const;
};

//---- AnyBinaryBufImpl -----------------------------------------------------------------
class AnyBinaryBufImpl: public Coast::SegStorAllocatorNewDelete<AnyBinaryBufImpl>, public AnyImpl {
	String fBuf;

public:
	AnyBinaryBufImpl(void const *buf, long len, Allocator *a) :
		AnyImpl(a), fBuf(buf, len, a) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	AnyImpl *Clone(Allocator *a) const;

	AnyImplType GetType() const {
		return AnyVoidBufType;
	}

	const char *AsCharPtr(const char *dflt) const {
		return fBuf.Capacity() > 0 ? fBuf.cstr() : dflt;
	}

	const char *AsCharPtr(const char *, long &buflen) const;

	long AsLong(long) const {
		return reinterpret_cast<long> (fBuf.cstr());
	}

	double AsDouble(double dft) const {
		return dft;
	}

	String AsString(const char *) const {
		return fBuf;
	}

	bool IsEqual(AnyImpl const *impl) const {
		return (dynamic_cast<AnyImpl const *> (this) == impl);
	}

	virtual void Accept(AnyVisitor &v, long lIdx, const char *slotname) const;
};

//---- AnyStringImpl -----------------------------------------------------------------
class AnyStringImpl: public Coast::SegStorAllocatorNewDelete<AnyStringImpl>, public AnyImpl {
	String fString;

public:
	AnyStringImpl(const char *s, long l, Allocator *a) :
		AnyImpl(a), fString(s, l, a) {
	}

	AnyStringImpl(const String &s, Allocator *a) :
		AnyImpl(a), fString(s, a) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	AnyImpl *Clone(Allocator *a) const;

	AnyImplType GetType() const {
		return AnyCharPtrType;
	}

	long Contains(const char *k) const {
		return Compare(k);
	}

	long AsLong(long dflt) const;

	double AsDouble(double dflt) const;

	bool IsEqual(AnyImpl const *anyImpl) const {
		return fString.IsEqual(anyImpl->AsCharPtr(0));
	}

	const char *AsCharPtr(const char *) const;

	const char *AsCharPtr(const char *, long &buflen) const;

	String AsString(const char *) const;

	virtual void Accept(AnyVisitor &v, long lIdx, const char *slotname) const;

protected:
	long Compare(const char *other) const;

};

class AnyArrayImpl;
//---- AnyKeyTable --------------------------------------------------
class AnyKeyTable : public Coast::AllocatorNewDelete
{
public:
	enum {
		cInitCapacity = 17
	};

	AnyKeyTable(AnyArrayImpl *table, long initCapacity = cInitCapacity);
	~AnyKeyTable();

	void Clear();

	long Append(const char *key, long lIdx);
	void Update(long fromIndex);
	void Update(long fromIndex, long size);

	long At(const char *key, long hashhint = -1, u_long hashhint1 = 0) const;

	void PrintHash() const;


protected:
	void InitTable(long cap);
	long DoHash(const char *key, bool append = false, long sizehint = -1, u_long hashhint = 0) const;
	void Rehash(long newCap);

private:
	AnyArrayImpl *fKeyTable; // shared with AnyArrayImpl
	long *fHashTable;
	long fThreshold, fCapacity;
	Allocator *fAllocator;
	AnyKeyTable(AnyKeyTable const &);
	AnyKeyTable& operator=(AnyKeyTable const &);
};

//---- AnyIndTable --------------------------------------------------
class AnyIndTable : public Coast::AllocatorNewDelete
{
public:
	AnyIndTable(long initCapacity, Allocator *a);
	~AnyIndTable();

	long At(long);
	long At(long) const;
	void Remove(long slot);

	void Swap(long l, long r);
	void SetIndex(long slot, long index);
	void InsertReserve(long slot, long size);

protected:
	void InitTable(long cap);
	void InitIndices(long slot, long *ot);
	void InitEmpty(long oldCap, long newCap);
	void Expand(long slot);
	void Clear();
	void PrintTable() const;

private:
	long *fIndexTable;
	long *fEmptyTable;
	long fCapacity;
	long fSize;
	Allocator *fAllocator;
	AnyIndTable(AnyIndTable const &);
	AnyIndTable & operator=(AnyIndTable const &);
};

class AnyKeyAssoc;
class AnyComparer;
//---- AnyArrayImpl -----------------------------------------------------------------
class AnyArrayImpl : public Coast::SegStorAllocatorNewDelete<AnyArrayImpl>, public AnyImpl {
	AnyKeyAssoc **fContents;
	AnyKeyTable *fKeys;
	AnyIndTable *fInd;
	long fCapacity, fSize;
	long fBufSize, fNumOfBufs;

	friend class AnyKeyTable;

public:
	AnyArrayImpl(Allocator *a);
	~AnyArrayImpl();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	AnyImpl *Clone(Allocator *a) const;

	Anything &At(long i);
	Anything const& At(long i) const;

	Anything &operator[](long i) {
		return At(i);
	}
	Anything const& operator[](long i) const;
	void Expand(long c);

	void InsertReserve(long pos, long size);

	void Remove(long slot);

	long GetSize() const {
		return fSize;
	}

	long Contains(const char *k) const;

	long FindIndex(const char *k, long sizehint = -1, u_long hashhint = 0) const;

	long FindIndex(const long lIdx) const;

	Anything &At(const char *key);
	Anything const& At(const char *key) const;

	Anything &operator[](const char *key) {
		return At(key);
	}
	Anything const& operator[](const char *key)const;

	const char *AsCharPtr(const char *) const;

	const char *AsCharPtr(const char *dflt, long &buflen) const;

	String AsString(const char *) const;

	double AsDouble(double dflt) const {
		return dflt;
	}

	long AsLong(long dflt) const {
		return dflt;
	}

	AnyImplType GetType() const {
		return AnyArrayType;
	}

	const char *SlotName(long slot) const;

	const String &VisitSlotName(long slot) const;

	virtual bool IsEqual(AnyImpl const *) const {
		return false;
	}

	void PrintKeys() const;

	void PrintHash() const;

	long IntAt(long at) {
		return fInd->At(at);
	}
	long IntAt(long at) const {
		return fInd->At(at);
	}

	long IntAtBuf(long at) const {
		return at / fBufSize;
	}

	long IntAtSlot(long at) const {
		return at % fBufSize;
	}

	void Accept(AnyVisitor &v, long lIdx, const char *slotname) const;

	// new from SOP
	//!reorder Array using sort order of the keys
	void SortByKey();

	//! this method is only relevant for legacy behaviour of SlotNameSorter.
	//! We do not need to sort descending, since we always can iterate from the end
	void SortReverseByKey();

	//!reorder Array using sort order defined by the AnyComparer
	void SortByAnyComparer(const AnyComparer &comparer);

	//!rebuild hash map after sorting O(fSize)
	void RecreateKeyTable();

	//!similar to SlotName(at)
	const String &Key(long slot) const;

	//!works with index into fContents
	const Anything &IntValue(long at) const;

	//!works with index into fContents
	const String &IntKey(long at) const;

	//!interface for internal comparing during sort
	class AnyIntCompare
	{
	public:
		virtual int Compare(AnyArrayImpl &that, long leftInt, long rightInt) const {
			return 0;
		}
	};

	void MergeSortByComparer(long left, long right, const AnyIntCompare &comparer);

	void MergeByComparer(long left, long right, long middle, const AnyIntCompare &comparer);

#if 0
	void Qsort(long left, long right);
	void BuildHeap();
	void DownHeap(long start, long end);
	void HeapSort();
	int CompareKeys(long i, long j);
#endif

	void Swap(long l, long r) {
		fInd->Swap(l, r);
	}

	void AllocMemory();

	long AdjustCapacity(long cap) {
		return ((cap + fBufSize - 1) / fBufSize) * fBufSize; // make it a multiple of fBufSize
	}

	static class AnyIntKeyCompare: public AnyIntCompare {
	public:
		virtual int Compare(AnyArrayImpl &that, long leftInt, long rightInt) const {
			return that.IntKey(leftInt).Compare(that.IntKey(rightInt));
		}
	} theKeyComparer;

	static class AnyIntReverseKeyCompare: public AnyIntCompare {
	public:
		virtual int Compare(AnyArrayImpl &that, long leftInt, long rightInt) const {
			return -that.IntKey(leftInt).Compare(that.IntKey(rightInt));
		}
	} theReverseKeyComparer;

	class AnyIntComparerCompare: public AnyIntCompare {
		const AnyComparer &ac;
	public:
		AnyIntComparerCompare(const AnyComparer &theComparer) :
			ac(theComparer) {
		}
		virtual int Compare(AnyArrayImpl &that, long leftInt, long rightInt) const;
	};

private:
	AnyImpl *DoDeepClone(AnyImpl *res, Allocator *a, Anything &xreftable) const;
};

// convenience macros for AnyImpl simplification
template <typename AsImpl>
inline AsImpl const *DynamicAsImpl(AnyImpl const *anyimpl){
	return dynamic_cast<AsImpl const *>(anyimpl);
}
template <typename AsImpl>
inline AsImpl  *DynamicAsImpl(AnyImpl  *anyimpl){
	return dynamic_cast<AsImpl  *>(anyimpl);
}
#define LongImpl(anyimpl) (DynamicAsImpl<AnyLongImpl>(anyimpl))
#define IsLongImpl(anyimpl) (LongImpl(anyimpl)!=0)
#define ObjectImpl(anyimpl) (DynamicAsImpl<AnyObjectImpl>(anyimpl))
#define IsObjectImpl(anyimpl) (ObjectImpl(anyimpl) !=0)
#define ArrayImpl(anyimpl) (DynamicAsImpl<AnyArrayImpl>(anyimpl))
#define IsArrayImpl(anyimpl) (ArrayImpl(anyimpl) !=0)

#endif
