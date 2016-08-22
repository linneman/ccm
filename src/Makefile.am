AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -I.


bin_PROGRAMS = tcm
tcm_SOURCES = \
	tcm_server.h \
	tcm_server.c \
	tcm_config.h \
	tcm_config.c \
	tcm_scheme.c \
	tcm_scheme.h \
	tcm_scheme_ext.c \
	tcm_scheme_ext.h \
	tcm_segfaulthandler.c \
	tcm_segfaulthandler.h \
	tcm_log.h \
	tcm_log.c \
	base_channel.h \
	dev_channel.h \
	dev_channel.c \
	client_sock_channel.h \
	client_sock_channel.c \
	server_sock_channel.h \
	server_sock_channel.c \
	revision.h \
	revision.c \
	utils.c \
	utils.h \
	common.h \
	fmemopen.c \
	fmemopen.h

dist_bin_SCRIPTS = routes.scm \
                   tcm.scm

tcm_LDFLAGS = -lpthread $(tinyscheme_LIBS) $(libintercom_LIBS) $(GLIB_LIBS)
tcm_CPPFLAGS = -DSCHEMESCRIPTDIR=\"$(bindir)\" $(tinyscheme_CFLAGS) $(libintercom_CFLAGS) $(GLIB_CFLAGS)
