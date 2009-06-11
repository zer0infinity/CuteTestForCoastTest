import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 'PerfTestTest' : {
                     'targetType'       : 'LibraryShared',
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'includeSubdir'    : '',
                     'linkDependencies' : ['testfwWDBase', 'CoastPerfTest'],
#                     'appendUnique'     : { 'CPPDEFINES' : 'fooX' },
                    },
                 'PerfTestRunner' : {
                     'targetType'       : 'AppTest',
                     'requires'         : ['CoastStdDataAccess', 'CoastActions', 'CoastRenderers', 'PerfTestTest.PerfTestTest'],
                     'usedTarget'       : 'wdapp.wdapp',
                     'configFiles'      : StanfordUtils.listFiles(['*.txt',
                                                                   '*.html',
                                                                   '*.any']),
                     },
                 }

StanfordUtils.createTargets(packagename, buildSettings)
