/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "NTLMAuthMessage.h"

//--- project modules used -----------------------------------------------------
#include "NTLMCoder.h"
//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- NTLMAuthMessage ----------------------------------------------------------------

long	NTLMAuthMessage::GetMessageType()
{
	return DecodeLong(8L);
}
String NTLMAuthMessage::GetAsBase64()
{
	return NTLMCoder::MakeBase64(fMsg);
}

long NTLMAuthMessage::DecodeLong(long pos)
{
	return DecodeShort(pos) + (DecodeShort(pos + 2) << 16);
}
long NTLMAuthMessage::DecodeShort(long pos)
{
	return (unsigned char)fMsg[pos] + ((unsigned char)fMsg[pos+1L] << 8);
}
String NTLMAuthMessage::DecodeString(long pos, bool striputf)
{
	long len = DecodeShort(pos);
	long off = DecodeShort(pos + 4);
	if (striputf) {
		return NTLMCoder::StripUtf16(fMsg.SubString(off, len));
	}
	return fMsg.SubString(off, len);
}
bool NTLMAuthMessage::IsValid()
{
	return fMsg.StartsWith("NTLMSSP");
}
NTLMAuthMessage *NTLMAuthMessage::MakeMessage(const String &binarybuffer)
{
	NTLMAuthMessage base(binarybuffer);
	switch ( base.GetMessageType()) {
		case 1:
			return new NTLMAuthClientMsgType1(binarybuffer);
		case 2:
			return new NTLMAuthServerMsgType2(binarybuffer);
		case 3:
			return new NTLMAuthClientMsgType3(binarybuffer);
		default:
			return new NTLMAuthMessage(binarybuffer);
	}
}

unsigned long	NTLMAuthClientMsgType1::GetFlags()
{
	return DecodeLong(12L);
}
bool NTLMAuthClientMsgType1::IsValid()
{
	return NTLMAuthMessage::IsValid();
}
String NTLMAuthClientMsgType1::GetDomain()
{
	return DecodeString(16L);
}
String NTLMAuthClientMsgType1::GetHost()
{
	return DecodeString(24L);
}
unsigned long	NTLMAuthServerMsgType2::GetFlags()
{
	return DecodeLong(20L);
}
String NTLMAuthServerMsgType2::GetNonce()
{
	return fMsg.SubString(24L, 8L);
}
bool NTLMAuthServerMsgType2::IsValid()
{
	return NTLMAuthMessage::IsValid();
}
unsigned long	NTLMAuthClientMsgType3::GetFlags()
{
	return DecodeLong(60L);
}
bool NTLMAuthClientMsgType3::IsValid()
{
	return NTLMAuthMessage::IsValid();
}
String NTLMAuthClientMsgType3::GetDomain()
{
	return DecodeString(28L, (GetFlags() & 0x01));
}
String NTLMAuthClientMsgType3::GetHost()
{
	return DecodeString(44L, (GetFlags() & 0x01));
}
String NTLMAuthClientMsgType3::GetUser()
{
	return DecodeString(36L, (GetFlags() & 0x01));
}
String NTLMAuthClientMsgType3::GetNTHash()
{
	return DecodeString(20L);
}
String NTLMAuthClientMsgType3::GetLMHash()
{
	return DecodeString(12L);
}
