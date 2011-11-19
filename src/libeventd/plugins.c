/*
 * libeventd - Internal helper
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

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <libeventd-client.h>
#include <libeventd-event.h>
#include <eventd-plugin.h>
#include <libeventd-plugins.h>

static void
_libeventd_plugins_load_dir(GList **plugins, const gchar *plugins_dir_name, gpointer user_data)
{
    GError *error;
    GDir *plugins_dir;
    const gchar *file;

    if ( ! g_module_supported() )
    {
        g_warning("Couldn’t load plugins: %s", g_module_error());
        return;
    }

    #if DEBUG
    g_debug("Scannig plugins dir: %s", plugins_dir_name);
    #endif /* DEBUG */

    plugins_dir = g_dir_open(plugins_dir_name, 0, &error);
    if ( ! plugins_dir )
    {
        g_warning("Can’t read the plugins directory: %s", error->message);
        g_clear_error(&error);
        return;
    }

    while ( ( file = g_dir_read_name(plugins_dir) ) != NULL )
    {
        gchar *full_filename;
        EventdPlugin *plugin;
        EventdPluginGetInfoFunc get_info;
        void *module;

        full_filename = g_build_filename(plugins_dir_name, file, NULL);

        if ( g_file_test(full_filename, G_FILE_TEST_IS_DIR) )
            goto next;

        module = g_module_open(full_filename, G_MODULE_BIND_LAZY|G_MODULE_BIND_LOCAL);
        if ( module == NULL )
        {
            g_warning("Couldn’t load module '%s': %s", file, g_module_error());
            goto next;
        }

        if ( ! g_module_symbol(module, "eventd_plugin_get_info", (void **)&get_info) )
            goto next;

        #if DEBUG
        g_debug("Loading plugin '%s'", file);
        #endif /* ! DEBUG */

        plugin = g_new0(EventdPlugin, 1);
        plugin->id = NULL;
        plugin->module = module;
        get_info(plugin);

        plugin->id = g_strdup(plugin->id ?: file);

        if ( plugin->start != NULL )
            plugin->start(user_data);

        *plugins = g_list_prepend(*plugins, plugin);

    next:
        g_free(full_filename);
    }
}

void
libeventd_plugins_load(GList **plugins, const gchar *plugins_subdir, gpointer user_data)
{
    const gchar *env_base_dir;
    gchar *plugins_dir;

    if ( plugins == NULL )
        return;

    env_base_dir = g_getenv("EVENTD_PLUGINS_DIR");
    if ( env_base_dir != NULL )
    {
        if ( env_base_dir[0] == '~' )
            plugins_dir = g_build_filename(g_get_home_dir(), env_base_dir+2, plugins_subdir, NULL);
        else
            plugins_dir = g_build_filename(env_base_dir, plugins_subdir, NULL);

        if ( g_file_test(plugins_dir, G_FILE_TEST_IS_DIR) )
            _libeventd_plugins_load_dir(plugins, plugins_dir, user_data);
        g_free(plugins_dir);
    }

    plugins_dir = g_build_filename(g_get_user_data_dir(), PACKAGE_NAME, plugins_subdir, NULL);
    if ( g_file_test(plugins_dir, G_FILE_TEST_IS_DIR) )
        _libeventd_plugins_load_dir(plugins, plugins_dir, user_data);
    g_free(plugins_dir);

    plugins_dir = g_build_filename(DATADIR, PACKAGE_NAME, plugins_subdir, NULL);
    if ( g_file_test(plugins_dir, G_FILE_TEST_IS_DIR) )
        _libeventd_plugins_load_dir(plugins, plugins_dir, user_data);
    g_free(plugins_dir);

    plugins_dir = g_build_filename(LIBDIR, PACKAGE_NAME, plugins_subdir, NULL);
    if ( g_file_test(plugins_dir, G_FILE_TEST_IS_DIR) )
        _libeventd_plugins_load_dir(plugins, plugins_dir, user_data);
    g_free(plugins_dir);
}

static void
_libeventd_plugins_plugin_free(gpointer data)
{
    EventdPlugin *plugin = data;

    if ( plugin->stop != NULL )
        plugin->stop();
    g_module_close(plugin->module);
    g_free(plugin->id);
    g_free(plugin);
}

void
libeventd_plugins_unload(GList **plugins)
{
    if ( ( plugins == NULL ) || ( *plugins == NULL ) )
        return;
    g_list_free_full(*plugins, _libeventd_plugins_plugin_free);
    *plugins = NULL;
}

void
libeventd_plugins_foreach(GList *plugins, GFunc func, gpointer user_data)
{
    if ( plugins == NULL )
        return;
    g_list_foreach(plugins, func, user_data);
}


static void
_libeventd_plugins_config_init(gpointer data, gpointer user_data)
{
    EventdPlugin *plugin = data;

    if ( plugin->config_init != NULL )
        plugin->config_init();
}

void
libeventd_plugins_config_init_all(GList *plugins)
{
    libeventd_plugins_foreach(plugins, _libeventd_plugins_config_init, NULL);
}

static void
_libeventd_plugins_config_clean(gpointer data, gpointer user_data)
{
    EventdPlugin *plugin = data;

    if ( plugin->config_clean != NULL )
        plugin->config_clean();
}

void
libeventd_plugins_config_clean_all(GList *plugins)
{
    libeventd_plugins_foreach(plugins, _libeventd_plugins_config_clean, NULL);
}

typedef struct {
    const gchar *type;
    const gchar *event;
    GKeyFile *config_file;
} EventdEventParseData;

static void
_libeventd_plugins_event_parse(gpointer data, gpointer user_data)
{
    EventdPlugin *plugin = data;
    EventdEventParseData *parse_data = user_data;
    if ( plugin->event_parse != NULL )
        plugin->event_parse(parse_data->type, parse_data->event, parse_data->config_file);
}

void libeventd_plugins_event_parse_all(GList *plugins, const gchar *type, const gchar *event, GKeyFile *config_file)
{
    EventdEventParseData data = {
        .type = type,
        .event = event,
        .config_file = config_file,
    };
    libeventd_plugins_foreach(plugins, _libeventd_plugins_event_parse, &data);
}

typedef struct {
    GHashTable *ret;
    const gchar *prefix;
} EventdEventActionReturnData;

static gboolean
_libeventd_plugins_event_action_return(gpointer key, gpointer value, gpointer user_data)
{
    gchar *name = key;
    gchar *content = value;
    EventdEventActionReturnData *return_data = user_data;
    gchar *full_name;

    full_name = g_strconcat(return_data->prefix, "-", name, NULL);
    g_free(name);

    g_hash_table_insert(return_data->ret, full_name, content);

    return TRUE;
}

typedef struct {
    EventdClient *client;
    EventdEvent *event;
    GHashTable *ret;
} EventdEventActionData;

static void
libeventd_plugins_event_action(gpointer data, gpointer user_data)
{
    EventdPlugin *plugin = data;
    EventdEventActionData *action_data = user_data;
    GHashTable *ret;

    if ( plugin->event_action == NULL )
        return;

    ret = plugin->event_action(action_data->client, action_data->event);
    if ( ret != NULL )
    {
        EventdEventActionReturnData ret_data = {
            .ret = action_data->ret,
            .prefix = plugin->id
        };
        g_hash_table_foreach_steal(ret, _libeventd_plugins_event_action_return, &ret_data);
        g_hash_table_unref(ret);
    }
}

GHashTable *
libeventd_plugins_event_action_all(GList *plugins, EventdClient *client, EventdEvent *event)
{
    EventdEventActionData data = {
        .client = client,
        .event = event
    };
    data.ret = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    libeventd_plugins_foreach(plugins, libeventd_plugins_event_action, &data);

    if ( g_hash_table_size(data.ret) < 1 )
        data.ret = (g_hash_table_unref(data.ret), NULL);
    return data.ret;
}
