/*
 * eventd - Small daemon to act on remote or local events
 *
 * Copyright © 2011-2012 Quentin "Sardem FF7" Glidic
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

#ifndef __EVENTD_GLIB_COMPAT_H__
#define __EVENTD_GLIB_COMPAT_H__

#if ! GLIB_CHECK_VERSION(2, 31, 0)
#define g_hash_table_contains(h, k) (g_hash_table_lookup_extended(h, k, NULL, NULL))
#define g_thread_try_new(n, f, d, e) g_thread_create(f, d, TRUE, e)
#define g_thread_new(n, f, d) g_thread_try_new(n, f, d, NULL)

#define g_queue_free_full(q, f) G_STMT_START { g_queue_foreach(q, (GFunc) f, NULL); g_queue_free(q); } G_STMT_END

#define g_socket_connection_is_connected(c) g_socket_is_connected(g_socket_connection_get_socket(c))

#define g_test_undefined() (FALSE)
#endif

#endif /* __EVENTD_GLIB_COMPAT_H__ */
