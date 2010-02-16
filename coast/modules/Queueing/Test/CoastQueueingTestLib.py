import os, pdb
import SConsider
from stat import *

packagename = SConsider.getPackageName( __name__ )

buildSettings = {
    packagename : {
        'includeSubdir'    : '',
        'linkDependencies' : ['CoastQueueing', 'testfwFoundation'],
        'requires'         : ['CoastRenderers', 'CoastAppLog'],
        'sourceFiles'      : SConsider.listFiles( ['*.cpp'] ),
        'targetType'       : 'ProgramTest',
        'copyFiles'        : [(SConsider.findFiles(['config'], ['.any']), S_IRUSR | S_IRGRP | S_IROTH)],
        'public' : {
        }
    }
}

SConsider.createTargets( packagename, buildSettings )
