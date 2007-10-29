/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

// =========================================================================

#ifndef _LDAPDataAccessImpls_H
#define _LDAPDataAccessImpls_H

#include "config_LDAPDA.h"

//--- superclass interface ---
#include "DataAccessImpl.h"

//--- other includes ---
#include "PersistentLDAPConnection.h"

//---- LDAPAbstractDAI ----------------------------------------------------------
//! <B>Mother of all LDAP DA Impl classes.</B>
/*!
<B>Configuration:</B><PRE>
{
	/LDAPServer		optional, default = localhost
	/LDAPPort		optional, default = 389
	/LDAPTimeout	optional, default = 10 sec
	/LDAPBindName	optional, default = Anonymous
	/LDAPBindPW		optional (mandatory, if BindName is defined)
}
}</PRE>
This class handles LDAP Requests. It establishes a connection to the
given server. If this works out, it fetches a query from the context
(using the associated ParameterMapper) and sends it off to the LDAP
directory (implementation: concrete subclasses). The result is
stored (using the associated ResultMapper) in the context.
Since we do asynchronous requests, an LDAP operation is always split
into DoLdapRequest and WaitForResult.<br>
Configuration of the LDAP connection/session is defined a) as Defaults
b) Globally in Config.any or c) in DAImplMeta.any (to override globally
set values)<br>
In general, an ldap operation is performed as follows (without
error checking, assuming everything is ok):
<ol>
<li>GetQuery using parameter mapper
<li>DoCheckQuery (abstr)
<li>Connect
<li>DoLDAPRequest (abstr)
<li>WaitForResult
<li>(DoCheckResult) (default:no)
<li>PutResult
<li>(Disconnect) (as soon as connection is not used anymore)
</ol>
*/
class EXPORTDECL_LDAPDA LDAPAbstractDAI: public DataAccessImpl
{
public:
	/*! \param name defines the name of the data access */
	LDAPAbstractDAI(const char *name) : DataAccessImpl(name) {};
	~LDAPAbstractDAI() {};

	//! should never be called, because this class is supposed to be abstract
	IFAObject *Clone() const {
		return new LDAPAbstractDAI(fName);
	};

	//! executes the transaction
	//  param c The context of the transaction */
	virtual bool Exec(Context &c, ParameterMapper *getter, ResultMapper *putter);

	/*! Check attributes for Add/Modify/Delete to only contain valid values and attribute names.
		\param attrs attributes to check and adjust if needed
		\param bRemoveNullValues must be set to false for checking of attributes to delete, otherwise the name of the attribute to delete gets lost
		\param eh error message handler
		\return true in case we did not detect any errors */
	bool CheckAttributes(Anything &attrs, bool bRemoveNullValues, LDAPErrorHandler &eh);

protected:
	//! hook, retrieves query from context + checks it.
	//! by default checks, if base DN is defined (must not be "").
	//! may be overridden by subclasses that should retrieve, add and
	//! check additionally required parameters.
	virtual bool DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler &eh);

	//! abstract hook, called by Exec after connection
	//! returns message id of started request
	virtual int DoLDAPRequest(Context &ctx, ParameterMapper *getter, LDAPConnection *lc, ROAnything query) {
		Assert(false);
		return -1;
	}

	/*! Check attributes for Add/Modify/Delete to only contain valid values and attribute names. Subclasses can overwrite for their specific needs.
		\param attrs attributes to check and adjust if needed
		\param bRemoveNullValues must be set to false for checking of attributes to delete, otherwise the name of the attribute to delete gets lost
		\param eh error message handler
		\return true in case we did not detect any errors */
	bool DoCheckAttributes(Anything &attrs, bool bRemoveNullValues, LDAPErrorHandler &eh);

	//! executes the transaction
	// param c The context of the transaction
	bool DoExec(Context &c, ParameterMapper *getter, ResultMapper *putter, LDAPErrorHandler &eh);

private:
	// constructor
	LDAPAbstractDAI();
	LDAPAbstractDAI(const LDAPAbstractDAI &);
	// assignment
	LDAPAbstractDAI &operator=(const LDAPAbstractDAI &);

	// builds a query from pieces in config/store
	void GetQuery(ParameterMapper *getter, Context &ctx, Anything &query);

	// Factory method, creates LDAPConnections
	LDAPConnection *LDAPConnectionFactory(ROAnything cp);
};

// =========================================================================

//---- LDAPModifyDAI ----------------------------------------------------------
//! <B>Implements an LDAP modify DA.</B>
/*!
<B>Configuration:</B> See LDAPAbstractDAI<br>
<B>LDAP Query spec:</B><pre>
{
	/LDAPBase		required	base DN where addition is to be made
	/LDAPMods		required	list modifications to perform on entry
}
<b>Mods spec:</b>
/LDAPMods {
	/ConfigMapper {
		/add {
			/AttrName1	ValToAdd
			/AttrName2 {
				Val1
				Val2
			}
		}
		/delete {
			/AttrName1	ValToDelete
			/AttrName2 {
				Val1
				Val2
			}
			/AttrName3	*	# deletes whole attribute
		}
		/replace {
			/AttrName1	{
				Val1
				Val2
			}
			/AttrName2	Val
			/AttrName3	*	# delete whole attribute
		}
	}
}
</pre>
The list of given modifications (add,del,repl) is performed in
the order they are given. Add will create an attribute if neccessary.
Delete will delete an attribute, if no values remain. Deletion of
an attribute can be forced by setting an attribute explicitely to *
(null). After a replace operation, the attribute will have the
values listed, be deleted or created if necessary.
*/
class EXPORTDECL_LDAPDA LDAPModifyDAI: public LDAPAbstractDAI
{
public:
	/*! \param name defines the name of the data access */
	LDAPModifyDAI(const char *name) : LDAPAbstractDAI(name) {};
	~LDAPModifyDAI() {};

	//! returns a new TRX object
	IFAObject *Clone() const {
		return new LDAPModifyDAI(fName);
	};

protected:
	//! add + check mods slot (no defaults are given)
	//! checks, if mods parameter is not empty
	bool DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler &eh);

	//! hook, called by Exec after connection.
	//! returns message id of started replace request
	int DoLDAPRequest(Context &ctx, ParameterMapper *getter, LDAPConnection *lc, ROAnything query);

	/*! collect attributes/values to modify
		\param ldapmods container to carry attributes/values to modify
		\param modcode operation for modification entry [LDAP_MOD_ADD|LDAP_MOD_DELETE|LDAP_MOD_REPLACE] and optionally LDAP_MOD_BVALUES
		\param lModsCounter reference to count number of entries added  to ldapmods
		\param attrmods ROAnything holding attributes/values to modify
		\param bBinaryOperation set to true if we need to handle binary values
		\return number of attributes added to ldapmods */
	int IntPrepareLDAPMods(LDAPMod **ldapmods, int modcode, int &lModsCounter, ROAnything attrmods, bool bBinaryOperation);

	/*! specific ldap_ method to call
		\param lc LDAPConnection class to get ldap handle from
		\param base dn of entry to modify
		\param ldapmods attributes/values to modify
		\return return code of ldap method */
	virtual int DoSpecificOperation(LDAPConnection *lc, String base, LDAPMod **ldapmods);

private:
	// constructor
	LDAPModifyDAI();
	LDAPModifyDAI(const LDAPModifyDAI &);
	// assignement
	LDAPModifyDAI &operator=(const LDAPModifyDAI &);
};

// =========================================================================

//---- LDAPAddDAI ----------------------------------------------------------
//! <B>Implements an LDAP add DA.</B>
/*!
<B>Configuration:</B> See LDAPAbstractDAI<br>
<B>LDAP Query spec:</B><pre>
{
	/LDAPBase	required	base DN where addition is to be made
	/LDAPAttrs		required	list of attributes and values to add
}
<b>Attrs spec:</b>
/LDAPAttrs {
	/ConfigMapper {
		/uid		"kgu"
		/coworker	{ "mke", "jva" "pso" }
	}
}
</pre>
Adds a leaf entry to the ldap directory. Parent of element to add must
exist or be empty (i.e. root)! Attrs is an Anything, where slotname
is attribute's type and slotvalue is attribute's single or
multiple value (see example). You should use a config mapper to get
a literal config, i.e. exactly the attribute/value pairings that you wish
to get. (This mapper stops scripting and returns its config as anything)
*/
class EXPORTDECL_LDAPDA LDAPAddDAI: public LDAPModifyDAI
{
public:
	/*! \param name defines the name of the data access */
	LDAPAddDAI(const char *name) : LDAPModifyDAI(name) {};
	~LDAPAddDAI() {};

	//! returns a new TRX object
	IFAObject *Clone() const {
		return new LDAPAddDAI(fName);
	};

protected:
	//! add + check attributes slot (no defaults are given)
	bool DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler &eh);

	/*! specific ldap_ method to call
		\param lc LDAPConnection class to get ldap handle from
		\param base dn of entry to modify
		\param ldapmods attributes/values to modify
		\return return code of ldap method */
	int DoSpecificOperation(LDAPConnection *lc, String base, LDAPMod **ldapmods);

private:
	// constructor
	LDAPAddDAI();
	LDAPAddDAI(const LDAPAddDAI &);

	// assignement
	LDAPAddDAI &operator=(const LDAPAddDAI &);
};

// =========================================================================

//---- LDAPCompareDAI ----------------------------------------------------------
//! <B>Implements an LDAP compare DA.</B>
/*!
<B>Configuration:</B> See LDAPAbstractDAI<br>
<B>LDAP Query spec:</B><pre>
{
	/LDAPBase		required	base DN where comparison is to be made
	/LDAPAttrName	required	attribute of entry to be compared
	/LDAPAttrValue	required	value of attribute to be compared against directory
}
</pre>
Compares value of an attribute with value in LDAP directory.
*/
class EXPORTDECL_LDAPDA LDAPCompareDAI: public LDAPAbstractDAI
{
public:
	/*! \param name defines the name of the data access */
	LDAPCompareDAI(const char *name) : LDAPAbstractDAI(name) {};
	~LDAPCompareDAI() {};

	//! returns a new TRX object
	IFAObject *Clone() const {
		return new LDAPCompareDAI(fName);
	};

protected:
	//! add + check for base, attrname and attrvalue to compare
	bool DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler &eh);

	//! hook, called by Exec after connection.
	//! returns message id of started compare request
	int DoLDAPRequest(Context &ctx, ParameterMapper *getter, LDAPConnection *lc, ROAnything query);

private:
	// constructor
	LDAPCompareDAI();
	LDAPCompareDAI(const LDAPCompareDAI &);
	// assignement
	LDAPCompareDAI &operator=(const LDAPCompareDAI &);
};

// =========================================================================

//---- LDAPDeleteDAI ----------------------------------------------------------
//! <B>Implements an LDAP delete DA.</B>
/*!
<B>Configuration:</B> See LDAPAbstractDAI<br>
<B>LDAP Query spec:</B><pre>
{
	/LDAPBase		required	base DN pointing to leaf entry to delete
}
</pre>
Deletes a whole entry. Base must be a leaf entry or delete will fail.
*/
class EXPORTDECL_LDAPDA LDAPDeleteDAI: public LDAPAbstractDAI
{
public:
	/*! \param name defines the name of the data access */
	LDAPDeleteDAI(const char *name) : LDAPAbstractDAI(name) {};
	~LDAPDeleteDAI() {};

	//! returns a new TRX object
	IFAObject *Clone() const {
		return new LDAPDeleteDAI(fName);
	};

protected:
	//! hook, called by Exec after connection.
	//! returns message id of started delete request
	int DoLDAPRequest(Context &ctx, ParameterMapper *getter, LDAPConnection *lc, ROAnything query);

private:
	// constructor
	LDAPDeleteDAI();
	LDAPDeleteDAI(const LDAPDeleteDAI &);
	// assignement
	LDAPDeleteDAI &operator=(const LDAPDeleteDAI &);

};

// =========================================================================

//---- LDAPSearchDAI ----------------------------------------------------------
//! <B>Implements an LDAP search DA.</B>
/*!
<B>Configuration:</B> See LDAPAbstractDAI<br>
<B>LDAP Query spec:</B><pre>
{
	/LDAPBase		required
	/LDAPScope		optional	one of {base,subtree,onelevel}, defaults to "base"
	/LDAPFilter		optional 	filter string, defaults to "(objectclass=*)"
	/LDAPAttrs		optional	list of attributes to return, defaults to all
	/LDAPAttrsOnly	optional	return attributes only? one of {1,0}. defaults to no
	/LDAPSizeLimit	optional	give a sizelimit for the result. defaults to LDAP_NO_LIMIT (0)
	/LDAPTimeLimit	optional	give an own timeout. defaults to timeout of session/connection.
}</pre>
*/
class EXPORTDECL_LDAPDA LDAPSearchDAI: public LDAPAbstractDAI
{
public:
	/*! \param name defines the name of the data access */
	LDAPSearchDAI(const char *name) : LDAPAbstractDAI(name) {};
	~LDAPSearchDAI() {};

	//! returns a new TRX object
	IFAObject *Clone() const {
		return new LDAPSearchDAI(fName);
	};

protected:
	//! call superclass DoGetQuery, then add + check
	//! scope, filter, attrs values and attrsOnly switch
	//! (define defaults if not found in context)
	bool DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler &eh);

	//! hook, called by Exec after connection.
	//! returns message id of started search request
	int DoLDAPRequest(Context &ctx, ParameterMapper *getter, LDAPConnection *lc, ROAnything query);

private:
	// constructor
	LDAPSearchDAI();
	LDAPSearchDAI(const LDAPSearchDAI &);
	// assignement
	LDAPSearchDAI &operator=(const LDAPSearchDAI &);
};

#endif		//not defined _LDAPDataAccessImpls_H
