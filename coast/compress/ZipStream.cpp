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
#include "ConversionUtils.h"

//--- c-modules used -----------------------------------------------------------
#include <string.h>	// memset
#include "gzio.h"	// DEF_MEM_LEVEL

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
	for (long lIdx = 0; lIdx < GzipHdr::eModificationTimeLen; lIdx++) {
		ModificationTime[lIdx] = ( lModTime & 0xff );
		lModTime >>= 8;
	}
}

ostream &operator<<(ostream &os, GzipHdr &header)
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
#if defined(DEBUG)
	{
		OStringStream ostr;
		strHeader.DumpAsHex(ostr);
		Trace("Writing a Header of " << strHeader.Length() << "bytes:\n" << ostr.str());
	}
#endif
	os.write(strHeader, strHeader.Length());
	return os;
}

istream &operator>>(istream &is, GzipHdr &header)
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
ZipStreamBuf::ZipStreamBuf(Allocator *alloc)
	: isInitialized(false)
	, fAllocator(alloc)
	, fCrcData(0UL)
	, fStore(Z_BUFSIZE, fAllocator)
	, fCompressed(Z_BUFSIZE, fAllocator)
{
	z_stream_s aInit = { 0 };
	fZip = aInit;
}

//---- ZipOStreamBuf ----------------------------------------------------------------
ZipOStreamBuf::ZipOStreamBuf(ostream &os, Allocator *alloc)
	: ZipStreamBuf(alloc ? alloc : Storage::Current())
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
	Trace("crc is:" << (long)fCrcData);
	Trace("total_out: " << (long) fZip.total_out);

	// now write crc and length
	putLong(fCrcData);
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

	if (!isInitialized) {
		zipinit();
		isInitialized = true;
	}

	fZip.next_in = (unsigned char *) pbase();
	fZip.avail_in = pptr() - pbase();
	Trace("sync avail_in=" << (long)fZip.avail_in);
	Trace("sync avail_out=" << (long)fZip.avail_out);
	long err = Z_OK;
	fCrcData = crc32(fCrcData, (const Bytef *)pbase(), pptr() - pbase());
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
	bFlag ? ( fHeader.Flags |= GzipHdr::eFHCRC ) : ( fHeader.Flags ^= GzipHdr::eFHCRC );
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
	if ( strBuf.Length() ) {
		fHeader.Flags |= GzipHdr::eFEXTRA;
		fHeader.XLEN = strBuf.Length();
		fHeader.ExtraField = strBuf;
	} else {
		fHeader.Flags ^= GzipHdr::eFEXTRA;
		fHeader.XLEN = 0;
		fHeader.ExtraField.Trim(0);
	}
}

void ZipOStreamBuf::setFilename(String strFilename)
{
	StartTrace1(ZipOStreamBuf.setFilename, "Given Filename [" << strFilename << "]");
	if ( strFilename.Length() ) {
		fHeader.Flags |= GzipHdr::eFNAME;
		fHeader.FileName = strFilename;
	} else {
		fHeader.Flags ^= GzipHdr::eFNAME;
		fHeader.FileName.Trim(0);
	}
}

void ZipOStreamBuf::setComment(String strComment)
{
	StartTrace1(ZipOStreamBuf.setComment, "Given Comment [" << strComment << "]");
	if ( strComment.Length() ) {
		fHeader.Flags |= GzipHdr::eFCOMMENT;
		fHeader.Comment = strComment;
	} else {
		fHeader.Flags ^= GzipHdr::eFCOMMENT;
		fHeader.Comment.Trim(0);
	}
}

void ZipOStreamBuf::setModificationTime(TimeStamp aStamp)
{
	StartTrace1(ZipOStreamBuf.setModificationTime, "TimeStamp [" << aStamp.AsString());
	fHeader.SetModificationTime(aStamp);
}

void ZipOStreamBuf::zipinit()
{
	StartTrace(ZipOStreamBuf.zipinit);
	(*fOs) << fHeader;

	fZip.next_in = 0;
	fZip.avail_in = 0;
	fZip.next_out = 0;
	fZip.avail_out = 0;
	fZip.zalloc = ZipAlloc;
	fZip.zfree = ZipFree;
	fZip.opaque = fAllocator;

	long err = deflateInit2(&fZip,
							fCompLevel,
							Z_DEFLATED,
							-MAX_WBITS,
							DEF_MEM_LEVEL,
							fCompStrategy);
	fZip.next_in = (unsigned char *)(const char *)fStore;
	fZip.next_out = (unsigned char *)(const char *)fCompressed;
	fZip.avail_out = fCompressed.Capacity();
	Assert(Z_OK == err);
	if (Z_OK != err) {
		Trace("zlib error " << err);
	}
	fCrcData = crc32(0L, Z_NULL, 0);
}

ZipIStreamBuf::ZipIStreamBuf(istream &zis, istream &is, Allocator *alloc)
	: ZipStreamBuf(alloc ? alloc : Storage::Current())
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

bool ZipIStreamBuf::getExtraField(String &strBuf)
{
	if ( fHeader.Flags & GzipHdr::eFEXTRA ) {
		strBuf = fHeader.ExtraField;
		return true;
	}
	return false;
}

bool ZipIStreamBuf::getFilename(String &strFilename)
{
	if ( fHeader.Flags & GzipHdr::eFNAME ) {
		strFilename = fHeader.FileName;
		return true;
	}
	return false;
}

bool ZipIStreamBuf::getComment(String &strComment)
{
	if ( fHeader.Flags & GzipHdr::eFCOMMENT ) {
		strComment = fHeader.Comment;
		return true;
	}
	return false;
}

TimeStamp ZipIStreamBuf::getModificationTime()
{
	return fHeader.GetModificationTime();
}

unsigned long ZipIStreamBuf::getLong()
{
	unsigned long value = 0;
	for (size_t n = 0; n < sizeof(value); n++) {
		if (fZip.avail_in < 1) {
			fillCompressed();
		}

		unsigned char ucRead = *fZip.next_in++;
		fZip.avail_in--;

		value += ucRead << (8 * n);
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

	if ( (gptr() >= egptr()) && (fZipErr == Z_STREAM_END || (sync() != 0)) ) {
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

	Trace("CRC check " << (long) crc << " vs. " << (long) fCrcData);
	Trace("Length check: " << (long) len << " vs. " << (long) fZip.total_out);

	if (crc != fCrcData || fZip.total_out != len) {
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
		(*fIs) >> fHeader;
		if ( !fHeader.IsValid() ) {
			Trace("Incomplete or wrong header");
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

	fCrcData = crc32(0L, Z_NULL, 0);
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
	fCrcData = crc32(fCrcData, (unsigned char *) gptr(), postStoreLen );

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
