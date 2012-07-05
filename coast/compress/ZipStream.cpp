/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ZipStream.h"
#include "Anything.h"
#include "Tracer.h"
#include "ConversionUtils.h"
#include "SystemLog.h"
#include <cstring>	// memset

// following section copied out of zutil.h
// DO NOT TRY TO INCLUDE IT DIRECTLY BECAUSE OF SOME REDEFINITIONS!

#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif

#if defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_CODE  GzipHdr::eFAT
#elif defined(AMIGA)
#  define OS_CODE  GzipHdr::eAmiga
#elif defined(OS2)
#  define OS_CODE  GzipHdr::eHPFS
#elif defined(TOPS20)
#  define OS_CODE  GzipHdr::eTOPS_20
#elif defined(WIN32)
#  if !defined(__CYGWIN__)  /* Cygwin is Unix, not Win32 */
#    define OS_CODE  GzipHdr::eNTFS
#  endif
#elif defined(__50SERIES) /* Prime/PRIMOS */
#  define OS_CODE  GzipHdr::ePrime_PRIMOS
#elif defined(__linux__) || defined(__sun)
#  define OS_CODE  GzipHdr::eUnix
#else
#  define OS_CODE  GzipHdr::eUnknownOS
#endif

GzipHdr::GzipHdr()
	: ID1(gz_magic[0])
	, ID2(gz_magic[1])
	, CompressionMethod(GzipHdr::eDeflate)
	, Flags(0)
	, ExtraFlags(GzipHdr::eUnspecified)
	, OperatingSystem(OS_CODE)
	, XLEN(0)
	, CRC16(0L)
	, fbValid(true)
{
	memset(ModificationTime, '\0', 4L);
}

bool GzipHdr::IsValid()
{
	StartTrace(GzipHdr.IsValid);
	// fbValid could be set to false in case we could not read enough header bytes from stream
	// and check the magic numbers
	if ( fbValid && ID1 == gz_magic[0] && ID2 == gz_magic[1] ) {
		return true;
	}
	return false;
}

TimeStamp GzipHdr::GetModificationTime() const
{
	StartTrace(GzipHdr.GetModificationTime);
	long lModTime = 0L;
	ConversionUtils::GetValueFromBuffer(ModificationTime, lModTime);
	Trace("Modtime as long:" << lModTime);
	return TimeStamp(lModTime);
}

void GzipHdr::SetModificationTime(TimeStamp aStamp)
{
	StartTrace(GzipHdr.SetModificationTime);
	long lModTime = aStamp.AsLong();
	Trace("Modtime " << aStamp.AsString() << ", as long:" << lModTime);
	for (long lIdx = 0; lIdx < GzipHdr::eModificationTimeLen; ++lIdx) {
		ModificationTime[lIdx] = ( lModTime & 0xff );
		lModTime >>= 8;
	}
}

std::ostream &operator<<(std::ostream &os, GzipHdr &header)
{
	StartTrace(GzipHdr.operator << );
	String strHeader((void *)&header, 10);
	if ( header.Flags & GzipHdr::eFEXTRA ) {
		strHeader.Append(char(header.XLEN & 0xff));
		strHeader.Append(char((header.XLEN >> 8) & 0xff));
		strHeader.Append(header.ExtraField);
	}
	if ( header.Flags & GzipHdr::eFNAME ) {
		// write filename including terminating 0
		strHeader.Append(header.FileName);
		strHeader.Append('\0');
	}
	if ( header.Flags & GzipHdr::eFCOMMENT ) {
		// write comment including terminating 0
		strHeader.Append(header.Comment);
		strHeader.Append('\0');
	}
	if ( header.Flags & GzipHdr::eFHCRC ) {
		header.CRC16 = 0L;
		header.CRC16 = crc32(header.CRC16, (const Bytef *)(const char *)strHeader, strHeader.Length());
		Trace("CRC32: " << header.CRC16);
		header.CRC16 = (header.CRC16 & 0xffff);
		Trace("CRC16: " << header.CRC16);
		strHeader.Append(char(header.CRC16 & 0xff));
		strHeader.Append(char((header.CRC16 >> 8) & 0xff));
	}
	Trace("Writing a Header of " << strHeader.Length() << "bytes:\n" << strHeader.DumpAsHex());
	os.write(strHeader, strHeader.Length());
	return os;
}

std::istream &operator>>(std::istream &is, GzipHdr &header)
{
	StartTrace(GzipHdr.operator >> );
	// set to false if we could not read at lest 10 bytes of the header
	if ( ( header.fbValid = !is.read((char *)&header, 10L).eof() ) ) {
		String strHeader((void *)&header, 10L);
		Trace("10 bytes of header could be read");
		// check magic numbers
		bool bCont = header.IsValid();
		if ( bCont && header.Flags & GzipHdr::eFEXTRA ) {
			bCont = !ConversionUtils::GetValueFromStream(is, header.XLEN).eof();
			strHeader.Append(char(header.XLEN & 0xff));
			strHeader.Append(char((header.XLEN >> 8) & 0xff));
			if ( bCont ) {
				char *pArr = new char[header.XLEN];
				if ( ( bCont = !is.read(pArr, header.XLEN).eof() ) ) {
					header.ExtraField.Trim(0);
					header.ExtraField.Append((void *)pArr, header.XLEN);
					strHeader.Append(header.ExtraField);
				}
				delete[] pArr;
			}
		}
		if ( bCont && header.Flags & GzipHdr::eFNAME ) {
			char aChar;
			header.FileName.Trim(0);
			// read filename including terminating 0
			while ( bCont && ( bCont = !is.read(&aChar, 1L).eof() ) && aChar != '\0' ) {
				header.FileName.Append(aChar);
			}
			Trace("Filename is [" << header.FileName << "]");
			strHeader.Append(header.FileName);
			strHeader.Append('\0');
		}
		if ( bCont && header.Flags & GzipHdr::eFCOMMENT ) {
			char aChar;
			header.Comment.Trim(0);
			// read comment including terminating 0
			while ( bCont && ( bCont = !is.read(&aChar, 1L).eof() ) && aChar != '\0' ) {
				header.Comment.Append(aChar);
			}
			Trace("Comment is [" << header.Comment << "]");
			strHeader.Append(header.Comment);
			strHeader.Append('\0');
		}
		if ( bCont && header.Flags & GzipHdr::eFHCRC ) {
			if ( !ConversionUtils::GetValueFromStream(is, header.CRC16).eof() ) {
				long locCRC16 = 0L;
				locCRC16 = crc32(locCRC16, (const Bytef *)(const char *)strHeader, strHeader.Length());
				locCRC16 = (locCRC16 & 0xffff);
				Trace("header CRC16: " << locCRC16);
				Trace("file   CRC16: " << header.CRC16);
				bCont = ( locCRC16 == header.CRC16 );
			}
		}
		header.fbValid = bCont;
	}
	return is;
}

//---- ZipStreamBuf ----------------------------------------------------------------
ZipStreamBuf::ZipStreamBuf(ZipStream::eStreamMode aMode, Allocator *alloc)
	: isInitialized(false)
	, fStreamMode(aMode)
	, fAllocator(alloc)
	, fCrcData(0UL)
	, fStore(Z_BUFSIZE, fAllocator)
	, fCompressed(Z_BUFSIZE, fAllocator)
{
	z_stream aInit = { 0 };
	fZip = aInit;
}

//---- ZipOStreamBuf ----------------------------------------------------------------
ZipOStreamBuf::ZipOStreamBuf(std::ostream &os, ZipStream::eStreamMode aMode, Allocator *alloc)
	: ZipStreamBuf(aMode, ( alloc ? alloc : Coast::Storage::Current() ) )
	, fOs(&os)
	, fCompLevel(Z_BEST_COMPRESSION)
	, fCompStrategy(Z_DEFAULT_STRATEGY)
{
	xinit();
}

ZipOStreamBuf::~ZipOStreamBuf()
{
	close();
}

const GzipHdr &ZipOStreamBuf::Header()
{
	return fHeader;
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
	Trace("total_out: " << (long) fZip.total_out);

	if ( HasTrailer() ) {
		Trace("crc is:" << (long)fCrcData);
		// now write crc and length
		putLong(fCrcData);
		putLong(fZip.total_in);
	}
	err = deflateEnd(&fZip);
	if (err != Z_OK) {
		Trace("deflateEnd error:" << err);
	}
	fOs->flush();
	fOs = 0;
}

void ZipOStreamBuf::putLong(unsigned long value)
{
	for (size_t n = 0; n < sizeof(value); ++n) {
		fOs->put(char((value & 0xff)));
		value >>= 8;
	}
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
}

int ZipOStreamBuf::sync()
{
	StartTrace(ZipOStreamBuf.sync);

	// ensure buffer is initialized
	zipinit();

	fZip.next_in = (unsigned char *) pbase();
	fZip.avail_in = pptr() - pbase();
	Trace("sync avail_in=" << (long)fZip.avail_in);
	Trace("sync avail_out=" << (long)fZip.avail_out);
	long err = Z_OK;
	if ( HasTrailer() ) {
		fCrcData = crc32(fCrcData, (const Bytef *)pbase(), pptr() - pbase());
	}
	while (fZip.avail_in > 0 && err == Z_OK) {
		flushCompressedIfNecessary();

		err = deflate(&fZip, Z_NO_FLUSH);
		Trace("deflate err = " << err);
	}
	pinit();

	return (err == Z_OK) ? 0 : -1;
}

int ZipOStreamBuf::overflow(int c)
{
	StartTrace(ZipOStreamBuf.overflow);
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
	StartTrace(ZipOStreamBuf.underflow);
	return EOF;
}

void ZipOStreamBuf::xinit()
{
	StartTrace(ZipOStreamBuf.xinit);
	setg(0, 0, 0);
	pinit();
}

void ZipOStreamBuf::pinit()
{
	StartTrace(ZipOStreamBuf.pinit);
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

void ZipOStreamBuf::setHeaderCRC(bool bFlag)
{
	StartTrace(ZipOStreamBuf.setHeaderCRC);
	if ( !isInitialized ) {
		bFlag ? ( fHeader.Flags |= GzipHdr::eFHCRC ) : ( fHeader.Flags ^= GzipHdr::eFHCRC );
	} else {
		SYSINFO("wrote header already, unable to modify header crc flag");
	}
}

int ZipOStreamBuf::setCompression(int comp_level, int comp_strategy)
{
	StartTrace1(ZipOStreamBuf.setCompression, "level:" << (long)comp_level << " strategy:" << (long)comp_strategy);
	fCompLevel = comp_level;
	fCompStrategy = comp_strategy;
	if ( isInitialized ) {
		// only set deflate params this way if we are initialized
		return deflateParams(&fZip, fCompLevel, fCompStrategy);
	}
	return Z_OK;
}

void ZipOStreamBuf::setExtraField(const String &strBuf)
{
	StartTrace(ZipOStreamBuf.setExtraField);
	if ( !isInitialized ) {
		if ( strBuf.Length() ) {
			fHeader.Flags |= GzipHdr::eFEXTRA;
			fHeader.XLEN = strBuf.Length();
			fHeader.ExtraField = strBuf;
		} else {
			fHeader.Flags ^= GzipHdr::eFEXTRA;
			fHeader.XLEN = 0;
			fHeader.ExtraField.Trim(0);
		}
	} else {
		SYSINFO("wrote header already, unable to modify extra buffer");
	}
}

void ZipOStreamBuf::setFilename(String strFilename)
{
	StartTrace1(ZipOStreamBuf.setFilename, "Given Filename [" << strFilename << "]");
	if ( !isInitialized ) {
		if ( strFilename.Length() ) {
			fHeader.Flags |= GzipHdr::eFNAME;
			fHeader.FileName = strFilename;
		} else {
			fHeader.Flags ^= GzipHdr::eFNAME;
			fHeader.FileName.Trim(0);
		}
	} else {
		SYSINFO("wrote header already, unable to modify filename");
	}
}

void ZipOStreamBuf::setComment(String strComment)
{
	StartTrace1(ZipOStreamBuf.setComment, "Given Comment [" << strComment << "]");
	if ( !isInitialized ) {
		if ( strComment.Length() ) {
			fHeader.Flags |= GzipHdr::eFCOMMENT;
			fHeader.Comment = strComment;
		} else {
			fHeader.Flags ^= GzipHdr::eFCOMMENT;
			fHeader.Comment.Trim(0);
		}
	} else {
		SYSINFO("wrote header already, unable to modify comment");
	}
}

void ZipOStreamBuf::setModificationTime(TimeStamp aStamp)
{
	StartTrace1(ZipOStreamBuf.setModificationTime, "TimeStamp [" << aStamp.AsString());
	if ( !isInitialized ) {
		fHeader.SetModificationTime(aStamp);
	} else {
		SYSINFO("wrote header already, unable to modify time");
	}
}

void ZipOStreamBuf::zipinit()
{
	StartTrace(ZipOStreamBuf.zipinit);
	if ( !isInitialized ) {
		isInitialized = true;

		if ( HasHeader() ) {
			(*fOs) << fHeader;
		}
		fZip.next_in = 0;
		fZip.avail_in = 0;
		fZip.next_out = 0;
		fZip.avail_out = 0;
		fZip.zalloc = ZipAlloc;
		fZip.zfree = ZipFree;
		fZip.opaque = fAllocator;

		//! using negative MAX_WBITS disables zlib-internal gzip-header writing
		//! using default positive MAX_WBITS, writes default z-compression header
		int wbits = -MAX_WBITS;
		if ( !HasHeader() ) {
			// use default z-compression header
			wbits = MAX_WBITS;
		}
		long err = deflateInit2(&fZip, fCompLevel, Z_DEFLATED, wbits, DEF_MEM_LEVEL, fCompStrategy);
		fZip.next_in = (unsigned char *)(const char *)fStore;
		fZip.next_out = (unsigned char *)(const char *)fCompressed;
		fZip.avail_out = fCompressed.Capacity();
		Assert(Z_OK == err);
		if (Z_OK != err) {
			Trace("zlib error " << err);
		}
		if ( HasTrailer() ) {
			fCrcData = crc32(0L, Z_NULL, 0);
		}
	}
}

ZipIStreamBuf::ZipIStreamBuf(std::istream &zis, std::istream &is, ZipStream::eStreamMode aMode, Allocator *alloc)
	: ZipStreamBuf(aMode, ( alloc ? alloc : Coast::Storage::Current() ) )
	, fZis(zis)
	, fIs(&is)
	, fZipErr(Z_OK)
{
	xinit();
}

ZipIStreamBuf::~ZipIStreamBuf()
{
	close();
}

const GzipHdr &ZipIStreamBuf::Header()
{
	if ( !isInitialized ) {
		zipinit();
	}
	return fHeader;
}

bool ZipIStreamBuf::getExtraField(String &strBuf)
{
	if ( Header().Flags & GzipHdr::eFEXTRA ) {
		strBuf = Header().ExtraField;
		return true;
	}
	return false;
}

bool ZipIStreamBuf::getFilename(String &strFilename)
{
	if ( Header().Flags & GzipHdr::eFNAME ) {
		strFilename = Header().FileName;
		return true;
	}
	return false;
}

bool ZipIStreamBuf::getComment(String &strComment)
{
	if ( Header().Flags & GzipHdr::eFCOMMENT ) {
		strComment = Header().Comment;
		return true;
	}
	return false;
}

TimeStamp ZipIStreamBuf::getModificationTime()
{
	return Header().GetModificationTime();
}

unsigned long ZipIStreamBuf::getLong()
{
	unsigned long value = 0;
	for (size_t n = 0; n < sizeof(value); ++n) {
		if (fZip.avail_in < 1) {
			fillCompressed();
		}
		value += *fZip.next_in++ << (n << 3);
		--fZip.avail_in;
	}
	return value;
}

int ZipIStreamBuf::overflow(int c)
{
	return EOF;
}

int ZipIStreamBuf::underflow()
{
	StartTrace(ZipIStreamBuf.underflow);
	Trace( "gptr:" << (long)gptr() << " egptr:" << (long)egptr() << " fZipErr:" << (long)fZipErr);
	if ( gptr() >= egptr() ) {
		Trace("no more data in get area");
		if ( fZipErr != Z_STREAM_END ) {
			Trace("not at zstream end yet, syncing");
			if ( sync() != 0 ) {
				Trace("EOF, fZipErr: [" << fZipErr << "]");
				fZis.setstate(std::ios::failbit | std::ios::eofbit);
				return EOF;
			}
		} else {
			Trace("zstream end set, gptr:" << (long)gptr() << " egptr:" << (long)egptr() );
		}
		if ( gptr() >= egptr() ) {
			Trace("no more data to read, signalling eof, fZipErr: [" << fZipErr << "]");
			fZis.setstate(std::ios::failbit | std::ios::eofbit);
			return EOF;
		}
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

	if ( HasTrailer() ) {
		unsigned long crc = getLong();
		unsigned long len = getLong();

		Trace("CRC check " << (long) crc << " vs. " << (long) fCrcData);
		Trace("Length check: " << (long) len << " vs. " << (long) fZip.total_out);

		if (crc != fCrcData || fZip.total_out != len) {
			fZis.setstate(std::ios::badbit);
		}
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

	if (!isInitialized) {
		isInitialized = true;

		fZip.next_in = 0;
		fZip.avail_in = 0;
		fZip.next_out = 0;
		fZip.avail_out = 0;
		fZip.zalloc = ZipAlloc;
		fZip.zfree = ZipFree;
		fZip.opaque = fAllocator;

		//! using negative MAX_WBITS disables zlib-internal gzip-header writing
		//! using default positive MAX_WBITS, writes default z-compression header
		int wbits = -MAX_WBITS;
		if ( !HasHeader() ) {
			// use default z-compression header
			wbits = MAX_WBITS;
		}
		fZipErr = inflateInit2(&fZip, wbits);

		fZip.avail_in = 0;

		Assert(Z_OK == fZipErr);

		if (Z_OK == fZipErr) {
			if ( HasHeader() ) {
				(*fIs) >> fHeader;
				if ( !fHeader.IsValid() ) {
					Trace("Incomplete or wrong header");
					fZis.setstate(std::ios::badbit);
					fZipErr = Z_DATA_ERROR;
				}
			}
		} else {
			Trace("zlib error " << fZipErr);
		}
		if ( HasTrailer() ) {
			fCrcData = crc32(0L, Z_NULL, 0);
		}
	}
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
		fZis.clear(std::ios::badbit | fZis.rdstate());
#else
		fZis.setstate(std::ios::badbit);
#endif
	}

	unsigned long postStoreLen = preStoreLen - fZip.avail_out;
	pinit(postStoreLen);
	if ( HasTrailer() ) {
		fCrcData = crc32(fCrcData, (unsigned char *) gptr(), postStoreLen );
	}
	Trace("fCompressed: " << (long)fZip.avail_in << "/" << fCompressed.Capacity());
	Trace("fStore: " << (long) (egptr() - gptr()));
	Trace("total in: " << (long) fZip.total_in << " total out: " << (long) fZip.total_out << " fZipErr: " << (long) fZipErr);
}

int ZipIStreamBuf::sync()
{
	StartTrace(ZipIStreamBuf.sync);

	// ensure buffer is initialized
	zipinit();

	if ( fZipErr == Z_OK && (gptr() >= egptr()) ) {
		fillCompressed();
		runInflate();
	}

	if ( !(fIs->good()) && fZip.avail_in == 0 && fZipErr == Z_OK ) {
		Trace("premature end of stream");
#if defined(WIN32)
		fZis.clear(std::ios::failbit | fZis.rdstate());
#else
		fZis.setstate(std::ios::failbit);
#endif
		return -1;
	}

	return (fZipErr == Z_OK || fZipErr == Z_STREAM_END) ? 0 : -1;
}
