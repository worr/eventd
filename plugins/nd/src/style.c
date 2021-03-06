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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <glib.h>
#include <pango/pango.h>

#include <libeventd-config.h>
#include <libeventd-nd-notification-template.h>

#include "style.h"

struct _EventdNdStyle {
    LibeventdNdNotificationTemplate *template;

    EventdNdStyle *parent;

    struct {
        gboolean set;

        gint   min_width;
        gint   max_width;

        gint   padding;
        gint   radius;
        Colour colour;
    } bubble;

#ifdef ENABLE_GDK_PIXBUF
    struct {
        gboolean set;

        gint max_width;
        gint max_height;
        gint margin;
    } image;

    struct {
        gboolean set;

        EventdNdStyleIconPlacement placement;
        EventdNdAnchor             anchor;
        gint                       max_width;
        gint                       max_height;
        gint                       margin;
        gdouble                    fade_width;
    } icon;
#endif /* ENABLE_GDK_PIXBUF */

    struct {
        gboolean set;

        PangoFontDescription *font;
        Colour colour;
    } title;

    struct {
        gboolean set;

        gint   spacing;
        guint8 max_lines;
        PangoFontDescription *font;
        Colour colour;
    } message;
};

static void
_eventd_nd_style_init_defaults(EventdNdStyle *style)
{
    style->bubble.set = TRUE;

    /* bubble geometry */
    style->bubble.min_width = 200;
    style->bubble.max_width = -1;

    /* bubble style */
    style->bubble.padding   = 10;
    style->bubble.radius    = 10;
    style->bubble.colour.r = 0.15;
    style->bubble.colour.g = 0.15;
    style->bubble.colour.b = 0.15;
    style->bubble.colour.a = 1.0;

#ifdef ENABLE_GDK_PIXBUF
    /* image */
    style->image.set = TRUE;
    style->image.max_width  = 50;
    style->image.max_height = 50;
    style->image.margin     = 10;

    /* icon */
    style->icon.set = TRUE;
    style->icon.placement  = EVENTD_ND_STYLE_ICON_PLACEMENT_BACKGROUND;
    style->icon.anchor     = EVENTD_ND_ANCHOR_VCENTER;
    style->icon.max_width  = 50;
    style->icon.max_height = 50;
    style->icon.margin     = 10;
    style->icon.fade_width = 0.75;
#endif /* ENABLE_GDK_PIXBUF */

    /* title */
    style->title.set = TRUE;
    style->title.font        = pango_font_description_from_string("Linux Libertine O Bold 9");
    style->title.colour.r    = 0.9;
    style->title.colour.g    = 0.9;
    style->title.colour.b    = 0.9;
    style->title.colour.a    = 1.0;

    /* message */
    style->message.set = TRUE;
    style->message.spacing     = 5;
    style->message.max_lines   = 10;
    style->message.font        = pango_font_description_from_string("Linux Libertine O 9");
    style->message.colour.r    = 0.9;
    style->message.colour.g    = 0.9;
    style->message.colour.b    = 0.9;
    style->message.colour.a    = 1.0;
}

EventdNdStyle *
eventd_nd_style_new(EventdNdStyle *parent)
{
    EventdNdStyle *style;

    style = g_new0(EventdNdStyle, 1);

    if ( parent == NULL )
        _eventd_nd_style_init_defaults(style);
    else
        style->parent = parent;

    return style;
}

void
eventd_nd_style_update(EventdNdStyle *self, GKeyFile *config_file, gint *images_max_width, gint *images_max_height)
{
    self->template = libeventd_nd_notification_template_new(config_file);

    if ( g_key_file_has_group(config_file, "NotificationBubble") )
    {
        self->bubble.set = TRUE;

        Int integer;
        Colour colour;

        if ( libeventd_config_key_file_get_int(config_file, "NotificationBubble", "MinWidth", &integer) == 0 )
            self->bubble.min_width = integer.value;
        else if ( self->parent != NULL )
            self->bubble.min_width = eventd_nd_style_get_bubble_min_width(self->parent);

        if ( libeventd_config_key_file_get_int(config_file, "NotificationBubble", "MaxWidth", &integer) == 0 )
            self->bubble.max_width = integer.value;
        else if ( self->parent != NULL )
            self->bubble.max_width = eventd_nd_style_get_bubble_max_width(self->parent);

        if ( libeventd_config_key_file_get_int(config_file, "NotificationBubble", "Padding", &integer) == 0 )
            self->bubble.padding = integer.value;
        else if ( self->parent != NULL )
            self->bubble.padding = eventd_nd_style_get_bubble_padding(self->parent);

        if ( libeventd_config_key_file_get_int(config_file, "NotificationBubble", "Radius", &integer) == 0 )
            self->bubble.radius = integer.value;
        else if ( self->parent != NULL )
            self->bubble.radius = eventd_nd_style_get_bubble_radius(self->parent);

        if ( libeventd_config_key_file_get_colour(config_file, "NotificationBubble", "Colour", &colour) == 0 )
            self->bubble.colour = colour;
        else if ( self->parent != NULL )
            self->bubble.colour = eventd_nd_style_get_bubble_colour(self->parent);

        /* We ignore the minimum width if larger than the maximum */
        if ( ( self->bubble.min_width > -1 ) && ( self->bubble.max_width > -1 ) && ( self->bubble.min_width > self->bubble.max_width ) )
            self->bubble.min_width = -1;
    }

    if ( g_key_file_has_group(config_file, "NotificationTitle") )
    {
        self->title.set = TRUE;

        gchar *string;
        Colour colour;

        if ( libeventd_config_key_file_get_string(config_file, "NotificationTitle", "Font", &string) == 0 )
        {
            pango_font_description_free(self->title.font);
            self->title.font = pango_font_description_from_string(string);
        }
        else if ( self->parent != NULL )
        {
            pango_font_description_free(self->title.font);
            self->title.font = pango_font_description_copy_static(eventd_nd_style_get_title_font(self->parent));
        }

        if ( libeventd_config_key_file_get_colour(config_file, "NotificationTitle", "Colour", &colour) == 0 )
            self->title.colour = colour;
        else if ( self->parent != NULL )
            self->title.colour = eventd_nd_style_get_title_colour(self->parent);

    }

    if ( g_key_file_has_group(config_file, "NotificationMessage") )
    {
        self->message.set = TRUE;

        Int integer;
        gchar *string;
        Colour colour;

        if ( libeventd_config_key_file_get_int(config_file, "NotificationMessage", "Spacing", &integer) == 0 )
            self->message.spacing = integer.value;
        else if ( self->parent != NULL )
            self->message.spacing = eventd_nd_style_get_message_spacing(self->parent);

        if ( libeventd_config_key_file_get_int(config_file, "NotificationMessage", "MaxLines", &integer) == 0 )
            self->message.max_lines = integer.value;
        else if ( self->parent != NULL )
            self->message.max_lines = eventd_nd_style_get_message_max_lines(self->parent);

        if ( libeventd_config_key_file_get_string(config_file, "NotificationMessage", "Font", &string) == 0 )
        {
            pango_font_description_free(self->message.font);
            self->message.font = pango_font_description_from_string(string);
        }
        else if ( self->parent != NULL )
        {
            pango_font_description_free(self->message.font);
            self->message.font = pango_font_description_copy_static(eventd_nd_style_get_message_font(self->parent));
        }

        if ( libeventd_config_key_file_get_colour(config_file, "NotificationMessage", "Colour", &colour) == 0 )
            self->message.colour = colour;
        else if ( self->parent != NULL )
            self->message.colour = eventd_nd_style_get_message_colour(self->parent);

    }

#ifdef ENABLE_GDK_PIXBUF
    if ( g_key_file_has_group(config_file, "NotificationImage") )
    {
        self->image.set = TRUE;

        Int integer;

        if ( libeventd_config_key_file_get_int(config_file, "NotificationImage", "MaxWidth", &integer) == 0 )
        {
            self->image.max_width = integer.value;
            *images_max_width  = MAX(*images_max_width, integer.value);
        }
        else if ( self->parent != NULL )
            self->image.max_width = eventd_nd_style_get_image_max_width(self->parent);

        if ( libeventd_config_key_file_get_int(config_file, "NotificationImage", "MaxHeight", &integer) == 0 )
        {
            self->image.max_height = integer.value;
            *images_max_height  = MAX(*images_max_height, integer.value);
        }
        else if ( self->parent != NULL )
            self->image.max_height = eventd_nd_style_get_image_max_height(self->parent);

        if ( libeventd_config_key_file_get_int(config_file, "NotificationImage", "Margin", &integer) == 0 )
            self->image.margin = integer.value;
        else if ( self->parent != NULL )
            self->image.margin = eventd_nd_style_get_image_margin(self->parent);

    }

    if ( g_key_file_has_group(config_file, "NotificationIcon") )
    {
        self->icon.set = TRUE;

        gchar *string;
        Int integer;

        if ( libeventd_config_key_file_get_string(config_file, "NotificationIcon", "Placement", &string) == 0 )
        {
            if ( g_strcmp0(string, "Background") == 0 )
                self->icon.placement = EVENTD_ND_STYLE_ICON_PLACEMENT_BACKGROUND;
            else if ( g_strcmp0(string, "Overlay") == 0 )
                self->icon.placement = EVENTD_ND_STYLE_ICON_PLACEMENT_OVERLAY;
            else if ( g_strcmp0(string, "Foreground") == 0 )
                self->icon.placement = EVENTD_ND_STYLE_ICON_PLACEMENT_FOREGROUND;
            else
                g_warning("Wrong placement value '%s'", string);
            g_free(string);
        }
        else if ( self->parent != NULL )
            self->icon.placement = eventd_nd_style_get_icon_placement(self->parent);

        if ( libeventd_config_key_file_get_string(config_file, "NotificationIcon", "Anchor", &string) == 0 )
        {
            if ( g_strcmp0(string, "Top") == 0 )
                self->icon.anchor = EVENTD_ND_ANCHOR_TOP;
            else if ( g_strcmp0(string, "Bottom") == 0 )
                self->icon.anchor = EVENTD_ND_ANCHOR_BOTTOM;
            else if ( g_strcmp0(string, "Center") == 0 )
                self->icon.anchor = EVENTD_ND_ANCHOR_VCENTER;
            else
                g_warning("Wrong anchor value '%s'", string);
            g_free(string);
        }
        else if ( self->parent != NULL )
            self->icon.anchor = eventd_nd_style_get_icon_anchor(self->parent);

        if ( libeventd_config_key_file_get_int(config_file, "NotificationIcon", "MaxWidth", &integer) == 0 )
        {
            self->icon.max_width = integer.value;
            *images_max_width  = MAX(*images_max_width, integer.value);
        }
        else if ( self->parent != NULL )
            self->icon.max_width = eventd_nd_style_get_icon_max_width(self->parent);

        if ( libeventd_config_key_file_get_int(config_file, "NotificationIcon", "MaxHeight", &integer) == 0 )
        {
            self->icon.max_height = integer.value;
            *images_max_height  = MAX(*images_max_height, integer.value);
        }
        else if ( self->parent != NULL )
            self->icon.max_height = eventd_nd_style_get_icon_max_height(self->parent);

        if ( libeventd_config_key_file_get_int(config_file, "NotificationIcon", "Margin", &integer) == 0 )
            self->icon.margin = integer.value;
        else if ( self->parent != NULL )
            self->icon.margin = eventd_nd_style_get_icon_margin(self->parent);

        if ( libeventd_config_key_file_get_int(config_file, "NotificationIcon", "FadeWidth", &integer) == 0 )
        {
            if ( ( integer.value < 0 ) || ( integer.value > 100 ) )
            {
                g_warning("Wrong percentage value '%jd'", integer.value);
                integer.value = CLAMP(integer.value, 0, 100);
            }
            self->icon.fade_width = (gdouble) integer.value / 100.;
        }
        else if ( self->parent != NULL )
            self->icon.fade_width = eventd_nd_style_get_icon_fade_width(self->parent);
    }
#endif /* ENABLE_GDK_PIXBUF */
}

void
eventd_nd_style_free(gpointer data)
{
    EventdNdStyle *style = data;
    if ( style == NULL )
        return;

    pango_font_description_free(style->title.font);
    pango_font_description_free(style->message.font);

    libeventd_nd_notification_template_free(style->template);

    g_free(style);
}


LibeventdNdNotificationTemplate *
eventd_nd_style_get_template(EventdNdStyle *self)
{
    return self->template;
}

gint
eventd_nd_style_get_bubble_min_width(EventdNdStyle *self)
{
    if ( self->bubble.set )
        return self->bubble.min_width;
    return eventd_nd_style_get_bubble_min_width(self->parent);
}

gint
eventd_nd_style_get_bubble_max_width(EventdNdStyle *self)
{
    if ( self->bubble.set )
        return self->bubble.max_width;
    return eventd_nd_style_get_bubble_max_width(self->parent);
}

gint
eventd_nd_style_get_bubble_padding(EventdNdStyle *self)
{
    if ( self->bubble.set )
        return self->bubble.padding;
    return eventd_nd_style_get_bubble_padding(self->parent);
}

gint
eventd_nd_style_get_bubble_radius(EventdNdStyle *self)
{
    if ( self->bubble.set )
        return self->bubble.radius;
    return eventd_nd_style_get_bubble_radius(self->parent);
}

Colour
eventd_nd_style_get_bubble_colour(EventdNdStyle *self)
{
    if ( self->bubble.set )
        return self->bubble.colour;
    return eventd_nd_style_get_bubble_colour(self->parent);
}

#ifdef ENABLE_GDK_PIXBUF
gint
eventd_nd_style_get_image_max_width(EventdNdStyle *self)
{
    if ( self->image.set )
        return self->image.max_width;
    return eventd_nd_style_get_image_max_width(self->parent);
}

gint
eventd_nd_style_get_image_max_height(EventdNdStyle *self)
{
    if ( self->image.set )
        return self->image.max_height;
    return eventd_nd_style_get_image_max_height(self->parent);
}

gint
eventd_nd_style_get_image_margin(EventdNdStyle *self)
{
    if ( self->image.set )
        return self->image.margin;
    return eventd_nd_style_get_image_margin(self->parent);
}

EventdNdStyleIconPlacement
eventd_nd_style_get_icon_placement(EventdNdStyle *self)
{
    if ( self->icon.set )
        return self->icon.placement;
    return eventd_nd_style_get_icon_placement(self->parent);
}

EventdNdAnchor
eventd_nd_style_get_icon_anchor(EventdNdStyle *self)
{
    if ( self->icon.set )
        return self->icon.anchor;
    return eventd_nd_style_get_icon_anchor(self->parent);
}

gint
eventd_nd_style_get_icon_max_width(EventdNdStyle *self)
{
    if ( self->icon.set )
        return self->icon.max_width;
    return eventd_nd_style_get_icon_max_width(self->parent);
}

gint
eventd_nd_style_get_icon_max_height(EventdNdStyle *self)
{
    if ( self->icon.set )
        return self->icon.max_height;
    return eventd_nd_style_get_icon_max_height(self->parent);
}

gint
eventd_nd_style_get_icon_margin(EventdNdStyle *self)
{
    if ( self->icon.set )
        return self->icon.margin;
    return eventd_nd_style_get_icon_margin(self->parent);
}

gdouble
eventd_nd_style_get_icon_fade_width(EventdNdStyle *self)
{
    if ( self->icon.set )
        return self->icon.fade_width;
    return eventd_nd_style_get_icon_fade_width(self->parent);
}
#endif /* ENABLE_GDK_PIXBUF */

const PangoFontDescription *
eventd_nd_style_get_title_font(EventdNdStyle *self)
{
    if ( self->title.set )
        return self->title.font;
    return eventd_nd_style_get_title_font(self->parent);
}

Colour
eventd_nd_style_get_title_colour(EventdNdStyle *self)
{
    if ( self->title.set )
        return self->title.colour;
    return eventd_nd_style_get_title_colour(self->parent);
}

gint
eventd_nd_style_get_message_spacing(EventdNdStyle *self)
{
    if ( self->message.set )
        return self->message.spacing;
    return eventd_nd_style_get_message_spacing(self->parent);
}

guint8
eventd_nd_style_get_message_max_lines(EventdNdStyle *self)
{
    if ( self->message.set )
        return self->message.max_lines;
    return eventd_nd_style_get_message_max_lines(self->parent);
}

const PangoFontDescription *
eventd_nd_style_get_message_font(EventdNdStyle *self)
{
    if ( self->message.set )
        return self->message.font;
    return eventd_nd_style_get_message_font(self->parent);
}

Colour
eventd_nd_style_get_message_colour(EventdNdStyle *self)
{
    if ( self->message.set )
        return self->message.colour;
    return eventd_nd_style_get_message_colour(self->parent);
}
