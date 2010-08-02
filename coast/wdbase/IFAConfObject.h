/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _IFACONFOBJECT_H
#define _IFACONFOBJECT_H

#include "config_wdbase.h"

#include "IFAObject.h"
#include "LookupInterface.h"
#include "Anything.h"

class Registry;
class InstallerPolicy;
class TerminationPolicy;

//---- RegisterableObject ----------------------------------------------------------
//!defines api to support registerable objects; objects registered with a name in a category
class EXPORTDECL_WDBASE RegisterableObject : public NamedObject
{
public:
	//! Named object constructor
	/*! @param name Used to distinguish between objects of same type */
	RegisterableObject(const char *name);

	//! register api; objects can be registered with a category and a name
	void Register(const char *name, const char *category);

	//! remove object from the registry
	void Unregister(const char *name, const char *category);

	//! object marked by static initializer object as statically allocated; this prevents this object from deletion by the terminators
	void MarkStatic() {
		fStaticallyInitialized = true;
	}

	//! query static initialized flag
	bool IsStatic()	  {
		return fStaticallyInitialized;
	}

	bool Initialize(const char *category = NULL);

	bool Finalize();

	/*! Check if object has successfully called DoInitialize()
		\return true in case both steps were executed successfully, false otherwise */
	bool IsInitialized() const {
		return DoIsInitialized();
	}

	//! implements API for naming support
	virtual void SetName(const char *str)	{
		fName = str;
	}

	//! implements API for naming support
	virtual bool GetName(String &str) const {
		str = fName;
		return true;
	}

	//! implements API for getting name
	virtual const char *GetName() const {
		return (const char *)fName;
	}

	const String& getInstalledCategory() const { return fCategory; }

	// support for registerable named objects
	// this method gets called during initialization

	//!installs additional objects according to installerspec and installer policy into category
	static bool Install(const ROAnything installerSpec, const char *category, InstallerPolicy *ip);
	//!terminates category removing all objects from the registry as implemented by the termination policy
	static bool Terminate(const char *category, TerminationPolicy *tp);
	//!terminates category removing only the objects which are not installed by the static initializers to allow reinitialization
	static bool ResetTerminate(const char *category, TerminationPolicy *terminator);
	//!terminates category removing only the objects which are not installed by the static initializers and installs again with installerSpec
	static bool Reset(const ROAnything installerSpec, const char *category, InstallerPolicy *installer, TerminationPolicy *terminator);

	friend class WDModuleTest;

protected:
	/*! subclass initialize api; specific things can be done here, like configuration loading and so on
		\return true in case of success, false otherwise */
	virtual bool DoInitialize() {
		return true;
	};

	/*! subclass finalize api; specific things can be done here, like configuration unloading and so on
		\return true in case of success, false otherwise */
	virtual bool DoFinalize() {
		return true;
	};

	/*! Check if object has successfully called DoInitialize()
		\return true in case both steps were executed successfully, false otherwise */
	virtual bool DoIsInitialized() const {
		return fbInitialized;
	}

	//! object name represented as string
	String fName, fCategory;

	//! flag to know if object is allocated by static registerer
	bool fStaticallyInitialized;

	//! flag to track if object was initialized
	bool fbInitialized;

	//!sets flag to reset cache
	static void ResetCache(bool resetCache);

	//!flag that triggers reset of registry cache
	static bool fgResetCache;

private:
	/*! subclass initialize api; specific things can be done here, like configuration loading and so on
		\return true in case of success, false otherwise */
	virtual bool IntInitialize(const char *category);

	/*! subclass finalize api; specific things can be done here, like configuration unloading and so on
		\return true in case of success, false otherwise */
	virtual bool IntFinalize();

	//!do not use
	RegisterableObject();
	//!do not use
	RegisterableObject(const RegisterableObject &);
	//!do not use
	RegisterableObject &operator=(const RegisterableObject &);
};

//!objects are generated as static variables by macro RegisterObject; installs RegisterableObject r with name in category
class EXPORTDECL_WDBASE RegisterableObjectInstaller
{
public:
	//!installs r into category with name; caches r into fObject
	RegisterableObjectInstaller(const char *name, const char *category, RegisterableObject *r);
	//!deletes fObject
	~RegisterableObjectInstaller();

protected:
	//!the object that was installed in the registry
	RegisterableObject *fObject;

	//!the category of the installed object for debugging purposes
	String fCategory;
};

//!creates static variable of type RegisterableObjectInstaller
#define RegisterObject(name, category) \
	static RegisterableObjectInstaller _NAME3_(name,category,Registerer) (_QUOTE_(name), _QUOTE_(category), new name(_QUOTE_(name)))

//!creates static variable of type RegisterableObjectInstaller with a short name
#define RegisterShortName(sname, name, instname) \
	static RegisterableObjectInstaller _NAME3_(sname,instname,Registerer) (_QUOTE_(sname), _QUOTE_(instname), new name(_QUOTE_(sname)))

//!creates definition for a Find method for category
#define RegCacheDef(category) 										\
	static _NAME1_(category) *_NAME2_(Find, category)(const char *)

//!creates implementation for a Find method for category
#define RegCacheImpl(category) 																	\
	_NAME1_(category) *_NAME1_(category)::_NAME2_(Find, category)(const char *name) 			\
	{ 																							\
		static Registry *fgRegistry= 0;															\
		if ( !fgRegistry || RegisterableObject::fgResetCache ) fgRegistry= Registry::GetRegistry(_NAME1_(_QUOTE_(category))); 				\
		_NAME1_(category) *catMember = 0;														\
		if (name) {																				\
			catMember= SafeCast(fgRegistry->Find(name),_NAME1_(category));						\
		}																						\
		StatTrace(_NAME1_(category)._NAME2_(Find, category), "Looking for <" << NotNull(name) << "> in category <" << _QUOTE_(category) << ">" << (catMember?" succeeded":" failed"), Storage::Current());\
		return catMember;																		\
	}

#define RegCacheImplInline(category) 										\
	static _NAME1_(category) *_NAME2_(Find, category)(const char *name)		\
	{ 																							\
		static Registry *fgRegistry= 0;															\
		if ( !fgRegistry || RegisterableObject::fgResetCache ) fgRegistry= Registry::GetRegistry(_NAME1_(_QUOTE_(category))); 				\
		_NAME1_(category) *catMember = 0;														\
		if (name) {																				\
			catMember= SafeCast(fgRegistry->Find(name),_NAME1_(category));						\
		}																						\
		StatTrace(_NAME1_(category)._NAME2_(Find, category), "Looking for <" << NotNull(name) << "> in category <" << _QUOTE_(category) << ">" << (catMember?" succeeded":" failed"), Storage::Current());\
		return catMember;																		\
	}

//---- NotCloned ----------------------------------------------------------
//!RegisterableObject as singleton; only aliases to the same object are installed
//! NotCloned provides a special clone method that just returns this
//! therefore it installes simple aliases ( entries in the registry, that
//! access the same object under different names eg. HTMLTemplateRenderer and
//! HTML )
class EXPORTDECL_WDBASE NotCloned : public RegisterableObject
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	NotCloned(const char *name)
		: RegisterableObject(name)
	{}

	//! Public api to return reference to this object instead of cloning, e.g. like a singleton
	/*! @return pseudo clone of this, like a singleton
	 */
	IFAObject *Clone() const {
		NotCloned *nonconst_this = (NotCloned *) this;
		return nonconst_this;
	}

private:
	//!do not use
	NotCloned();
	//!do not use
	NotCloned(const NotCloned &);
	//!do not use
	NotCloned &operator=(const NotCloned &);
};

//---- ConfNamedObject ----------------------------------------------------------
/*! configurable object; api for configuration support with default implementation
ConfNamedObject provides a protocol for handling configuration data
it already provides a default implementation that is sufficient
for most classes ( eg. Role and Page ) */
class EXPORTDECL_WDBASE ConfNamedObject : public RegisterableObject, public virtual LookupInterface
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	ConfNamedObject(const char *name)
		: RegisterableObject(name)
		, fbConfigLoaded(false)
	{}

	//! Public api to create a configured clone object. The real work is delegated to DoConfiguredClone().
	/*! After successful cloning, the objects configuration data gets loaded if possible.
	 * @param category Name of the category in which the objects configuration will be stored in the Cache
	 * @param name Name of the clone
	 * @param bInitializeConfig If set to true, the objects configuration will be loaded based on the cloned objects settings
	 * @return New object clone with configuration initialized if requested */
	ConfNamedObject *ConfiguredClone(const char *category, const char *name, bool bInitializeConfig);

	//! Check if object has loaded its config already.
	/*! @return true in case the config is loaded
	 * @return false otherwise */
	bool IsConfigLoaded() const {
		return fbConfigLoaded;
	}

	/*! Intermediary function to force set or merge the objects configuration. The usual mechanism by loading an appropriate file will be skipped.
	 * If this function gets called on an already initialized object, the existing configuration will not be replaced but merged. This is important
	 * because already existing ROAnything references would otherwise be dangling and lead to pure virtual method calls...
	 * \param category primary key in Registry to store objects of this kind
	 * \param key name of the ConfNamedObject to set the configuration
	 * \param newConfig configuration to merge in */
	void SetConfig(const char *category, const char *key, ROAnything newConfig);

	//! Getting the objects configuration if any
	/*! @return returns the objects configuration as ROAnything
	*/
	ROAnything GetConfig() const {
		return fConfig;
	}

	//! Get this objects associated config file name
	/*! @return associated conffiguration file name if any
	 */
	const String GetConfigName() const {
		String configFileName;
		DoGetConfigName(getInstalledCategory(), GetName(), configFileName);
		return configFileName;
	}

protected:
	/*! subclass initialize api; specific things can be done here, like configuration loading and so on
		\return true in case of success, false otherwise */
	virtual bool DoInitialize();

	/*! subclass finalize api; specific things can be done here, like configuration unloading and so on
		\return true in case of success, false otherwise */
	virtual bool DoFinalize();

	/*! Check if object has successfully called DoCheckConfig() and DoInitialize()
		\return true in case both steps were executed successfully, false otherwise */
	virtual bool DoIsInitialized() const {
		return ( fbConfigLoaded && RegisterableObject::DoIsInitialized() );
	}

	/*! Check if configuration is loaded; load it if not done or bInitializeConfig = true
		\param category Name of the category in which the objects configuration will be stored in the Cache
		\param bInitializeConfig Reload configuration regardless of already being configured
		\return true in case the object could be configured */
	bool DoCheckConfig(const char *category, bool bInitializeConfig = false);

	/*! Unload configuration of given category
		\return true in case the config could be unloaded successfully */
	bool DoUnloadConfig();

	//! Creates a new object through cloning having a different name.
	/*!	@copydetails ConfiguredClone() */
	virtual ConfNamedObject *DoConfiguredClone(const char *category, const char *name, bool bInitializeConfig);

	//! Generate the config file name out of category and objName
	/*! The generated name is without extension, it is assumed to be \em .any
	 * The default implementation just uses the objName as configuration file name.
	 * @param category Name of the category in which the objects configuration will be stored in the Cache
	 * @param objName Name of the configured object to get a configuration file name for
	 * @param configFileName String to hold the resulting config file name
	 * @return true in case either fConfigName was already set or objName was not the empty string
	 * @return false otherwise */
	virtual bool DoGetConfigName(const char *category, const char *objName, String &configFileName) const;

	//! Load the objects configuration from an anything (file), subclasses may overwrite this hook.
	/*! The loaded configuration file will be stored in the CacheHandler and this objects keeps a ROAnything reference in fConfig
		If you provide your own implementation, be careful about the lifetime of the underlying Anything as the fConfig
		ROAnything must always point to a valid AnyImpl */
	virtual bool DoLoadConfig(const char *category);

	//! Subclassed implementation of the LookupInterface, which checks if the key can be found inside our configuration (fConfig)
	/*! @copydetails LookupInterface::DoLookup() */
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;

	//! Set the name of the underlying configuration file.
	/*! This name will differ from the objects name when a ConfiguredClone is made.
	 * @param cfgName Name of the underlying configuration file */
	void SetConfigName(const char *cfgName) {
		fConfigName = cfgName;
	}

	//! the configuration of this object
	ROAnything fConfig;

	//! Configuration filename associated with this object
	String fConfigName;

	//! flag to track if config of object was loaded
	bool fbConfigLoaded;

private:
	/*! subclass initialize api; specific things can be done here, like configuration loading and so on
		\return true in case of success, false otherwise */
	virtual bool IntInitialize(const char *category);

	/*! subclass finalize api; specific things can be done here, like configuration unloading and so on
		\return true in case of success, false otherwise */
	virtual bool IntFinalize();

	//!do not use
	ConfNamedObject();
	//!do not use
	ConfNamedObject(const ConfNamedObject &);
	//!do not use
	ConfNamedObject &operator=(const ConfNamedObject &);
};

//---- HierarchConfNamed ----------------------------------------------------------
//!configurable object; api for configuration support with hierachical object relationships (inheritance)
//!implements inheritance relationship of configurations through super objects
class EXPORTDECL_WDBASE HierarchConfNamed : public ConfNamedObject
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	HierarchConfNamed(const char *name)
		: ConfNamedObject(name)
		, fSuper(0)
	{}

	//! hierarchical relationship API; set super object
	void SetSuper(HierarchConfNamed *super);

	//! hierarchical relationship API; get super object
	const HierarchConfNamed *GetSuper() const;

protected:
	//! Creates a new object through cloning using a different name. The main feature is a configuration support with hierachical object relationships (inheritance).
	/*!	@copydetails ConfNamedObject::ConfiguredClone() */
	virtual ConfNamedObject *DoConfiguredClone(const char *category, const char *name, bool bInitializeConfig);

	//! Subclassed implementation of the LookupInterface, which checks if the key can be found inside our configuration of any of our super classes.
	/*! @copydetails LookupInterface::DoLookup() */
	virtual bool DoLookup(const char *key, class ROAnything &resultconst, char delim, char indexdelim) const;

	//!pointer to super object
	HierarchConfNamed *fSuper;

private:
	//!do not use
	HierarchConfNamed();
	//!do not use
	HierarchConfNamed(const HierarchConfNamed &);
	//!do not use
	HierarchConfNamed &operator=(const HierarchConfNamed &);
};

#endif
