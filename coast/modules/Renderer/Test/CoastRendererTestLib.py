import os, pdb
import SConsider
from stat import *

packagename = SConsider.getPackageName( __name__ )

def setUp( target, source, env ):
	pass

def tearDown( target, source, env ):
	pass

buildSettings = {
    packagename : {
        'targetType'       : 'ProgramTest',
        'linkDependencies' : ['CoastRenderers', 'testfwWDBase'],
        'requires'         : [],
		'includeSubdir'    : '',
        'sourceFiles'      : SConsider.listFiles( ['*.cpp'] ),
        'copyFiles'        : [( SConsider.findFiles( ['.'], ['.any', '.txt', '.tst', '.html'] ), S_IRUSR | S_IRGRP | S_IROTH ),
                              ( SConsider.findFiles( ['config'], ['.sh'] ), S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR )],
        'runConfig'        : {
#			'setUp': setUp,
#			'tearDown': tearDown,
		},
    }
}

SConsider.createTargets( packagename, buildSettings )

