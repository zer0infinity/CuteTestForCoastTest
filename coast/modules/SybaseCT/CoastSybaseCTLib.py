import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
                 'CoastSybaseCT' : {
                     'includeSubdir'    : '',
                     'linkDependencies'       : ['sybase', 'CoastDataAccess'],
                     'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
                     'targetType'       : 'LibraryShared',
                     'appendUnique'     : { 'CPPDEFINES' : [packagename.upper() + '_IMPL'] },
                     'public' : {
                                 'includes'     : StanfordUtils.listFiles(['*.h']),
#                                'appendUnique' : { 'CPPDEFINES' : 'fooX' },
                    }
                 }
                }

StanfordUtils.createTargets(packagename, buildSettings)
