/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ZipStream.h"

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

// definition og GzHeader see RFC 1952
const unsigned char ZipStreamBuf::fgcGzSimpleHeader[10] =  {
	0x1f, 0x8b,
	fgcZDeflated,
	0, // flags
	0, 0, 0, 0, // time
	0, // xflags
	fgcOsCode
}; /* gzip magic header */

const unsigned char ZipStreamBuf::fgcZDeflated = 0x08;
#if defined(WIN32)
// nt os code
const unsigned char ZipOStreamBuf::fgcOsCode = 0x0b;
#else
// unix os code
const unsigned char ZipStreamBuf::fgcOsCode = 0x03;
#endif
const long ZipStreamBuf::fgcStoreSize = 16384; // zlib's Z_BUFSIZE

//---- ZipOStreamBuf ----------------------------------------------------------------
ZipOStreamBuf::ZipOStreamBuf(ostream &os, Allocator *alloc)
	: fAllocator(alloc ? alloc : Storage::Current())
	, fStore(fgcStoreSize, fAllocator)
	, fCompressed(fgcStoreSize, fAllocator)
	, fOs(&os)
	, fCrc(0)
	, isInitialized(false)
{
	xinit();
}

ZipOStreamBuf::~ZipOStreamBuf()
{
	close();
}

void ZipOStreamBuf::close()
{
	StartTrace(ZipOStreamBuf.close);

	if (!fOs) {
		return;
	}

	sync();
	flushCompressedIfNecessary();

	fZip.avail_in = 0;
	long err = Z_OK;
	do {
		err = deflate(&fZip, Z_FINISH);
		Trace("deflate finish err=" << err);
		if (err == Z_OK || err == Z_STREAM_END) {
			flushCompressed();
		}
	} while (err == Z_OK);

	Trace("bytes compressed:" << (long)fZip.total_in << " into : " << (long)fZip.total_out);
	Trace("crc is:" << (long)fCrc);
	Trace("total_out: " << (long) fZip.total_out);

// now write crc and length
	putLong(fCrc);
	putLong(fZip.total_in);
	err = deflateEnd(&fZip);
	if (err != Z_OK) {
		Trace("deflateEnd error:" << err);
	}
	fOs->flush();
	fOs = 0;
}

void ZipOStreamBuf::putLong(unsigned long value)
{
	for (size_t n = 0; n < sizeof(value); n++) {
		fOs->put(char((value & 0xff)));
		value >>= 8;
	}
}

unsigned long ZipIStreamBuf::getLong()
{
	unsigned long value = 0;
	for (size_t n = 0; n < sizeof(value); n++) {
		if (fZip.avail_in < 1) {
			fillCompressed();
		}

		unsigned char read = *fZip.next_in++;
		fZip.avail_in--;

		value += read << (8 * n);
	}
	return value;
}
void ZipOStreamBuf::flushCompressedIfNecessary()
{
	StartTrace(ZipOStreamBuf.flushCompressedIfNecessary);

	if (fZip.avail_out == 0) {
		Trace("intermediate flush of compressed buffer");
		flushCompressed();
	}
}

void ZipOStreamBuf::flushCompressed()
{
	StartTrace(ZipOStreamBuf.flushCompressed);
	Trace("buffer capacity: " << fCompressed.Capacity());
	Trace("available:       " << (long)fZip.avail_out);

	long len = fCompressed.Capacity() - fZip.avail_out;

	if (len > 0) {
		fZip.next_out = (unsigned char *)(const char *)fCompressed;
		fZip.avail_out = fCompressed.Capacity();

		fOs->write((const char *)fZip.next_out, len);
	}

	Trace("writing bytes:" << len << "at address :" << (long)fZip.next_out);

//	String strBuf;
//	StringStream stream(strBuf);
//	stream << "at address :" << hex << (long)fZip.next_out << flush;
//	Trace(strBuf);
}

int ZipOStreamBuf::sync()
{
	StartTrace(ZipOStreamBuf.sync);

	if (!isInitialized) {
		zipinit();
		isInitialized = true;
	}

	fZip.next_in = (unsigned char *) pbase();
	fZip.avail_in = pptr() - pbase();
	Trace("sync avail_in=" << (long)fZip.avail_in);
	Trace("sync avail_out=" << (long)fZip.avail_out);
	long err = Z_OK;
	fCrc = crc32(fCrc, (const Bytef *)pbase(), pptr() - pbase());
	while (fZip.avail_in > 0 && err == Z_OK) {
		flushCompressedIfNecessary();

		err = deflate(&fZip, Z_NO_FLUSH);
		Trace("deflate err = " << err);
	}
	pinit();

	return (err == Z_OK) ? 0 : -1;
}

#if !defined(ONLY_STD_IOSTREAM)
streampos ZipOStreamBuf::seekpos(streampos, int mode)
{
	return streampos(0);
}

streampos ZipOStreamBuf::seekoff(streamoff, ios::seek_dir, int mode)
{
	return streampos(0);
}
#endif

int ZipOStreamBuf::overflow(int c)
{
	sync();
	if (c != EOF && (pptr() < epptr())) // guard against recursion
#if defined(__GNUG__) && __GNUC__ < 3 /* only gnu defined xput_char() std uses sputc() */
		xput_char(c);	// without check
#else
		sputc(c);
#endif
	return 0L;  // return 0 if successful
}

int ZipOStreamBuf::underflow()
{
	return EOF;
}

void ZipOStreamBuf::xinit()
{
	setg(0, 0, 0);
	pinit();
}

void ZipOStreamBuf::pinit()
{
	char *sc = (char *)(const char *)fStore;
	char *endptr = sc + fStore.Capacity();
	setp(sc, endptr);
}

static void *ZipAlloc(void *a, uInt items, uInt size)
{
	return ((Allocator *)a)->Calloc(items, size);
}

static void ZipFree(void *a, voidpf address)
{
	((Allocator *)a)->Free(address);
}

void ZipOStreamBuf::zipinit()
{
	StartTrace(ZipOStreamBuf.zipinit);
	fOs->write((const char *)fgcGzSimpleHeader, 10);
	fZip.next_in = 0;
	fZip.avail_in = 0;
	fZip.next_out = 0;
	fZip.avail_out = 0;
	fZip.zalloc = ZipAlloc;
	fZip.zfree = ZipFree;
	fZip.opaque = fAllocator;
	long err = deflateInit2(&fZip,
							Z_BEST_COMPRESSION,	//Z_DEFAULT_COMPRESSION, doesn't work?
							Z_DEFLATED,
							-MAX_WBITS,
							8, //MEM_LEVEL
							Z_DEFAULT_STRATEGY);
	fZip.next_in = (unsigned char *)(const char *)fStore;
	fZip.next_out = (unsigned char *)(const char *)fCompressed;
	fZip.avail_out = fCompressed.Capacity();
	Assert(Z_OK == err);
	if (Z_OK != err) {
		Trace("zlib error " << err);
	}
	fCrc = crc32(0L, Z_NULL, 0);
}

ZipIStreamBuf::ZipIStreamBuf(istream &zis, istream &is, Allocator *a)
	: fStore(fgcStoreSize),
	  fCompressed(fgcStoreSize),
	  fZis(zis),
	  fIs(&is),
	  fAllocator(a),
	  fCrc(0),
	  isInitialized(false),
	  fZipErr(Z_OK)
{
	xinit();
}

ZipIStreamBuf::~ZipIStreamBuf()
{
	close();
}

int ZipIStreamBuf::overflow(int c)
{
	return EOF;
}

int ZipIStreamBuf::underflow()
{
	StartTrace(ZipIStreamBuf.underflow);

	if ((gptr() >= egptr())
		&& (fZipErr == Z_STREAM_END || (sync() != 0))) {
		Trace("EOF, fZipErr: ");

		return EOF;
	}

	Trace("still data");

	return *gptr();
}

void ZipIStreamBuf::close()
{
	StartTrace(ZipIStreamBuf.close);

	if (!fIs) {
		return;
	}

	unsigned long crc = getLong();
	unsigned long len = getLong();

	Trace("CRC check " << (long) crc << " vs. " << (long) fCrc);
	Trace("Length check: " << (long) len << " vs. " << (long) fZip.total_out);

	if (crc != fCrc || fZip.total_out != len) {
#if defined(WIN32) && !defined(ONLY_STD_IOSTREAM)
		fZis.clear(ios::badbit | fZis.rdstate());
#else
		fZis.setstate(ios::badbit);
#endif
	}

	if (Z_STREAM_END == fZipErr || Z_OK == fZipErr) {
		fZipErr = inflateEnd(&fZip);
	}

	fIs = 0;
}

void ZipIStreamBuf::xinit()
{
	StartTrace(ZipIStreamBuf.xinit);

	setp(0, 0);
	fStore = "";
	pinit(0);
}

void ZipIStreamBuf::pinit(long size)
{
	StartTrace(ZipIStreamBuf.pinit);

	char *sc = (char *)(const char *)fStore;
	setg(sc, sc, sc + size);
}

void ZipIStreamBuf::zipinit()
{
	StartTrace(ZipIStreamBuf.zipinit);

	fZip.next_in = 0;
	fZip.avail_in = 0;
	fZip.next_out = 0;
	fZip.avail_out = 0;
	fZip.zalloc = ZipAlloc;
	fZip.zfree = ZipFree;
	fZip.opaque = fAllocator;

	fZipErr = inflateInit2(&fZip, -MAX_WBITS);

	fZip.avail_in = 0;

	Assert(Z_OK == fZipErr);

	if (Z_OK == fZipErr) {
		unsigned char header[10];
		fIs->read((char *)header, sizeof(header));

		if (!(header[0] == fgcGzSimpleHeader[0] && header[1] == fgcGzSimpleHeader[1])) {
			Trace("Wrong magic number");
#if defined(WIN32) && !defined(ONLY_STD_IOSTREAM)
			fZis.clear(ios::badbit | fZis.rdstate());
#else
			fZis.setstate(ios::badbit);
#endif
			fZipErr = Z_DATA_ERROR;
		}
	} else {
		Trace("zlib error " << fZipErr);
	}

	fCrc = crc32(0L, Z_NULL, 0);
}

// assume fCompressed is empty
void ZipIStreamBuf::fillCompressed()
{
	StartTrace(ZipIStreamBuf.fillCompressed);

	Trace("Still available: " << (long)fZip.avail_in);
	Trace("fCompressed: " << (long)fZip.avail_in << "/" << fCompressed.Capacity());
	Trace("fStore: " << (long)(egptr() - gptr()));

	if (fZip.avail_in == 0) {
		fZip.next_in = (unsigned char *)(const char *)fCompressed;

		if (fIs->good()) {
			char *startRead = (char *)(const char *)fCompressed;
			long maxRead = fCompressed.Capacity();

			long nread = fIs->read(startRead, maxRead).gcount();

			fZip.avail_in += nread;
			Trace("Read: " << nread << " fCompressed: " << (long)fZip.avail_in);
		} else {
			Trace("!fIs->good()");
		}
	}
}

void ZipIStreamBuf::runInflate()
{
	StartTrace(ZipIStreamBuf.RunInflate);

	Trace("avail_in " << (long) fZip.avail_in << " avail_out " << (long) fZip.avail_out);

	fZip.next_out = (unsigned char *)(const char *)fStore;
	fZip.avail_out = fStore.Capacity();

	unsigned long preStoreLen = fZip.avail_out;

	while (fZipErr == Z_OK && fZip.avail_in > 0 && fZip.avail_out > 0) {
		fZipErr = inflate(&fZip, Z_NO_FLUSH);
	}
	if (fZipErr != Z_OK && fZipErr != Z_STREAM_END ) {
		Trace("zlib error: " << fZipErr);
#if defined(WIN32)
		fZis.clear(ios::badbit | fZis.rdstate());
#else
		fZis.setstate(ios::badbit);
#endif
	}

	unsigned long postStoreLen = preStoreLen - fZip.avail_out;
	pinit(postStoreLen);
	fCrc = crc32(fCrc, (unsigned char *) gptr(), postStoreLen );

	Trace("fCompressed: " << (long)fZip.avail_in << "/" << fCompressed.Capacity());
	Trace("fStore: " << (long) (egptr() - gptr()));
	Trace("total in: " << (long) fZip.total_in << " total out: " << (long) fZip.total_out << " fZipErr: " << (long) fZipErr);
}

int ZipIStreamBuf::sync()
{
	StartTrace(ZipIStreamBuf.sync);

	if (!isInitialized) {
		zipinit();
		isInitialized = true;
	}

	if (fZipErr == Z_OK && (gptr() >= egptr())) {
		fillCompressed();
		runInflate();
	}

	if (!(fIs->good()) && fZip.avail_in == 0 && fZipErr == Z_OK) {
		Trace("premature end of stream");
#if defined(WIN32)
		fZis.clear(ios::failbit | fZis.rdstate());
#else
		fZis.setstate(ios::failbit);
#endif
		return -1;
	}

	return (fZipErr == Z_OK || fZipErr == Z_STREAM_END) ? 0 : -1;
}
