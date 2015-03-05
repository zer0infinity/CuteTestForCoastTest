/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "NTLMAuthAction.h"
#include "Renderer.h"
#include "AnythingUtils.h"
#include "NTLMCoder.h"

RegisterAction(NTLMAuthAction);

namespace {
	char const* _MessageKeyName = "Message";
	char const* _HostKeyName = "Host";
	char const* _DomainKeyName = "Domain";
	char const* _DestinationKeyName = "Destination";
	char const* _UserKeyName = "User";
	char const* _PasswordKeyName = "Password";
}

bool NTLMAuthAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace(NTLMAuthAction.DoExecAction);
	ROAnything destConfig;
	if (not config.LookupPath(destConfig, _DestinationKeyName, '\000')) {
		return false;
	}
	TraceAny(destConfig, "Destination");
	ROAnything hostConfig, domainConfig, messageConfig;
	String host, domain;
	config.LookupPath(hostConfig, _HostKeyName, '\000');
	config.LookupPath(domainConfig, _DomainKeyName, '\000');
	host = Renderer::RenderToStringWithDefault(ctx, hostConfig, "dummyhost");
	domain = Renderer::RenderToStringWithDefault(ctx, domainConfig, "dummydomain");

	Anything value;
	ROAnything userConfig, passwordConfig;
	if (config.LookupPath(messageConfig, _MessageKeyName, '\000') && config.LookupPath(userConfig, _UserKeyName, '\000')
			&& config.LookupPath(passwordConfig, _PasswordKeyName, '\000')) {
		String user = Renderer::RenderToString(ctx, userConfig);
		String password = Renderer::RenderToString(ctx, passwordConfig);
		String type2message = Renderer::RenderToString(ctx, messageConfig);
		Trace("[" << type2message << "]");
		type2message.TrimFront(type2message.StrChr(' ') + 1);
		Trace("len:" << type2message.Length() << " b64 [" << type2message << "]");
		String msgDecoded;
		NTLMCoder::DecodeBase64(msgDecoded, type2message);
		Trace("len:" << msgDecoded.Length() << "\n" << msgDecoded.DumpAsHex(16));
		String nonce;
		NTLMCoder::DecodeServerNonce(type2message, nonce);
		Trace("nonce [" << nonce << "]");
		String lmhash = NTLMCoder::EncodeLMPassword(password);
		String nthash = NTLMCoder::EncodeNTPassword(password);
		value = NTLMCoder::EncodeResponse(nonce, lmhash, nthash, domain, host, user);
	} else {
		value = NTLMCoder::EncodeClientMsg(domain, host);
	}
	TraceAny(value, "message to store");
	StorePutter::Operate(value, ctx, destConfig);
	return true;
}
