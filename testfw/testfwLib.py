import os,platform

def generate(env, **kw):
    # this libraries dependencies
    # flags / settings used by this library and users of it
    # export library dependency when we are not building ourselves
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['testfw'])
    # where our public headers can be found => must match registered package
    env.AppendUnique(CPPPATH = ['#include/coast_testfw'])

def exists(env):
    return true
