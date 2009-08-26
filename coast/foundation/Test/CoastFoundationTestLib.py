import os, pdb
import StanfordUtils
from stat import *

packagename = StanfordUtils.getPackageName(__name__)

def tearDown(target, source, env):
    # delete generated files
    pdb.set_trace()
    filesToDelete = StanfordUtils.listFiles(['include.any', 'config/PipeExecutorTest.txt', 'tmp/*.res', 'tmp/*.tst', 'tmp/*.txt', 'testout2.txt' ])
    for fname in filesToDelete:
        os.unlink(fname)

buildSettings = {
    packagename : {
        'linkDependencies' : ['CoastFoundation', 'testfwFoundation'],
        'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
        'targetType'       : 'ProgramTest',
        'copyFiles'        : [(StanfordUtils.findFiles(['.'],['.any','.txt','.tst']), S_IRUSR|S_IRGRP|S_IROTH),
                              (StanfordUtils.findFiles(['config'],['.sh']), S_IRUSR|S_IRGRP|S_IROTH|S_IXUSR)],
        'runConfig'        : {
                              'tearDown': tearDown,
                             },
    }
}

StanfordUtils.createTargets(packagename, buildSettings)
