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

basepath = ''

def SystemTestModifyConfig( env, cfgfilename ):
    global basepath
    basepath = os.path.join( os.path.join( env['BASEOUTDIR'].Dir( env['RELTARGETDIR'] ).abspath, 'makedirextendtest' ), 'test' )
    tdirabs = env['BASEOUTDIR'].Dir( env['RELTARGETDIR'] ).abspath
    fname = os.path.join( tdirabs, cfgfilename )
    ## set writable cfgfilename
    if os.path.isfile( fname ):
        os.chmod( fname, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH )
        ## replace some token in Tests configuration file to store number of existing directories so far
        ## the number of existing dirs is dirnumber + 2 ('.' and '..')
        StanfordUtils.replaceRegexInFile( fname, "##DIREXTENDBASEDIR##", basepath )
        if str( env['PLATFORM'] ) == 'cygwin':
            StanfordUtils.replaceRegexInFile( fname, "##TMPDIR##", 'c:/temp' )
            StanfordUtils.replaceRegexInFile( fname, "##ROOTFS##", 'c:/' )
        else:
            StanfordUtils.replaceRegexInFile( fname, "##TMPDIR##", '/tmp' )
            StanfordUtils.replaceRegexInFile( fname, "##ROOTFS##", '/' )

def SystemTestMakeDirectoryExtendCleanup():
    global basepath
    for p in [basepath + '_ex0', basepath, os.path.split( basepath )[0]]:
        if os.path.isdir( p ):
            try:
                os.rmdir( p )
            except: pass

def setUp( target, source, env ):
    SystemTestModifyConfig( env, 'config/SystemTest.any' )
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
