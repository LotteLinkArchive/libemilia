hexhive_sources = [
	'status.c',
	'pstruct.c',
	'register.c',
	'svec.c',
	'cuckoo.c',
	'mt19937-64.c',
	'hashmap.c'
]
hexhive = library('hexhive', hexhive_sources, version : '0.0.0', soversion : '0', include_directories : hexhive_incdir, dependencies : [xxhash_dep], install : true)
