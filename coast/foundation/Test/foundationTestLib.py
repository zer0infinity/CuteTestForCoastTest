import os, platform, pdb, traceback, string
import SCons
import StanfordUtils
from SCons.Script import Dir

_packagename = string.replace(__name__, 'Lib', '')
_target = None
_includeSubdir = ''
_libDepends = ['CoastFoundation', 'testfwFoundation']

def depends(env, modules):
    for mod in modules:
        StanfordUtils.DependsOn(env, mod)

def generate(env, **kw):
    global _target, _packagename, _includeSubdir, _libDepends

    # this libraries dependencies
    depends(env, _libDepends)

    # flags / settings used by this library and users of it

    return _target

def exists(env):
    return true

progEnv = StanfordUtils.CloneBaseEnv()
depends(progEnv, _libDepends)

_target = progEnv.Program(_packagename, StanfordUtils.listFiles(['*.cpp']))

progEnv.Tool('registerObjects',
             package=_packagename,
             testApps=[_target],
             config=StanfordUtils.listFiles(['config/*.any', 'config/*.txt', 'config/*.tst', 'config/testscript.sh', 'config/prjconfig.sh', 'tmp/*.any', '*.any']))
