/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AppBooter.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "System.h"
#include "SysLog.h"
#include "Application.h"
#include "Registry.h"
#include "WDModule.h"
#include "URLUtils.h"
#include "Dbg.h"

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

//--- c-library modules used ---------------------------------------------------
#if !defined(WIN32)
// used for dynamic load of libraries
#include <dlfcn.h>
#endif

class DynLibLoader
{
public:
	DynLibLoader(const char *name);
	virtual ~DynLibLoader();

	virtual bool DLOpen() = 0;
	virtual bool DLClose() = 0;
	virtual long DLHandle() = 0;
	virtual String DLError() = 0;

protected:
	String fName;
};

#if defined(WIN32)

#define Sys(className)		_NAME2_(Win32,className)

class Win32DynLibLoader: public DynLibLoader
{
public:
	Win32DynLibLoader(const char *name);
	Win32DynLibLoader(const char *name, long handle);
	virtual ~Win32DynLibLoader();

	virtual bool DLOpen();
	virtual bool DLClose();
	virtual long DLHandle();
	virtual String DLError();

protected:
	HINSTANCE	fHandle;
};

#else
#define Sys(className)		_NAME2_(Unix,className)

class UnixDynLibLoader: public DynLibLoader
{
public:
	UnixDynLibLoader(const char *name);
	UnixDynLibLoader(const char *name, long handle);
	virtual ~UnixDynLibLoader();

	virtual bool DLOpen();
	virtual bool DLClose();
	virtual long DLHandle();
	virtual String DLError();

protected:
	void *fHandle;
};

#endif

//---- AppBooter ----------------------------------------------------------------
AppBooter::AppBooter()	{ }
AppBooter::~AppBooter()	{ }

void AppBooter::HandleArgs(int argc, char *argv[], Anything &args )
{
	for (long i = 0; i < argc; ++i) {
		URLUtils::Pair(argv[i], '=', args);
	}
}

void AppBooter::SetSignalMask()
{
	StartTrace(AppBooter.SetSignalMask);
#if !defined(WIN32)
	// Block all signals for this and all subsequently created threads
	// (signal set is inherited to child threads)
	// -> InterruptHandler-Thread will then use sigwait to trap the ones needed
	sigset_t set;
	sigfillset(&set);
	THRSETSIGMASK(SIG_BLOCK, &set, NULL);
#endif
}

int AppBooter::Run(int argc, char *argv[], bool doHalt)
{
	StartTrace(AppBooter.Run);
	// this is the main routine for the Application
	// framework. It shouldn't be necessary for clients
	// to patch that code
	long retVal(-1);	// the return value of this process
	Anything config;	// the global configuration anything

	// Boot resets config so HandleArgs here
	HandleArgs(argc, argv, config ); // evaluate commandline arguments
	if (Boot(config)) {	// proceed only if you can load the config file
		// load the application instance by name
		// configured in the global config file
		String applicationName;

		// this is *the* application of this system
		Application *application = FindApplication(config, applicationName);
		Trace("Using application<" << applicationName << ">");

		if (application) {
			// initialization sequence
			// let the application initialize the ressources
			// necessary for its operation
			if ( (retVal = application->GlobalInit(argc, argv, config)) == 0 ) {
				// let the application do the main loop
				retVal = application->GlobalRun();

				// termination sequence
				// let the application terminate its ressources
				retVal = application->GlobalTerminate(retVal);
			} else {
				// terminate processing if something goes wrong
				// with the initialization
				// and give the app the chance to terminate decently.
				application->GlobalTerminate(retVal);
				String logMsg("Initialization failed: ");
				logMsg << retVal;
				SysLog::Error(logMsg);
			}
			application = 0;
		} else {
			String logMsg("Configured Application Name (");
			logMsg << applicationName << ") not found in Registry.";
			SysLog::Error(logMsg);
			retVal = -2;
		}
		if (doHalt) {
			// cannot do it twice so disable it for tests
			Halt(config);
		}
	} else {
		SysLog::Error("Can't load initial configuration! exiting...");
	}
	return retVal;
}

String AppBooter::PrepareBootFileLoading(const ROAnything &roconfig)
{
	StartTrace(AppBooter.PrepareBootFileLoading);

	System::InitPath(
		roconfig["WD_ROOT"].AsCharPtr(0),
		roconfig["WD_PATH"].AsCharPtr(0)
	);

	return roconfig["WD_BOOTFILE"].AsCharPtr("Config");
}

void AppBooter::MergeConfigWithArgs(Anything &config, const Anything &args)
{
	StartTrace(AppBooter.MergeConfigWithArgs);
	TraceAny(args, "args:");
	SubTraceAny(TraceConfig, config, "config:");

	// fill in command line arguments into the applications configuration
	for (long i = 0; i < args.GetSize(); ++i) {
		String slot = args.SlotName(i);
		if ( slot.Length() ) {
			// store params at top level to allow overriding of Config.any settings
			// skip programname at top level
			if (i > 0) {
				config[slot] = args[i];
			}
			// also add information in subslot Arguments to allow direct access to arg[x]
			config["Arguments"][slot] = args[i];
		} else {
			// store params at top level to allow overriding of Config.any settings
			// skip programname at top level
			if (i > 0) {
				config.Append(args[i]);
			}
			// also add information in subslot Arguments to allow direct access to arg[x]
			config["Arguments"].Append(args[i]);
		}
	}
	SubTraceAny(TraceConfig, config, "resulting config:");
}

bool AppBooter::Boot(Anything &args) // access the intial config file
{
	StartTrace(AppBooter.Boot);

	int numberOfCpus = 1;							// a reasonable assumption
#if !defined(WIN32)
	// POSIX call
	if ((numberOfCpus = sysconf(_SC_NPROCESSORS_ONLN)) != -1)
#endif
	{
		THRSETCONCURRENCY(numberOfCpus);			// macro for sun os
	}
	//--- setting default environment
	MetaThing config;
	bool ret = ReadFromFile(config, PrepareBootFileLoading(args));
	if (ret) {
		// block signals caught by interrupt handlers
		SetSignalMask();

		// fill in command line arguments into the applications configuration
		MergeConfigWithArgs(config, args);

		// beware: this assignment has to be made after copying args into config
		// otherwise if args == fgConfig we keep appending endlessly
		Application::InitializeGlobalConfig(config);//Application::fgConfig= config;

		// initialize syslog channel
		SysLog::Init(config["AppId"].AsCharPtr("Coast"));
		// log first event
		SysLog::Info("started");
		OStringStream os;
		os <<  setw(20) << "" << " I will be using " << numberOfCpus << " cpus !\n";
		os.flush();
		SysLog::WriteToStderr(os.str());

		// load the shared objects defined in the config file
		// those are the client parts not known at link time of this
		// executable
		if (!OpenLibs(config)) {
			return false;
		}

		// add modules configured in the Config file
		if ( WDModule::Install(config) != 0 ) {
			return false;
		}
		args = config;
	}
	SubTraceAny(TraceConfig, args, "Config");
	return ret;
}

void AppBooter::Halt(const Anything &config)
{
	StartTrace(AppBooter.Halt);

	// terminate according to modules list
	WDModule::Terminate(config);

	CloseLibs();
}

bool AppBooter::ReadFromFile(Anything &config, const char *filename)
{
	StartTrace1(AppBooter.ReadFromFile, "filename<" << NotNull(filename) << ">");
	istream *ifp = System::OpenStream(filename, "any");
	if (ifp == 0) {
		String logMsg;
		SysLog::Error(logMsg << "AppBooter::ReadFromFile: can't open file " << NotNull(filename) << ".any. Are WD_ROOT/WD_PATH correctly set?");
		return false;
	}
	config.Import(*ifp);
	delete ifp;

	return true;
}

bool AppBooter::OpenLibs(const Anything &config)
{
	StartTrace(AppBooter.OpenLibs);
	Anything libConfig;
	if (config.LookupPath(libConfig, "DLL")) {
		long sz = libConfig.GetSize();

		for (long i = 0; i < sz; ++i) {
			const char *dllName = libConfig[i].AsCharPtr(0);
			if ( dllName ) {
				Trace("opening DLL [" << dllName << "]");
				Sys(DynLibLoader) dllLoader(dllName);
				if (!dllLoader.DLOpen()) {
					String msg(dllName);
					msg << ": " << dllLoader.DLError();
					SysLog::Alert(msg);
					return false;
				} else {
					String msg("successfully opened DLL: ");
					msg << dllName;
					SysLog::Info(msg);
					fLibArray[dllName] = dllLoader.DLHandle();
				}
			}
		}
	}
	return true;
}

bool AppBooter::CloseLibs()
{
	StartTrace(AppBooter.CloseLibs);
	long sz = fLibArray.GetSize();
	TraceAny(fLibArray, "libraries");
	bool ret = true;
	for (long i = sz - 1; i >= 0; --i) {
		Trace("closing down [" << fLibArray.SlotName(i) << "]");
		Sys(DynLibLoader) dllLoader(fLibArray.SlotName(i), fLibArray[i].AsLong(0));
		if ( !dllLoader.DLClose() ) {
			// PS: we've got an error
			String msg("failed to close DLL: ");
			msg << fLibArray.SlotName(i) << " " << dllLoader.DLError();
			SysLog::Error(msg);
			SysLog::WriteToStderr(msg << "\n");
			ret = false;
		} else {
			String msg("closed DLL: ");
			msg << NotNull(fLibArray.SlotName(i));
			SysLog::Info(msg);
			SysLog::WriteToStderr(msg << "\n");
		}
	}
	return ret;
}

Application *AppBooter::FindApplication(ROAnything config, String &applicationName)
{
	StartTrace(AppBooter.FindApplication);
	Trace("application:" << applicationName);
	SubTraceAny(TraceConfig, config, "config");
	ROAnything applicationConf;
	Application *application = 0;

	if (config.LookupPath(applicationConf, "Application") || config.LookupPath(applicationConf, "Server")  ) {
		for (long i = 0, sz = applicationConf.GetSize() && !application; i < sz; ++i) {
			// iterate over the applicationname list
			applicationName = applicationConf[i].AsCharPtr(0);
			if ( applicationName.Length() > 0 ) {
				Trace("trying application " << applicationName);
				// return the first application object found by name
				application = Application::FindApplication(applicationName);
			}
		}
	} else {
		Trace("no Application or Server configured -- using Registry");
		// if no application object is configured in the config any
		// return the first in the list
		RegistryIterator ri(Registry::GetRegistry("Application"), false);
		for ( ; ri.HasMore() && !application ; application = SafeCast(ri.Next(applicationName), Application));
	}
	Trace("found application " << applicationName << " at " << long(application));
	return application;
}

//--- dynamic library loading ---
DynLibLoader::DynLibLoader(const char *name)
	:	fName(name)
{
	// remove platform specific extensions
	long lStart = fName.Contains(".so");
	if (lStart > 0) {
		fName = fName.SubString(0, lStart);
	}

	// prepend lib if not already there
	lStart = fName.Contains("lib");
	if (lStart !=  0) {
		String tmpName("lib");
		tmpName << fName;
		fName = tmpName;
	}
}

DynLibLoader::~DynLibLoader()
{ }

#if defined(WIN32)

Win32DynLibLoader::Win32DynLibLoader(const char *name)
	: DynLibLoader(name), fHandle(0)
{
	// append platform specific extension
	fName.Append(".dll");
}

Win32DynLibLoader::Win32DynLibLoader(const char *name, long handle)
	: DynLibLoader(name), fHandle((HINSTANCE)handle)
{
	// append platform specific extension
	fName.Append(".dll");
}

Win32DynLibLoader::~Win32DynLibLoader()
{
}

bool Win32DynLibLoader::DLOpen()
{
	fHandle = LoadLibraryEx(fName, NULL, 0);
	return (fHandle != NULL);
}

bool Win32DynLibLoader::DLClose()
{
	return (FreeLibrary(fHandle) != 0);
}

String Win32DynLibLoader::DLError()
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
	);
	// Display the string.
	String msg(fName);
	msg  << " " << LPCTSTR(lpMsgBuf);
	// Free the buffer.
	LocalFree( lpMsgBuf );
	return msg;
}

long Win32DynLibLoader::DLHandle()
{
	return (long)fHandle;
}

#else

UnixDynLibLoader::UnixDynLibLoader(const char *name)
	: DynLibLoader(name), fHandle(0)
{
	// append platform specific extension
	fName.Append(".so");
}

UnixDynLibLoader::UnixDynLibLoader(const char *name, long handle)
	: DynLibLoader(name), fHandle((void *)handle)
{
	// append platform specific extension
	fName.Append(".so");
}

UnixDynLibLoader::~UnixDynLibLoader()
{
}

bool UnixDynLibLoader::DLOpen()
{
	fHandle = dlopen(fName, RTLD_NOW | RTLD_GLOBAL );
	return (fHandle != 0);
}

bool UnixDynLibLoader::DLClose()
{
	if (fHandle) {
		int ret = dlclose(fHandle);
		if ( ret ) {
			SysLog::WriteToStderr(String(dlerror()) << "\n");
		}
		return (ret == 0);
	}
	return false;
}

long UnixDynLibLoader::DLHandle()
{
	return (long)fHandle;
}

String UnixDynLibLoader::DLError()
{
	StartTrace(UnixDynLibLoader.DLError);
	String errorMsg(dlerror());
	Trace("DLError<" << errorMsg);
	return errorMsg;
}

#endif
