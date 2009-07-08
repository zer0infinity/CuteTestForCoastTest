from __future__ import with_statement
import os, subprocess, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

def runSQL(connectstring, filename, logpath):
    targets = StanfordUtils.programLookup.getPackageTarget('oracle', 'sqlplus')
    target = targets['target']
    if not target or not os.path.isfile(filename):
        return False
    args = [target.abspath, connectstring]
    res = 0
    logfilebasename = target.name + '.' + os.path.basename(filename)
    with open(filename) as file:
        proc = subprocess.Popen(args, stdin=file,
                                     stderr=subprocess.PIPE,
                                     stdout=subprocess.PIPE)
        res = proc.wait()
        perr = proc.stderr.read()
        pout = proc.stdout.read()

        if not os.path.isdir(logpath):
            os.makedirs(logpath)
        if perr:
            with open(os.path.join(logpath, logfilebasename + '.stderr'), 'w') as errfile:
                errfile.write(perr)
        if pout:
            with open(os.path.join(logpath, logfilebasename + '.stdout'), 'w') as outfile:
                outfile.write(pout)

    return res

def setUp(target, source, env):
    logpath = env['BASEOUTDIR'].Dir(os.path.join('tests', packagename, env['LOGDIR'], env['VARIANTDIR']))
    path = env['BASEOUTDIR'].Dir(os.path.join('tests', packagename)).Dir('config')
    runSQL("oratest/oratest@sifs-coast1.hsr.ch:1521/orcl", path.File('drop_oratest_schema.sql').abspath, logpath.abspath)
    runSQL("oratest/oratest@sifs-coast1.hsr.ch:1521/orcl", path.File('oratest_schema.sql').abspath, logpath.abspath)

def tearDown(target, source, env):
    logpath = env['BASEOUTDIR'].Dir(os.path.join('tests', packagename, env['LOGDIR'], env['VARIANTDIR']))
    path = env['BASEOUTDIR'].Dir(os.path.join('tests', packagename)).Dir('config')
    runSQL("oratest/oratest@sifs-coast1.hsr.ch:1521/orcl", path.File('drop_oratest_schema.sql').abspath, logpath.abspath)

buildSettings = {
                 packagename : {
                     'targetType'       : 'ProgramTest',
                     'linkDependencies' : ['CoastOracle', 'testfwWDBase'],
                     'requires'         : ['CoastAppLog', 'CoastRenderers', 'oracle.sqlplus'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'configFiles'      : StanfordUtils.listFiles(['config/*.any',
                                                                   'config/*.txt',
                                                                   'config/*.tst',
                                                                   'config/*.sql',
                                                                   'tmp/*.any']),
                     'runConfig'        : {
                                           'setUp': setUp,
                                           'tearDown': tearDown,
                                          },
                     },
                 }

StanfordUtils.createTargets(packagename, buildSettings)
