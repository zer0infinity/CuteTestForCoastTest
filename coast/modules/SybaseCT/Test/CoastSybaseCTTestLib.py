import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 packagename : {
                     'targetType'       : 'ProgramTest',
                     'linkDependencies' : ['CoastSybaseCT', 'testfwWDBase'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'configFiles'      : StanfordUtils.listFiles(['config/*.any',
                                                                   'config/*.txt',
                                                                   'config/*.tst',
                                                                   'config/testscript.sh',
                                                                   'config/prjconfig.sh',
                                                                   'tmp/*.any']),
                     },
                 }

StanfordUtils.createTargets(packagename, buildSettings)
