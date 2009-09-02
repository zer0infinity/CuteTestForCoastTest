import os
import StanfordUtils
from stat import *

packagename = StanfordUtils.getPackageName( __name__ )

def setUp( target, source, env ):
    None

def tearDown( target, source, env ):
    # delete generated files
    None

buildSettings = {
    packagename : {
        'linkDependencies' : ['CoastMTFoundation', 'testfwFoundation'],
        'sourceFiles'      : StanfordUtils.listFiles( ['*.cpp'] ),
        'targetType'       : 'ProgramTest',
        'copyFiles'        : [( StanfordUtils.findFiles( ['config'], ['.any'] ), S_IRUSR | S_IRGRP | S_IROTH )],
        'runConfig'        : {
            'setUp'        : setUp,
            'tearDown'     : tearDown,
        },
    }
}

StanfordUtils.createTargets( packagename, buildSettings )
