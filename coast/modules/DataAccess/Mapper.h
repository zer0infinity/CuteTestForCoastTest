/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MAPPER_H
#define _MAPPER_H

#include "Context.h"
#include "WDModule.h"

class Registry;

//---- MappersModule -----------------------------------------------------------
class MappersModule: public WDModule {
public:
	MappersModule(const char *name) :
		WDModule(name) {
	}

	virtual bool Init(const ROAnything config);
	virtual bool ResetFinis(const ROAnything);
	virtual bool Finis();
};

//----------------------- ParameterMapper aka ParameterMapper --------------------
//! Base class for getting parameters or content out of the context
/*! This Mapper supports behavior configuration. Put specific configuration into InputMapperMeta.any for the corresponding Mapper alias name.
\par Configuration
\code
{
	/<Get-Key>	{					Anything	optional, key specific Mapper-Script to execute when hit
		/<ParameterMapperName>	*				The specified Mapper will be called using an empty script as its configuration, eg. no submapper will be called.
		/<ParameterMapperName> {				The specified Mapper will be called using the given script to be able to delegate Parameter retrieval again.
			<specific mapper config>
		}
	}
	...
	/Delim			char		optional, default '.', specify path delimiting character for Lookup() operations from within DoFinalGetAny() and DoFinalGetStream()
	/IndexDelim		char		optional, default ':', specify index delimiting character for Lookup() operations from within DoFinalGetAny() and DoFinalGetStream()
}
\endcode
*/
class ParameterMapper : public HierarchConfNamed
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	ParameterMapper(const char *name);
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	//---- registry api
	RegCacheDef(ParameterMapper);	// FindParameterMapper()

	static const char* gpcCategory;
	static const char* gpcConfigPath;

	//! Pulls out an int according to key. Clients use this method to pull out an int value with name key from the Mapper
	/*! @param key the name defines kind of value to get
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Get(const char *key, int &value, Context &ctx);

#if !defined(BOOL_NOT_SUPPORTED)
	//! Pulls out an bool according to key. Clients use this method to pull out an bool value with name key from the Mapper
	/*! @param key the name defines kind of value to get
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Get(const char *key, bool &value, Context &ctx);
#endif
	//! Pulls out an long according to key. Clients use this method to pull out an long value with name key from the Mapper
	/*! @param key the name defines kind of value to get
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Get(const char *key, long &value, Context &ctx);

	//! Pulls out an float according to key. Clients use this method to pull out an float value with name key from the Mapper
	/*! @param key the name defines kind of value to get
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Get(const char *key, float &value, Context &ctx);

	//! Pulls out an double according to key. Clients use this method to pull out an double value with name key from the Mapper
	/*! @param key the name defines kind of value to get
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Get(const char *key, double &value, Context &ctx);

	//! Pulls out an String according to key. Clients use this method to pull out an String value with name key from the Mapper
	/*! @param key the name defines kind of value to get
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Get(const char *key, String &value, Context &ctx);

	//! Pulls out an Anything according to key. Clients use this method to pull out an Anything value with name key from the Mapper
	/*! @param key the name defines kind of value to get
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Get(const char *key, Anything &value, Context &ctx);

	//! Pulls out an Anything according to key. Clients use this method to pull out an Anything value with name key from the Mapper
	/*! @param key the name defines kind of value to get
		@param ctx the thread context of the invocation
		@return mapped value as Anything, will be Null-Anything if not found or mapping failure */
	Anything Get(const char *key, Context &ctx);

	//! Write to the ostream os values according to key. Clients use this method to write to the stream values associated with the key
	/*! @param key the name defines kind of values to write
		@param os the stream to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Get(const char *key, std::ostream &os, Context &ctx);

	//! Get mapper script for the given key if any. Subclasses can overwrite DoSelectScript() to avoid scripting (recursion) or tailor to their needs.
	/*! Default is to return script[key].
	 * Mappers that want complete scripting should return whole script for interpretation regardless of key given.
	 * @param key current mapping key
	 * @param script script in which to check for the given key, if omitted or the key was not found, we will check using hierarchic Lookup() mechanism.
	 * @param ctx Context which might be used for special cases
	 * @return new script according to given key or null Anything in case nothing was found
	 */
	ROAnything SelectScript(const char *key, ROAnything script, Context &ctx) const {
		return DoSelectScript(key, script, ctx);
	}

	//! Defines the name space where to get values from, like a prefix. Subclasses can overwrite DoGetSourceSlot().
	/*! Default is to Lookup("SourceSlot") in our own config. If it is not found, \em fName.SourceSlot will be searched in Context.
	 * If it is still not found, an empty default value will be returned, i.e. the key will then be looked up directly without any prefix.
	 * The lookup will be of the form [sourceslot.]key, where [sourceslot] may itself be an Anything::LookupPath() like path.
	 * @param ctx Context used to lookup the SourceSlot prefix.
	 * @return value of SourceSlot lookup
	 * @return empty when not specified in config
	 */
	String GetSourceSlot(Context &ctx) const {
		return DoGetSourceSlot(ctx);
	}

	//! Obtain mapper specific named slot delimiter used when looking up values using Anything.LookupPath() semantics.
	/*! Lookup() slot \em Delim in mapper configuration to get the delimiter or use a dot '.' as default.
	 * @return First character of \em Delim as specified in mapper configuration
	 * @return Otherwise a dot '.'
	 */
	const char getDelim() const {
		return Lookup( "Delim", "." )[0L];
	}

	//! Obtain mapper specific indexed slot delimiter used when looking up values using Anything.LookupPath() semantics.
	/*! Lookup() slot \em IndexDelim in mapper configuration to get the delimiter or use a colon ':' as default.
	 * @return First character of IndexDelim as specified in mapper configuration
	 * @return Otherwise a colon ':'
	 */
	const char getIndexDelim() const {
		return Lookup( "IndexDelim", ":" )[0L];
	}

	//!FIXME: remove Finds InputMappers, too
	static ParameterMapper *FindInputMapper(const char *name);

	//! convenience function:
	static void PlaceIntoAnyOrAppendIfNotEmpty(Anything &var, ROAnything theValue);

protected:
	//! Major hook method for subclasses, default does script interpretation
	/*! @param key the name defines kind of value to get or the slot in the script to use
		@param value collects data within script
		@param ctx the thread context of the invocation
		@param script to be interpreted if any, for subclasses this is the config to use
		@return returns true if the mapping was successful otherwise false */
	virtual bool DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script);

	//! Major hook method for subclasses, default does script interpretation
	/*! @param key the name defines kind of values to get or the slot in the script to use
		@param os The stream to map values onto
		@param ctx the thread context of the invocation
		@param script to be interpreted if any, for subclasses this is the config to use
		@return returns true if the mapping was successful otherwise false */
	virtual bool DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything script);

	//! Hook for recursion stopper in Mapper script interpretation returning an Anything value.
	/*! Looks for value in context using the key and assigns or appends to value depending if value was Null before.
	 * @param key the name defines kind of values to get or the slot in the script to use
	 * @param value Result value collector, if value is not AnyNullImpl, values will get appended
	 * @param ctx Context used to lookup the \em key
	 * @return true in case the key was found and resulted in any value; a Null Anything is also a valid value
	 * @return false otherwise
	 */
	virtual bool DoFinalGetAny(const char *key, Anything &value, Context &ctx);

	//! Hook for recursion stopper in Mapper script interpretation printing value of DoFinalGetAny() onto stream.
	/*! Composite Anything values are printed on os, non-array values are rendered literally as String.
	 * @param key the name defines kind of values to get or the slot in the script to use
	 * @param os The stream to map/print values onto
	 * @param ctx Context used to lookup the \em key
	 * @return true in case the key was found and resulted in any value; a Null Anything is also a valid value
	 * @return false otherwise
	 */
	virtual bool DoFinalGetStream(const char *key, std::ostream &os, Context &ctx);

	//! Generate the config file name (without extension, which is assumed to be any). Is simply the concatenation of category and "Meta". Iif category is "ParameterMapper" we use "ParameterMapper" instead, to keep compatibility.
	virtual bool DoGetConfigName(const char *category, const char *, String &configFileName) const;

	//! Load an anything and make it available by storing a reference in fConfig. The default implementation uses the cache handler. If you provide your own implementation be careful about the lifetime of the loaded anything otherwise fConfig points to invalid data.
	virtual bool DoLoadConfig(const char *category);

	//! Subclass hook which can be overridden for special behavior
	/*! @copydetails SelectScript()
	 */
	virtual ROAnything DoSelectScript(const char *key, ROAnything script, Context &ctx) const;

	//! Subclass hook to be overridden for special behavior
	/*! @copydetails GetSourceSlot()
	 */
	virtual String DoGetSourceSlot(Context &ctx) const;

private:
	ParameterMapper();
	ParameterMapper(const ParameterMapper &);
	ParameterMapper &operator=(const ParameterMapper &);
};

//---------------- EagerParameterMapper ------------------------------
//! A ParameterMapper eager to interpret its config, interprets full config if key is not found
class EagerParameterMapper : public ParameterMapper
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	EagerParameterMapper(const char *name): ParameterMapper(name) {}
	EagerParameterMapper(const char *name, ROAnything config);
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) EagerParameterMapper(fName);
	}

protected:
	virtual ROAnything DoSelectScript(const char *key, ROAnything script, Context &ctx) const;

private:
	EagerParameterMapper();
	EagerParameterMapper(const EagerParameterMapper &);
	EagerParameterMapper &operator=(const EagerParameterMapper &);
};

//---------------- ResultMapper ------------------------------
//! Base class for putting results into context
/*! This Mapper supports behavior configuration. Put the following into OutputMapperMeta.any for the corresponding Mapper alias name.
\par Configuration
\code
{
	/PutPolicy		String		optional, default "Put" or "Append" if a value already exists
								"Put": always a non-array-any slot, set value as 'slots' content
								"Append": always an array-any slot, append value below slot
	/Delim			char		optional, default '.', specify path delimiting character for Lookup operations within DoGetDestinationAny()
	/IndexDelim		char		optional, default ':', specify index delimiting character for Lookup operations within DoGetDestinationAny()
	/<Put-Key>	{					Anything	optional, key specific Mapper-Script to execute when hit
		/<ResultMapperName>	*				The specified ResultMapper will be called using an empty script as its configuration, eg. mapping ends with called ResultMapper
		/<ResultMapperName> {				The specified ResultMapper will be called using the given script to be able to further delegate result mapping.
			<specific mapper config>
		}
	}
}
\endcode
*/
class ResultMapper : public HierarchConfNamed
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	ResultMapper(const char *name);
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	//---- registry api
	RegCacheDef(ResultMapper);	// FindResultMapper()

	static const char* gpcCategory;
	static const char* gpcConfigPath;

	//! Takes an int for mapping according to key. Clients use this method to push an int value with name key into the Mapper
	/*! @param key the name defines kind of input for the value
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Put(const char *key, int value, Context &ctx);

#if !defined(BOOL_NOT_SUPPORTED)
	//! Takes an bool for mapping according to key. Clients use this method to push an bool value with name key into the Mapper
	/*! @param key the name defines kind of input for the value
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Put(const char *key, bool value, Context &ctx);
#endif

	//! Takes an long for mapping according to key. Clients use this method to push an long value with name key into the Mapper
	/*! @param key the name defines kind of input for the value
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Put(const char *key, long value, Context &ctx);

	//! Takes an float for mapping according to key. Clients use this method to push an float value with name key into the Mapper
	/*! @param key the name defines kind of input for the value
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Put(const char *key, float value, Context &ctx);

	//! Takes an double for mapping according to key. Clients use this method to push an double value with name key into the Mapper
	/*! @param key the name defines kind of input for the value
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Put(const char *key, double value, Context &ctx);

	//! Takes an String for mapping according to key. Clients use this method to push an String value with name key into the Mapper
	/*! @param key the name defines kind of input for the value
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Put(const char *key, String value, Context &ctx);

	//! Takes an const Anything & for mapping according to key. Clients use this method to push an const Anything & value with name key into the Mapper
	/*! @param key the name defines kind of input for the value
		@param value the value to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Put(const char *key, Anything value, Context &ctx);

	//! Reads from istream is values according to key. Clients use this method to let the mapper read values associated with the key directly from a istream
	/*! @param key the name defines kind of input for the value
		@param is the stream to be mapped
		@param ctx the thread context of the invocation
		@return returns true if the mapping was successful otherwise false */
	bool Put(const char *key, std::istream &is, Context &ctx);

	/*! Calls DoGetDestinationSlot to get the name - or empty string - which will be used as base path in tmpstore. The 'base' path - the one returned here - will be stored in ResultMapper.DestinationSlot for later usage.
		@param ctx the context in which to look for the destination slot
		@return the name of the slot for later lookup or the empty string */
	String GetDestinationSlot(Context &ctx);

	//! Get mapper script for the given key if any. Subclasses can overwrite DoSelectScript() to avoid scripting (recursion) or tailor to their needs.
	/*! Default is to return script[key].
	 * Mappers that want complete scripting should return whole script for interpretation regardless of key given.
	 * @param key current mapping key
	 * @param script script in which to check for the given key, if omitted or the key was not found, we will check using hierarchic Lookup() mechanism.
	 * @param ctx Context which might be used for special cases
	 * @return new script according to given key or null Anything in case nothing was found
	 */
	ROAnything SelectScript(const char *key, ROAnything script, Context &ctx) const {
		return DoSelectScript(key, script, ctx);
	}

	//! Obtain mapper specific named slot delimiter used when looking up values using Anything.LookupPath() semantics.
	/*! Lookup() slot \em Delim in mapper configuration to get the delimiter or use a dot '.' as default.
	 * @return First character of \em Delim as specified in mapper configuration
	 * @return Otherwise a dot '.'
	 */
	const char getDelim() const {
		return Lookup("Delim", ".")[0L];
	}

	//! Obtain mapper specific indexed slot delimiter used when looking up values using Anything.LookupPath() semantics.
	/*! Lookup() slot \em IndexDelim in mapper configuration to get the delimiter or use a colon ':' as default.
	 * @return First character of IndexDelim as specified in mapper configuration
	 * @return Otherwise a colon ':'
	 */
	const char getIndexDelim() const {
		return Lookup("IndexDelim", ":")[0L];
	}

protected:
	//! Major hook for subclasses that want to do something with their config passed as script. The default is to interpret the script and put a value for every script item used. Recursion will be stopped by DoFinalPutAny which places its value under slot key below given DoGetDestinationSlot()
	/*! @param key the key usually defines the associated kind of output-value
		@param value the value to be mapped
		@param ctx the context of the invocation
		@param script current mapper configuration as ROAnything
		@return returns true if the mapping was successful otherwise false */
	virtual bool DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script);

	//! Almost the same as DoPutAny but it uses the given slotname as new key for further processing. This allows key redefinition based on slotnames which are not Mappers.
	/*! @param key the key usually defines the associated kind of output-value
		@param value the value to be mapped
		@param ctx the context of the invocation
		@param script current mapper configuration as ROAnything
		@param slotname new key to use for further processing
		@return returns true if the mapping was successful otherwise false */
	virtual bool DoPutAnyWithSlotname(const char *key, Anything &value, Context &ctx, ROAnything script, const char *slotname);

	//! Reads from istream and puts its content according to key. Default behavior is to read in the content of the stream into an anything an call the DoPutAny() method. Clients use this method to let the mapper read values associated with the key directly from a istream.
	/*! @param key the key usually defines the associated kind of output-value
		@param is stream whose content will be mapped
		@param ctx the context of the invocation
		@param script current mapper configuration as ROAnything
		@return returns true if the mapping was successful otherwise false */
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything script);

	/*! Hook for breaking recursion in mapper script interpretation. Store the value in tmpstore under tmp.slot.key, where slot is retrieved with GetDestinationSlot(). if slot == "", then value is stored under tmp.key directly key may NOT be empty (fails otherwise).
		@param key name used to distinguish kind of output
		@param value value to store
		@param ctx the context in which to store the value
		@return true in case we could store away the value */
	virtual bool DoFinalPutAny(const char *key, Anything &value, Context &ctx);

	//! Hook for breaking recursion in mapper script interpretation. Stores everything readable from the stream as a String using DoFinalPutAny.
	/*! @param key the key usually defines the associated kind of output-value
		@param is stream whose content will be mapped
		@param ctx the context of the invocation
		@return returns true if the mapping was successful otherwise false */
	virtual bool DoFinalPutStream(const char *key, std::istream &is, Context &ctx);

	//! Defines the base name space where to put values into tmpstore, default is "Mapper". May return a "."-separated path, such as x.y.z. If empty string, tmpstore will be used as base.
	/*!	@param ctx the context in which to look for the destination slot
		@param pcDefault default value to return if we could not find a name either within mappers config or within context
		@return the name of the slot for later lookup or the empty string */
	virtual String DoGetDestinationSlot(Context &ctx, const char *pcDefault);

	//! Looks up the Anything at key in Context using Slotfinder
	/*!	@param key the key usually defines the associated kind of output-value
		@param targetAny Anything reference into TmpStore to finally put values at. It uses DestinationSlot and key to get the correct location in Context.
		@param ctx the context of the invocation */
	virtual void DoGetDestinationAny(const char *key, Anything &targetAny, Context &ctx);

	//! Generate the config file name (without extension, which is assumed to be any) out of category and objName. The default implementation just takes the objName.
	virtual bool DoGetConfigName(const char *category, const char *objName, String &configFileName) const;

	//! Load an anything and make it available by storing a reference in fConfig. The default implementation uses the cache handler. If you provide your own implementation be careful about the lifetime of the loaded anything otherwise fConfig points to invalid data.
	virtual bool DoLoadConfig(const char *category);

	//! Subclass hook which can be overridden for special behavior
	/*! @copydetails SelectScript() */
	virtual ROAnything DoSelectScript(const char *key, ROAnything script, Context &ctx) const;

private:
	ResultMapper();
	ResultMapper(const ResultMapper &);
	ResultMapper &operator=(const ResultMapper &);
};

//---------------- EagerResultMapper ------------------------------
//! A ResultMapper eager to do something with its config, interprets full config if key is not found
class EagerResultMapper: public ResultMapper {
	EagerResultMapper();
	EagerResultMapper(const EagerResultMapper &);
	EagerResultMapper &operator=(const EagerResultMapper &);
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	EagerResultMapper(const char *name) :
		ResultMapper(name) {
	}
	EagerResultMapper(const char *name, ROAnything config);
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) EagerResultMapper(fName);
	}

protected:
	virtual ROAnything DoSelectScript(const char *key, ROAnything script, Context &ctx) const;
};

#define RegisterParameterMapper(name) RegisterObject(name, ParameterMapper)
#define RegisterParameterMapperAlias(name,classname) RegisterShortName(name,classname,ParameterMapper)
#define RegisterResultMapper(name) RegisterObject(name, ResultMapper)
#define RegisterResultMapperAlias(name,classname) RegisterShortName(name,classname,ResultMapper)

//  ========================== Other Mappers ===============================

//  -------------------------- RootMapper ------------------------------
//! Result mapper which stores Results directly at root of TmpStore
/*! This could be configured in config, but it would be necessary to write a config every time a RootMapper is used.
*/
class RootMapper : public ResultMapper
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	RootMapper(const char *name) : ResultMapper(name) {}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) RootMapper(fName);
	}

protected:
	String DoGetDestinationSlot(Context &ctx, const char *pcDefault) {
		return "";
	}

private:
	RootMapper();
	RootMapper(const RootMapper &);
	RootMapper &operator=(const RootMapper &);
};

//  -------------------------- ConfigMapper ----------------------------
//! Eager mapper that simply returns it's config (Scripting-Blocker)
/*! Config: any config, is returned identically when Get("Anykey", Anything, ctx) is called.<br>
Scripting can be re-activated with /MapperScript { ... a script ... }, the script is then evaluated and the results are inserted in place of the call.<br>
Example:<pre>
/ConfigMapper {
	/foo "bar"
	/nested {
		/inner {
			/value 1
			/value 2
		}
		/calculated {
			/MapperScript {
				/MyMapper {
					/config	"of MyMapper"
				}
			}
		}
	}
}
</pre>Result: exactly the config above, with the difference that the value of "calculated" slot will be assigned the results of the call to MyMapper
*/
class ConfigMapper : public EagerParameterMapper
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	ConfigMapper(const char *name) : EagerParameterMapper(name) {};
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) ConfigMapper(fName);
	};

protected:
	bool DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything config);
	virtual void EvaluateConfig(ROAnything config, Anything &value, Context &ctx);

private:
	ConfigMapper();
	ConfigMapper(const ConfigMapper &);
	ConfigMapper &operator=(const ConfigMapper &);
};

#endif
