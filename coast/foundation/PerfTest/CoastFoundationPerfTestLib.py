import os, pdb
import StanfordUtils
from stat import *

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
    packagename : {
        'linkDependencies' : ['CoastFoundation', 'testfwFoundation'],
        'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
        'targetType'       : 'ProgramTest',
        'copyFiles'        : [(StanfordUtils.findFiles(['.'],['.any']), S_IRUSR|S_IRGRP|S_IROTH)],
    }
}

StanfordUtils.createTargets(packagename, buildSettings)
