from __future__ import with_statement
import os, subprocess
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

def runSQL(connectstring, filename):
    targets = StanfordUtils.programLookup.getPackageTarget('oracle', 'sqlplus')

    if not targets['target'] or not os.path.isfile(filename):
        return False

    args = [targets['target'].abspath, connectstring]
    res = 0
    with open(filename) as file:
        res = subprocess.call(args, stdin=file)
    return res

def setUp(target, source, env):
    path = StanfordUtils.programLookup.getPackageDir(packagename).Dir('config')
    runSQL("oratest/oratest@sifs-coast1.hsr.ch:1521/orcl", path.File('drop_oratest_schema.sql').abspath)
    runSQL("oratest/oratest@sifs-coast1.hsr.ch:1521/orcl", path.File('oratest_schema.sql').abspath)

def tearDown(target, source, env):
    path = StanfordUtils.programLookup.getPackageDir(packagename).Dir('config')
    runSQL("oratest/oratest@sifs-coast1.hsr.ch:1521/orcl", path.File('drop_oratest_schema.sql').abspath)

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
