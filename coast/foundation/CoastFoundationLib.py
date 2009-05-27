import os, platform, pdb, traceback, string
import SCons
import StanfordUtils
from SCons.Script import Dir

_packagename = string.replace(__name__, 'Lib', '')
_target = None
_includeSubdir = ''
_libDepends = ['loki', 'boost']

def depends(env, modules):
    for mod in modules:
        StanfordUtils.DependsOn(env, mod)

def generate(env, **kw):
    global _target, _packagename, _includeSubdir, _libDepends

    # this libraries dependencies
    depends(env, _libDepends)

    env.Tool('addLibrary', library=[_packagename])

    # flags / settings used by this library and users of it
#    env.AppendUnique(CPPDEFINES =['SOMEFLAG'])

    #FIXME: unfortunately this is copy wasted from registerObjects.py because I was not able to get values out of a tool
    installPath = env['BASEOUTDIR'].Dir(os.path.join(env['INCDIR'], _packagename))
    if not _includeSubdir == '':
        installPath = installPath.Dir(_includeSubdir)
    env.AppendUnique(CPPPATH=[installPath])

    return _target

def exists(env):
    return true

libEnv = StanfordUtils.CloneBaseEnv()
depends(libEnv, _libDepends)

# win32 specific define to export all symbols when creating a DLL
libEnv.AppendUnique(CPPDEFINES=[_packagename.upper() + '_IMPL'])

# specify public headers here

# maybe we need to add this libraries local include path when building it (if different from .)
if not _includeSubdir == '':
    libEnv.AppendUnique(CPPPATH=[Dir(_includeSubdir)])

_target = libEnv.SharedLibrary(_packagename, StanfordUtils.listFiles(['*.cpp']))

progEnv = StanfordUtils.CloneBaseEnv()
progEnv.Tool('registerObjects',
             package=_packagename,
             libraries=[_target],
             includes=StanfordUtils.listFiles([os.path.join(_includeSubdir, '*.h')]))
