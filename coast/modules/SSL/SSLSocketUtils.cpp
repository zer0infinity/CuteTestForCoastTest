/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SSLSocketUtils.h"
#include "Dbg.h"

//---- SSLSocketUtils ----------------------------------------------------------------
SSLSocketUtils::SSLSocketUtils()
{
	StartTrace(SSLSocketUtils.SSLSocketUtils);
}

SSLSocketUtils::~SSLSocketUtils()
{
	StartTrace(SSLSocketUtils.~SSLSocketUtils);
}

// auxiliary for GetPeer... methods
String SSLSocketUtils::GetNameFromX509Name(X509_NAME *x509_name, unsigned long flags)
{
	StartTrace(SSLSocketUtils.GetNameFromX509Name);
	String result;
	/* Sigh...do it the hard way. */
	BIO *mem = BIO_new(BIO_s_mem());
	char *data = NULL;
	long data_len = 0, ok;

	if ((ok = X509_NAME_print_ex(mem, x509_name, 0, flags))) {
		data_len = BIO_get_mem_data(mem, &data);
	}

	if (data) {
		result.Append((void *)data, data_len);
	}
	BIO_free(mem);
	Trace("returning [" << result << "]");
	return result;
}

Anything SSLSocketUtils::ParseDN(String dn)
{
	StartTrace(SSLSocketUtils.ParseDN);
	Anything result;
	StringTokenizer comma(dn, ',');
	String current;
	while (comma(current)) {
		while (current[current.Length()-1L] == '\\') {
			// a masked comma, continue
			String tmp;
			comma(tmp);
			current.Trim(current.Length() - 1);
			current.Append(',').Append(tmp);
		}
		long equalindex = current.StrChr('=');
		Assert(equalindex > 0);
		result[current.SubString(0, equalindex)] =
			current.SubString(equalindex + 1, current.Length() - equalindex);
	}
	return result;
}

bool SSLSocketUtils::VerifyDN(String filter, ROAnything dnParts)
{
	StartTrace(SSLSocketUtils.VerifyDN);
	StringTokenizer comma(filter, ',');
	String current;
	while (comma(current)) {
		long equalindex = current.StrChr('=');
		Assert(equalindex > 0);
		String dnElement(current.SubString(0, equalindex));
		String dnElementValue(current.SubString(equalindex + 1, current.Length() - equalindex));
		if ( !dnParts.IsDefined(dnElement) || dnElementValue != dnParts[dnElement].AsString() ) {
			return false;
		}
	}
	return true;
}

String SSLSocketUtils::GetPeerAsString(X509 *cert)
{
	StartTrace(SSLSocketUtils.GetPeerAsString);
	String result = "";
	if (cert) {
		result = GetNameFromX509Name(X509_get_subject_name(cert), XN_FLAG_RFC2253);
	}
	return result;
}

String SSLSocketUtils::GetPeerIssuerAsString(X509 *cert)
{
	StartTrace(SSLSocketUtils.GetPeerIssuerAsString);
	String result = "";
	if (cert) {
		result = GetNameFromX509Name(X509_get_issuer_name(cert), XN_FLAG_RFC2253);
	}
	return result;

}
