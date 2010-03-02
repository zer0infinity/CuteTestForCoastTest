import os
import SConsider
from stat import *

packagename = SConsider.getPackageName(__name__)

buildSettings = {
    packagename : {
        'targetType'       : 'ProgramTest',
        'linkDependencies' : ['CoastSybaseCT', 'testfwWDBase'],
        'sourceFiles'      : SConsider.listFiles(['*.cpp']),
        'copyFiles'        : [(SConsider.findFiles(['.'],['.any','.txt','.tst','.sql']), S_IRUSR|S_IRGRP|S_IROTH)],
    },
}

SConsider.createTargets(packagename, buildSettings)
