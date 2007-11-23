/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPDAImpl_H
#define _LDAPDAImpl_H

#include "config_ldap.h"
#include "DataAccessImpl.h"
#include "LDAPAPI.h"

//---- LDAPDAImpl -----------------------------------------------------------
class EXPORTDECL_LDAP LDAPDAImpl : public DataAccessImpl
{
public:
	LDAPDAImpl(const char *name);
	~LDAPDAImpl();
	IFAObject *Clone() const;

	//--- TRX handling ---
	//! executes the dataaccess (ldapsearch) to an LDAP
	//! Clients use this method to invoke a 'DataAccess' (Search) to LDAP
	//! \param ctx the thread context of the invocation
	//! \param in input mapper which generates the inputparameters
	//! \param out ouput mapper which collects the outputparameters
	//! \return returns true if the 'DataAccess' was successful otherwise false
	virtual bool Exec(Context &c, ParameterMapper *in, ResultMapper *out);

protected:
	//!binds to an LDAP-Server
	//! invoke a anonymous or authenticated bind to an LDAP-Server
	//! \param ctx the thread context of the invocation
	//! \param in input mapper which gets inputparameters
	//! \param out ouput mapper which puts outputparameters to context
	//! \return returns a ldaphandle pointer if bind was successful otherwise NULL
	LDAP *Connect(Context &ctx, ParameterMapper *in, ResultMapper *out);

	//!gets response/data from LDAP-Server
	//! gets data and fill it to result
	//! \param result the data we have received from ldap
	//! \param ldapHdl ldap handle (we determined by Connect method above)
	//! \param ctx the thread context of the invocation
	//! \param out ouput mapper which puts outputparameters to context
	//! \return void
	void GetData( LDAPMessage *result, LDAP *ldapHdl, Context &c, OutputMapper *out );

	unsigned long ldap_utf8getcc( const char **src);

	//!converts textual attribute values and distinguished names (UTF-8) in HTML-specific format
	//!LDAPv3 usese the UTF-8 (Unicode Transformation Format-8) characterset
	//! no op if (brain dead) config flag "NoHTMLCharMapping" is set to true
	//! \param str input and result (converted string) e.g. ä=&auml
	//! \return void
	void MapUTF8Chars(String &str);

	//!Get components of the distinguished name
	bool GetDNComponents(const char *dn, Anything &dnParts);

	//!The Do... methods invoke the loop to get the reply from the server. Because the client
	//!honours the timeouts only in async mode, we use the async api. In sync mode the client has
	//!to rely on the server to honour the timeout passed to the server, e.g. network failure
	//!conditions wouldn't be detected.
	bool DoSearch( LDAP *ldapHdl, String messageIn,
				   int msgId, int all,
				   Context &c, ParameterMapper *in, ResultMapper *out);

	bool DoConnect( LDAP *ldapHdl, String messageIn,
					int msgId, int all,
					Context &c, ParameterMapper *in, ResultMapper *out);

	bool DoAddModifyDel( LDAP *ldapHdl, String messageIn,
						 int msgId, int all,
						 Context &c, ParameterMapper *in, ResultMapper *out);

	//!To achieve a polling effect, you have to give a /Timeout of 0 seconds, which is interpeted as
	//!wait for ever. This is done by callin ldap_result with a null pointer.
	int WaitForResult(LDAPMessage **result, LDAP *ldapHdl, String messageIn,
					  timeval *timeLimitIn, int msgId, int all,
					  Context &c, ParameterMapper *in, ResultMapper *out);

	//!The Check... methods do the operation specific checking.
	bool CheckConnectResult( int opRet, bool &outcome, LDAPMessage *result, LDAP *ldapHdl, String messageIn,
							 timeval *timeLimitIn, int msgId,
							 Context &c, ParameterMapper *in, ResultMapper *out);
	bool CheckSearchResult( int opRet, bool &outcome, LDAPMessage *result, LDAP *ldapHdl, String messageIn,
							timeval *timeLimitIn, int msgId,
							Context &c, ParameterMapper *in, ResultMapper *out);

	bool CheckAddDelModifyResult( int opRet, bool &outcome, LDAPMessage *result, LDAP *ldapHdl, String messageIn,
								  timeval *timeLimitIn, int msgId,
								  Context &c, ParameterMapper *in, ResultMapper *out);

	//!Depending of the setting of /Timeout it is an error to recieve a timeout (/Timeout not 0) or we try again.
	bool HandleTimeout(int opRet, bool &outcome, LDAPMessage *result, LDAP *ldapHdl, String messageIn,
					   timeval *timeLimitIn, int msgId,   Context &c, ParameterMapper *in, ResultMapper *out);

	//!Logs error and closes handle
	void HandleError(LDAP *ldapHdl, Context &ctx, ResultMapper *out, const String &msg);
};

//---- LDAPCompareDAImpl -----------------------------------------------------------
class EXPORTDECL_LDAP LDAPCompareDAImpl : public LDAPDAImpl
{
public:
	LDAPCompareDAImpl(const char *name);
	~LDAPCompareDAImpl();
	IFAObject *Clone() const;

	//--- TRX handling ---
	//! executes the dataaccess (ldapcompare) to an LDAP
	//! Clients use this method to invoke a 'DataAccess' (Compare a Value) to LDAP
	//! \param ctx the thread context of the invocation
	//! \param in input mapper which generates the inputparameters
	//! \param out ouput mapper which collects the outputparameters
	//! \return returns true if the 'DataAccess' was successful otherwise false
	virtual bool Exec(Context &ctx, ParameterMapper *in, ResultMapper *out);
	bool  CheckCompareResult( int opRet, bool &outcome, LDAPMessage *result, LDAP *ldapHdl, String messageIn,
							  timeval *timeLimitIn, int msgId,
							  Context &c, ParameterMapper *in, ResultMapper *out);
protected:
};

//---- LDAPModifyDAImpl -----------------------------------------------------------
class EXPORTDECL_LDAP LDAPModifyDAImpl : public LDAPDAImpl
{
public:
	LDAPModifyDAImpl(const char *name);
	~LDAPModifyDAImpl();
	IFAObject *Clone() const;

	//--- TRX handling ---
	//! executes the dataaccess (ldapmodify) to an LDAP
	//! Clients use this method to invoke a 'DataAccess' (modify value(s)) to LDAP
	//! \param ctx the thread context of the invocation
	//! \param in input mapper which generates the inputparameters
	//! \param out ouput mapper which collects the outputparameters
	//! \return returns true if the 'DataAccess' was successful otherwise false
	virtual bool Exec(Context &c, ParameterMapper *in, ResultMapper *out);
};

//---- LDAPAddDAImpl -----------------------------------------------------------
class EXPORTDECL_LDAP LDAPAddDAImpl : public LDAPDAImpl
{
public:
	LDAPAddDAImpl(const char *name);
	~LDAPAddDAImpl();
	IFAObject *Clone() const;

	//--- TRX handling ---
	//! executes the dataaccess (ldapadd) to an LDAP
	//! Clients use this method to invoke a 'DataAccess' (adding value(s)) to LDAP
	//! \param ctx the thread context of the invocation
	//! \param in input mapper which generates the inputparameters
	//! \param out ouput mapper which collects the outputparameters
	//! \return returns true if the 'DataAccess' was successful otherwise false
	virtual bool Exec(Context &c, ParameterMapper *in, ResultMapper *out);
};

//---- LDAPDelDAImpl -----------------------------------------------------------
class EXPORTDECL_LDAP LDAPDelDAImpl : public LDAPDAImpl
{
public:
	LDAPDelDAImpl(const char *name);
	~LDAPDelDAImpl();
	IFAObject *Clone() const;

	//--- TRX handling ---
	//! executes the dataaccess (ldapdel) to an LDAP
	//! Clients use this method to invoke a 'DataAccess' (deleting value(s)) from LDAP
	//! \param ctx the thread context of the invocation
	//! \param in input mapper which generates the inputparameters
	//! \param out ouput mapper which collects the outputparameters
	//! \return returns true if the 'DataAccess' was successful otherwise false
	virtual bool Exec(Context &c, ParameterMapper *in, ResultMapper *out);
};

#endif
