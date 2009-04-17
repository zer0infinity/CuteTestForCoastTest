import os,platform,pdb,traceback
import SCons

def generate(env, **kw):
    # this libraries dependencies
    env.Tool('foundationLib')
    # flags / settings used by this library and users of it
    env.AppendUnique(CPPDEFINES =['_POSIX_THREADS'])
    env.AppendUnique(CPPDEFINES =['_POSIX_PTHREAD_SEMANTICS'])
    env.AppendUnique(CPPDEFINES =['_REENTRANT'])
    env.AppendUnique(LIBS=['pthread'])
    #FIXME: is this lib also needed when using sun-CC or only when using gcc?
    if env['PLATFORM'] == "sunos":
        env.AppendUnique(LIBS=['posix4'])
    # export library dependency when we are not building ourselves
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['mtfoundation'])
    # where our public headers can be found => must match registered package
    env.AppendUnique(CPPPATH = ['#include/coast_mtfoundation'])

def exists(env):
    return true
