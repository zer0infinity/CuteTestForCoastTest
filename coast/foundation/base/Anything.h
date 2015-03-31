/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ANYTHING_H
#define _ANYTHING_H

#include "ITOString.h"//lint !e537
#include "AnyImplTypes.h"//lint !e537
#include "AnythingIterator.h" // new version of STL compliant iterators

class AnyImpl;
class ROAnything;
class IFAObject;
class AnyVisitor;
class AnyComparer;

/*! Flexible data container that can store any basic data type and combines hashtable and array behaviour
Anything define an easy to use data structure that comprehends built in data structures, arrays
and dictionaries (associative access aka Hashtable). It's primary use is representation of configuration
information and flexible internal datastructures like Tmp-, Role- and SessionStore.
<P>
Unless under the special circumstances described in the "memory management" section Anything
arrays display reference semantics.
<P>
<b>Multi-Threading:</b><br>Anythings are <b>not thread-safe because of the RefCounting!</b> It is therefore prohibited
to use them across thread boundaries. This implies that Anythings can only be used
within one thread. Every other use has to be read-only through ROAnything or
globally mutexed!
<P>
<b>Memory management:</b><br> To allow for scalability when used on multiprocessor machines Anything uses
its own memory management. Eventhough this is usually transparent to the user - through the use
of default parameters -  be aware that each Anything uses one specific Allocator to acquire its
memory.

Usually you will be concerned - within one thread of execution - only with Anythings that are all
using the same Allocator. Under these "default" circumstances Anything arrays will exhibit
reference semantics (this is where the story ends in most cases).
However reference semantics cannot be supported as soon as two Anythings involved in one
operation are not using the same allocator. Instead the source Anything will then be copied
transparently into the memory supplied by the Allocator of the target Anything, i.e. the target
will use a copy of the source Anything but not of a reference to the source Anything!

\note It is crucial for methods to document when they use other than the default Allocator for
returned Anythings. For documentation purposes instead of a regular Anything a TrickyThing should
be used.
*/
class Anything
{
public:
	//! Type information, can be retrieved with GetType().

	//! Constructs an Anything of type eNull (without allocator info... allocator is only used for impls)
	Anything( Allocator *a = coast::storage::Current());

	//! Constructs an Anything of type eLong
	Anything(int, Allocator *a = coast::storage::Current());

#if !defined(BOOL_NOT_SUPPORTED)
	//! Constructs an Anything of type eLong
	Anything(bool, Allocator *a = coast::storage::Current());
#endif

	//! Constructs an Anything of type eLong
	Anything(long, Allocator *a = coast::storage::Current());

	//! Constructs an Anything of type eDouble
	Anything(float, Allocator *a = coast::storage::Current());

	//! Constructs an Anything of type eDouble
	Anything(double, Allocator *a = coast::storage::Current());

	//! Constructs an Anything of type eCharPtr
	Anything(const char *, long len = -1, Allocator *a = coast::storage::Current());

	//! Constructs an Anything of type eCharPtr
	Anything(const String &, Allocator *a = coast::storage::Current());

	//! Constructs an Anything of type eVoidBuf
	Anything(void *buf, long len, Allocator *a = coast::storage::Current());

	//! Constructs an Anything of type eObject
	Anything(IFAObject *, Allocator *a = coast::storage::Current());

	struct ArrayMarker{};
	//! constructs an ArrayImpl Anything by using a marker type together with the allocator
	Anything(ArrayMarker m,Allocator *a = coast::storage::Current());

	//! Copy constructor
	Anything(const Anything &any, Allocator *a = coast::storage::Current());

	//! destructor
	~Anything();

	/*! Clones this Anything and all its content recursively.
		\param a allocator to use for the Anything and copied Impls
		\return copy of this Anything */
	Anything DeepClone(Allocator *a = coast::storage::Current()) const;

	/*! Retrieve this Anything's type information
		\return the type of this Anything, see AnyImplType. */
	AnyImplType GetType() const;

	/*! Checks if this Anything is empty i.e. of type eNull
		\return true if this Anything is of type eNull
		\return false otherwise */
	bool IsNull() const;

	/*! Retrieve the number of slots in this Anything
		\return The number of slots in this Anything */
	long GetSize() const;

	//!assignment operator creates Anything of type a.GetType() returns
	Anything &operator= (const Anything &a);

	/*! Appends a new slot and stores <I>a</I> in it.
		\param a the Anything added at the end of this Anything.
		\return the index of the slot where <I>a</I> is now stored. */
	long Append(const Anything &a);

	/*! Comparison of this Anything against other.
		\param other Anything to compare with
		\return true if there was an exact recursive match
		\note Both anythings must have exactly the same structure to be equal. Named slots must also be in the same sequence! Use AnyUtils::AnyCompareEqual to test for non-exact matches */
	bool IsEqual(const Anything &other) const;

	//! comparison of this Anything.AsCharPtr("") against other
	bool IsEqual(const char *other) const;

	friend inline bool operator== (const Anything &a1, const Anything &a2);
	friend inline bool operator!= (const Anything &a1, const Anything &a2);

	friend inline bool operator== (const Anything &a1, const char *a2);
	friend inline bool operator!= (const Anything &a1, const char *a2);
	friend inline bool operator== (const char *a1, const Anything &a2);
	friend inline bool operator!= (const char *a1, const Anything &a2);

	/*! Checks if the string <I>k</I> is stored in this Anything.
		A <I>strcmp</I> with k and the slots content AsCharPtr() is performed for all slots, until
		a match is encountered.
		<BR>Searches only in this Anything's slots, no recursive search in contained Anythings is performed.
		\param k The string to search for.
		\return <I>true</I> if any one of the strcmp test returns true; <I>false</I> otherwise. */
	bool Contains(const char *k) const;

	/*! Checks if the string <I>k</I> is stored in this Anything and returns the slotnumber.
		A <I>strcmp</I> with k and the slots content AsCharPtr() is performed for all slots, until
		a match is encountered.
		<BR>Searches only in this Anything's slots, no recursive search in contained Anythings is performed.
		\param k The string to search for.
		\return the index of the slot that contains <I>k</I> if this Anything contains <I>k</I>, -1 if not. */
	long FindValue(const char *k) const;

	/*! Remove a slot by index from this Anything. If it is not an array anything, Remove(0L) will remove the current content and initialize with an empty Anything().
		\param slot the index of the slot that is going to be removed.
		\return true in case a slot was removed, false otherwise */
	bool Remove(long slot);

	/*! Remove slot with a key from this Anything.
		\param k the key of the slot that is going to be removed.
		\return true in case a slot was removed, false otherwise */
	bool Remove(const char *k);

	/*! returns slot index of key k if defined
		\todo should not be public
		\param k the key of the slot we are looking for
		\param sizehint the length of the key, optimization used in LookupPath
		\param hashhint the hashvalue of the key, optimization used in LookupPath
		\return slot index of key k if defined
		\return -1 if key is not defined */
	long FindIndex(const char *k, long sizehint = -1, u_long hashhint = 0) const;

	/*! returns slot index of key index if defined
		\param lIdx the index of the slot we are looking for
		\return slot index of key k if defined
		\return -1 if key is not defined */
	long FindIndex(const long lIdx) const;

	//! checks if k is defined on the top level of this Anything
	bool IsDefined(const char *k) const;

	//! checks if index is defined on the top level of this Anything
	/*! \note should be similar to (lIdx >=0 && lIdx < GetSize())
		\return true if index is defined */
	bool IsDefined(const long lIdx) const;

	//!returns name of slot (if any)
	const char *SlotName(long slot) const;

	/*! accesses element slot in array allocates all elements up to slot if not there
		\param slot index into Anything array
		\return reference to the Anything at slot */
	const Anything &At(long slot) const;

	/*! accesses element slotname in array allocates it if not there
		\param slotname a key into the Anything dictionary that is mapped to an index
		\return reference to the Anything at slotname */
	const Anything &At(const char *slotname) const;

	/*! accesses element slot in array allocates all elements up to slot if not there
		\param slot index into Anything array
		\return reference to the Anything at slot */
	Anything &At(long slot);

	/*! accesses element slotname in array allocates it if not there
		\param slotname a key into the Anything dictionary that is mapped to an index
		\return reference to the Anything at slotname */
	Anything &At(const char *slotname);

	//! shorthand for At(long slot) const
	const Anything &operator[](long slot) const;
	//! shorthand for At(long slot) const
	const Anything &operator[](int slot) const;

	//! shorthand for At(const char *slotname) const
	const Anything &operator[](const char *slotname) const;

	//! shorthand for At(long slot)
	Anything &operator[](long slot);
	//! shorthand for At(long slot)
	Anything &operator[](int slot);

	//!shorthand for At(const char *slotname)
	Anything &operator[](const char *slotname);

	/*! Tries to retrieve a sub Anything at the given path into the result Anything.
		\param result resulting Anything at path if it is defined or Anything of type eNull if nothing was found
		\param path a path expression delimited by delimSlot and/or delimIdx, e.g. first.second:1 searches for key first and within first for key second and within second for 'second' sub-Anything
		\param delimSlot delimiter in path expression for named slots
		\param delimIdx delimiter in path expression for unnamed slots
		\return true if the path matched an Anything, else false
		\note Unmatched slots of the path expression <b>will not be created</b> in this Anything */
	bool LookupPath(Anything &result, const char *path, char delimSlot = '.', char delimIdx = ':') const;

	/*! returns a const char * representation of the implementation if any is set else the default
		this method doesn't copy memory */
	const char *AsCharPtr(const char *dflt = 0) const;

	/*! returns a const char * representation of the implementation if any is set else the default
		this method doesn't copy memory */
	const char *AsCharPtr(const char *dflt, long &buflen) const;

	/*! returns a String representation of the implementation if any is set else the default, the string copies memory
		beware of temporaries */
	String AsString(const char *dflt = 0) const;

	//! returns a long representation of the implementation if possible else the default
	long AsLong(long dflt = 0) const;

	//! returns a bool representation of the implementation if possible else the default
	bool AsBool(bool dflt = false) const;

	//! returns a double representation of the implementation if possible else the default
	double AsDouble(double dlft = 0.0) const;

	//! returns a double representation of the implementation if possible else the default
	IFAObject *AsIFAObject(IFAObject *dflt = 0) const;

	/*! serialize content to external format
		\param os ostream to write to
		\param pretty if true creates output with newlines */
	std::ostream &PrintOn(std::ostream &os, bool pretty = true) const;
	friend inline std::ostream &operator<< (std::ostream &os, const Anything &a);
	friend inline std::istream &operator>> (std::istream &is, Anything &a);

	/*! serialize content to ostream in external format
		\param os ostream to write to
		\param level  number of tabs to insert in front of main structure */
	void Export(std::ostream &os, int level = 0) const;

	/*! import Anything from istream, fname is a hint for the error handler if an error occurs
		\param is istream to read from
		\param fname name of the file, if reading from a file, used for error handling
		\return true if the reading was successful, false if an syntax error occurred */
	bool Import(std::istream &is, const char *fname = 0);

	//!RefCount accessor for debugging
	long RefCount() const;

	//!return the allocator that is used by this Anything (once set the allocator cannot be changed)
	Allocator *GetAllocator() const;
	//! manually set the allocator (should not usually be used...)
	bool SetAllocator(Allocator *a);

	/*! return an error message if result is false, empty string otherwise
		used for assertAnyEqual macro in Test.h of the Testframework */
	static String CompareForTestCases(const ROAnything &expected, const ROAnything &actual, bool &result);

	/*! ensure the given anything is really an array
		\param anyToEnsure anything to check and adjust if not already an AnyArrayImpl */
	static void EnsureArrayImpl(Anything &anyToEnsure);

	void Accept(AnyVisitor &v, long lIdx = -1, const char *slotname = 0) const;
	//!in-core sort of Anything by its keys, fast!
	void SortByKey();
	//!legacy variant only useful for SlotnameSorter legacy behavior.
	/*! we do not need to sort in descending order,
		since we can always iterate from the end (GetSize()-1) */
	void SortReverseByKey();
	//!in-core sort of Anything by String values, temporary, will provide comparer interface
	void SortByStringValues();
	//!in-core sort of Anything by String values, temporary, will provide comparer interface
	void SortByAnyComparer(const AnyComparer &comparer);

//------ support STL compliant container behavior
	typedef Anything_iterator iterator;
	typedef Anything_const_iterator const_iterator;
	typedef std::reverse_iterator<Anything_iterator> reverse_iterator;
	typedef std::reverse_iterator<Anything_const_iterator> const_reverse_iterator;
	typedef long difference_type;
	typedef long size_type;
	typedef Anything &reference;
	typedef const Anything &const_reference;
	typedef Anything value_type;
	iterator begin();
	iterator end();
	const_iterator begin()const;
	const_iterator end() const;
	reverse_iterator rbegin();
	reverse_iterator rend();
	const_reverse_iterator rbegin()const;
	const_reverse_iterator rend() const;
	void clear();
	bool empty() const {
		return 0 == size();   // an empty array is not IsNull()
	}
	iterator erase(iterator pos);
	iterator erase(iterator from, iterator to);
	size_type max_size() const {
		return 0;   //SOP: determine max size...
	}
	size_type size() const {
		return GetSize();
	}
	void swap(Anything &that);
	reference at(size_type n) {
		return At(n);
	}
	const_reference at(size_type n) const {
		return At(n);
	}
	reference back() {
		return at(size() - 1);
	}
	const_reference back() const {
		return at(size() - 1);
	}
	reference front() {
		return at(0);
	}
	const_reference front() const {
		return at(0);
	}
	// operator [] already defined
	void pop_back() {
		erase(end() - 1);
	}
	void pop_front() {
		erase(begin());
	}
	void push_back(const value_type &a) {
		Append(a);
	}
	void push_front(const value_type &a) {
		insert(begin(), a);
	}
	template <typename InputIterator>
	void assign(InputIterator first, InputIterator last) {
		clear();
		for (; first != last; ++first) {
			Append(*first);
		}
	}
	void assign(size_type n, const value_type &val) {
		clear();
		for (; n > 0; --n) {
			Append(val);
		}
	}
	// deal with case where an integral value is assigned n times
	void assign(size_type n, size_type val) {
		assign(n, value_type(val));
	}
	// sequence ctors:
	Anything(size_type n, const value_type &v, Allocator *a = coast::storage::Current())
		: fAnyImp(0) {
		SetAllocator(a);
		for (; n > 0; --n) {
			Append(v);
		}
	}

	template <typename InputIterator>
	Anything(InputIterator first, InputIterator last): fAnyImp(0) {
		SetAllocator(coast::storage::Current());
		for (; first != last; ++first) {
			Append(*first);
		}
	}

	template <typename InputIterator>
	Anything(InputIterator first, InputIterator last, Allocator *a): fAnyImp(0) {
		SetAllocator(a);
		for (; first != last; ++first) {
			Append(*first);
		}
	}
	// special case for size_type integral assignment, sorry not DRY,
	// because of unavailable constructor chaining in C++
	// and because of inability to use default arguments in specialized templates
	Anything(size_type n, size_type v): fAnyImp(0) {
		SetAllocator(coast::storage::Current());
		for (; n > 0; --n) {
			Append(v);
		}
	}

	Anything(size_type n, size_type v, Allocator *a): fAnyImp(0) {
		SetAllocator(a);
		for (; n > 0; --n) {
			Append(v);
		}
	}

	iterator insert(iterator pos, const value_type &v) {
		return do_insert(pos, 1, v);
	}
	void insert(iterator pos, size_type n, const value_type &v) {
		do_insert(pos, n, v);
	}
	template <typename InputIterator>
	void insert(iterator pos, InputIterator first, InputIterator last) {
		// first a naive impl, could be more efficient for Random-access iterators, but... see stl impl of vector::insert():
		for (; first != last; ++first, ++pos) {
			do_insert(pos, 1, value_type(*first));
		}
	}
	// specialize for integral types to mean non-template insert of above
	void insert(iterator pos, size_type first, long last) {
		do_insert(pos, first, last);
	}
	// additional methods close to STL's associative (hashed) containers
	typedef const char *key_type;
	size_type erase(key_type k) {
		return Remove(k);
	}
	iterator find(key_type k) {
		long found = FindIndex(k);
		return (found < 0 ? end() : iterator(*this, found));
	}
	const_iterator find(key_type k)const {
		long found = FindIndex(k);
		return (found < 0 ? end() : const_iterator(*this, found));
	}
	size_type count(key_type k)const {
		return IsDefined(k) ? 1 : 0;
	}
	typedef std::pair<iterator, iterator> range;
	range equal_range(key_type k) {
		long found = FindIndex(k);
		return (found < 0 ? range(end(), end())
				: range(iterator(*this, found), iterator(*this, found + 1)));
	}
	typedef std::pair<const_iterator, const_iterator> const_range;
	const_range equal_range(key_type k)const {
		long found = FindIndex(k);
		return (found < 0 ? const_range(end(), end())
				: const_range(const_iterator(*this, found), const_iterator(*this, found + 1)));
	}

protected:
	// same impl for both value inserts
	iterator do_insert(iterator pos, size_type n, const value_type &v);
	//!changes fAnyImp from simple type to array
	void Expand();
	//!makes room for insert() implementation, moves elements eventually from slot to slot+size
	void InsertReserve(long slot, long sz);

	//!get Anything at i; create necessary entries if i is out of range
	Anything& DoAt(long i);

	//!get Anything at i after check that i is a valid index
	Anything& DoGetAt(long i);

	//!get Anything at s create entry if necessary
	Anything& DoAt(const char *s);

	//!get Anything at i; return dummy in case of non-existance
	Anything const &DoAt(long i) const;

	//!get Anything at i after check that i is a valid index
	Anything const &DoGetAt(long i) const;

	//!get Anything at s return dummy if not found
	Anything const &DoAt(const char *s) const;

	/*! Clones this Anything and all its content recursively.
		\return the copy of this Anything */
	Anything DeepClone(Allocator *a, Anything &xref) const;

	friend class AnyArrayImpl; //!@FIXME: needed because of DeepClone(Allocator,Anything) above from within AnyArrayImpl
	friend class ROAnything;
	friend struct Sorter;
	friend class AnyImpl;

	Anything(AnyImpl *);
	//!@TODO: this is a loophole that discards const-ness
	AnyImpl *GetImpl();
	AnyImpl const *GetImpl()const;
	Allocator *GetImplAllocator() const ;
	union {
		AnyImpl *fAnyImp;
		Allocator *fAlloc;
		std::ptrdiff_t bits;
	};
	friend class AnythingTest;
};

long IFAHash(const char *key, long &len, char stop1 = '\0', char stop2 = '\0');

/*! Use this class to get a slot from an Anything according to configuration
Takes care of both . and : path separators.
To use this class call Operate on it.
The config Anything should have the form
<PRE>
{
	/Slot	Level1.Level2	mandatory, spec producing the Slot that is searched - if it does not exists it is created
	/Delim   				optional, default ".", first char is taken as delimiter for named slots
	/IndexDelim				optional, default ":", first char is taken as delimiter for indexed slots
}
</PRE>
You can specify the Slotname as a dot/colon separated list of names to retrieve slots from any
hierarchy level (e.g fields.System).
*/
class SlotFinder
{
public:
	/*! looks up the slot in the given Anything, creates it if not found.
		\param source The source Anything in which the slot should be found
		\param dest The Anything reference to the slot found
		\param config the configuration
		\post dest = source.LookupPath(config["Slot"].AsString("")) */
	static void Operate(Anything &source, Anything &dest, const Anything &config);

	/*! looks up the slot in the given Anything, creates it if not found.
		\param source The source Anything in which the slot should be found
		\param dest The Anything reference to the slot found
		\param config the configuration
		\post dest = source.LookupPath(config["Slot"].AsString("")) */
	static void Operate(Anything &source, Anything &dest, const ROAnything &config);

	/*! looks up the slot in the given Anything, creates it if not found.
		\param source The source Anything in which the slot should be found
		\param dest The Anything reference to the slot found
		\param destSlotname name of slot to find using LookupPath semantics
		\param delim LookupPath slot delimiter
		\param indexdelim LookupPath index delimiter
		\post dest = source.LookupPath(destSlotname, delim, indexdelim) */
	static void Operate(Anything &source, Anything &dest, String destSlotname, char delim = '.', char indexdelim = ':' );

	/*! returns the source Anything in such way that an assignment can be made in the form dest[destSlotname] = xxx; or dest[destIdx] = xxx;
		\param dest resulting Anything, one level below for easy assignment operations
		\param destSlotname last segment of config[Slot] or empty if it is an index
		\param destIdx last segment of config[Slot] as index or -1 if not an index
		\param delim LookupPath slot delimiter
		\param indexdelim LookupPath index delimiter */
	static bool IntOperate(Anything &dest, String &destSlotname, long &destIdx, char delim = '.', char indexdelim = ':');
};
/*! Use this class to put a Anything into another Anything using lookup-syntax
To use this class call Operate on it.
The config Anything should have the form
<PRE>
{
	/Slot	Level1.Level2	mandatory, spec producing the Slot that is assigned to source - if it does not exists it is created
	/Append   				optional, default false, append source to destination, overwrite otherwise
	/Delim   				optional, default ".", first char is taken as delimiter for named slots
	/IndexDelim				optional, default ":", first char is taken as delimiter for indexed slots
}</PRE>
You can specify the Slotname as a dot separated list of names to retrieve slots from any
hierarchy level (e.g fields.System).
If /Slot contains an empty string ("") nothing will happen
*/
class SlotPutter
{
public:
	/*! puts the Anything source into dest using a <I>LookupPath</I>-like slot specification in config /Slot
		\param source The Anything that provides the data, remains unchanged
		\param dest The Anything that is updated
		\param config the configuration
		\post dest.LookupPath(config["Slot"].AsString("")) = source */
	static void Operate(Anything &source, Anything &dest, const Anything &config);

	/*! puts the Anything source into dest using a <I>LookupPath</I>-like slot specification in config /Slot
		\param source The Anything that provides the data, remains unchanged
		\param dest The Anything that is updated
		\param config the configuration
		\post dest.LookupPath(config["Slot"].AsString("")) = source */
	static void Operate(Anything &source, Anything &dest, const ROAnything &config);

	/*! puts the Anything source into dest using a <I>LookupPath</I>-like slot specification
		\param source The Anything that provides the data, remains unchanged
		\param dest The Anything that is updated
		\param destSlotname slotname to put destination Anything into
		\param append set to true if the source anything should be appended to existing content
		\param delim LookupPath slot delimiter
		\param indexdelim LookupPath index delimiter
		\post dest.LookupPath(destSlotname, delim, delimIdx)[.Append] = source */
	static void Operate(Anything &source, Anything &dest, String destSlotname, bool append = false, char delim = '.', char indexdelim = ':');
};
/*! Use this class to remove a slot from a Anything using lookup-syntax
To use this class call Operate on it.
The config Anything should have the form
<PRE>
{
	/Slot	Level1.Level2	mandatory, spec producing the Slot that is assigned to source - if it does not exists it is created
	/RemoveLast				optional, bool(0L|1L), default false(0L), remove last slot in given path otherwise the whole Anything at path will be removed
	/Delim   				optional, default ".", first char is taken as delimiter for named slots
	/IndexDelim				optional, default ":", first char is taken as delimiter for indexed slots
}</PRE>
You can specify the Slotname as a dot separated list of names to retrieve slots from any
hierarchy level (e.g fields.System).
If /Slot contains an empty string ("") nothing will happen
*/
class SlotCleaner
{
public:
	/*! removes the Anything from dest using a <I>LookupPath</I>-like slot specification in config /Slot
		\param dest The Anything that is updated
		\param config the configuration
		\post dest.LookupPath(config["Slot"].AsString("")) is removed */
	static void Operate(Anything &dest, const Anything &config);

	/*! removes the Anything from dest using a <I>LookupPath</I>-like slot specification in config /Slot
		\param dest The Anything that is updated
		\param config the configuration
		\post dest.LookupPath(config["Slot"].AsString("")) is removed */
	static void Operate(Anything &dest, const ROAnything &config);

	/*! removes the Anything from dest using a <I>LookupPath</I>-like slot specification
		\param dest The Anything that is updated
		\param slotName name of slot to remove
		\param removeLast set to true if only the last element within the slotName should be removed
		\param delim LookupPath slot delimiter
		\param indexdelim LookupPath index delimiter
		\post dest.LookupPath(slotName, delim, indexdelim) is removed */
	static void Operate(Anything &dest, String slotName, bool removeLast = false, char delim = '.', char indexdelim = ':');
};
/*! Use this class to copy slots from an Anything to another
To use this class call Operate on it.
The config Anything should have the form
<PRE>
{
	/SourceSlotName		DestinationSlotName
...
}</PRE>
The method iterates over all slots in config. If source.LookupPath(SourceSlotName) returns an Anything,
it is copied into dest[DestinationSlotName].<BR>
Note, you can specify SourceSlotName as a dot/colon separated list of slotnames to retrieve slots from any
hierarchy level (e.g fields.System). The result is always copied into toplevel slot of dest.
*/
class SlotCopier
{
public:
	/*! Copies slots from source to dest accoring to config
		\param source The Anything that provides the data, remains unchanged
		\param dest The Anything that is updated.
		\param config Contains the list of source and destination slotnames.
		\param delim LookupPath slot delimiter
		\param indexdelim LookupPath index delimiter */
	static void Operate(Anything &source, Anything &dest, const Anything &config, char delim = '.', char indexdelim = ':');
	/*! Copies slots from source to dest accoring to config
		\param source The Anything that provides the data, remains unchanged
		\param dest The Anything that is updated.
		\param config Contains the list of source and destination slotnames.
		\param delim LookupPath slot delimiter
		\param indexdelim LookupPath index delimiter */
	static void Operate(Anything &source, Anything &dest, const ROAnything &config, char delim = '.', char indexdelim = ':');
};

/*! Interface for comparing Anythings. Subclasses may be used to sort Any-Arrays by value
 * see AnyComparers.h
 */
class AnyComparer
{
public:
	//! compare left and right, return <0 when left is smaller, >0 when left is greater, 0 on equality
	virtual int Compare(const Anything &left, const Anything &right)const = 0;
	//! virtual dtor for subclass convenience
	virtual ~AnyComparer() {}
};

//!Sorts the slots of an Anything by Slotname, legacy (SOP)
/*! bad api design, should better use bool for reverse */
class SlotnameSorter
{
public:
	enum EMode { asc, desc };
	/*! Sorts the Anything <I>toSort</I> by slotname
		\param toSort in/out - the Anything to be sorted
		\param mode define sort mode, ascending or descending, default is ascending */
	static void Sort(Anything &toSort, EMode mode = asc);
};

/*! TrickyThing is a special Anything that is used exclusively for documentation purposes.
	Under special circumstances the memory management employed for Anythings may lead to
	non intuitive behavior which requires special attention by programmers. TrickyThing
	is used to mark these few places in the source code:

	Anything ensures that memory allocated for the implementation always is contained in
	just <B>ONE</B> memory region served by <B>ONE</B> allocator.

	Within the same allocator, Anything employs reference semantics, the usual case.
	In a simple or single threaded application there is only one memory allocator.
	A multi-threaded application might employ multiple thread-specific allocators.

	Unfortunately reference semantics for Anythings cannot be supported across allocator
	boundaries. This means an Anything allocated by one allocator will never refer to
	an Anything allocated by a different allocator. Instead the referenced Anything will
	automatically be copied into the allocator of the target Anything (you get a reference
	to a clone instead of the original reference).

	To avoid confusion about unexpected clones it is important to recognize the few places
	where Anythings with different allocators might be involved. TrickyThing will point
	you to all these places where special care is in order. In all the other places
	you can safely assume Anythings to have reference semantics.

	usage:
<pre>
	TrickyThing t(context.GetRoleStore()); // no deep copy, assume different allocator
	// work on with t like an Anything but keep reference semantics to the
	// source of it.
	Anything b = 5; // assume local allocator
	t["foo"] = b; // deep copy b to t's part
	TrickyThing tt ( t["foo"]); // keep reference semantic
	tt["baz"] = t["bar"]; // no deep copy
</pre>
 */
class TrickyThing : public Anything
{
public:
	/*! constructor used for tricky member variables of long-lived classes
		typically those using global allocators */
	TrickyThing(Allocator *a = coast::storage::Current()) :
		Anything(Anything::ArrayMarker(),a) {
	}

	/*! constructor used for tricky Anything using potentially a different allocator
		be careful if the memory of the tricky thing is short lived!!!
		\param any the tricky Anything we use its allocator for reference semantics */
	TrickyThing(TrickyThing &any) : Anything(any, any.GetAllocator()) {}
};//lint !e1509

/*! ROAnything is an Anything which is immutable for MT-Safe reasons.
	Every operation which would change the underlying data is therefore disabled
	This is the mandatory implementation for Anything data that is shared between
	threads. It is read only and prohibits writing. An ROAnything is empty or has
	an underlying Anything that manages the memory
 */
class ROAnything
{
public:
	ROAnything();
	ROAnything(const Anything &a);
	//! Copy constructor
	ROAnything(const ROAnything &a);

	/*! Clones this Anything and recursivley all its content.
		\return the copy of this Anything */
	Anything DeepClone(Allocator *a = coast::storage::Current()) const;

	/*! Retrieve this ROAnything's type information
		\return the type of this ROAnything, see Enumeration AnyImplType. */
	AnyImplType GetType() const;
	/*! Checks if this ROAnything is empty i.e. of type eNull
		\return <I>true</I> if this ROAnything is of type eNull; <i>false</i> otherwise */
	bool IsNull() const;
	/*! Retrieve the number of slots in this ROAnything
		\return The number of slots in this ROAnything */
	long GetSize() const;

	ROAnything &operator= (const ROAnything &a);
	ROAnything &operator= (const Anything &a);

public:
	// comparison
	bool IsEqual(const ROAnything &other) const;
	bool IsEqual(const Anything &other) const;
	bool IsEqual(const char *other) const;

	friend inline bool operator== (const ROAnything &a1, const ROAnything &a2);
	friend inline bool operator!= (const ROAnything &a1, const ROAnything &a2);

	friend inline bool operator== (const ROAnything &a1, const char *a2);
	friend inline bool operator!= (const ROAnything &a1, const char *a2);
	friend inline bool operator== (const char *a1, const ROAnything &a2);
	friend inline bool operator!= (const char *a1, const ROAnything &a2);

	/*! Checks if the string <I>k</I> is stored in this Anything.
		A <I>strcmp</I> with k and the slots content AsCharPtr() is performed for all slots, until
		a match is encountered.
		<BR>Searches only in this Anything's slots, no recursive search in contained Anythings is performed.
		\param k The string to search for.
		\return <I>true</I> if any one of the strcmp test returns true; <I>false</I> otherwise. */
	bool Contains(const char *k) const;
	/*! Checks if the string <I>k</I> is stored in this Anything and returns the slotnumber.
		A <I>strcmp</I> with k and the slots content AsCharPtr() is performed for all slots, until
		a match is encountered.
		<BR>Searches only in this Anything's slots, no recursive search in contained Anythings is performed.
		\param k The string to search for.
		\return the number of the slot that contains <I>k</I> if this Anything contains <I>k</I>, -1 if not. */
	long FindValue(const char *k) const;
	// FindIndex shouldn't be public, since its return value is of no use outside, except for -1 saying undefined
	// returns slot index
	long FindIndex(const char *k, long sizehint = -1, u_long hashhint = 0) const;
	// not useful anyway?
	long FindIndex(const long lIdx) const;
	bool IsDefined(const char *k) const;
	//! eventually synonym to (lIdx >=0 && lIdx < GetSize()), really useful?
	bool IsDefined(const long lIdx) const;

	// returns name of slot (if any)
	const char *SlotName(long slot) const;
	const String &VisitSlotName(long slot) const;

	const ROAnything At(long slot) const;
	const ROAnything At(const char *slotname) const;
	const ROAnything operator[](long slot) const;
	const ROAnything operator[](int slot) const;
	const ROAnything operator[](const char *slotname) const;

	bool LookupPath(ROAnything &result, const char *path, char delimSlot = '.', char delimIdx = ':') const;

	// conversion
	const char *AsCharPtr(const char *dflt = 0) const;
	const char *AsCharPtr(const char *dflt, long &buflen) const;
	String AsString(const char *dflt = 0) const;

	long AsLong(long dflt = 0) const;
	bool AsBool(bool dflt = false) const;
	double AsDouble(double dlft = 0.0) const;
	IFAObject *AsIFAObject(IFAObject *dflt = 0) const;

	// output only
	std::ostream &PrintOn(std::ostream &os, bool pretty = true) const;
	friend inline std::ostream &operator<< (std::ostream &os, const ROAnything &a);

	void Export(std::ostream &fp, int level = 0) const;

	void Accept(AnyVisitor &v, long lIdx = -1, const char *slotname = 0) const;

protected:
	long AssertRange(long) const;
	long AssertRange(const char *) const;

	AnyImpl const *fAnyImp;
	//hack for visitor...
	friend class AnyArrayImpl;
	ROAnything(AnyImpl const *imp)
		: fAnyImp( imp )
	{}
};

inline const Anything &Anything::operator[](long slot) const
{
	return At(slot);
}
inline const Anything &Anything::operator[](int slot) const
{
	return At(long(slot));
}
inline const Anything &Anything::operator[](const char *slotname) const
{
	return At(slotname);
}

inline Anything &Anything::operator[](long slot)
{
	return At(slot);
}
inline Anything &Anything::operator[](int slot)
{
	return At(long(slot));
}
inline Anything &Anything::operator[](const char *slotname)
{
	return At(slotname);
}

inline bool Anything::IsDefined(const long lIdx) const
{
	return FindIndex(lIdx) >= 0;
}
inline bool Anything::IsDefined(const char *k) const
{
	return FindIndex(k) >= 0;
}
inline bool Anything::IsNull() const
{
	return (GetImpl() == 0);
}
inline std::ostream &operator<< (std::ostream &os, const Anything &a)
{
	return a.PrintOn(os);
}
inline std::istream &operator>> (std::istream &is, Anything &a)
{
	a.Import(is);
	return is;
}

inline bool operator== (const Anything &a1, const Anything &a2)
{
	return a1.IsEqual(a2);
}
inline bool operator!= (const Anything &a1, const Anything &a2)
{
	return !a1.IsEqual(a2);
}

inline bool operator== (const Anything &a, const char *s)
{
	return a.IsEqual(s);
}
inline bool operator!= (const Anything &a, const char *s)
{
	return !a.IsEqual(s);
}
inline bool operator== (const char *s, const Anything &a)
{
	return a.IsEqual(s);
}
inline bool operator!= (const char *s, const Anything &a)
{
	return !a.IsEqual(s);
}

inline const ROAnything ROAnything::operator[](long slot) const
{
	return At(slot);
}
inline const ROAnything ROAnything::operator[](int slot) const
{
	return At(long(slot));
}
inline const ROAnything ROAnything::operator[](const char *slotname) const
{
	return At(slotname);
}

inline bool ROAnything::IsDefined(const char *k) const
{
	return FindIndex(k) >= 0;
}
inline bool ROAnything::IsDefined(const long lIdx) const
{
	return FindIndex(lIdx) >= 0;
}
inline bool ROAnything::IsNull() const
{
	return (fAnyImp == 0);
}

inline std::ostream &operator<< (std::ostream &os, const ROAnything &a)
{
	return a.PrintOn(os);
}

inline bool operator== (const ROAnything &a1, const ROAnything &a2)
{
	return a1.IsEqual(a2);
}
inline bool operator!= (const ROAnything &a1, const ROAnything &a2)
{
	return !a1.IsEqual(a2);
}

inline bool operator== (const ROAnything &a, const char *s)
{
	return a.IsEqual(s);
}
inline bool operator!= (const ROAnything &a, const char *s)
{
	return !a.IsEqual(s);
}
inline bool operator== (const char *s, const ROAnything &a)
{
	return a.IsEqual(s);
}
inline bool operator!= (const char *s, const ROAnything &a)
{
	return !a.IsEqual(s);
}

#endif
