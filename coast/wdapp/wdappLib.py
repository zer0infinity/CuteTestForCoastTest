import os, platform, pdb, traceback, string
import SCons
import StanfordUtils
from SCons.Script import Dir

_packagename = string.replace(__name__, 'Lib', '')
_target = None

def generate(env, **kw):
    global _target, _packagename

    # this libraries dependencies
    StanfordUtils.DependsOn(env, 'CoastWDBase')

    # flags / settings used by this library and users of it
    # export library dependency when we are not building ourselves
    return _target

def exists(env):
    return true

progEnv = StanfordUtils.CloneBaseEnv()
StanfordUtils.DependsOn(progEnv, _packagename)

_target = progEnv.Program(_packagename, StanfordUtils.listFiles(['Main.cpp']))
