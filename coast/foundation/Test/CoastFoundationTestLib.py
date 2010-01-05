import os, socket, pdb
import SocketServer, threading, BaseHTTPServer
import StanfordUtils
from stat import *

packagename = StanfordUtils.getPackageName(__name__)
basepath = ''

def SystemTestModifyConfig(env, cfgfilename):
    global basepath
    basepath = os.path.join(os.path.join(env['BASEOUTDIR'].Dir(env['RELTARGETDIR']).abspath, 'makedirextendtest'), 'test')
    tdirabs = env['BASEOUTDIR'].Dir(env['RELTARGETDIR']).abspath
    fname = os.path.join(tdirabs, cfgfilename)
    ## set writable cfgfilename
    if os.path.isfile(fname):
        os.chmod(fname, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
        ## replace some token in Tests configuration file to store number of existing directories so far
        ## the number of existing dirs is dirnumber + 2 ('.' and '..')
        searchReplace = [(r"##DIREXTENDBASEDIR##", basepath) ]
        if str(env['PLATFORM']) == 'cygwin':
            searchReplace.extend([ (r"##TMPDIR##", 'c:/temp'), (r"##ROOTFS##", 'c:/') ])
        else:
            searchReplace.extend([ (r"##TMPDIR##", '/tmp'), (r"##ROOTFS##", '/') ])
        StanfordUtils.replaceRegexInFile(fname, searchReplace)

def SystemTestMakeDirectoryExtendCleanup():
    global basepath
    for p in [basepath + '_ex0', basepath, os.path.split(basepath)[0]]:
        if os.path.isdir(p):
            try:
                os.rmdir(p)
            except: pass

class AcceptorHandler(SocketServer.StreamRequestHandler):
    def handle(self):
        data = self.rfile.readlines()
        self.request.send('')

class HTTPHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.end_headers()
        pass

class NoSendRecvHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        pass

servers = []
def startServer(host, port, server_class, handler_class):
    server = server_class((host, port), handler_class)
    servers.append(server)
    server.serve_forever()

def setUp(target, source, env):
    SystemTestModifyConfig(env, 'config/SystemTest.any')
    # setup listener threads for socket tests
    threading.Thread(target=startServer, args=['localhost',9876, SocketServer.TCPServer, AcceptorHandler]).start()
    threading.Thread(target=startServer, args=['localhost',9875, BaseHTTPServer.HTTPServer, HTTPHandler]).start()
    threading.Thread(target=startServer, args=['localhost',9874, SocketServer.TCPServer, NoSendRecvHandler]).start()

def tearDown(target, source, env):
    for server in servers:
        server.shutdown()
    
    # delete generated files
    StanfordUtils.removeFiles(StanfordUtils.findFiles([env['BASEOUTDIR'].Dir(env['RELTARGETDIR'])], extensions=['.res', '.tst', '.txt' ], matchfiles=['include.any']))
    SystemTestMakeDirectoryExtendCleanup()

buildSettings = {
    packagename : {
        'linkDependencies' : ['CoastFoundation', 'testfwFoundation'],
        'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
        'targetType'       : 'ProgramTest',
        'copyFiles'        : [(StanfordUtils.findFiles(['.'], ['.any', '.txt', '.tst']), S_IRUSR | S_IRGRP | S_IROTH),
                              (StanfordUtils.findFiles(['config'], ['.sh']), S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR)],
        'runConfig'        : {
                              'setUp': setUp,
                              'tearDown': tearDown,
                             },
    }
}

StanfordUtils.createTargets(packagename, buildSettings)
