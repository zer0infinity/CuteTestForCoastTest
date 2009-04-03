import os,platform

def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['testfw'])

def exists(env):
    return true
