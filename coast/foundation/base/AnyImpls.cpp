/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnyImpls.h"
#include "AnyVisitor.h"
#include "SystemLog.h"
#include "Tracer.h"
#include "SystemBase.h"
#include <iostream>
#include <cstring>

#if defined(COAST_TRACE)
#define aimplStatTrace(trigger, msg, allocator) 	StatTrace(trigger, msg, allocator)
#define aimplStartTrace(trigger)					StartTrace(trigger)
#define aimplStartTrace1(trigger, msg)			StartTrace1(trigger, msg)
#define aimplTrace(msg)							Trace(msg);
#define aimplStatTraceAny(trigger, any, msg, allocator) StatTraceAny(trigger, any, msg, allocator);
#define aimplTraceAny(any, msg)					TraceAny(any, msg);
#else
#define aimplStatTrace(trigger, msg, allocator)
#define aimplStartTrace(trigger)
#define aimplStartTrace1(trigger, msg)
#define aimplTrace(msg)
#define aimplStatTraceAny(trigger, any, msg, allocator)
#define aimplTraceAny(any, msg)
#endif

namespace {
	//!< avoid temporary
	const String fgStrEmpty(coast::storage::Global());
}

String AnyImpl::ThisToHex(Allocator *a) const {
	const int bufSize = 1+2*sizeof(std::ptrdiff_t);
	char buf[bufSize] = {0};
	static char const *const fmt = (sizeof(this)>4)?"%016tx":"%08tx"; // assume large pointers are 64bit = 8 Bytes large
	int charsStoredOrRequired = coast::system::SnPrintf(buf, bufSize, fmt, reinterpret_cast<std::ptrdiff_t>(this));
	String hexStr(buf, charsStoredOrRequired, a);
	aimplStatTrace(AnyImpl.ThisToHex, "converted number is " << hexStr, coast::storage::Current());
	return hexStr;
}

AnyImpl *AnyImpl::DeepClone(Allocator *a, Anything &xreftable) const {
	aimplStartTrace1(AnyImpl.DeepClone, "my-a:&" << (long)MyAllocator() << " a:&" << (long)a);
	AnyImpl *pObj = 0;
	// SOP: only use xreftable if 'this' has more than one reference on it, otherwise it does not make sense
	if ( RefCount() > 1L ) {
		String adr = ThisToHex();
		Anything &pRefEntry = (xreftable[adr]);
		pObj = reinterpret_cast<AnyImpl*>(pRefEntry.AsIFAObject()); //TODO might not be the best way to do it (requires friendship of Anything and AnyImpl)
		if (pObj != 0) {
			aimplTraceAny(pRefEntry, "found existing entry, adding reference " << (pObj?pObj->ThisToHex():"0"));
			pObj->Ref(); // do not forget to count
			return pObj;
		}
		pObj = this->Clone(a);
		pRefEntry = reinterpret_cast<IFAObject*>(pObj);
		aimplTrace("stored xref entry for adr: " << adr << " is " << pObj->ThisToHex());
	} else {
		pObj = this->Clone(a);
	}
	// let the specific impl modify the cloned entry if it needs to (like in AnyArrayImpl)
	return this->DoDeepClone(pObj, a, xreftable);
}

AnyLongImpl::AnyLongImpl(long l, Allocator *a) :
	AnyImpl(a), fLong(l), fBuf(a) {
	fBuf.Append(fLong);
}

String AnyLongImpl::AsString(const char *) const {
	return fBuf;
}

const char *AnyLongImpl::AsCharPtr(const char *) const {
	return fBuf.cstr();
}

const char *AnyLongImpl::AsCharPtr(const char *, long &buflen) const {
	buflen = fBuf.Length();
	return fBuf.cstr();
}

void AnyLongImpl::Accept(AnyVisitor &v, long lIdx, const char *slotname) const {
	v.VisitLong(fLong, this, lIdx, slotname);
}

AnyImpl *AnyLongImpl::Clone(Allocator *a) const {
	return new ((a) ? a : coast::storage::Current()) AnyLongImpl(this->fLong, this->fBuf, a);
}

static const char *gcObjectText = "IFAObject";

const char *AnyObjectImpl::AsCharPtr(const char *, long &buflen) const {
	buflen = strlen(gcObjectText);
	return gcObjectText;
}

const char *AnyObjectImpl::AsCharPtr(const char *) const {
	return gcObjectText;
}

String AnyObjectImpl::AsString(const char *) const {
	return gcObjectText;
}

AnyImpl *AnyObjectImpl::Clone(Allocator *a) const {
	return new ((a) ? a : coast::storage::Current()) AnyObjectImpl(this->fObject, a);
}

void AnyObjectImpl::Accept(AnyVisitor &v, long lIdx, const char *slotname) const {
	v.VisitObject(fObject, this, lIdx, slotname);
}

AnyDoubleImpl::AnyDoubleImpl(double d, Allocator *a) :
	AnyImpl(a), fDouble(d), fBuf(64, a) {
	fBuf.Append(d);
}

String AnyDoubleImpl::AsString(const char *) const {
	return fBuf;
}

const char *AnyDoubleImpl::AsCharPtr(const char *dflt) const {
	return fBuf.cstr();
}

const char *AnyDoubleImpl::AsCharPtr(const char *dflt, long &buflen) const {
	buflen = fBuf.Length();
	return fBuf.cstr();
}

AnyImpl *AnyDoubleImpl::Clone(Allocator *a) const {
	return new ((a) ? a : coast::storage::Current()) AnyDoubleImpl(this->fDouble, this->fBuf, a);
}

void AnyDoubleImpl::Accept(AnyVisitor &v, long lIdx, const char *slotname) const {
	v.VisitDouble(fDouble, this, lIdx, slotname);
}

const char *AnyBinaryBufImpl::AsCharPtr(const char *dflt, long &buflen) const {
	if (fBuf.Capacity() > 0) {
		buflen = fBuf.Length();
		return fBuf.cstr();
	} else {
		buflen = 0;
		return dflt;
	}
}

void AnyBinaryBufImpl::Accept(AnyVisitor &v, long lIdx, const char *slotname) const {
	v.VisitVoidBuf(fBuf, this, lIdx, slotname);
}

AnyImpl *AnyBinaryBufImpl::Clone(Allocator *a) const {
	return new ((a) ? a : coast::storage::Current()) AnyBinaryBufImpl((this->fBuf.cstr()), this->fBuf.Length(), a);
}

long AnyStringImpl::Compare(const char *other) const {
	if ( fString.Compare(other) == 0 ) {
		return 0;
	}
	return -1;
}

long AnyStringImpl::AsLong(long dflt) const {
	return fString.AsLong(dflt);
}

double AnyStringImpl::AsDouble(double dflt) const {
	return fString.AsDouble(dflt);
}

String AnyStringImpl::AsString(const char *) const {
	return fString;
}

const char *AnyStringImpl::AsCharPtr(const char *) const {
	return fString.cstr(); // PS: fString.AsCharPtr(dft); use operator const char * instead, no i have been wrong!
}

const char *AnyStringImpl::AsCharPtr(const char *, long &buflen) const {
	buflen = fString.Length();
	return fString.cstr();
}

void AnyStringImpl::Accept(AnyVisitor &v, long lIdx, const char *slotname) const {
	v.VisitCharPtr(fString, this, lIdx, slotname);
}

AnyImpl *AnyStringImpl::Clone(Allocator *a) const {
	return new ((a) ? a : coast::storage::Current()) AnyStringImpl(this->fString, a);
}

class AnyKeyAssoc : public coast::SegStorAllocatorNewDelete<AnyKeyAssoc> {
public:
	AnyKeyAssoc(const Anything &value, const char *key = 0) :
		fValue(value), fKey(key, -1, value.GetAllocator()) {
	}
	AnyKeyAssoc(const AnyKeyAssoc &aka) :
		fValue(aka.fValue), fKey(aka.fKey) {
	}
	AnyKeyAssoc() {
	}
	// used when allocating arrays... CAUTION: elements then must be initialized manually with Init()!
	void Init(Allocator *a) {
		fValue.SetAllocator(a);
		fKey.SetAllocator(a);
	}
	Anything &Value() {
		return fValue;//lint !e1536
	}
	const Anything &Value() const {
		return fValue;
	}
	const String &Key() const {
		return fKey;
	}
	void SetKey(const char *key) {
		fKey = key;
	}
	void SetValue(const Anything &val) {
		fValue = val;
	}
	AnyKeyAssoc &operator=(const AnyKeyAssoc &aka) {
		fValue = aka.Value();
		if (aka.Key()) {
			fKey = aka.Key();
		}
		return *this;
	}//lint !e1529
	Allocator *MyAllocator() {
		return fValue.GetAllocator();
	}
private:
	Anything fValue;
	String fKey;
};//lint !e1510

#define LOADFACTOR 0.75

long IFANextPrime(long x) {
	if (x <= 3) {
		return 3;
	}
	if ( !(x & 0x01) ) {
		++x;
	}

	for (;;) {
		long n;
		for (n = 3; (n *n <= x) && ((x % n) != 0); ++++n) {
		}
		if (n * n > x) {
			return x;
		}
		++++x;
	}
}

AnyKeyTable::AnyKeyTable(AnyArrayImpl *table, long initCapacity) :
	fKeyTable(table), fHashTable(0), fThreshold(0), fCapacity(0), fAllocator(fKeyTable->MyAllocator()) {
	InitTable(initCapacity);
}

AnyKeyTable::~AnyKeyTable() {
	if (fHashTable) {
		Clear();
		fAllocator->Free(static_cast<void*>(fHashTable));
		fKeyTable = 0;
		fHashTable = 0;
	}
}//lint !e1579

void AnyKeyTable::InitTable(long cap) {
	if (cap < cInitCapacity) {
		cap = cInitCapacity;
	}
	fCapacity = IFANextPrime(cap);
	fHashTable = static_cast<long *>(fAllocator->Malloc(fCapacity * sizeof(long)));
	fThreshold = (3 * fCapacity) / 4;
	Clear();
}

void AnyKeyTable::Clear() {
	// only works with 2 complement binary arithmetic!
	memset(fHashTable, -1, sizeof(long) * fCapacity);
}

long AnyKeyTable::DoHash(const char *key, bool append, long sizehint, u_long hashhint) const {
	// calculate some index into fHashTable
	long keylen = sizehint;
	long hashval = (hashhint) ? hashhint : IFAHash(key, keylen);
	long hash = hashval % fCapacity;

	// look for next free slot
	// do wrap around search
	long bufSz = AnyArrayImpl::ARRAY_BUF_SIZE;

	long i = hash;
	do {
		long lIdx = fHashTable[i];
		switch (lIdx) {
			case -1:
				return i;
			case -2:// slot is deleted
				if (append) {
					return i;
				} else {
					break;
				}
			default: {
				long at = fKeyTable->IntAt(lIdx);
				const String &keyAtVal = fKeyTable->fContents[at/bufSz][at%bufSz].Key(); // might be null
				if (keylen == keyAtVal.Length()) {
					const char *keyPtr = key, *keyAtValPtr = keyAtVal;
					const char *eptr = keyPtr + keylen;
					for ( ; keyPtr < eptr; ++keyPtr, ++keyAtValPtr) {
						if ( *keyPtr != *keyAtValPtr ) {
							goto loop;
						}
					}
					return i; // we found the key
				}
			}
		}
	loop:
		// increment and modulo capacity -- NO modulo is slow!
		// ensure we wrap arround in the table
		if (++i >= fCapacity) {
			i -= fCapacity;
		}
	} while ( i != hash ); // finish loop if wrapped around
	return -1;
}

long AnyKeyTable::Append(const char *key, long atIndex) {
	if ( key ) {
		if ( fKeyTable->fSize > fThreshold) {
			// everything gets a new place in the hashtable
			// so we're done with it
			Rehash(fCapacity * 2);
		}
		// calculate hash index and put key table index
		// into it
		fHashTable[DoHash(key, true)] = atIndex;
	}
	return atIndex;
}
void AnyKeyTable::Update(long fromIndex) {
	for (long i = 0; i < fCapacity; ++i) {
		long lIdx = fHashTable[i];
		if ( lIdx == fromIndex ) {
			fHashTable[i] = -2;    // mark as deleted
		} else if ( lIdx > fromIndex ) {
			fHashTable[i] = lIdx - 1;    // update position in keytable
		}
	}
}

void AnyKeyTable::Update(long fromIndex, long size) {
	for (long i = 0; i < fCapacity; i++) {
		long lIdx = fHashTable[i];
		if ( size < 0 && lIdx == fromIndex ) {
			fHashTable[i] = -2;    // mark as deleted
		} else if ( lIdx >= fromIndex ) {
			fHashTable[i] = lIdx + size;    // update position in keytable
		}
	}
}

long AnyKeyTable::At(const char *key, long sizehint, u_long hashhint) const {
	// returns valid external index into key table
	// or -1= not found
	long lIdx = DoHash(key, false, sizehint, hashhint);
	if ( lIdx > -1 ) {
		return fHashTable[lIdx];
	}
	return lIdx;
}

void AnyKeyTable::Rehash(long newCap) {
	long oldCapacity = fCapacity;
	long *ot = fHashTable;

	// allocate new table with new capacity
	// table may expand or shrink
	InitTable(newCap);

	// iterate over the old table and rehash
	// values
	for ( long i = 0; i < oldCapacity; ++i ) {
		register long slot = ot[i];

		if (slot > -1) {	// assumption: we found an index for a key
			long at = fKeyTable->IntAt(slot);
			register const char *key = fKeyTable->fContents[fKeyTable->IntAtBuf(at)][fKeyTable->IntAtSlot(at)].Key();
			Assert(key);
			long lIdx = DoHash(key);
			Assert(lIdx > -1 && lIdx < fCapacity);
			fHashTable[lIdx] = slot;
		}
	}
	// free old table
	fAllocator->Free(ot);
}

void AnyKeyTable::PrintHash() const {
	for (long i = 0; i < fCapacity; ++i) {
		if ( fHashTable[i] > -1 ) {
			String m;
			m << "[" << i << "]<" << fHashTable[i] << "> ";
			SystemLog::WriteToStderr(m);
		}
	}
	SystemLog::WriteToStderr("\n", 1);
}

AnyIndTable::AnyIndTable(long initCapacity, Allocator *a) :
	fIndexTable(0), fEmptyTable(0), fCapacity(initCapacity), fSize(0), fAllocator(a) {
	fEmptyTable = 0;
	InitTable(initCapacity);
	InitEmpty(0, fCapacity);
}

AnyIndTable::~AnyIndTable() {
	if (fIndexTable) {
		Clear();//lint !e1551
		fAllocator->Free(fIndexTable);//lint !e1551
		fAllocator->Free(fEmptyTable);//lint !e1551

		fIndexTable = 0;
	}
}//lint !e1579//lint !e1579

void AnyIndTable::InitTable(long cap) {
	fCapacity = cap;
	fIndexTable = static_cast<long *>(fAllocator->Malloc(fCapacity * sizeof(long)));
	Clear();
}

void AnyIndTable::Clear() {
	memset(fIndexTable, -1, sizeof(long) * fCapacity);
}

void AnyIndTable::Expand(long slot) {
	Assert( slot >= fCapacity );

	// save old index table and capacity
	long *old = fIndexTable;
	long oldCap = fCapacity;

	// check for sufficient new capacity
	if ( slot >= fCapacity * 2 ) {
		fCapacity = slot;
	}

	// initalize index table with new capacity
	InitTable(fCapacity * 2);

	// initialize the empty table
	InitEmpty(oldCap, fCapacity);

	// initalize the indices in the index table
	InitIndices(slot, old);

	fAllocator->Free(old);
}

void AnyIndTable::InsertReserve(long pos, long size) {
	Assert(fSize + pos <= fCapacity); Assert(fSize + size <= fCapacity);
	std::rotate(fIndexTable + pos, fIndexTable + fSize - size, fIndexTable + fSize);
}

void AnyIndTable::InitEmpty(long oldCap, long newCap) {
	long *old = fEmptyTable;
	long sz = 0;
	long i = 0;

	// allocate the new size of the empty table
	fEmptyTable = static_cast<long *>(fAllocator->Malloc(newCap * sizeof(long)));

	// calculate the size to be copied
	if ( oldCap < newCap )	{
		sz = oldCap;
	} else	{
		sz = newCap;    // the size has shrunk
	}

	// copy the empty index array
	for (i = 0; i < sz; ++i)	{
		fEmptyTable[i] = old[i];
	}

	// fill in straight index (never used so far)
	if ( sz < newCap )
		for (i = sz; i < newCap; ++i) {
			fEmptyTable[i] = i;
		}

	fAllocator->Free(old);
}

void AnyIndTable::InitIndices(long slot, long *ot) {
	Assert( fSize <= slot );

	long i = 0;

	// copy old indices if they exist
	if ( ot ) {
		for (i = 0; i < fSize; ++i) {
			fIndexTable[i] = ot[i];
		}
	}

	// reuse deleted slots if they exist
	for (i = fSize; i <= slot; ++i) {
		fIndexTable[i] = fEmptyTable[i];
	}
	fSize = slot + 1;
}

long AnyIndTable::At(long slot) {
	Assert( slot >= 0 );

	// check for capacity overflow
	if ( slot >= fCapacity ) {
		Expand(slot);
	}

	// check for size overflow
	if ( slot >= fSize ) {
		InitIndices(slot, 0);
	}

	// return the index
	Assert( slot < fSize );
	return fIndexTable[slot];
}

long AnyIndTable::At(long slot) const {
	// check for capacity overflow
	if (slot >= 0 && slot < fCapacity && slot < fSize) {
		return fIndexTable[slot];
	}
	return -1; //see what happens with that, may be zero PS 2010!
}

void AnyIndTable::Remove(long slot) {
	Assert( slot >= 0 && slot < fSize );

	// save the index for later reuse
	fEmptyTable[fSize-1] = fIndexTable[slot];

	// shift the table down
	for (long i = slot; i < fSize - 1; ++i) {
		fIndexTable[i] = fIndexTable[i+1];
	}
	--fSize;
}
void AnyIndTable::Swap(long l, long r) {
	Assert( l >= 0 && l < fSize ); Assert( r >= 0 && r < fSize );
	long t = fIndexTable[l];
	fIndexTable[l] = fIndexTable[r];
	fIndexTable[r] = t;
}
void AnyIndTable::SetIndex(long slot, long idx) {
	Assert( slot >= 0 && slot < fSize );
	if (slot >= 0 && slot < fSize) {
		fIndexTable[slot] = idx;
	} else {
		std::cerr << "OOPS, slot = " << slot << " index = " << idx << " fSize = " << fSize << std::endl;
	}
}

void AnyIndTable::PrintTable() const {
	String m("IndexTable: \n");
	for ( long i = 0; i < fSize; ++i) {
		m << "[" << i << "] <" << fIndexTable[i] << ">" << "\n";
	}
	SystemLog::WriteToStderr(m);
}

static const char *gcArrayText = "AnyArrayImpl";

AnyArrayImpl::AnyArrayImpl(Allocator *a) :
	AnyImpl(a), fContents(0), fKeys(0), fInd(0), fCapacity(4), fSize(0), fNumOfBufs(0) {
	// allocate a default size structure without keys
	fCapacity = AdjustCapacity(fCapacity);
	AllocMemory();
}

AnyArrayImpl::~AnyArrayImpl() {
	if (fContents) {
		// free the buffers themselves
		for (int j = 0; j < fNumOfBufs; ++j) {
			delete [] (fContents[j]);
		}
		// free the buffer Ptr
		MyAllocator()->Free(fContents);

		// free the key array
		if ( fKeys ) {
			delete fKeys;
		}

		// free the index array
		delete fInd;

		// reset everything
		fInd = 0;
		fContents = 0;
		fKeys = 0;
		fSize = fCapacity = 0;
	}
}

const char *AnyArrayImpl::AsCharPtr(const char *, long &buflen) const {
	buflen = strlen(gcArrayText);
	return gcArrayText;
}

Anything &AnyArrayImpl::At(long slot) //const/non-const overload
{
	// return an address of an anything residing at slot
	// expand the buffers as necessary to fulfill the request
	long newsz = slot + 1;
	// check for logical expansion
	if ( (newsz > fSize) ) {
		// reset the size
		fSize = newsz;
		// check for physical expansion
		if ( newsz >= fCapacity ) {
			Expand(newsz);
		}
	}
	// calculate the address of the anything
	slot = IntAt(slot);
	return fContents[IntAtBuf(slot)][IntAtSlot(slot)].Value();
}

Anything const& AnyArrayImpl::At(long slot) const {
	// return an address of an anything residing at slot
	// DO NOT expand the buffers as necessary to fulfill
	// the request, but throw instead, if out of range
	slot = IntAt(slot);
	if (slot >= 0) {
		return fContents[IntAtBuf(slot)][IntAtSlot(slot)].Value();
	}
	throw std::out_of_range("AnyArrayImpl::At(long)");
}

Anything const& AnyArrayImpl::operator [](long slot) const {
	return At(slot);
}

Anything &AnyArrayImpl::At(const char *key) {
	long slot = -1;
	if ( !fKeys ) {
		// no keys exist yet
		fKeys = new(MyAllocator()) AnyKeyTable(this);
	} else {
		// find index of key or return -1
		slot = fKeys->At(key);
	}

	if (slot < 0) {
		// key doesn't exist so append this key in the key array
		// with the according slot
		slot = fKeys->Append(key, fSize);//lint !e613
		slot = IntAt(slot);
		// set the key in the any key assoc structure
		fContents[IntAtBuf(slot)][IntAtSlot(slot)].SetKey(key);

		// return the element found
		// this creates a new element
		return At(fSize);
	}
	// the element already exists the slot is an internal slot
	return fContents[IntAtBuf(slot)][IntAtSlot(slot)].Value();
}

Anything const& AnyArrayImpl::At(const char *key) const {
	// calculate the adress of an anything given its key
	long slot = -1;
	if ( fKeys ) {
		// find index of key or return -1
		slot = fKeys->At(key);
	}
	if (slot >= 0) {
		return fContents[IntAtBuf(slot)][IntAtSlot(slot)].Value();
	}
	throw std::out_of_range("AnyArrayImpl::At(const char*)");
}

Anything const& AnyArrayImpl::operator [](const char *key) const {
	return At(key);
}

long AnyArrayImpl::FindIndex(const char *key, long sizehint, u_long hashhint) const {
	// find the index of an anything given
	// its key. It returns -1 if not defined

	if ( !fKeys ) {
		return -1;
	}

	// return index from keyarray
	return fKeys->At(key, sizehint, hashhint);
}

long AnyArrayImpl::FindIndex(const long lIdx) const {
	// find the index of an anything given
	// its index. It returns -1 if not defined
	if ( !fInd ) {
		return -1L;
	}

	// return index from indexarray
	return static_cast<const AnyIndTable*> (fInd)->At(lIdx);
}

long AnyArrayImpl::Contains(const char *k) const {
	// search the value in the array
	// assume an array of simple strings
	// otherwise the result could be misleading
	// e.g. an AnyArrayImpl returns always AnyArrayImpl

	Assert(k);	// require a valid search string

	long i; 	// external slot index
	long at;	// internal buffer index
	for (i = 0; i < fSize; ++i) {
		at = IntAt(i);	// calculate the internal index
		Assert(at >= 0 && at < fCapacity);
		if ( strcmp(fContents[IntAtBuf(at)][IntAtSlot(at)].Value().AsCharPtr(""), k) == 0 ) {
			return i;
		}
	}
	return -1;
}

void AnyArrayImpl::Remove(long slot) {
	// remove an anything from the internal structures

	// check the slot range
	if (slot >= 0 && slot < fSize) {
		// delete the internal key assoc
		// at slot index
		long at = IntAt(slot);
		fContents[IntAtBuf(at)][IntAtSlot(at)] = AnyKeyAssoc(MyAllocator());	// reset it to initial empty assoc

		// remove the slot from the index array
		fInd->Remove(slot);

		// update the key array if it exists
		if ( fKeys ) {
			fKeys->Update(slot);
		}

		// update the size
		--fSize;
	} else {
		String msg;
		SYSERROR(msg.Append("index ").Append(slot).Append(" out of range"));
	}
}

const String &AnyArrayImpl::Key(long slot) const {
	if (slot >= 0 && slot < fSize) {
		long at = IntAt(slot);
		return fContents[IntAtBuf(at)][IntAtSlot(at)].Key();
	}
	return fgStrEmpty;
}

const String &AnyArrayImpl::IntKey(long at) const
{
	if (at >= 0 && at < fCapacity) {
		return fContents[IntAtBuf(at)][IntAtSlot(at)].Key();
	}
	return fgStrEmpty;
}

const Anything &AnyArrayImpl::IntValue(long at) const {
	if (at >= 0 && at < fCapacity) {
		return fContents[IntAtBuf(at)][IntAtSlot(at)].Value();
	}
	throw std::out_of_range("AnyArrayImpl::IntValue");
}

const char *AnyArrayImpl::SlotName(long slot) const {
	// calculate the slot name given an index
	const String &k = Key(slot);
	return (k.Length() > 0) ? k.cstr() : reinterpret_cast<const char *>(0);
}

const String &AnyArrayImpl::VisitSlotName(long slot) const {
	// calculate the slot name given an index
	// first check the range
	if (slot >= 0 && slot < fSize) {
		long at = IntAt(slot);
		return fContents[IntAtBuf(at)][IntAtSlot(at)].Key();
	}
	return fgStrEmpty;
}

void AnyArrayImpl::Expand(long newsize) {
	long oldCap = fCapacity;

	// set the new capacity
	if (newsize > fCapacity * 2) {
		fCapacity = AdjustCapacity(newsize);
	} else {
		fCapacity = AdjustCapacity(fCapacity * 2);
	}

	// check for the range of the capacity
	Assert((fCapacity % ARRAY_BUF_SIZE) == 0);

	// calculate the number of buffe	long numOfExistingBufs = fNumOfBufs;
	long numOfExistingBufs = fNumOfBufs;
	long numOfNewBufs = fCapacity / static_cast<long>(ARRAY_BUF_SIZE); //fCapacity / ARRAY_BUF_SIZE + 1;

	Assert(numOfNewBufs *ARRAY_BUF_SIZE >= newsize);

	// allocate new ptr buffer if necessary
	if ( numOfNewBufs > fNumOfBufs ) {
		// expand the number of needed ptr buffers
		fNumOfBufs = numOfNewBufs;

		// allocate the new size
		AnyKeyAssoc **old = fContents;
		fContents = reinterpret_cast<AnyKeyAssoc **>(MyAllocator()->Calloc(fNumOfBufs, sizeof(AnyKeyAssoc *)));
		if (fContents == 0) {
			static const char crashmsg[] = "FATAL: AnyArrayImpl::Expand calloc failed (increasing pointer buffer). I will crash :-(\n";
			SystemLog::WriteToStderr(crashmsg, strlen(crashmsg));

			fContents = old;
			fCapacity = oldCap;
			return;
		}

		for (long bufs = 0; bufs < numOfExistingBufs; ++bufs) {
			fContents[bufs] = old[bufs];
		}
		MyAllocator()->Free(old); // frees the old ptr buffer array not the contents buffer
	}

	AllocBuffersFrom(numOfExistingBufs);
}

void AnyArrayImpl::AllocBuffersFrom(long idx) {
	for (long i = idx; i < fNumOfBufs; ++i) {
		// must not use calloc to ensure proper initialization of Anything instance variables
		Assert(MyAllocator() != 0);
		fContents[i] = new (MyAllocator()) AnyKeyAssoc[ARRAY_BUF_SIZE];

		if ( fContents[i] == 0 ) {
			static const char crashmsg[] =
					"FATAL: AnyArrayImpl::Expand calloc failed (assigning memory to increased pointer buffers).\nI will crash :-(\n";
			SystemLog::WriteToStderr(crashmsg, sizeof(crashmsg));
			return;
		}

		for ( size_t keyAssocKeyCnt = 0L; keyAssocKeyCnt < ARRAY_BUF_SIZE; ++keyAssocKeyCnt ) {
			fContents[i][keyAssocKeyCnt].Init(MyAllocator());
		}
	}
}

void AnyArrayImpl::InsertReserve(long pos, long size) {
	At(fSize + size - 1); // make room for size new elements
	fInd->InsertReserve(pos, size);
	if (fKeys) {
		fKeys->Update(pos, size);
	}
}

void AnyArrayImpl::AllocMemory() {
	// calculate the number of needed buffers
	// allocate at least ARRAY_BUF_SIZE buffers of the size ARRAY_BUF_SIZE
	fNumOfBufs = fCapacity / static_cast<long>(ARRAY_BUF_SIZE); // round to the next multiple
	//(fCapacity / ARRAY_BUF_SIZE > ARRAY_BUF_SIZE) ? fCapacity/ARRAY_BUF_SIZE : ARRAY_BUF_SIZE;
	fContents = static_cast<AnyKeyAssoc **>( MyAllocator()->Calloc(fNumOfBufs, sizeof(AnyKeyAssoc *)));

	// allocate the index table
	fInd = new (MyAllocator()) AnyIndTable(fCapacity, MyAllocator());

	// allocate the buffers holding the
	// Any Key Assocs
	AllocBuffersFrom(0L);
}

void AnyArrayImpl::PrintKeys() const {
	long hash = -1;
	for (long i = 0; i < fSize; ++i) {
		long at = IntAt(i);
		if (fKeys) {
			hash = 	fKeys->At(fContents[IntAtBuf(at)][IntAtSlot(at)].Key());
		}
		String m;
		m << "[" << i << "]<" << NotNullStr(fContents[IntAtBuf(at)][IntAtSlot(at)].Key()) << ">(" << hash << ")" << "\n";//lint !e666
		SystemLog::WriteToStderr(m);
	}
}

void AnyArrayImpl::PrintHash() const {
	if (fKeys) {
		fKeys->PrintHash();
	} else {
		SystemLog::WriteToStderr("*", 1);
	}
}

AnyImpl *AnyArrayImpl::Clone(Allocator *a) const {
	return new ((a) ? a : coast::storage::Current()) AnyArrayImpl(a);
}

AnyImpl *AnyArrayImpl::DoDeepClone(AnyImpl *pObj, Allocator *a, Anything &xreftable) const {
	aimplStartTrace(AnyArrayImpl.DoDeepClone);
	long count = this->GetSize();
	AnyArrayImpl *pImpl = dynamic_cast<AnyArrayImpl*> (pObj);
	for (long i = 0; pImpl && i < count; ++i) {
		pImpl->At(this->SlotName(i)) = this->At(i).DeepClone(a, xreftable);
	}
	return pImpl;
}

void AnyArrayImpl::MergeByComparer(long lo, long hi, long m, const AnyIntCompare &comparer) {
	if (hi < m + 1 || lo > m) {
		return;    // nothing to merge
	}
	long i, j, k;
	long const sz = m - lo + 1;
	long a[sz];					// temporary array of lower half
	for (k = 0, i = lo; i <= m && k < sz; ++i, ++k) {
		a[k] = IntAt(i);
	} Assert(k == sz); Assert(i > m);
	j = m + 1;
	k = 0;
	i = lo;
	while (k < sz && j <= hi) {
		Assert(i < j);
		if (comparer.Compare(*this, a[k], IntAt(j)) <= 0) {
			fInd->SetIndex(i, a[k]);
			++k;
		} else {
			fInd->SetIndex(i, IntAt(j));
			++j;
		}
		++i;
	}
	// copy the remainder
	while ( k < sz && i < j ) {
		Assert(j > hi);
		fInd->SetIndex(i, a[k]);
		++i;
		++k;
	}
	Assert(i == j);
	Assert(k == sz);
}

void AnyArrayImpl::MergeSortByComparer(long low, long high, const AnyIntCompare &comparer) {
	if (low >= high) {
		return;
	}
	long middle = (low + high) / 2;
	MergeSortByComparer(low, middle, comparer);
	MergeSortByComparer(middle + 1, high, comparer);
	MergeByComparer(low, high, middle, comparer);
}

void AnyArrayImpl::SortByKey() {
	//HeapSort();
	//Qsort(0,GetSize()-1);
	MergeSortByComparer(0, GetSize() - 1, theKeyComparer);
	RecreateKeyTable();
}

void AnyArrayImpl::SortReverseByKey() {
	MergeSortByComparer(0, GetSize() - 1, theReverseKeyComparer);
	RecreateKeyTable();
}

void AnyArrayImpl::SortByAnyComparer(const AnyComparer &comparer) {
	MergeSortByComparer(0, GetSize() - 1, AnyIntComparerCompare(comparer));
	RecreateKeyTable();
}

void AnyArrayImpl::RecreateKeyTable() {
	if (fKeys) {
		fKeys->Clear();
		for (long i = 0; i < fSize; ++i) {
			const char *sn = SlotName(i);
			if (sn) {
				fKeys->Append(sn, i);
			}
		}
	}
}

const char *AnyArrayImpl::AsCharPtr(const char *) const {
	return gcArrayText;
}

String AnyArrayImpl::AsString(const char *) const {
	return gcArrayText;
}

void AnyArrayImpl::Accept(AnyVisitor &v, long lIdx, const char *slotname) const {
	ROAnything wrapit(dynamic_cast<AnyImpl const *>(this)); //is there a nicer way
	Assert(wrapit.fAnyImp == (AnyImpl *)this);		// check for auto conversion by compiler
	v.VisitArray(wrapit, this, lIdx, slotname);
}

int AnyArrayImpl::AnyIntComparerCompare::Compare(AnyArrayImpl &that, long leftInt, long rightInt) const {
	return ac.Compare(that.IntValue(leftInt), that.IntValue(rightInt));
}

AnyArrayImpl::AnyIntKeyCompare AnyArrayImpl::theKeyComparer;
AnyArrayImpl::AnyIntReverseKeyCompare AnyArrayImpl::theReverseKeyComparer;

#if 0
void AnyArrayImpl::Qsort(long left, long right)
{
	// taken from wirth
	long i = left, j = right;
	long median = (left + right) / 2;
	const String &mediankey = Key(median);
	do {
		while (mediankey.Compare(Key(i)) > 0) {
			++i;
		}
		while (mediankey.Compare(Key(j)) < 0) {
			--j;
		}
		if (i <= j) {
			Swap(i, j);
			++i;
			--j;
		}
	} while (i <= j);
	if (left < j) {
		Qsort(left, j);
	}
	if (right > i) {
		Qsort(i, right);
	}
}
int AnyArrayImpl::CompareKeys(long i, long j)
{
	return Key(i).Compare(Key(j));
}
void AnyArrayImpl::DownHeap (long v, long n)
{
	long w = 2 * v + 1;           // first descendant of v
	while (w < n) {
		if (w + 1 < n)           // is there a second descendant?
			if (CompareKeys(w + 1, w) > 0) {
				++w;
			}
		// w is the descendant of v with maximum label

		if (CompareKeys(v, w) >= 0) {
			return;    // v has the heap property
		}
		// otherwise
		Swap(v, w);;      // exchange the labels of v and w
		v = w;
		w = 2 * v + 1;      // continue
	}
}
void AnyArrayImpl::BuildHeap ()
{
	long n = GetSize();
	for (long v = n / 2 - 1; v >= 0; --v) {
		DownHeap(v, n);
	}
}
void AnyArrayImpl::HeapSort()
{
	BuildHeap();
	long n = GetSize();
	for (long v = n - 1; v > 0; --v) {
		Swap(0, v);
		DownHeap(0, v);
	}
}
#endif
