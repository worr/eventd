# Helper library for nd plugin
pkglib_LTLIBRARIES += \
	libeventd-nd.la

pkginclude_HEADERS += \
	include/eventd-nd-style.h \
	include/eventd-nd-types.h

libeventd_nd_la_SOURCES = \
	src/plugins/nd/helper/style.c

libeventd_nd_la_CFLAGS = \
	$(AM_CFLAGS) \
	-D G_LOG_DOMAIN=\"libeventd-nd\" \
	$(GDK_PIXBUF_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GLIB_CFLAGS)

libeventd_nd_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-avoid-version

libeventd_nd_la_LIBADD = \
	libeventd-event.la \
	libeventd.la \
	$(GDK_PIXBUF_LIBS) \
	$(GOBJECT_LIBS) \
	$(GLIB_LIBS)
