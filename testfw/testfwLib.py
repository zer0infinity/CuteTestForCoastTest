import os, platform, pdb, traceback, string
import SCons
import SomeUtils
from SomeUtils import *
from SCons.Script import Dir

_added = None
_packagename = None
_includeSubdir = ''

def generate(env, **kw):
    global _added, _packagename, _publicIncludeSet
    if not _added:
        _added = 1
        _packagename = string.replace(__name__, 'Lib', '')

    # this libraries dependencies
    # flags / settings used by this library and users of it
    # export library dependency when we are not building ourselves
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library=['testfw'])
        #FIXME: unfortunately this is copy wasted from registerObjects.py because I was not able to
        installPath = env['INCDIR'].Dir(_packagename)
        if not _includeSubdir == '':
            installPath = installPath.Dir(_includeSubdir)
        env.AppendUnique(CPPPATH=[installPath])
    else:
        # specify public headers here
        env.Tool('registerObjects', package=_packagename, includes=listFiles([os.path.join(_includeSubdir, '*.h')]))
        # maybe we need to add this libraries local include path when building it (if different from .)
        if not _includeSubdir == '':
            env.AppendUnique(CPPPATH=[Dir(_includeSubdir)])

def exists(env):
    return true
