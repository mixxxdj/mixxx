## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign 1.6

if HAVE_THREAD
  MAYBE_THREAD = common/thread
  MAYBE_THREAD_LIB = common/thread/libicethread.la
endif

if HAVE_VORBIS
  MAYBE_VORBIS = codec_vorbis.c
endif

if HAVE_THEORA
  MAYBE_THEORA = codec_theora.c
endif

if HAVE_SPEEX
  MAYBE_SPEEX = codec_speex.c
endif

if HAVE_TLS
  MAYBE_TLS = tls.c
endif

SUBDIRS = common/avl common/net common/timing common/httpp $(MAYBE_THREAD)

lib_LTLIBRARIES = libshout-idjc.la
libshout_idjc_la_LDFLAGS = -version-info 5:0:2

EXTRA_DIST = codec_theora.c codec_speex.c tls.c
noinst_HEADERS = format_ogg.h shout_private.h util.h
PROTOCOLS=proto_http.c proto_xaudiocast.c proto_icy.c proto_roaraudio.c
FORMATS=format_ogg.c format_webm.c format_mpeg.c format_text.c
CODECS=codec_opus.c $(MAYBE_VORBIS) $(MAYBE_THEORA) $(MAYBE_SPEEX)
libshout_idjc_la_SOURCES = shout.c legacy.c icy.c util.c queue.c connection.c $(PROTOCOLS) $(FORMATS) $(CODECS) $(MAYBE_TLS)
AM_CFLAGS = @XIPH_CFLAGS@
AM_CPPFLAGS = -I$(top_builddir)/include -I$(srcdir)/common @XIPH_CPPFLAGS@

libshout_idjc_la_LIBADD = common/net/libicenet.la common/timing/libicetiming.la common/avl/libiceavl.la\
		common/httpp/libicehttpp.la $(MAYBE_THREAD_LIB) $(THEORA_LIBS) $(OGG_LIBS) $(VORBIS_LIBS) $(SPEEX_LIBS) @XIPH_LIBS@


debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
