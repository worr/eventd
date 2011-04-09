/*
 * Eventd - Small daemon to act on remote or local events
 *
 * Copyright © 2011 Sardem FF7
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __EVENTD_H__
#define __EVENTD_H__

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef ENABLE_NLS
	#include <locale.h>
	#include <libintl.h>
	#define _(x) dgettext(GETTEXT_PACKAGE, x)
	#ifdef dgettext_noop
		#define N_(x) dgettext_noop(GETTEXT_PACKAGE, x)
	#else
		#define N_(x) (x)
	#endif
#else /* ! ENABLE_NLS */
	#include <locale.h>
	#define _(x) (x)
	#define ngettext(Singular, Plural, Number) ((Number == 1) ? (Singular) : (Plural))
	#define N_(x) (x)
#endif /* ! ENABLE_NLS */

#define RUN_DIR ".local/run/eventd"

#endif /* __EVENTD_H__ */
