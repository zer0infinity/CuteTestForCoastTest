import os,platform,pdb,traceback

def generate(env, **kw):
    env.Tool('addLibrary', library = env['lokiLibs'])
    #env.Tool('addLibrary', library = env['boostLibs'])
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['foundation'])

def exists(env):
    return true
