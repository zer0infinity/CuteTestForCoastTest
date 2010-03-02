import os
import SConsider
from stat import *

packagename = SConsider.getPackageName(__name__)

buildSettings = {
    packagename : {
        'targetType'       : 'LibraryShared',
        'sourceFiles'      : SConsider.listFiles(['*.cpp']),
        'includeSubdir'    : '',
        'linkDependencies' : ['testfwWDBase', 'CoastPerfTest'],
    },
    'Runner' : {
        'targetType'       : 'AppTest',
        'requires'         : ['CoastStdDataAccess', 'CoastActions', 'CoastRenderers', packagename + '.' + packagename],
        'usedTarget'       : 'wdapp.wdapp',
        'copyFiles'        : [(SConsider.findFiles(['.'],['.txt', '.html', '.any']), S_IRUSR|S_IRGRP|S_IROTH)],
    },
}

SConsider.createTargets(packagename, buildSettings)
