import os,platform,pdb
import SCons

def generate(env, **kw):
    env.Tool('testfwLib')
    tpath=kw.get('tpath', '')
    if tpath != '':
        pdb.set_trace()
#        tinc=SCons.Script.Dir(tpath)
#        print 'tinc [%s]' % tinc.path
    env.AppendUnique(CPPPATH = ['#include/coast_testfw'])

def exists(env):
    return true
