project('hexhive', 'c')

cc = meson.get_compiler('c')
xxhash_dep = cc.find_library('xxhash', required : true)
libbsd_dep = cc.find_library('bsd', required : true)

hexhive_incdir = include_directories('include')
subdir('include')
subdir('src')
hexhive_dep = declare_dependency(include_directories : hexhive_incdir, link_with : hexhive)
subdir('tests')
