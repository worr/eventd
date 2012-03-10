# Helper library for nd plugin
pkglib_LTLIBRARIES += \
	libeventd-nd-cairo.la

pkginclude_HEADERS += \
	include/eventd-nd-cairo.h

EXTRA_DIST += \
	src/plugins/nd/cairo/libeventd-nd-cairo.sym

libeventd_nd_cairo_la_SOURCES = \
	src/plugins/nd/cairo/icon.h \
	src/plugins/nd/cairo/icon.c \
	src/plugins/nd/cairo/cairo.c

libeventd_nd_cairo_la_CFLAGS = \
	$(AM_CFLAGS) \
	-D G_LOG_DOMAIN=\"libeventd-nd-cairo\" \
	-D SYSCONFDIR=\"$(sysconfdir)\" \
	-D LIBDIR=\"$(libdir)\" \
	-D DATADIR=\"$(datadir)\" \
	$(GDK_PIXBUF_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(PANGO_CFLAGS) \
	$(GLIB_CFLAGS)

libeventd_nd_cairo_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-avoid-version \
	-export-symbols $(top_srcdir)/src/plugins/nd/cairo/libeventd-nd-cairo.sym

libeventd_nd_cairo_la_LIBADD = \
	libeventd.la \
	libeventd-nd.la \
	$(GDK_PIXBUF_LIBS) \
	$(CAIRO_LIBS) \
	$(PANGO_LIBS) \
	$(GLIB_LIBS)