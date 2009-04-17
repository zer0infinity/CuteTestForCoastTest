import os,platform,pdb,traceback
import SCons

def generate(env, **kw):
    # this libraries dependencies
    env.Tool('addLibrary', library = env['lokiLibs'])
#    env.Tool('addLibrary', library = env['boostLibs'])
    # flags / settings used by this library and users of it
#    env.AppendUnique(CPPDEFINES =['ONLY_STD_IOSTREAM'])
    #FIXME: is this lib also needed when using sun-CC or only when using gcc?
    if env['PLATFORM'] == "sunos":
        env.AppendUnique(LIBS=['socket'])
    # export library dependency when we are not building ourselves
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['foundation'])
    # where our public headers can be found => must match registered package
    env.AppendUnique(CPPPATH = ['#include/coast_foundation'])

def exists(env):
    return true
