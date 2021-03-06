# sound plugin

if ENABLE_SOUND
plugins_LTLIBRARIES += \
	sound.la

man5_MANS += \
	plugins/sound/man/eventd-sound.event.5
endif


sound_la_SOURCES = \
	plugins/sound/src/pulseaudio.h \
	plugins/sound/src/pulseaudio.c \
	plugins/sound/src/sound.c

sound_la_CFLAGS = \
	$(AM_CFLAGS) \
	-D G_LOG_DOMAIN=\"eventd-sound\" \
	$(SNDFILE_CFLAGS) \
	$(PULSEAUDIO_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GLIB_CFLAGS)

sound_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-avoid-version -module

sound_la_LIBADD = \
	libeventd-event.la \
	libeventd-plugin.la \
	libeventd.la \
	$(SNDFILE_LIBS) \
	$(PULSEAUDIO_LIBS) \
	$(GOBJECT_LIBS) \
	$(GLIB_LIBS)
