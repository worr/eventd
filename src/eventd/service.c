/*
 * eventd - Small daemon to act on remote or local events
 *
 * Copyright © 2011 Quentin "Sardem FF7" Glidic
 *
 * This file is part of eventd.
 *
 * eventd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eventd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eventd. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "eventd.h"

#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>

#include <glib.h>
#include <gio/gio.h>
#if ENABLE_GIO_UNIX
#include <gio/gunixsocketaddress.h>
#endif /* ENABLE_GIO_UNIX */

#if ENABLE_SOUND
#include "pulse.h"
#endif /* ENABLE_SOUND */

#if ENABLE_NOTIFY
#include "notify.h"
#endif /* ENABLE_NOTIFY */

#include "events.h"
#include "service.h"

#define BUFFER_SIZE 1024

static gboolean
connection_handler(
	GThreadedSocketService *service,
	GSocketConnection      *connection,
	GObject                *source_object,
	gpointer                user_data)
{
	GDataInputStream *input = NULL;
	GDataOutputStream *output = NULL;
	GError *error = NULL;
	gsize size = 0;
	gchar *type = NULL;
	gchar *name = NULL;
	gchar *line = NULL;
	gchar *action_name = NULL;
	gchar *action = NULL;
	gchar *data = NULL;
	gint64 delay = 0;
	gint64 last_action = 0;

	input = g_data_input_stream_new(g_io_stream_get_input_stream((GIOStream *)connection));
	output = g_data_output_stream_new(g_io_stream_get_output_stream((GIOStream *)connection));
	delay = eventd_config_get_guint64("delay") * 1e6;

	while ( ( line = g_data_input_stream_read_upto(input, "\n", -1, &size, NULL, &error) ) != NULL )
	{
		g_data_input_stream_read_byte(input, NULL, &error);
		if ( error )
			break;
		#if DEBUG
		g_debug("Line received: %s", line);
		#endif /* DEBUG */
		if ( g_ascii_strncasecmp(line, "EVENT", 5) == 0 )
		{
			gchar **event = NULL;

			event = g_strsplit(line+6, " ", 2);
			action = g_strdup(g_strstrip(event[0]));
			if ( event[1] != NULL )
				action_name = g_strdup(g_strstrip(event[1]));
			else
				action_name = g_strdup(name);

			g_strfreev(event);
		}
		else if ( g_ascii_strncasecmp(line, "BYE", 3) == 0 )
		{
			g_data_output_stream_put_string(output, "BYE\n", NULL, &error);
			break;
		}
		else if ( g_ascii_strncasecmp(line, "HELLO", 5) == 0 )
		{
			gchar **hello = NULL;

			if ( ! g_data_output_stream_put_string(output, "HELLO\n", NULL, &error) )
				break;

			hello = g_strsplit(line+6, " ", 2);
			type = g_strdup(g_strstrip(hello[0]));
			if ( hello[1] != NULL )
				name = g_strdup(g_strstrip(hello[1]));
			else
				name = g_strdup(type);
			g_strfreev(hello);
		}
		else if ( action )
		{
			if ( g_ascii_strcasecmp(line, ".") == 0 )
			{
				gint64 action_time = 0;

				action_time = g_get_monotonic_time();
				if ( action_time > ( last_action + delay ) )
				{
					last_action = action_time;
					event_action(type, action_name, action, data);
				}
				g_free(action_name);
				g_free(action);
				g_free(data);
				action_name = NULL;
				action = NULL;
				data = NULL;
			}
			else if ( data )
			{
				gchar *old = NULL;

				old = data;
				data = g_strjoin("\n", old, ( line[0] == '.' ) ? line+1 : line, NULL);

				g_free(old);
			}
			else
				data = g_strdup(( line[0] == '.' ) ? line+1 : line);
		}
		else
			g_warning("Unknown message");

		g_free(line);
	}
	if ( error )
		g_warning("Can't read the socket: %s", error->message);
	g_clear_error(&error);

	g_free(type);
	g_free(name);

	if ( ! g_io_stream_close((GIOStream *)connection, NULL, &error) )
		g_warning("Can't close the stream: %s", error->message);
	g_clear_error(&error);

	return TRUE;
}

static GMainLoop *loop = NULL;

static void
sig_quit_handler(int sig)
{
	if ( loop )
		g_main_loop_quit(loop);
	else
		exit(1);
}


GSocket *
eventd_get_inet_socket(guint16 port)
{
	GSocket *socket = NULL;
	GError *error = NULL;
	GInetAddress *inet_address = NULL;
	GSocketAddress *address = NULL;

	if ( port == 0 )
		goto fail;

	if ( ( socket = g_socket_new(G_SOCKET_FAMILY_IPV6, G_SOCKET_TYPE_STREAM, 0, &error)  ) == NULL )
	{
		g_warning("Unable to create an IPv6 socket: %s", error->message);
		goto fail;
	}

	inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV6);
	address = g_inet_socket_address_new(inet_address, port);
	if ( ! g_socket_bind(socket, address, TRUE, &error) )
	{
		g_warning("Unable to bind the IPv6 socket: %s", error->message);
		goto fail;
	}

	if ( ! g_socket_listen(socket, &error) )
	{
		g_warning("Unable to listen with the IPv6 socket: %s", error->message);
		goto fail;
	}

	return socket;

fail:
	g_free(socket);
	g_clear_error(&error);
	return NULL;
}

#if ENABLE_GIO_UNIX
GSocket *
eventd_get_unix_socket(gchar *path)
{
	GSocket *socket = NULL;
	GError *error = NULL;
	GSocketAddress *address = NULL;

	if ( path == NULL )
		goto fail;

	if ( ( socket = g_socket_new(G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM, 0, &error)  ) == NULL )
	{
		g_warning("Unable to create an UNIX socket: %s", error->message);
		goto fail;
	}

	address = g_unix_socket_address_new(path);
	if ( ! g_socket_bind(socket, address, TRUE, &error) )
	{
		g_warning("Unable to bind the UNIX socket: %s", error->message);
		goto fail;
	}

	if ( ! g_socket_listen(socket, &error) )
	{
		g_warning("Unable to listen with the UNIX socket: %s", error->message);
		goto fail;
	}

	return socket;

fail:
	g_free(socket);
	g_clear_error(&error);
	return NULL;
}
#endif /* ENABLE_GIO_UNIX */

int
eventd_service(GList *sockets)
{
	int retval = 0;
	GError *error = NULL;
	GList *socket = NULL;
	GSocketService *service = NULL;

	signal(SIGTERM, sig_quit_handler);
	signal(SIGINT, sig_quit_handler);


	#if ENABLE_NOTIFY
	eventd_notify_start();
	#endif /* ENABLE_NOTIFY */

	#if ENABLE_SOUND
	eventd_pulse_start();
	#endif /* ENABLE_SOUND */

	eventd_config_parser();

	service = g_threaded_socket_service_new(5);

	for ( socket = g_list_first(sockets) ; socket ; socket = g_list_next(socket) )
	{
		if ( ! g_socket_listener_add_socket((GSocketListener *)service, socket->data, NULL, &error) )
			g_warning("Unable to add socket: %s", error->message);
		g_clear_error(&error);
	}

	g_signal_connect(G_OBJECT(service), "run", G_CALLBACK(connection_handler), NULL);

	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	g_socket_service_stop(service);
	g_socket_listener_close((GSocketListener *)service);

	eventd_config_clean();

	#if ENABLE_SOUND
	eventd_pulse_stop();
	#endif /* ENABLE_SOUND */

	#if ENABLE_NOTIFY
	eventd_notify_stop();
	#endif /* ENABLE_NOTIFY */

	return retval;
}