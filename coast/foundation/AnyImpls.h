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
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "ITOString.h"
#include "AnyImplTypes.h"

//---- forward declaration -----------------------------------------------
class EXPORTDECL_FOUNDATION Anything;
class EXPORTDECL_FOUNDATION IFAObject;
class EXPORTDECL_FOUNDATION AnyVisitor;

//---- AnyImpl --------------------------------------------------------------
class EXPORTDECL_FOUNDATION AnyImpl
{
	friend class Anything;

public:
	AnyImpl(Allocator *a)
		: fRefCount(1)
		, fAllocator((a) ? a : Storage::Current())
	{}
	virtual ~AnyImpl() {
		Assert(fRefCount <= 0);
	}

	virtual AnyImplType GetType() const = 0;

	virtual long GetSize() {
		return 1;
	}
	virtual long Contains(const char *) {
		return -1;
	}

	virtual long AsLong(long dflt) = 0;

	virtual IFAObject *AsIFAObject(IFAObject *dflt) const {
		return dflt;
	}

	virtual double AsDouble(double dflt) = 0;

	virtual const char *AsCharPtr(const char *dflt) const = 0;

	virtual const char *AsCharPtr(const char *dflt, long &buflen) const = 0;

	virtual String AsString(const char *dflt) const = 0;

	virtual bool IsEqual(AnyImpl *) const = 0;

	AnyImpl *DeepClone(Allocator *a, Anything &xreftable);

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

	String ThisToHex(Allocator *a = Storage::Current()) const {
		String hexStr(a);
		return hexStr.Append((long)this);
	}

	static void *operator new(size_t size, Allocator *a);

	static void operator delete(void *d);

#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
	static void operator delete(void *d, Allocator *a);
#endif

protected:
	Allocator	*MyAllocator() {
		return fAllocator;
	}

private:
	virtual AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable) = 0;

	AnyImpl(const AnyImpl &);
	long		fRefCount;
	Allocator	*fAllocator;
};

//---- AnyLongImpl -----------------------------------------------------------------
class EXPORTDECL_FOUNDATION AnyLongImpl : public AnyImpl
{
	long fLong;
	String fBuf;

	AnyLongImpl(long l, const String &buf, Allocator *a)
		: AnyImpl(a)
		, fLong(l)
		, fBuf(buf, a)
	{ }

public:
	AnyLongImpl(long l, Allocator *a);
	~AnyLongImpl()  { }

	AnyImplType GetType() const				{
		return AnyLongType;
	}

	long AsLong(long)							{
		return fLong;
	}

	double AsDouble(double) 					{
		return (double) fLong;
	}

	const char *AsCharPtr(const char *) const;

	const char *AsCharPtr(const char *, long &buflen) const;

	bool IsEqual(AnyImpl *fAnyImp) const		{
		return fLong == fAnyImp->AsLong(-1);
	}

	String AsString(const char *) const;

	void Accept(AnyVisitor &v, long lIdx, const char *slotname) const;

private:
	AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable);
};

//---- AnyObjectImpl -----------------------------------------------------------------
class EXPORTDECL_FOUNDATION AnyObjectImpl : public AnyImpl
{
	IFAObject *fObject;

public:
	AnyObjectImpl(IFAObject *o, Allocator *a) : AnyImpl(a), fObject(o) { }

	AnyImplType GetType() const				{
		return AnyObjectType;
	}

	long AsLong(long) 							{
		return (long) fObject;
	}

	IFAObject *AsIFAObject(IFAObject *) const	{
		return fObject;
	}

	const char *AsCharPtr(const char *) const;

	const char *AsCharPtr(const char *, long &buflen) const;

	String AsString(const char *) const;

	bool IsEqual(AnyImpl *) const				{
		return false;
	}

	double AsDouble(double dft) 				{
		return dft;
	}

private:
	// PS,JW: check if we need fObject->Clone() instead
	AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable);

	void Accept(AnyVisitor &v, long lIdx, const char *slotname) const;
};

//---- AnyDoubleImpl -----------------------------------------------------------------
class EXPORTDECL_FOUNDATION AnyDoubleImpl : public AnyImpl
{
	double fDouble;
	String fBuf;

	AnyDoubleImpl(double d, const String &buf, Allocator *a)
		: AnyImpl(a)
		, fDouble(d)
		, fBuf(buf, a)
	{ }

public:
	AnyDoubleImpl(double d, Allocator *a);

	~AnyDoubleImpl()							{ }

	AnyImplType GetType() const				{
		return AnyDoubleType;
	}

	long AsLong(long) 							{
		return (long) fDouble;
	}

	double AsDouble(double) 					{
		return fDouble;
	}

	bool IsEqual(AnyImpl *fAnyImp) const		{
		return fDouble == fAnyImp->AsDouble(-1);
	}

	const char *AsCharPtr(const char *dflt) const;

	const char *AsCharPtr(const char *dflt, long &buflen) const;

	String AsString(const char *) const;

	void Accept(AnyVisitor &v, long lIdx, const char *slotname) const;

private:
	AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable);
};

//---- AnyBinaryBufImpl -----------------------------------------------------------------
class EXPORTDECL_FOUNDATION AnyBinaryBufImpl : public AnyImpl
{
	String fBuf;

public:
	AnyBinaryBufImpl(void *buf, long len, Allocator *a)
		: AnyImpl(a)
		, fBuf(buf, len, a)
	{ }

	~AnyBinaryBufImpl()								{  }

	AnyImplType GetType() const					{
		return AnyVoidBufType;
	}

	const char *AsCharPtr(const char *) const		{
		return fBuf.Capacity() > 0 ? (const char *)fBuf : 0;
	}

	const char *AsCharPtr(const char *, long &buflen) const;

	long AsLong(long) 								{
		return (long)(const char *)fBuf;
	}

	double AsDouble(double dft) 					{
		return dft;
	}

	String AsString(const char *) const				{
		return fBuf;
	}

	bool IsEqual(AnyImpl *impl) const				{
		return ((AnyImpl *)this == impl);
	}

	virtual void Accept(AnyVisitor &v, long lIdx, const char *slotname) const;

private:
	AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable);
};

//---- AnyStringImpl -----------------------------------------------------------------
class EXPORTDECL_FOUNDATION AnyStringImpl : public AnyImpl
{
	String fString;

public:
	AnyStringImpl(const char *s, long l, Allocator *a)
		: AnyImpl(a)
		, fString(s, l, a)
	{ }

	AnyStringImpl(const String &s, Allocator *a) : AnyImpl(a),  fString(s, a) { }

	AnyImplType GetType() const					{
		return AnyCharPtrType;
	}

	long Contains(const char *k)					{
		return Compare(k);
	}

	long AsLong(long dflt);

	double AsDouble(double dflt);

	bool IsEqual(AnyImpl *anyImpl) const			{
		return fString.IsEqual(anyImpl->AsCharPtr(0)) ;
	} //String("")); }

	const char *AsCharPtr(const char *) const;

	const char *AsCharPtr(const char *, long &buflen) const;

	String AsString(const char *) const;

	virtual void Accept(AnyVisitor &v, long lIdx, const char *slotname) const;

protected:
	long Compare(const char *other);

private:
	AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable);
};

class EXPORTDECL_FOUNDATION AnyArrayImpl;
//---- AnyKeyTable --------------------------------------------------
class EXPORTDECL_FOUNDATION AnyKeyTable
{
public:
	enum { cInitCapacity = 17 };

	AnyKeyTable(AnyArrayImpl *table, long initCapacity = cInitCapacity);
	~AnyKeyTable();

	void Clear();

	long Append(const char *key, long lIdx);
	void Update(long fromIndex);

	long At(const char *key, long hashhint = -1, u_long hashhint1 = 0	) const;

	void PrintHash();

	static void *operator new(size_t size, Allocator *a);
	static void operator delete(void *d);
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
	static void operator delete(void *d, Allocator *a);
#endif

protected:
	void InitTable(long cap);
	long DoHash(const char *key, bool append = false, long sizehint = -1, u_long hashhint = 0) const;
	void Rehash(long newCap);

private:
	AnyArrayImpl *fKeyTable;	// shared with AnyArrayImpl
	long *fHashTable;
	long fThreshold, fCapacity;
	Allocator *fAllocator;
};

//---- AnyIndTable --------------------------------------------------
class EXPORTDECL_FOUNDATION AnyIndTable
{
public:
	AnyIndTable(long initCapacity, Allocator *a);
	~AnyIndTable();

	long At(long);
	// return -1 if slot could not be found
	long FindAt(long slot);
	void Remove(long slot);

	static void *operator new(size_t size, Allocator *a);
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
	static void operator delete(void *d, Allocator *a);
#endif
	static void operator delete(void *d);
	void Swap(long l, long r);
	void SetIndex(long slot, long index);

protected:
	void InitTable(long cap);
	void InitIndices(long slot, long *ot);
	void InitEmpty(long oldCap, long newCap);
	void Expand(long slot);
	void Clear();
	void PrintTable();

private:
	long *fIndexTable;
	long *fEmptyTable;
	long fCapacity;
	long fSize;
	Allocator *fAllocator;
};

class EXPORTDECL_FOUNDATION AnyKeyAssoc;
class EXPORTDECL_FOUNDATION AnyComparer;
//---- AnyArrayImpl -----------------------------------------------------------------
class EXPORTDECL_FOUNDATION AnyArrayImpl : public AnyImpl
{
	AnyKeyAssoc **fContents;
	AnyKeyTable *fKeys;
	AnyIndTable *fInd;
	long fCapacity, fSize;
	long fBufSize, fNumOfBufs;

	friend class AnyKeyTable;

public:
	AnyArrayImpl(Allocator *a);

	~AnyArrayImpl();

	Anything &At(long i);

	Anything &operator[](long i)						{
		return At(i);
	}

	void Expand(long c);

	void Remove(long slot);

	long GetSize()										{
		return fSize;
	}

	long Contains(const char *k);

	long FindIndex(const char *k, long sizehint = -1, u_long hashhint = 0);

	long FindIndex(const long lIdx);

	Anything &At(const char *key);

	Anything &operator[](const char *key)				{
		return At(key);
	}

	const char *AsCharPtr(const char *) const;

	const char *AsCharPtr(const char *dflt, long &buflen) const;

	String AsString(const char *) const;

	double AsDouble(double dflt) 						{
		return dflt;
	}

	long AsLong(long dflt)								{
		return dflt;
	}

	AnyImplType GetType() const						{
		return AnyArrayType;
	}

	const char *SlotName(long slot);

	const String &VisitSlotName(long slot);

	virtual bool IsEqual(AnyImpl *) const				{
		return false;
	}

	void PrintKeys();

	void PrintHash();

	long IntAt(long at)			{
		return fInd->At(at);
	}

	long IntAtBuf(long at)		{
		return at / fBufSize;
	}

	long IntAtSlot(long at)		{
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
	void RecreateKeyTabe();

	//!similar to SlotName(at)
	const String &Key(long slot) ;

	//!works with index into fContents
	const Anything &IntValue(long at);

	//!works with index into fContents
	const String &IntKey(long at) ;

	//!interface for internal comparing during sort
	class EXPORTDECL_FOUNDATION AnyIntCompare
	{
	public:
		virtual ~AnyIntCompare() {};
		virtual int	Compare(AnyArrayImpl &that, long leftInt, long rightInt) const {
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
	int CompareKeys(long i, long j) ;
#endif

	void Swap(long l, long r) {
		fInd->Swap(l, r);
	}

	void AllocMemory();

	long AdjustCapacity(long cap) {
		return ((cap + fBufSize - 1) / fBufSize) * fBufSize;    // make it a multiple of fBufSize
	}

	static class EXPORTDECL_FOUNDATION AnyIntKeyCompare : public AnyIntCompare
	{
	public:
		virtual int	Compare(AnyArrayImpl &that, long leftInt, long rightInt) const {
			return that.IntKey(leftInt).Compare(that.IntKey(rightInt));
		}
	}	theKeyComparer;

	static class EXPORTDECL_FOUNDATION AnyIntReverseKeyCompare : public AnyIntCompare
	{
	public:
		virtual int	Compare(AnyArrayImpl &that, long leftInt, long rightInt) const {
			return - that.IntKey(leftInt).Compare(that.IntKey(rightInt));
		}
	}	theReverseKeyComparer;

	class EXPORTDECL_FOUNDATION AnyIntComparerCompare : public AnyIntCompare
	{
		const AnyComparer	&ac;
	public:
		AnyIntComparerCompare(const AnyComparer &theComparer): ac(theComparer) {}
		virtual int	Compare(AnyArrayImpl &that, long leftInt, long rightInt) const;
	};

private:
	AnyImpl *DoDeepClone(Allocator *a, Anything &xreftable);
};

// convenience macros for AnyImpl simplification
#define IsLongImpl(anyimpl) ((anyimpl) && (AnyLongType)==(anyimpl)->GetType())
#define LongImpl(anyimpl) ((AnyLongImpl*)(anyimpl))
#define IsObjectImpl(anyimpl) ((anyimpl) && (AnyObjectType)==(anyimpl)->GetType())
#define ObjectImpl(anyimpl) ((AnyObjectImpl*)(anyimpl))
#define IsArrayImpl(anyimpl) ((anyimpl) && (AnyArrayType)==(anyimpl)->GetType())
#define ArrayImpl(anyimpl) ((AnyArrayImpl*)(anyimpl))

#endif
