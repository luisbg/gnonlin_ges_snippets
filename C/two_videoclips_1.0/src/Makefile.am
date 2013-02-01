NULL =

public_headers = 

c_sources = \
	snippet.c

CLEANFILES =

bin_PROGRAMS = snippet

snippet_SOURCES = $(c_sources)
snippet_CFLAGS = $(GST_CFLAGS) $(GTK2_CFLAGS) -DGESDEMO_DATA_DIR="\"$(pkgdatadir)\""
snippet_LDFLAGS = -export-dynamic
snippet_LDADD = $(GST_LIBS) $(GTK2_LIBS)

noinst_HEADERS = $(public_headers)

