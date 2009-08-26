import os
import StanfordUtils
from stat import *

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
    packagename : {
        'targetType'       : 'LibraryShared',
        'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
        'includeSubdir'    : '',
        'linkDependencies' : ['testfwWDBase', 'CoastPerfTest'],
    },
    'PerfTestRunner' : {
        'targetType'       : 'AppTest',
        'requires'         : ['CoastStdDataAccess', 'CoastActions', 'CoastRenderers', packagename + '.' + packagename],
        'usedTarget'       : 'wdapp.wdapp',
        'copyFiles'        : [(StanfordUtils.findFiles(['.'],['.txt', '.html', '.any']), S_IRUSR|S_IRGRP|S_IROTH)],
    },
}

StanfordUtils.createTargets(packagename, buildSettings)
