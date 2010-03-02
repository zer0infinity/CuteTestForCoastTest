import os, pdb
import SConsider
from stat import *

packagename = SConsider.getPackageName( __name__ )

def setUp( target, source, env ):
    None

def tearDown( target, source, env ):
    # delete generated files
    SConsider.removeFiles( SConsider.findFiles( [env['BASEOUTDIR'].Dir( env['RELTARGETDIR'] )], matchfiles = ['MultCprsTest.gz', 'big.gz', 'testzip.gz', 'tt.gz', 'empty.gz', 'empty2.gz' ] ) )

buildSettings = {
    packagename : {
        'linkDependencies' : ['CoastCompress', 'testfwFoundation'],
        'sourceFiles'      : SConsider.listFiles( ['*.cpp'] ),
        'targetType'       : 'ProgramTest',
        'copyFiles'        : [( SConsider.findFiles( ['.'], ['.any', '.html', '.gz'] ), S_IRUSR | S_IRGRP | S_IROTH )],
        'runConfig'        : {
            'setUp'        : setUp,
            'tearDown'     : tearDown,
        },
    }
}

SConsider.createTargets( packagename, buildSettings )
