project('emilia', 'c')

cc = meson.get_compiler('c')
xxhash_dep = cc.find_library('xxhash', required : true)

emilia_incdir = include_directories('include')
subdir('include')
subdir('src')
emilia_dep = declare_dependency(include_directories : emilia_incdir, link_with : emilia)
subdir('tests')
