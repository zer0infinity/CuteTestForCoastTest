/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringStreamBuf_H
#define _StringStreamBuf_H

#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION

//! StringStreamBuf adapts String objects to the iostream framework
/*! the underlying string is used directly as the buffer to save copying overhead */
namespace NSStringStream
{
	enum IoDirectionValues { eIn, eOut };
};

using std::streambuf;

template
<
typename BufferType,
		 typename IoDirType
		 >
class EXPORTDECL_FOUNDATION StringStreamBuf : public streambuf
{
public:
	typedef typename Loki::fooTypeTraits<BufferType>::ConstPlainTypeRef ConstPlainTypeRef;
	typedef typename Loki::fooTypeTraits<BufferType>::ConstPlainTypePtr ConstPlainTypePtr;
	typedef typename Loki::fooTypeTraits<BufferType>::PlainTypePtr PlainTypePtr;
	typedef typename Loki::fooTypeTraits<BufferType>::ConstCorrectPtr2RefType BufferTypeRef;

	/*! default ctor, allocates new internal String object for output
		\param mode ios modes, bitwise or of [in|out|app|ate]: if mode is ios::app or ios::ate output is appended */
	StringStreamBuf(int mode = ios::out)
		: fStore(0)
		, fDeleteStore(false)
		, fOpenMode(mode) {
		xinit();
		setbufpointers(0, 0);
	}

	/*! ctor usually used for input
		\param s contains characters to be read
		\param mode ios modes, bitwise or of [in|out|app|ate]: if mode is ios::app or ios::ate output is appended */
	StringStreamBuf(ConstPlainTypeRef s, int mode = ios::in)
		: fStore(0)
		, fDeleteStore(false)
		, fOpenMode(mode) {
		xinit((ConstPlainTypePtr)&s);
		setbufpointers(0, 0);
	}

	/*! ctor usually used for output
		\param s target string to be filled,
		\param mode ios modes, bitwise or of [in|out|app|ate]: if mode is ios::app or ios::ate output is appended */
	StringStreamBuf(PlainTypePtr s, int mode = ios::out)
		: fStore(0)
		, fDeleteStore(false)
		, fOpenMode(mode) {
		xinit(s);
		setbufpointers(0, 0);
	}

	/*! dtor, deletes string object if it has been allocated by this */
	virtual ~StringStreamBuf() {
		this->sync();
		if ( fDeleteStore ) {
			delete fStore;
			fStore = 0;
		}
		fOpenMode = 0;
	}

	/*! not much to do when synchronizing, just insert string termination character */
	virtual int sync() {
		return specificSync(IoDirType());
	}

	int specificSync(Loki::Int2Type<NSStringStream::eIn>) {
		return 0;
	}

	int specificSync(Loki::Int2Type<NSStringStream::eOut>) {
		AdjustStringLength(IoDirType());
		return 0;
	}

	/*! reveal underlying string.
		\return underlying string buffer
		\note do not change it when continuing using the stream */
	BufferTypeRef str() {
		this->sync();
		Assert(fStore);
		return *fStore;
	}

#if defined(ONLY_STD_IOSTREAM)
protected: // seekxxx are protected in the std..
	typedef std::streambuf::pos_type	pos_type;
	typedef std::streambuf::off_type	off_type;
	typedef std::ios::seekdir	seekdir;
	typedef std::ios::openmode	openmode;
#elif defined(WIN32) && !defined(ONLY_STD_IOSTREAM)
	typedef streampos pos_type;
	typedef	streamoff off_type;
	typedef ios::seek_dir seekdir;
	typedef ios::open_mode openmode;
#else
	typedef streampos pos_type;
	typedef	streamoff off_type;
	typedef ios::seek_dir seekdir;
	typedef ios::openmode openmode;
#endif

	/*! standard iostream behavior, adjust put or get position absolutely */
	virtual pos_type seekpos(pos_type p, openmode mode = (openmode)(ios::in | ios::out) ) {
		AdjustStringLength(IoDirType());
		if (long(p) >= long(fStore->Capacity())) {
			// we need to enlarge the string
			// we can only if we write
			if (! (mode & ios::out) || !reserve((long)p)) {
				// OOPS we got a problem
				return pos_type(EOF);
			}
		}
		if (mode & ios::in) {
			setgetpointer(p);
		}
		if (mode & ios::out) {
			if (fOpenMode & ios::app) { // do this on a best try basis
				if (p < fStore->Length() && fStore->Length() > 0) {
					p = fStore->Length();    // always go to the end
				}
			}
			// PS: should not be needed: if (p > fFileLength) fFileLength = p;
			setp(start(), epptr());
			if (pbase()) {
				pbump(p);    // needed, because we no longer carry base()
			}
		}
		return p;
	}

	/*! standard iostream behavior, adjust put or get position relatively */
	virtual pos_type seekoff(off_type of, seekdir dir, openmode mode = (openmode)(ios::in | ios::out) ) {
		//sync(); // will adjust fFileLength if needed
		AdjustStringLength(IoDirType()); // recognize where we have been with putting
		long pos = long(of);
		pos += (dir == ios::cur) ? long((mode & ios::in ? gptr() : pptr()) - pbase()) :
				   (dir == ios::end && fStore->Length() > 0) ? long(fStore->Length()) : 0L;
		if (pos < 0L ) {
			return pos_type(EOF);
		}
		return seekpos(pos, mode);
	}

protected:
	/*! standard iostream behavior, extends the String
		\return EOF if extension is not possible (this might be a big problem) */
	virtual int overflow(int c = EOF) {
		Assert( pbase() ? pbase() <= pptr() : 0 == pptr() );

		if (c != EOF) {
			if (reserve(pptr() - pbase() + 1)) { // this will extend the string
				Assert(pptr() < epptr());
				return sputc(c);
			}
			// we failed somehow. return EOF
			return EOF;
		}
		AdjustStringLength(IoDirType());
		return 0;
	}

	/*! standard iostream behavior, look for new characters available (no op for Strings)
		can't produce new chars because we always use the whole String
		\return EOF if no more characters for input available = EOF reached */
	virtual int underflow() {
		if (gptr() < egptr()) {
			return ZAPEOF(*gptr());    // we still got something, false alarm
		}
		return EOF; // we never handle underflow, because our buffer is the String content
	}

private:
	/*! auxiliary StringStreamBuf initialization */
	void xinit() {
		// adjust fOpenMode to contain valid combination of flags
		if (fOpenMode & (ios::ate | ios::app)) {
			fOpenMode |= ios::out;
		}
		if ( fStore && fDeleteStore ) {
			delete fStore;
		}
		fStore = new String(64L);
		fDeleteStore = true;
		Assert(fStore);
	}

	void xinit(PlainTypePtr s) {
		// adjust fOpenMode to contain valid combination of flags
		if (fOpenMode & (ios::ate | ios::app)) {
			fOpenMode |= ios::out;
		}
		if (fOpenMode & ios::trunc) {
			SS_TRACE("clear the string, i.e. ignore contents due to ios::trunc flag");
			s = 0;
		}
		// initialize buffer, allocate new String object if required
		if ( s ) {
			if ( fStore && fDeleteStore ) {
				delete fStore;
			}
			SS_TRACE("bare pointer assignment, no copy!");
			fStore = s;
			fDeleteStore = false;
		} else {
			SS_TRACE("plain vanilla empty new buffer");
			if ( fStore && fDeleteStore ) {
				delete fStore;
			}
			fStore = new String(16L);
			fDeleteStore = true;
		}
		Assert(fStore);
	}

	void xinit(ConstPlainTypePtr contents) {
		// adjust fOpenMode to contain valid combination of flags
		if (fOpenMode & (ios::ate | ios::app)) {
			fOpenMode |= ios::out;
		}
		if (fOpenMode & ios::trunc) {
			SS_TRACE("clear the string, i.e. ignore contents due to ios::trunc flag");
			contents = 0;
		}
		// initialize buffer, allocate new String object if required
		if ( contents ) {
			if ( fStore && fDeleteStore ) {
				delete fStore;
			}
			SS_TRACE("const bare pointer assignment, no copy!");
			fStore = contents;
			fDeleteStore = false;
		} else {
			SS_TRACE("!plain vanilla empty new buffer in const section!");
			if ( fStore && fDeleteStore ) {
				delete fStore;
			}
			fStore = new String(16L);
			fDeleteStore = true;
		}
		Assert(fStore);
	}

	/*! auxiliary operation to keep track of really output bytes, terminate string */
	void AdjustStringLength(Loki::Int2Type<NSStringStream::eIn>) {
	}

	void AdjustStringLength(Loki::Int2Type<NSStringStream::eOut>) {
		if (pbase() && (fOpenMode & ios::out)) {
			Assert(pptr() && pptr() >= start());
			long newlen = pptr() - pbase();
			Assert(newlen < fStore->Capacity());
			if ( fStore->GetImpl() && ( newlen > fStore->Length() ) ) {
				fStore->SetLength(newlen);
				fStore->GetContent()[newlen] = '\0'; // terminate the string
				setgetpointer(gptr() - eback()); // might be able to read more
			}
		}
	}

	/*! auxiliary operation to set iostream get buffer pointers according to parameters */
	void setgetpointer(long getoffset) {
		char *eg = 0;
		if (fStore && fStore->GetImpl()) {
			eg = const_cast<char *>(fStore->GetContent()) + fStore->Length();    // points after get area
		}
		setg((fOpenMode & ios::in) ? start() : eg, start() + getoffset , eg);
	}

	/*! auxiliary operation to set iostream buffer pointers according to parameters */
	void setbufpointers(long getoffset, long putoffset) {
		char *sc = 0;
		if (fStore->GetImpl()) {
			sc = const_cast<char *>(fStore->GetContent());
		}
		if ((fOpenMode & (ios::app | ios::ate)) && putoffset < fStore->Length()  ) {
			putoffset = fStore->Length(); // adjust it to the end
			if (fOpenMode & (ios::ate)) {
				fOpenMode &= ~ios::ate; // adjust it only once to the end
			}
		}
		// save 1 byte for '\0' termination
		char *endptr = sc + (fStore->Capacity() > 0 ? fStore->Capacity() - 1 : 0);
		// now init get and put area
		setgetpointer(getoffset);
		// if not reading the get area is defined empty.
		setp(sc, endptr);
		if (pbase()) {
			pbump(putoffset);    // needed because we no longer carry base()
		}
		if (! sc) {
			Assert(!start());
			Assert(!pbase());
			Assert(!gptr());
			Assert(!pptr());
		}
		Assert(!pptr() || ( pptr() >= fStore->GetContent() && pptr() < fStore->GetContent() + fStore->Capacity()));
		Assert(!gptr() || (gptr() >= start() && gptr() <= egptr()));
	}

	/*! enlarge the underlying String, adjust buffer pointers if needed (expand file size and mapped region) */
	bool reserve ( long newlength ) {
		// we might check if newlength is really bigger than fLength
		// second, only if fProtection includes eWrite it is possible to enlarge the file
		if (newlength >= fStore->Capacity()) {
			return doreserve(newlength, IoDirType());
		}
		return true; // space is still available, false alarm
	}

	bool doreserve( long newlength, Loki::Int2Type<NSStringStream::eIn> ) {
		return false;
	}

	bool doreserve( long newlength, Loki::Int2Type<NSStringStream::eOut> ) {
		AdjustStringLength(IoDirType());
		Assert((pptr() - pbase()) == fStore->Length());
		long putoffset = 0; // remember offsets if we readjust the mmap/file
		long getoffset = 0;
		Assert(start() == pbase());
		if (pbase() != 0 ) {
			if (pptr() > pbase()) {
				putoffset = pptr() - pbase();    // remember where we have been
			}
			if (gptr() > eback()) {
				getoffset = gptr() - eback();
			}
		}
		fStore->Reserve(newlength - fStore->Capacity() + 1); // allow 1 more for '\0'
		// optimization of enlargement is done by String class.
		setbufpointers(getoffset, putoffset); // readjust, pointer might have changed
		Assert((epptr() - pbase()) == fStore->Capacity() - 1); // allow 1 more for '\0'
		return true;
	}

	/*! auxiliary functions for interfacing to setp and setg, getting rid of
		old streambuf style setb() and base() */
	char *start() {
		return (fStore && fStore->GetImpl() ? const_cast<char *>(fStore->GetContent()) : 0);
	}

	char *endbuf() {
		if (fStore && fStore->GetImpl()) {
			return fStore->GetContent() + fStore->Capacity() - 1;
		}
		return 0;
	}

	char *endget() {
		if (fStore && fStore->GetImpl()) {
			return fStore->GetContent() + fStore->Length();
		}
		return 0;
	}

	//! internal buffer of stream
	BufferType fStore;
	//! flag for remembering who allocated fStore
	//! is true when the default constructor has created a string
	bool fDeleteStore;
	//! saves open mode from ctor params
	int fOpenMode;
};

//! adapts ios to a StringStream buffer --> THIS CLASS IS NOT TO BE INSTANTIATED
//! may be unsafe_ios
template
<
typename BufferType,
		 typename IoDirType
		 >
class EXPORTDECL_FOUNDATION StringStreambase : virtual public ios
{
public:
	typedef typename Loki::fooTypeTraits<BufferType>::ConstPlainTypeRef ConstPlainTypeRef;
	typedef typename Loki::fooTypeTraits<BufferType>::PlainTypePtr PlainTypePtr;
	typedef StringStreamBuf<BufferType, IoDirType> StreamBufType;
	typedef StringStreamBuf<BufferType, IoDirType>* StreamBufTypePtr;
	typedef typename StreamBufType::BufferTypeRef BufferTypeRef;

public:
	/*! default ctor, allocates new internal String object for output
		\param mode ios modes, bitwise or of [in|out|app|ate]: if mode is ios::app or ios::ate output is appended */
	StringStreambase(int mode = ios::out)
		: fSSBuf(mode) {
		// init from ios is needed, because ios() won't do the job; (see comment in iostream.h)
		init(&fSSBuf);
	}

	/*! ctor usually used for input
		\param s contains characters to be read
		\param mode ios modes, bitwise or of [in|out|app|ate]: if mode is ios::app or ios::ate output is appended */
	StringStreambase(ConstPlainTypeRef s, int mode = ios::in)
		: fSSBuf(s, mode) {
		init(&fSSBuf);
	}

	/*! ctor usually used for output
		\param s target string to be filled,
		\param mode ios modes, bitwise or of [in|out|app|ate]: if mode is ios::app or ios::ate output is appended */
	StringStreambase(PlainTypePtr s, int mode = ios::out)
		: fSSBuf(s, mode) {
		init(&fSSBuf);
	}

	//! dtor not much to do
	virtual ~StringStreambase() {}

	/*! reveal underlying streambuf implementation
		\return underlying streambuf */
	StreamBufTypePtr rdbuf()  {
		return &fSSBuf;
	}

	/*! reveal underlying string.
		\return underlying string buffer
		\note do not change it when continuing using the stream */
	BufferTypeRef str() {
		return fSSBuf.str();
	}

protected:
	//! the buffer with its underlying String
	StreamBufType fSSBuf;
};

#endif
