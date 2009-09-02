import os, pdb
import StanfordUtils
from stat import *

packagename = StanfordUtils.getPackageName( __name__ )

def setUp( target, source, env ):
    None

def tearDown( target, source, env ):
    # delete generated files
    StanfordUtils.removeFiles( StanfordUtils.findFiles( [env['BASEOUTDIR'].Dir( env['RELTARGETDIR'] )], matchfiles = ['MultCprsTest.gz', 'big.gz', 'testzip.gz', 'tt.gz', 'empty.gz', 'empty2.gz' ] ) )

buildSettings = {
    packagename : {
        'linkDependencies' : ['CoastCompress', 'testfwFoundation'],
        'sourceFiles'      : StanfordUtils.listFiles( ['*.cpp'] ),
        'targetType'       : 'ProgramTest',
        'copyFiles'        : [( StanfordUtils.findFiles( ['.'], ['.any', '.html', '.gz'] ), S_IRUSR | S_IRGRP | S_IROTH )],
        'runConfig'        : {
            'setUp'        : setUp,
            'tearDown'     : tearDown,
        },
    }
}

StanfordUtils.createTargets( packagename, buildSettings )
