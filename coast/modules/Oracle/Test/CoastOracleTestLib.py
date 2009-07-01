import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 packagename : {
                     'targetType'       : 'ProgramTest',
                     'linkDependencies' : ['CoastOracle', 'testfwWDBase'],
                     'requires'         : ['CoastAppLog', 'CoastRenderers'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'configFiles'      : StanfordUtils.listFiles(['config/*.any',
                                                                   'config/*.txt',
                                                                   'config/*.tst',
                                                                   'config/*.sql',
                                                                   'tmp/*.any']),
                     },
                 }

StanfordUtils.createTargets(packagename, buildSettings)
