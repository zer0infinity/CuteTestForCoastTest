import os,platform,SCons,glob,re,atexit,sys,traceback,commands

from SCons.Script import *

# SconsBuilder may work with earlier version,
# but it was build and tested against SCons 1.0.0
SCons.Script.EnsureSConsVersion(1,0,0)
# SconsBuilder may work with earlier version,
# but it was build and tested against Python 2.4
SCons.Script.EnsurePythonVersion(2,4)

baseEnv=Environment()

#baseEnv.Tool('generateScript')
baseEnv.Alias("NoTarget")
baseEnv.SourceCode(".", None)

variant = "Unknown"
if baseEnv['PLATFORM'] == "posix":
    #variant = platform.dist()[0]+platform.dist()[1]+"-"+platform.machine()+"-"+platform.architecture()[0]
    variant = platform.system()+"_"+platform.libc_ver()[0]+"-"+platform.libc_ver()[1]+"_"+platform.machine()+"-"+platform.architecture()[0]
if baseEnv['PLATFORM'] == "darwin":
    version = commands.getoutput("sw_vers -productVersion")
    cpu = commands.getoutput("arch")
    if version.startswith("10.5"):
        variant="leopard-"
    if version.startswith("10.4"):
        variant="tiger-"
    variant+=cpu+"-"
    if cpu.endswith("64"):
        variant+="64bit"
    else:
        variant+="32bit"
if baseEnv['PLATFORM'] == "win32":
    variant = platform.release()+"-"+"i386"+"-"+platform.architecture()[0]

objdir="."+variant
pbasepath=os.path.abspath(os.getcwd())

print "compilation variant [",variant,"]"
print "object output variant [",objdir,"]"
print "object base output dir [",pbasepath,"]"

#print "platform.dist:",platform.dist()
#print "platform.arch:",platform.architecture()
#print "platform.machine:",platform.machine()
#print "platform.libc:",platform.libc_ver()
#print "platform.release:",platform.release()
#print "platform.version:",platform.version()
#print "platform.proc:",platform.processor()
#print "platform.system:",platform.system()

AddOption('--supersede', dest='supersede', action='store', nargs=1, type='string', default='.', metavar='DIR', help='Directory containing packages superseding installed ones. Relative paths not supported!')
override = baseEnv.GetOption('supersede')

baseEnv.VariantDir(objdir,'.',duplicate=1)

#########################
#  Project Environment  #
#########################
baseEnv.Append(LIBDIR        = Dir(override).Dir('lib').Dir(variant))
baseEnv.Append(BINDIR        = Dir(override).Dir('exe').Dir(variant))
baseEnv.Append(SCRIPTDIR     = Dir(override).Dir('bin').Dir(variant))
baseEnv.Append(INCDIR        = Dir(override).Dir('include'))
baseEnv.Append(PFILESDIR     = Dir(override).Dir('syspfiles'))
baseEnv.Append(DATADIR       = Dir(override).Dir('data'))
baseEnv.Append(XMLDIR        = Dir(override).Dir('xml'))
baseEnv.Append(TOOLDIR       = Dir(override).Dir('sconsTools'))
baseEnv.Append(TESTDIR       = baseEnv['BINDIR'])
baseEnv.Append(TESTSCRIPTDIR = baseEnv['SCRIPTDIR'])

Export("baseEnv")

#########################
#  External Libraries   #
#########################
SConscript('externals.scons')

def listFiles(files, **kw):
    allFiles = []
    for file in files:
        globFiles = Glob(file)
        newFiles = []
        for globFile in globFiles:
            if 'recursive' in kw and kw.get('recursive') and os.path.isdir(globFile.srcnode().abspath) and os.path.basename(globFile.srcnode().abspath) != 'CVS':
                allFiles+=listFiles([str(Dir('.').srcnode().rel_path(globFile.srcnode()))+"/*"], recursive = True)
            if os.path.isfile(globFile.srcnode().abspath):
                allFiles.append(globFile)
    return allFiles

Export('listFiles')

def print_build_failures():
    from SCons.Script import GetBuildFailures
    if GetBuildFailures():
        print "scons: printing failed nodes"
        for bf in GetBuildFailures():
            if str(bf.action) != "installFunc(target, source, env)":
                print bf.node
        print "scons: done printing failed nodes"

atexit.register(print_build_failures)
