/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ITOStorage.h"

//--- interface include --------------------------------------------------------
#include "StorageTest.h"

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "DiffTimer.h"
#include "PoolAllocator.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- StorageTest ----------------------------------------------------------------
StorageTest::StorageTest(TString tname) : TestCaseType(tname)
{
}

StorageTest::~StorageTest()
{
}

void StorageTest::SimpleCallocSetsToZeroTest()
{
	GlobalAllocator ga;
	const long sz = 3000;
	char *ptr[sz];
	long i = 0;
	for ( i = 0 ; i < sz ; i++ ) {
		ptr[i] = (char *)ga.Malloc(i + 1); // add one otherwise we do not alloc something in the first step
		*ptr[i] = '\xA5';
		assertEqual('\xA5', *ptr[i]);
	}
	for ( i = 0 ; i < sz ; i++ ) {
		ga.Free(ptr[i]);
	}
	for ( i = 0 ; i < sz ; i++ ) {
		ptr[i] = (char *)ga.Calloc(i + 1, 1); // add one otherwise we do not alloc something in the first step
		TString msg("i = ");
		msg << i;
		assertEqualm((long)'\0', (long)*ptr[i], msg);
	}
	for ( i = 0 ; i < sz ; i++ ) {
		ga.Free(ptr[i]);
	}
}

void StorageTest::LeakTest()
{
	// the current implementation allows size testing, eg. tracking of allocated and freed memory only in Storage::GetStatisticLevel() >= 1
	if ( Storage::GetStatisticLevel() >= 1 ) {
		// watch the sequence of declaration
		MemChecker mc("Test Global", Storage::Global());
		String str(Storage::Global());
		{
			StartTraceMem(StorageTest.LeakTest);
			str = "Test a leak by allocating some more memory than was reserved initially";
		}
		assertCompare(str.Length(), equal_to, 70L);
		//PS: we now have a different measurement
		//assertEqual(12, mc.CheckDelta());
		assertComparem(mc.CheckDelta(), greater_equal, 12LL, "memory leaked as expected");
	}
}

void StorageTest::SimpleAllocTest(TString allocatorName, Allocator *pAllocator)
{
	// the current implementation allows size testing, eg. tracking of allocated and freed memory only in Storage::GetStatisticLevel() >= 1
	if ( Storage::GetStatisticLevel() >= 1 ) {
		MemChecker mc(allocatorName, pAllocator);
		const long sz = 3000;
		char *ptr[sz];
		long i = 0;
		for ( i = 0 ; i < sz ; i++ ) {
			ptr[i] = (char *)pAllocator->Malloc(i);
		}
		assertEqual(4498500, mc.CheckDelta());
		for ( i = 0 ; i < sz ; i++ ) {
			pAllocator->Free(ptr[i]);
		}
		pAllocator->PrintStatistic();
		assertEqual(0, mc.CheckDelta());
	}
}

void StorageTest::SimpleSizeHintTest(TString allocatorName, Allocator *pAllocator)
{
	const size_t sz = 3000;
	size_t i = 0;
	for ( i = 0 ; i < sz ; i++ ) {
		t_assert(pAllocator->SizeHint(i) >= i);
	}
}

void StorageTest::PoolSizeHintTest()
{
	PoolAllocator pa(1, 1022, 10);
	SimpleSizeHintTest("PoolAllocator", &pa);
	const size_t maxbucketsize = 0x2000;
	// test some significant values
	// must be adjusted if implementation of pool allocator is changed

	const u_long lMinimumSize = 16;
	assertEqual(lMinimumSize, pa.SizeHint(0));
	assertEqual(lMinimumSize, pa.SizeHint(1));
	assertEqual(lMinimumSize, pa.SizeHint(lMinimumSize));
	assertEqual(lMinimumSize << 1, pa.SizeHint(lMinimumSize + 1));
	assertEqual(lMinimumSize << 1, pa.SizeHint(lMinimumSize << 1));
	assertEqual(lMinimumSize << 2, pa.SizeHint((lMinimumSize << 1) + 1));
	assertEqual(0x2000, pa.SizeHint(0x1fff));
	assertEqual(0x2fff, pa.SizeHint(0x2fff));
	{
		for (size_t s = 0, expected = lMinimumSize; s < 0x7fff; s += 8 ) {
			if (expected < s) {
				expected <<= 1;
			}
			if (expected <= maxbucketsize) {
				assertEqual(expected, pa.SizeHint(s));
			} else {
				assertEqual(s, pa.SizeHint(s));
			}
		}
	}
	{
		for (size_t s = 1, expected = lMinimumSize; s < 0x7fff; s += 8 ) {
			if (expected < s) {
				expected <<= 1;
			}
			if (expected <= maxbucketsize) {
				assertEqual(expected, pa.SizeHint(s));
			} else {
				assertEqual(s, pa.SizeHint(s));
			}
		}
	}
}

void StorageTest::AllocatorTest()
{
	GlobalAllocator ga;
	SimpleSizeHintTest("GlobalAllocator",  &ga);
	SimpleAllocTest("GlobalAllocator",  &ga);
}

void StorageTest::AllocatorTiming()
{
	PoolAllocator pa(1, 8 * 1024, 21);
	DoTimingWith("PoolAllocator", &pa);
	GlobalAllocator ga;
	DoTimingWith("GlobalAllocator", &ga);
}

void StorageTest::DoTimingWith(TString allocatorName, Allocator *pAllocator)
{
	{
		StartTraceMem1(StorageTest.DoTimingWith, pAllocator);
		MemChecker mc("StorageTest.DoTimingWith", pAllocator);
		const int cAllocSz = 5;
		long allocSzArr[cAllocSz];
		void *allocPtr[cAllocSz];
		long allocSz = 16;
		const int cRunSize = 10000;
		l_long t;
		long i;
		for (i = 0; i < cAllocSz; i++) {
			// generate some alloc sizes
			allocSzArr[i] = allocSz;
			allocSz *= 2;
		}

		DiffTimer dt;
		dt.Start();
		for ( long lrun = cRunSize - 1; lrun >= 0; lrun--) {
			if ( lrun < (cRunSize - cAllocSz) ) {
				pAllocator->Free(allocPtr[(lrun + cAllocSz) % cAllocSz]);
			}
			allocPtr[lrun % cAllocSz] = pAllocator->Malloc(allocSzArr[lrun % cAllocSz]);
		}
		for (i = 0; i < cAllocSz; i++) {
			pAllocator->Free(allocPtr[i]);
		}
		t = dt.Reset();
		pAllocator->PrintStatistic();
		assertEqual(0, mc.CheckDelta());
	}
}

void StorageTest::AnyStorageGlobalAssignment()
{
	// test assignment with Storage::Global
	Anything a;
	Anything b;

	t_assert(a.GetAllocator() == Storage::Global());
	t_assert(b.GetAllocator() == Storage::Global());

	a = b;
	t_assert(a.GetAllocator() == Storage::Global());
	t_assert(b.GetAllocator() == Storage::Global());

	b = 1;
	t_assert(b.GetAllocator() == Storage::Global());
	a = b;
	t_assert(a.GetAllocator() == Storage::Global());
	t_assert(b.GetAllocator() == Storage::Global());
}

void StorageTest::AnyEqualAllocatorAssignment()
{
	GlobalAllocator ga;
	Anything a(&ga);
	Anything b(&ga);

	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &ga);

	a = b;
	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &ga);

	b = 1;
	t_assert(b.GetAllocator() == &ga);
	a = b;
	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &ga);

}

void StorageTest::AnyEqualPoolAllocatorAssignment()
{
	PoolAllocator pa(100, 10);
	Anything a(&pa);
	Anything b(&pa);

	t_assert(a.GetAllocator() == &pa);
	t_assert(b.GetAllocator() == &pa);

	a = b;
	t_assert(a.GetAllocator() == &pa);
	t_assert(b.GetAllocator() == &pa);

	b = 1;
	t_assert(b.GetAllocator() == &pa);
	a = b;
	t_assert(a.GetAllocator() == &pa);
	t_assert(b.GetAllocator() == &pa);

}

void StorageTest::AnyDifferentAllocatorAssignment()
{
	GlobalAllocator ga;
	PoolAllocator pa(100, 10);
	Anything a(&ga);
	Anything b(&pa);

	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &pa);

	a = b;
	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &pa);

	b = 1;
	t_assert(b.GetAllocator() == &pa);

	a = b;
	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &pa);

	b = a;
	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &pa);
}

void StorageTest::AnyPathology()
{
	//try to force a null allocator with SetAllocator
	Anything a;
	t_assert(a.GetAllocator() == Storage::Global());
	t_assert(!a.SetAllocator(0));
	t_assert(a.GetAllocator() == Storage::Global());
	a = "what a fool";
	t_assert(Storage::Global() == a.GetAllocator());
	assertEqual("what a fool", a.AsCharPtr());

	//try to force a null allocator in constructor
	Anything a1((Allocator *)0);
	t_assert(a1.GetAllocator() == Storage::Global());
	a1 = "what a fool";
	t_assert(Storage::Global() == a1.GetAllocator());
	assertEqual("what a fool", a1.AsCharPtr());
}

void StorageTest::AnyAssignment()
{
	AnyStorageGlobalAssignment();
	AnyEqualAllocatorAssignment();
	AnyEqualPoolAllocatorAssignment();
	AnyDifferentAllocatorAssignment();
	AnyPathology();
}

//#define AND_TRACE(msg)	std::cerr << _QUOTE_(msg) << std::endl
////#define AND_TRACE(msg)
//
//class AllocatorNewDelete
//{
//	friend class StorageTest;
//	//! enable compiler to generate default implementations
//	AllocatorNewDelete();
////	AllocatorNewDelete(const AllocatorNewDelete&);
////	AllocatorNewDelete& operator=(const AllocatorNewDelete&);
//	static void *operator new(size_t size) throw();
//public:
//	//! define virtual destructor for proper memory cleanup in derived classes
//	virtual ~AllocatorNewDelete()
//	{}
//
//	//! Access the allocator to allow derived classes to use it for further allocation operations
//	/*! \return allocator which was stored when we got constructed */
//	Allocator *GetAllocator() { return ((AllocatorNewDelete::Header*)ThisMemStart(this))->fAllocator; }
//
//	//! This method must be hidden so that the compiler has to always use the second form which specifies a valid Allocator
//	/*! \param size number of bytes to allocate
//		\return pointer to the memory location allocated */
//	// static void *operator new(size_t size);
//
//	//! This is the default form of operator new using the given Allocator to allocate memory on
//	/*! \param size number of bytes to allocate
//		\param alloc Allocator to allocate memory on
//		\return pointer to the memory location allocated */
//	static void *operator new(size_t size, Allocator *alloc) throw()
//	{
//		AND_TRACE((AllocatorNewDelete.new, "Allocator"));
//		//! use additional space to automatically store the allocator into
//		size_t sz(size);
//		sz += sizeof( AllocatorNewDelete::Header );
//		void *pMem(0);
//		if (alloc)
//		{
//			AND_TRACE((AllocatorNewDelete.new, "calling Allocator::Calloc"));
//			pMem = alloc->Calloc(1, sz);
//		}
//		else
//		{
//			AND_TRACE((AllocatorNewDelete.new, "calling ::operator new"));
//			pMem = ::operator new(sz);
//		}
//		if ( pMem )
//		{
//			((AllocatorNewDelete::Header*)pMem)->fAllocator = alloc;
//			return DerivedMemStart(pMem);
//		}
//		throw std::bad_alloc();
//	}
//
//	//! This is the default delete method, unfortunately the Allocator from which we allocated the memory previously is not passed as an argument. Therefor we have to assume that it was allocated on our allocator and try to free it.
//	/*! \param d pointer to memory location we want to free, destruction of the object has happened already
//		\param size size of memory to free, can be used as hint if we need to know about the blocksize to deallocate */
//	static void operator delete(void *d, size_t size)
//	{
//		AND_TRACE((AllocatorNewDelete.delete, "calling AllocatorNewDelete::operator delete, size:" << size));
//		AllocatorNewDelete::operator delete(d, ThisMemStart(d)->fAllocator);
//	}
//
//	//! This method will usually only get called during an exception from within a ctor. Here we also get delegated by the single param delete method.
//	/*! \param d pointer to memory location we want to free, destruction of the object has happened already
//		\param alloc allocator to free memory on */
//	static void operator delete(void *d, Allocator *alloc)
//	{
//		AND_TRACE((AllocatorNewDelete.delete, "Allocator"));
//		if (d)
//		{
//			if (alloc)
//			{
//				AND_TRACE((AllocatorNewDelete.delete, "calling Allocator::Free"));
//				alloc->Free( ThisMemStart(d) );
//			}
//			else
//			{
//				AND_TRACE((AllocatorNewDelete.delete, "calling ::operator delete"));
//				::operator delete( ThisMemStart(d) );
//			}
//		}
//	}
//
//	//! This is the default form of array operator new using the given Allocator to allocate memory on
//	/*! \param size number of bytes to allocate
//		\param alloc Allocator to allocate memory on
//		\return pointer to the memory location allocated */
//	static void *operator new[](size_t size,Allocator *alloc) throw()
//	{
//		AND_TRACE((AllocatorNewDelete.new[], "calling AllocatorNewDelete::operator new"));
//		return AllocatorNewDelete::operator new(size, alloc);
//	}
//
//	//! This is the default array delete method, unfortunately the Allocator from which we allocated the memory previously is not passed as an argument. Therefor we have to assume that it was allocated on our allocator and try to free it.
//	/*! \param d pointer to memory location we want to free, destruction of the object has happened already */
//	static void operator delete[](void *d)
//	{
//		AND_TRACE((AllocatorNewDelete.delete[], "calling AllocatorNewDelete::operator delete"));
//		AllocatorNewDelete::operator delete(d, ThisMemStart(d)->fAllocator);
//	}
//
//private:
//	struct Header {
//		Allocator *fAllocator;
//	};
//
//	static void *DerivedMemStart(void *vp)
//	{
//		if (vp)
//		{
//			return ( ((char*)vp) + sizeof( AllocatorNewDelete::Header ) );
//		}
//		return 0;
//	}
//
//	static AllocatorNewDelete::Header *ThisMemStart(void *vp)
//	{
//		if ( vp )
//		{
//			return (AllocatorNewDelete::Header*)( ((char*)vp) - sizeof( AllocatorNewDelete::Header ) );
//		}
//		return 0;
//	}
//};

#define AND_TRACE(trg, msg)	std::cerr << _QUOTE_(trg) << ": " << msg << std::endl
//#define AND_TRACE(msg)

class AllocatorNewDelete
{
	friend class StorageTest;
	//! enable compiler to generate default implementations
//	AllocatorNewDelete();
//	AllocatorNewDelete(const AllocatorNewDelete&);
//	AllocatorNewDelete& operator=(const AllocatorNewDelete&);
public:
	AllocatorNewDelete(Allocator *pAlloc = 0)
		: fAllocator(pAlloc ? pAlloc : ((long)fAllocator & 0x01L ? 0 : fAllocator)) {
		AND_TRACE(AllocatorNewDelete.AllocatorNewDelete, "size:" << sizeof(AllocatorNewDelete));
	}
	//! define virtual destructor for proper memory cleanup in derived classes
	virtual ~AllocatorNewDelete()
	{}

	//! Access the allocator to allow derived classes to use it for further allocation operations
	/*! \return allocator which was stored when we got constructed */
	Allocator *GetAllocator() {
		return ThisMemStart(this)->fAllocator;
	}

	//! This method must be hidden so that the compiler has to always use the second form which specifies a valid Allocator
	/*! \param size number of bytes to allocate
		\return pointer to the memory location allocated */
	// static void *operator new(size_t size);

	//! This is the default form of operator new using the given Allocator to allocate memory on
	/*! \param size number of bytes to allocate
		\param alloc Allocator to allocate memory on
		\return pointer to the memory location allocated */
	static void *operator new(size_t size, Allocator *alloc) throw() {
		AND_TRACE(AllocatorNewDelete.new, "Allocator");
		//! use additional space to automatically store the allocator into
		size_t sz(size);
//		sz += sizeof( AllocatorNewDelete::Header );
		void *pMem(0);
		if (alloc) {
			AND_TRACE(AllocatorNewDelete.new, "calling Allocator::Calloc");
			pMem = alloc->Calloc(1, sz);
		} else {
			AND_TRACE(AllocatorNewDelete.new, "calling ::operator new");
			pMem = ::operator new(sz);
		}
		if ( pMem ) {
//			((AllocatorNewDelete::Header*)pMem)->fAllocator = alloc;
			((AllocatorNewDelete *)pMem)->fAllocator = alloc;
			return DerivedMemStart(pMem);
		}
		throw std::bad_alloc();
	}

	//! This is the default delete method, unfortunately the Allocator from which we allocated the memory previously is not passed as an argument. Therefor we have to assume that it was allocated on our allocator and try to free it.
	/*! \param d pointer to memory location we want to free, destruction of the object has happened already
		\param size size of memory to free, can be used as hint if we need to know about the blocksize to deallocate */
	static void operator delete(void *d, size_t size) {
		AND_TRACE(AllocatorNewDelete.delete, "calling AllocatorNewDelete::operator delete, size:" << size);
		AllocatorNewDelete::operator delete(d, ThisMemStart(d)->fAllocator);
	}

	//! This method will usually only get called during an exception from within a ctor. Here we also get delegated by the single param delete method.
	/*! \param d pointer to memory location we want to free, destruction of the object has happened already
		\param alloc allocator to free memory on */
	static void operator delete(void *d, Allocator *alloc) {
		AND_TRACE(AllocatorNewDelete.delete, "Allocator");
		if (d) {
			if (alloc) {
				AND_TRACE(AllocatorNewDelete.delete, "calling Allocator::Free");
				alloc->Free( ThisMemStart(d) );
			} else {
				AND_TRACE(AllocatorNewDelete.delete, "calling ::operator delete");
				::operator delete( ThisMemStart(d) );
			}
		}
	}

	//! This is the default form of array operator new using the given Allocator to allocate memory on
	/*! \param size number of bytes to allocate
		\param alloc Allocator to allocate memory on
		\return pointer to the memory location allocated */
	static void *operator new[](size_t size, Allocator *alloc) throw() {
		AND_TRACE(AllocatorNewDelete.new[], "calling AllocatorNewDelete::operator new");
		return AllocatorNewDelete::operator new(size, alloc);
	}

	//! This is the default array delete method, unfortunately the Allocator from which we allocated the memory previously is not passed as an argument. Therefor we have to assume that it was allocated on our allocator and try to free it.
	/*! \param d pointer to memory location we want to free, destruction of the object has happened already */
	static void operator delete[](void *d) {
		AND_TRACE(AllocatorNewDelete.delete[], "calling AllocatorNewDelete::operator delete");
		AllocatorNewDelete::operator delete(d, ThisMemStart(d)->fAllocator);
	}

private:
//	struct Header {
	Allocator *fAllocator;
//	};

	static void *DerivedMemStart(void *vp) {
		return vp;
//		if (vp)
//		{
//			return ( ((char*)vp) + sizeof( AllocatorNewDelete::Header ) );
//		}
//		return 0;
	}

//	static AllocatorNewDelete::Header *ThisMemStart(void *vp)
	static AllocatorNewDelete *ThisMemStart(void *vp) {
		return (AllocatorNewDelete *)vp;
//		if ( vp )
//		{
//			return (AllocatorNewDelete::Header*)( ((char*)vp) - sizeof( AllocatorNewDelete::Header ) );
//		}
//		return 0;
	}
};

class NonVirtualTestClass : public AllocatorNewDelete
{
public:
	NonVirtualTestClass() {
	}
	~NonVirtualTestClass() {
	}
};

class VirtualBaseClass : public AllocatorNewDelete
{
public:
	VirtualBaseClass()
		: fpString(0) {
	}
	~VirtualBaseClass() {
	}
	virtual void DoBla() {
	}
	String *fpString;
};

class VirtualDerived : public VirtualBaseClass
{
public:
	VirtualDerived() {
	}
	~VirtualDerived() {
		delete fpString;
		fpString = NULL;
	}
	virtual void DoBla() {
		fpString = new String(16L, GetAllocator());
	}
};

void StorageTest::AllocatorNewDeleteTest()
{
	StartTrace(StorageTest.AllocatorNewDeleteTest);
	{
		Allocator *pAlloc(Storage::Global());
		StartTraceMem1(StorageTest.AllocatorNewDeleteTest, pAlloc);
		MemChecker mc("StorageTest.AllocatorNewDeleteTest", pAlloc);
		Trace("before automatic object");
		NonVirtualTestClass aTest;
		NonVirtualTestClass *pTest = &aTest;
		Trace("after automatic object");
		assertCompare(0L, equal_to, (long)(AllocatorNewDelete::ThisMemStart(pTest))->fAllocator);
		assertCompare(0L, equal_to, (long)pTest->GetAllocator());
		assertCompare(0LL, equal_to, mc.CheckDelta());
	}
	{
		GlobalAllocator ga;
		Allocator *pAlloc(&ga);
		StartTraceMem1(StorageTest.AllocatorNewDeleteTest, pAlloc);
		MemChecker mc("StorageTest.AllocatorNewDeleteTest", pAlloc);
		NonVirtualTestClass *pTest = new (pAlloc) NonVirtualTestClass();
		assertCompare((long)pAlloc, equal_to, (long)(AllocatorNewDelete::ThisMemStart(pTest))->fAllocator);
		assertCompare((long)pAlloc, equal_to, (long)pTest->GetAllocator());
		assertCompare(sizeof(NonVirtualTestClass), equal_to, (size_t)mc.CheckDelta());
//		assertCompare(sizeof(NonVirtualTestClass)+sizeof(AllocatorNewDelete::Header), equal_to, (size_t)mc.CheckDelta());
		delete pTest;
		assertCompare(0LL, equal_to, mc.CheckDelta());
	}
	{
		GlobalAllocator ga;
		Allocator *pAlloc(&ga);
		StartTraceMem1(StorageTest.AllocatorNewDeleteTest, pAlloc);
		MemChecker mc("StorageTest.AllocatorNewDeleteTest", pAlloc);
		VirtualBaseClass *pTest = new (pAlloc) VirtualDerived();
		assertCompare((long)pAlloc, equal_to, (long)(AllocatorNewDelete::ThisMemStart(pTest))->fAllocator);
		assertCompare((long)pAlloc, equal_to, (long)pTest->GetAllocator());
//		size_t szDerived(sizeof(VirtualDerived)+sizeof(AllocatorNewDelete::Header));
		size_t szDerived(sizeof(VirtualDerived));
		assertCompare(szDerived, equal_to, (size_t)mc.CheckDelta());
		pTest->DoBla();
		assertCompare(szDerived + (size_t)(pTest->fpString->Capacity() + (2 * 4L)), equal_to, (size_t)mc.CheckDelta());
		delete pTest;
		assertCompare(0LL, equal_to, mc.CheckDelta());
	}
	{
		PoolAllocator pa(321, 1248, 19);
		Allocator *pAlloc(&pa);
		StartTraceMem1(StorageTest.AllocatorNewDeleteTest, pAlloc);
		MemChecker mc("StorageTest.AllocatorNewDeleteTest", pAlloc);
		NonVirtualTestClass *pTest = new (pAlloc) NonVirtualTestClass();
		assertCompare((long)pAlloc, equal_to, (long)(AllocatorNewDelete::ThisMemStart(pTest))->fAllocator);
		assertCompare((long)pAlloc, equal_to, (long)pTest->GetAllocator());
//		size_t szDerived(sizeof(VirtualDerived)+sizeof(AllocatorNewDelete::Header));
		size_t szDerived(sizeof(VirtualDerived));
		assertCompare(pa.SizeHint(szDerived), equal_to, (size_t)mc.CheckDelta());
		delete pTest;
		assertCompare(0LL, equal_to, mc.CheckDelta());
	}
	{
		PoolAllocator pa(432, 1324, 19);
		Allocator *pAlloc(&pa);
		StartTraceMem1(StorageTest.AllocatorNewDeleteTest, pAlloc);
		MemChecker mc("StorageTest.AllocatorNewDeleteTest", pAlloc);
		VirtualBaseClass *pTest = new (pAlloc) VirtualDerived();
		assertCompare((long)pAlloc, equal_to, (long)(AllocatorNewDelete::ThisMemStart(pTest))->fAllocator);
		assertCompare((long)pAlloc, equal_to, (long)pTest->GetAllocator());
//		size_t szDerived(pa.SizeHint(sizeof(VirtualDerived)+sizeof(AllocatorNewDelete::Header)));
		size_t szDerived(pa.SizeHint(sizeof(VirtualDerived)));
		assertCompare(szDerived, equal_to, (size_t)mc.CheckDelta());
		pTest->DoBla();
		szDerived += pa.SizeHint(pTest->fpString->Capacity() + (2 * 4L));
		assertCompare(szDerived, equal_to, (size_t)mc.CheckDelta());
		delete pTest;
		assertCompare(0LL, equal_to, mc.CheckDelta());
	}
}

Test *StorageTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, StorageTest, PoolSizeHintTest);
	ADD_CASE(testSuite, StorageTest, SimpleCallocSetsToZeroTest);
	ADD_CASE(testSuite, StorageTest, AllocatorTest);
	ADD_CASE(testSuite, StorageTest, AllocatorTiming);
	ADD_CASE(testSuite, StorageTest, LeakTest);
	ADD_CASE(testSuite, StorageTest, AnyAssignment);
	ADD_CASE(testSuite, StorageTest, AllocatorNewDeleteTest);
	return testSuite;
}
