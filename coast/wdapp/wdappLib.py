import os, platform, pdb, traceback, string
import SCons
import StanfordUtils
from SCons.Script import Dir

_added = None
_packagename = None
_includeSubdir = ''

def generate(env, **kw):
    global _added, _packagename, _includeSubdir
    if not _added:
        _added = 1
        _packagename = string.replace(__name__, 'Lib', '')

    # this libraries dependencies
    StanfordUtils.DependsOn(env, 'CoastWDBase')

    # flags / settings used by this library and users of it
    # export library dependency when we are not building ourselves

def exists(env):
    return true
