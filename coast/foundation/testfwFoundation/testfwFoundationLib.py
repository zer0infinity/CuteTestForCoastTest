import os,platform,pdb
import SCons

def generate(env, **kw):
    env.Tool('testfwLib')

def exists(env):
    return true
