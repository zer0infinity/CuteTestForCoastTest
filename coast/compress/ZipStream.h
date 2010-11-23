/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ZipStream_H
#define _ZipStream_H

#include "StringStream.h"
#include "TimeStamp.h"
#include "zlib.h"

struct GzipHdr {
	GzipHdr();

	bool IsValid();
	TimeStamp GetModificationTime() const;
	void SetModificationTime(TimeStamp aStamp);

	friend std::ostream &operator<<(std::ostream &os, GzipHdr &header);
	friend std::istream &operator>>(std::istream &is, GzipHdr &header);

	unsigned char ID1;					/* gzip magic header[0] */
	unsigned char ID2;					/* gzip magic header[1] */

	unsigned char CompressionMethod;	/* 0-7 are reserved, 8 denotes the 'deflate' method used by gzip */
	enum eCompressionMethod {
		eDeflate = Z_DEFLATED,
	};

	unsigned char Flags;
	enum {
		eFTEXT = 0x01,					/* set if the content is propably ASCII text, optional indication */
		eFHCRC = 0x02,					/* set if a header crc16 sum is present */
		eFEXTRA = 0x04,					/* set if an extra field is present */
		eFNAME = 0x08,					/* set if an original, zero terminated, file name is present */
		eFCOMMENT = 0x10,				/* set if a zero terminated file comment is present */
	};

	enum {
		eModificationTimeLen = 4
	};
	unsigned char ModificationTime[eModificationTimeLen];	/* original file time if present, time of compression or left 0 */

	unsigned char ExtraFlags;			/* set if deflate compression method is given */
	enum eExtraFlags {
		eUnspecified = 0,
		eMaxCompression = 2,
		eFastestCompression = 4,
	};

	unsigned char OperatingSystem;
	enum eOperatingSystem {
		eFAT = 0,
		eAmiga = 1,
		eVMS = 2,
		eUnix = 3,
		eVM_CMS = 4,
		eAtari_TOS = 5,
		eHPFS = 6,
		eMacintosh = 7,
		eZ_System = 8,
		eCP_M = 9,
		eTOPS_20 = 10,
		eNTFS = 11,
		eQDOS = 12,
		eAcorn_RISCOS = 13,
		ePrime_PRIMOS = 15,
		eUnknownOS = 255,
	};

	unsigned short XLEN;
	String ExtraField;

	String FileName;

	String Comment;

	unsigned short CRC16;

	bool fbValid;
};

namespace ZipStream
{
	enum eStreamMode {
		ePlainMode = 0,
		eWriteHeader = 1 << 0,
		eWriteTrailer = 1 << 1,
		eGZipMode = ( eWriteHeader | eWriteTrailer ),
		eMaxMode = 1 << 2
	};
};

//---- ZipStreamBuf ----------------------------------------------------------
//! Does not do anything in particular except hold a few static constants.
//! Once could merge ZipOStreamBuf and ZipIStreamBuf into this class, but
//! this would result in a very complex class.
class ZipStreamBuf : public streambuf
{
public:
	ZipStreamBuf(ZipStream::eStreamMode aMode, Allocator *alloc);
	virtual const GzipHdr &Header() = 0;

protected:
	bool HasHeader() const {
		return ( ( fStreamMode & ZipStream::eWriteHeader ) == ZipStream::eWriteHeader );
	}
	bool HasTrailer() const {
		return ( ( fStreamMode & ZipStream::eWriteTrailer ) == ZipStream::eWriteTrailer );
	}
	bool isInitialized;
	unsigned char fStreamMode;
	Allocator *fAllocator;
	unsigned long fCrcData;
	z_stream fZip;
	//! the storage of the holding area buffer
	String fStore;
	//! the storage of the compressed buffer
	String fCompressed;
	GzipHdr fHeader;

private:
	ZipStreamBuf(const ZipStreamBuf &);
};

//---- ZipIStreamBuf ----------------------------------------------------------
//! wrap other istream objects with decompression
class ZipIStreamBuf : public ZipStreamBuf
{
public:
	//--- constructors
	ZipIStreamBuf(std::istream &zis, std::istream &, ZipStream::eStreamMode aMode = ZipStream::eGZipMode, Allocator *alloc = Storage::Current());
	~ZipIStreamBuf();

	void close();

	virtual const GzipHdr &Header();

	bool getExtraField(String &strBuf);
	bool getFilename(String &strFilename);
	bool getComment(String &strComment);
	TimeStamp getModificationTime();

protected:
	//! consumes chars of the put area
	virtual int overflow(int c = EOF);

	//! produces characters for the get area
	virtual int underflow();

	//! defines the holding area for the streambuf
	void xinit();
	void pinit(long);
	void zipinit();

	void runInflate();
	void fillCompressed();

	//! not much to do when synchronizing, just insert string termination character
	virtual int sync();

	//! get long from underlying stream
	unsigned long getLong();

	std::istream &fZis;
	std::istream *fIs;

private:
	long fZipErr;
};

//---- ZipOStreamBuf ----------------------------------------------------------
//! wrap other ostream objects with compression compression
class ZipOStreamBuf : public ZipStreamBuf
{
public:
	//--- constructors
	ZipOStreamBuf(std::ostream &, ZipStream::eStreamMode aMode = ZipStream::eGZipMode, Allocator *a = Storage::Current());
	~ZipOStreamBuf();

	//! not much to do when synchronizing, just insert string termination character
	virtual int sync();
	void close();

	virtual const GzipHdr &Header();

	void setHeaderCRC(bool bFlag = true);
	int setCompression(int comp_level, int comp_strategy = Z_DEFAULT_STRATEGY);
	void setExtraField(const String &strBuf);
	void setFilename(String strFilename);
	void setComment(String strComment);
	void setModificationTime(TimeStamp aStamp);

protected:
	//! consumes chars of the put area
	virtual int overflow(int c = EOF);

	//! produces characters for the get area
	virtual int underflow();
	//! defines the holding area for the streambuf
	void xinit();
	void pinit();
	void zipinit();
	void flushCompressed();
	void flushCompressedIfNecessary();
	void putLong(unsigned long);

	std::ostream *fOs;
	int fCompLevel, fCompStrategy;
};

namespace ZipStream
{
	struct _CRCType {
		bool fCrc;
	}; inline _CRCType setHeaderCRC(bool bFlag = true)
	{
		_CRCType aType = { bFlag };
		return aType;
	}
	struct _CprsType {
		int comp_level;
		int comp_strategy;
	}; inline _CprsType setCompression(int comp_level, int comp_strategy = Z_DEFAULT_STRATEGY)
	{
		_CprsType aType = { comp_level, comp_strategy };
		return aType;
	}
	struct _EFType {
		String fBuf;
	}; inline _EFType setExtraField(const String &strBuf)
	{
		_EFType aArgument;
		aArgument.fBuf = strBuf;
		return aArgument;
	}
	struct _FnameType {
		String fBuf;
	}; inline _FnameType setFilename(String strFilename)
	{
		_FnameType aArgument;
		aArgument.fBuf = strFilename;
		return aArgument;
	}
	struct _CommType {
		String fBuf;
	}; inline _CommType setComment(String strComment)
	{
		_CommType aArgument;
		aArgument.fBuf = strComment;
		return aArgument;
	}
	struct _ModTType {
		TimeStamp fStamp;
	}; inline _ModTType setModificationTime(TimeStamp aStamp)
	{
		_ModTType aArgument;
		aArgument.fStamp = aStamp;
		return aArgument;
	}
}
//---- ZipOStream ----------------------------------------------------------
//! wrap other ostream objects with compression
class ZipOStream : public std::ostream
{
public:
	//--- constructors
	ZipOStream(std::ostream &os, ZipStream::eStreamMode aMode = ZipStream::eGZipMode):
		std::ostream(&fBuf),
		fBuf(os, aMode) {
		init(&fBuf);
	}
	~ZipOStream() {
		close();
	}
	ZipOStreamBuf *rdbuf()  {
		return &fBuf;
	}
	void close() {
		fBuf.close();
	}

	friend inline ZipOStream &operator<<(ZipOStream &os, ZipStream::_CRCType aArgs) {
		os.fBuf.setHeaderCRC(aArgs.fCrc);
		return os;
	}
	friend inline ZipOStream &operator<<(ZipOStream &os, ZipStream::_CprsType aArgs) {
		os.fBuf.setCompression(aArgs.comp_level, aArgs.comp_strategy);
		return os;
	}
	friend inline ZipOStream &operator<<(ZipOStream &os, ZipStream::_EFType aArgs) {
		os.fBuf.setExtraField(aArgs.fBuf);
		return os;
	}
	friend inline ZipOStream &operator<<(ZipOStream &os, ZipStream::_FnameType aArgs) {
		os.fBuf.setFilename(aArgs.fBuf);
		return os;
	}
	friend inline ZipOStream &operator<<(ZipOStream &os, ZipStream::_CommType aArgs) {
		os.fBuf.setComment(aArgs.fBuf);
		return os;
	}
	friend inline ZipOStream &operator<<(ZipOStream &os, ZipStream::_ModTType aArgs) {
		os.fBuf.setModificationTime(aArgs.fStamp);
		return os;
	}
protected:
	ZipOStreamBuf fBuf;
};

//---- ZipOStreamBuf ----------------------------------------------------------
//! wrap other istream objects with decompression
class ZipIStream : public std::istream
{
public:
	//--- constructors
	ZipIStream(std::istream &is, ZipStream::eStreamMode aMode = ZipStream::eGZipMode):
		std::istream(&fBuf),
		fBuf(*this, is, aMode) {
		init(&fBuf);
	}
	~ZipIStream() {
		close();
	}
	ZipIStreamBuf *rdbuf()  {
		return &fBuf;
	}
	void close() {
		fBuf.close();
	}

	bool getExtraField(String &strBuf) {
		return fBuf.getExtraField(strBuf);
	}
	bool getFilename(String &strFilename) {
		return fBuf.getFilename(strFilename);
	}
	bool getComment(String &strComment) {
		return fBuf.getComment(strComment);
	}
	TimeStamp getModificationTime() {
		return fBuf.getModificationTime();
	}

protected:
	ZipIStreamBuf fBuf;
};

#endif
