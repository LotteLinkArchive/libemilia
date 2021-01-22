hexhive_sources = [
	'status.c',
	'pstruct.c',
	'svec.c',
	'mt19937-64.c',
	'entropygen.c',
	'assoca.c',
	'buf.c'
]
hexhive = library('hexhive', hexhive_sources, version : '0.0.0', soversion : '0', include_directories : hexhive_incdir, dependencies : [xxhash_dep], install : true)
