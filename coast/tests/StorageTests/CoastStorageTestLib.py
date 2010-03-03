import SConsider
from stat import *

packagename = SConsider.getPackageName( __name__ )

buildSettings = {
    packagename : {
        'includeSubdir'    : '',
        'linkDependencies' : ['CoastMTFoundation', 'loki.lokiObjects', 'testfwFoundation'],
        'sourceFiles'      : SConsider.listFiles( ['*.cpp'] ),
        'targetType'       : 'ProgramTest',
        'copyFiles'        : [(SConsider.findFiles(['config'], ['.any']), S_IRUSR | S_IRGRP | S_IROTH)],
        'public' : {
        }
    }
}

SConsider.createTargets( packagename, buildSettings )