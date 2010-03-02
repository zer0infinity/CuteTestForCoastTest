import os, pdb
import SConsider
from stat import *

packagename = SConsider.getPackageName(__name__)

buildSettings = {
    packagename : {
        'linkDependencies' : ['CoastFoundation', 'testfwFoundation'],
        'sourceFiles'      : SConsider.listFiles(['*.cpp']),
        'targetType'       : 'ProgramTest',
        'copyFiles'        : [(SConsider.findFiles(['.'],['.any']), S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR)],
    }
}

SConsider.createTargets(packagename, buildSettings)
