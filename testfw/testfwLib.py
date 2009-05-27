import os, pdb
import StanfordUtils

_packagename = StanfordUtils.getPackageName(__name__)
_target = None
_includeSubdir = ''
_libDepends = []

def generate(env, **kw):
    global _target, _packagename, _includeSubdir, _libDepends

    # this libraries dependencies
    StanfordUtils.setModuleDependencies(env, _libDepends)

    if _target:
        env.Tool('addLibrary', library=[_packagename])

    # flags / settings used by this library and users of it
#    env.AppendUnique(CPPDEFINES =['SOMEFLAG'])

    # specify public headers here
    StanfordUtils.setIncludePath(env, _packagename, _includeSubdir, internal=False)

    return _target

# create environment for target
targetEnv = StanfordUtils.CloneBaseEnv()

# update environment by adding dependencies to used modules
StanfordUtils.setModuleDependencies(targetEnv, _libDepends)

# win32 specific define to export all symbols when creating a DLL
##targetEnv.AppendUnique(CPPDEFINES=[_packagename.upper() + '_IMPL'])

# this lib should get lazy linked
targetEnv['_NONLAZYLINKFLAGS'] = ''

# maybe we need to add this libraries local include path when building it (if different from .)
StanfordUtils.setIncludePath(targetEnv, _packagename, _includeSubdir)

# specify this modules target
_target = targetEnv.SharedLibrary(_packagename, StanfordUtils.listFiles(['*.cpp']))

# either create a new environment to register this package with or use targetEnv when no real target gets created
registerEnv = StanfordUtils.CloneBaseEnv()
registerEnv.Tool('registerObjects',
             package=_packagename,
             libraries=[_target],
             includes=StanfordUtils.listFiles([os.path.join(_includeSubdir, '*.h')])
             )
