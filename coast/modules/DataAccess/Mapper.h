/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MAPPER_H
#define _MAPPER_H

#include "config_dataaccess.h"
#include "Context.h"
#include "WDModule.h"

#define HIERARCH_MAPPERS

class Registry;

//---- MappersModule -----------------------------------------------------------
class EXPORTDECL_DATAACCESS MappersModule : public WDModule
{
public:
	MappersModule(const char *name);
	~MappersModule();

	virtual bool Init(const Anything &config);
	virtual bool ResetFinis(const Anything &);
	virtual bool Finis();
};
//----------------------- ParameterMapper aka ParameterMapper --------------------
#if defined(HIERARCH_MAPPERS)
class EXPORTDECL_DATAACCESS ParameterMapper : public HierarchConfNamed
#else
class EXPORTDECL_DATAACCESS ParameterMapper : public ConfNamedObject
#endif
{
public:
	ParameterMapper(const char *name);
	virtual ~ParameterMapper();
	IFAObject *Clone() const;
	//---- registry api
	RegCacheDef(ParameterMapper);	// FindParameterMapper()
	//! pulls out an int according to key
	//! Clients use this method to pull out an int value with name key from the Mapper
	//! \param key the name defines kind of value to get
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Get(const char *key, int &value, Context &ctx);
#if !defined(BOOL_NOT_SUPPORTED)
	//! pulls out an bool according to key
	//! Clients use this method to pull out an bool value with name key from the Mapper
	//! \param key the name defines kind of value to get
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Get(const char *key, bool &value, Context &ctx);
#endif
	//! pulls out an long according to key
	//! Clients use this method to pull out an long value with name key from the Mapper
	//! \param key the name defines kind of value to get
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Get(const char *key, long &value, Context &ctx);
	//! pulls out an float according to key
	//! Clients use this method to pull out an float value with name key from the Mapper
	//! \param key the name defines kind of value to get
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Get(const char *key, float &value, Context &ctx);
	//! pulls out an double according to key
	//! Clients use this method to pull out an double value with name key from the Mapper
	//! \param key the name defines kind of value to get
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Get(const char *key, double &value, Context &ctx);
	//! pulls out an String according to key
	//! Clients use this method to pull out an String value with name key from the Mapper
	//! \param key the name defines kind of value to get
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Get(const char *key, String &value, Context &ctx);

	//! pulls out an Anything according to key
	//! Clients use this method to pull out an Anything value with name key from the Mapper
	//! \param key the name defines kind of value to get
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Get(const char *key, Anything &value, Context &ctx);

	//! write  to the ostream os values according to key
	//! Clients use this method to write to the stream values associated with the key
	//! \param key the name defines kind of values to write
	//! \param os the stream to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Get(const char *key, ostream &os, Context &ctx);

	//! major hook method for subclasses, default does script interpretation
	//! \param key the name defines kind of value to get or the slot in the script to use
	//! \param value collects data within script
	//! \param ctx the thread context of the invocation
	//! \param script to be interpreted if any, for subclasses this is the config to use
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script);
	//! major hook method for subclasses, default does script interpretation
	//! \param key the name defines kind of values to write or the slot in the script to use
	//! \param os the stream to be mapped on
	//! \param ctx the thread context of the invocation
	//! \param script to be interpreted if any, for subclasses this is the config to use
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoGetStream(const char *key, ostream &os, Context &ctx, ROAnything script);

	//! convenience function:
	static void PlaceIntoAnyOrAppendIfNotEmpty(Anything &var, ROAnything theValue);

protected:
	// generate the config file name (without extension, which is assumed to be any)
	// is simply the concatenation of category and "Meta"
	// (if category is "ParameterMapper" we use "ParameterMapper" instead, to keep compatibility)
	virtual bool DoGetConfigName(const char *category, const char *, String &configFileName);

	// load an anything and make it available by storing a reference in fConfig
	// the default implementation uses the cache handler
	// if you provide your own implementation be careful about the lifetime of
	// the loaded anything otherwise fConfig points to invalid data
	virtual bool DoLoadConfig(const char *category);

	//! subclasses might overwrite this method to avoid scripting (recursion)
	//! default is to pass script[key],
	//! mappers that want complete scripting should return whole script for interpretation if key not found
	virtual ROAnything DoSelectScript(const char *key, ROAnything script, Context &ctx);

	//! hook for recursion stoppper in Mapper script interpretation
	//! looks for value in context using the key and appends it to value
	virtual bool DoFinalGetAny(const char *key, Anything &value, Context &ctx);

	//! hook for recursion stopper in Mapper script interpretation
	//! default writes value from context found via key to os
	//! if value is a composite anything the anything is Printed on os.
	//! otherwise the value is rendered literally as a string
	virtual bool DoFinalGetStream(const char *key, ostream &os, Context &ctx);

	//! defines the name space where to get values from
	//! Default is -none-, i.e. the key will be looked up directly.
	//! If this method is overridden in subclasses, the lookup will be of the form
	//! [sourceslot].key, where [sourceslot] may also be a "."-separated path
	virtual String DoGetSourceSlot(Context &ctx);

private:
	ParameterMapper();
	ParameterMapper(const ParameterMapper &);
	ParameterMapper &operator=(const ParameterMapper &);

	friend class ParameterMapperTest;
	friend class MapperTest;
};

//---------------- EagerParameterMapper ------------------------------
//! A ParameterMapper eager to interpret its config, interprets full config if key is not found
class EXPORTDECL_DATAACCESS EagerParameterMapper : public ParameterMapper
{
public:
	EagerParameterMapper(const char *name): ParameterMapper(name) {}
	EagerParameterMapper(const char *name, ROAnything config);
	virtual ~EagerParameterMapper() {}
	IFAObject *Clone() const {
		return new EagerParameterMapper(fName);
	}

protected:
	virtual ROAnything DoSelectScript(const char *key, ROAnything script, Context &ctx);

private:
	EagerParameterMapper();
	EagerParameterMapper(const EagerParameterMapper &);
	EagerParameterMapper &operator=(const EagerParameterMapper &);

	friend class ParameterMapperTest;
};

//---------------- ResultMapper ------------------------------
/*! Base class for putting results into context
This Mapper supports behavior configuration. Put the following into OutputMapperMeta.any for the corresponding Mapper alias name.
<B>Configuration:</B><PRE>
{
	/AppendAnyAlways		long		optional, default 0, if set to value != 0 Put(key, Anything) will always append the given Anything at slot key. Otherways the Anything will only be appended when the slot already exists.
}</PRE>
*/
#if defined(HIERARCH_MAPPERS)
class EXPORTDECL_DATAACCESS ResultMapper : public HierarchConfNamed
#else
class EXPORTDECL_DATAACCESS ResultMapper : public ConfNamedObject
#endif
{
public:
	ResultMapper(const char *name);
	virtual ~ResultMapper();
	IFAObject *Clone() const;

	//---- registry api
	RegCacheDef(ResultMapper);	// FindResultMapper()
	//! Takes an int for mapping according to key
	//! Clients use this method to push an int value with name key into the Mapper
	//! \param key the name defines kind of input for the value
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Put(const char *key, int value, Context &ctx);
#if !defined(BOOL_NOT_SUPPORTED)
	//! Takes an bool for mapping according to key
	//! Clients use this method to push an bool value with name key into the Mapper
	//! \param key the name defines kind of input for the value
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Put(const char *key, bool value, Context &ctx);
#endif

	//! Takes an long for mapping according to key
	//! Clients use this method to push an long value with name key into the Mapper
	//! \param key the name defines kind of input for the value
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Put(const char *key, long value, Context &ctx);
	//! Takes an float for mapping according to key
	//! Clients use this method to push an float value with name key into the Mapper
	//! \param key the name defines kind of input for the value
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Put(const char *key, float value, Context &ctx);
	//! Takes an double for mapping according to key
	//! Clients use this method to push an double value with name key into the Mapper
	//! \param key the name defines kind of input for the value
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Put(const char *key, double value, Context &ctx);
	//! Takes an String for mapping according to key
	//! Clients use this method to push an String value with name key into the Mapper
	//! \param key the name defines kind of input for the value
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Put(const char *key, String value, Context &ctx);
	//! Takes an const Anything & for mapping according to key
	//! Clients use this method to push an const Anything & value with name key into the Mapper
	//! \param key the name defines kind of input for the value
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Put(const char *key, Anything value, Context &ctx);
	//! reads from istream is values according to key
	//! Clients use this method to let the mapper read values associated with the key directly from a istream
	//! \param key the name defines kind of input for the value
	//! \param is the stream to be mapped
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool Put(const char *key, istream &is, Context &ctx);

	//! major hook for subclasses that want to do something with their config passed as script
	//! default is to interpret the script and place value for every script item used
	//! recursion stopped by DoFinalPutAny that places value under slot key under slot given DoGetDestinationSlot()
	virtual bool DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script);

	//! reads from istream is values according to key
	//! Clients use this method to let the mapper read values associated with the key directly from a istream
	//! \param key the name defines kind of input for the value
	//! \param is the stream to be mapped
	//! \param ctx the thread context of the invocation
	//! \param config current mapper configuration as ROAnything
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoPutStream(const char *key, istream &is, Context &ctx, ROAnything script);

protected:
	/*! defines the base name space where to put values into tmpstore, default is "Mapper". May return a "."-separated path, such as x.y.z. If empty string, tmpstore will be used as base.
		\param ctx the context in which to look for the destination slot
		\return the name of the slot for later lookup or the empty string */
	virtual String DoGetDestinationSlot(Context &ctx);

	//! looks up the Anything at kPrefix in Context using Slotfinder
	virtual void DoGetDestinationAny(const char *key, Anything &targetAny, Context &ctx);

	/*! Hook for breaking recursion in mapper script interpretation. Store the value in tmpstore under tmp.slot.key, where slot is retrieved with GetDestinationSlot(). if slot == "", then value is stored under tmp.key directly key may NOT be empty (fails otherwise).
		\param key name used to distinguish kind of output
		\param value value to store
		\param ctx the context in which to store the value
		\return true in case we could store away the value */
	virtual bool DoFinalPutAny(const char *key, Anything value, Context &ctx);

	//! hook for breaking recursion in mapper script interpretation
	//! stores everything readable from the stream as a String using DoFinalPutAny
	virtual bool DoFinalPutStream(const char *key, istream &is, Context &ctx);

	// generate the config file name (without extension, which is assumed to be any)
	// out of category and objName
	// the default implementation just takes the objName
	virtual bool DoGetConfigName(const char *category, const char *objName, String &configFileName);

	// load an anything and make it available by storing a reference in fConfig
	// the default implementation uses the cache handler
	// if you provide your own implementation be careful about the lifetime of
	// the loaded anything otherwise fConfig points to invalid data
	virtual bool DoLoadConfig(const char *category);

	//! subclasses might overwrite this method to enable deep scripting (recursion)
	//! default is to pass script[key], mappers that want complete scripting can return whole script
	virtual ROAnything DoSelectScript(const char *key, ROAnything script, Context &ctx);

private:
	/*! Calls DoGetDestinationSlot to get the name - or empty string - which will be used as base path in tmpstore. The 'base' path - the one returned here - will be stored in ResultMapper.DestinationSlot for later usage.
		\param ctx the context in which to look for the destination slot
		\return the name of the slot for later lookup or the empty string */
	String GetDestinationSlot(Context &ctx);

	ResultMapper();
	ResultMapper(const ResultMapper &);
	ResultMapper &operator=(const ResultMapper &);

	friend class ResultMapperTest;
	friend class MapperTest;
};

//---------------- EagerResultMapper ------------------------------
//! A ResultMapper eager to do something with its config, interprets full config if key is not found
class EXPORTDECL_DATAACCESS EagerResultMapper : public ResultMapper
{
public:
	EagerResultMapper(const char *name): ResultMapper(name) {}
	EagerResultMapper(const char *name, ROAnything config);
	virtual ~EagerResultMapper() {}
	IFAObject *Clone() const {
		return new EagerResultMapper(fName);
	}

protected:
	virtual ROAnything DoSelectScript(const char *key, ROAnything script, Context &ctx);

private:
	EagerResultMapper();
	EagerResultMapper(const EagerResultMapper &);
	EagerResultMapper &operator=(const EagerResultMapper &);

	friend class ResultMapperTest;
};

#define RegisterParameterMapper(name) RegisterObject(name, ParameterMapper)
#define RegisterParameterMapperAlias(name,classname) RegisterShortName(name,classname,ParameterMapper)
#define RegisterResultMapper(name) RegisterObject(name, ResultMapper)
#define RegisterResultMapperAlias(name,classname) RegisterShortName(name,classname,ResultMapper)

//  ========================== Other Mappers ===============================

//  -------------------------- RootMapper ------------------------------
//! Result mapper which stores Results directly at root of TmpStore
//! This could be configured in config, but it would be necessary to
//! write a config every time a RootMapper is used.

class EXPORTDECL_DATAACCESS RootMapper : public ResultMapper
{
public:
	RootMapper(const char *name) : ResultMapper(name) {}
	virtual ~RootMapper() {}
	IFAObject *Clone() const {
		return new RootMapper(fName);
	}

protected:
	String DoGetDestinationSlot(Context &ctx) {
		return "";
	}

private:
	RootMapper();
	RootMapper(const RootMapper &);
	RootMapper &operator=(const RootMapper &);
};

//  -------------------------- ConfigMapper ----------------------------
//! <b>Eager mapper that simply returns it's config (Scripting-Blocker)</b>
//! Config: any config, is returned identically when Get("Anykey", Anything, ctx)
//! is called.<br>
//! Scripting can be re-activated with /MapperScript { ... a script ... }, the
//! script is then evaluated and the results are inserted in place of the call.<br>
/* Example:<pre>
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
<pre>Result: exactly the config above, with the difference that the value of "calculated"
slot will be assigned the results of the call to MyMapper
*/

class EXPORTDECL_DATAACCESS ConfigMapper : public EagerParameterMapper
{
public:
	ConfigMapper(const char *name) : EagerParameterMapper(name) {};
	IFAObject *Clone() const {
		return new ConfigMapper(fName);
	};

protected:
	bool DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything config);
	virtual void EvaluateConfig(ROAnything config, Anything &value, Context &ctx);

private:
	ConfigMapper();
	ConfigMapper(const ConfigMapper &);
	ConfigMapper &operator=(const ConfigMapper &);

	friend class ConfigMapperTest;
};

#endif
