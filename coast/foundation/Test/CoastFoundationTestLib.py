import os, socket
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

def setUp( target, source, env ):
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
    filesToDelete = StanfordUtils.listFiles( ['include.any', 'config/PipeExecutorTest.txt', 'tmp/*.res', 'tmp/*.tst', 'tmp/*.txt', 'testout2.txt' ] )
    for fname in filesToDelete:
        os.unlink( fname )

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
