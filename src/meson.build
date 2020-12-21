hexhive_sources = [
	'status.c',
	'pstruct.c',
	'register.c',
	'svec.c',
	'cuckoo.c'
]
hexhive = library('hexhive', hexhive_sources, version : '0.0.0', soversion : '0', include_directories : hexhive_incdir, dependencies : [xxhash_dep], install : true)
