import os, socket, pdb
import StanfordUtils
import SocketHelpers
from stat import *

packagename = StanfordUtils.getPackageName( __name__ )
acceptorThreads = []

def acceptorCallback( sock, addr ):
    data = SocketHelpers.sockrecv( sock )
    sock.send( '' )
    sock.shutdown( socket.SHUT_RDWR )
    sock.close()

def httpReplyCallback( sock, addr ):
    data = SocketHelpers.sockrecv( sock )
    sock.send( "HTTP/1.1 200 OK\n\n" )
    sock.shutdown( socket.SHUT_RDWR )
    sock.close()

def noSendRecvCallback( sock, addr ):
    sock.shutdown( socket.SHUT_RDWR )
    sock.close()

dirnumber = 1
basepath = '/tmp/dirextension/test'
relpath = 'pythondir_'

def SystemTestMakeDirectoryExtendSetup( env, cfgfilename ):
    global dirnumber, basepath, relpath
    if not os.path.exists( basepath ):
        os.makedirs( basepath )
    bdir = os.path.join( basepath, relpath )
    try:
        while True:
            if not os.path.isdir( bdir + str( dirnumber ) ):
                os.mkdir( bdir + str( dirnumber ) )
            dirnumber += 1
            if dirnumber % 1000 == 0:
                print "directory", bdir + str( dirnumber ), "created..."
    except:
        print "directory", bdir + str( dirnumber ), "could not be created anymore"

    dirnumber -= 1
    tdirabs = env['BASEOUTDIR'].Dir( env['RELTARGETDIR'] ).abspath
    fname = os.path.join( tdirabs, cfgfilename )
    ## set writable cfgfilename
    if os.path.isfile( fname ):
        os.chmod( fname, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH )
        ## replace some token in Tests configuration file to store number of existing directories so far
        ## the number of existing dirs is dirnumber + 2 ('.' and '..')
        StanfordUtils.replaceRegexInFile( fname, "##NUMHARDLINKS##", '/NumLinks ' + str( dirnumber + 2 ) )

def SystemTestMakeDirectoryExtendCleanup():
    global dirnumber, basepath, relpath
    bdir = os.path.join( basepath, relpath )
    try:
        while dirnumber >= 0:
            if os.path.isdir( bdir + str( dirnumber ) ):
                os.rmdir( bdir + str( dirnumber ) )
            dirnumber -= 1
            if dirnumber % 1000 == 0:
                print "directory", bdir + str( dirnumber ), "deleted..."
    except:
        print "directory", bdir + str( dirnumber ), "could not be deleted"
    for p in [basepath + '_ex0', basepath, os.path.split( basepath )[0]]:
        if os.path.isdir( p ):
            try:
                os.rmdir( p )
            except: pass

def setUp( target, source, env ):
    SystemTestMakeDirectoryExtendSetup( env, 'config/SystemTest.any' )
    # setup listener threads for socket tests
    global acceptorThreads
    acceptorThreads.append( SocketHelpers.AcceptorThread( addr = '', port = 9876, callback = acceptorCallback ) )
    acceptorThreads.append( SocketHelpers.AcceptorThread( addr = '', port = 9875, callback = httpReplyCallback ) )
    acceptorThreads.append( SocketHelpers.AcceptorThread( addr = '', port = 9874, callback = noSendRecvCallback ) )
    for t in acceptorThreads:
        t.start()

def tearDown( target, source, env ):
    global acceptorThreads
    for t in acceptorThreads:
        if t.isAlive():
            t.terminate()
            t.join()
    # delete generated files
    StanfordUtils.removeFiles( StanfordUtils.findFiles( [env['BASEOUTDIR'].Dir( env['RELTARGETDIR'] )], extensions = ['.res', '.tst', '.txt' ], matchfiles = ['include.any'] ) )
    SystemTestMakeDirectoryExtendCleanup()

buildSettings = {
    packagename : {
        'linkDependencies' : ['CoastFoundation', 'testfwFoundation'],
        'sourceFiles'      : StanfordUtils.listFiles( ['*.cpp'] ),
        'targetType'       : 'ProgramTest',
        'copyFiles'        : [( StanfordUtils.findFiles( ['.'], ['.any', '.txt', '.tst'] ), S_IRUSR | S_IRGRP | S_IROTH ),
                              ( StanfordUtils.findFiles( ['config'], ['.sh'] ), S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR )],
        'runConfig'        : {
                              'setUp': setUp,
                              'tearDown': tearDown,
                             },
    }
}

StanfordUtils.createTargets( packagename, buildSettings )
