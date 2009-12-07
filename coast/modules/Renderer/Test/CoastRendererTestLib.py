import os, pdb
import StanfordUtils
from stat import *

packagename = StanfordUtils.getPackageName( __name__ )

def setUp( target, source, env ):
	pass

def tearDown( target, source, env ):
	pass

buildSettings = {
    packagename : {
        'linkDependencies' : ['CoastRenderers', 'testfwWDBase'],
		'includeSubdir'    : 'src',
        'sourceFiles'      : StanfordUtils.listFiles( ['src/*.cpp'] ),
        'targetType'       : 'ProgramTest',
        'copyFiles'        : [( StanfordUtils.findFiles( ['.'], ['.any', '.txt', '.tst', '.html'] ), S_IRUSR | S_IRGRP | S_IROTH ),
                              ( StanfordUtils.findFiles( ['config'], ['.sh'] ), S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR )],
        'runConfig'        : {
#			'setUp': setUp,
#			'tearDown': tearDown,
		},
    }
}

StanfordUtils.createTargets( packagename, buildSettings )

