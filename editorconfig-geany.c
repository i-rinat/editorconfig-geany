/*
  Copyright (c) 2011-2012 EditorConfig Team
  All rights reserved.

  This file is part of EditorConfig-geany.

  EditorConfig-geany is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation, either version 2 of the License, or (at your option) any
  later version.

  EditorConfig-geany is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  EditorConfig-geany. If not, see <http://www.gnu.org/licenses/>.
*/

#include <editorconfig/editorconfig.h>
#include <geanyplugin.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
#if 1
#define TRACE(...)  do { } while (0)
#else
#define TRACE(...) printf(__VA_ARGS__)
#endif
// clang-format on

static const int INDENT_SIZE_TAB = -1000;

static GtkWidget *menu_item_reload_editorconfig;

static int
load_editorconfig(const GeanyDocument *gd)
{
    struct {
        const char *end_of_line;
        const char *indent_style;
        int indent_size;
        int max_line_length;
        int tab_width;
    } ec_conf = {};

    editorconfig_handle eh = editorconfig_handle_init();
    ScintillaObject *sci = gd->editor->sci;

    // start parsing
    TRACE("DOC_FILENAME(gd) = %s\n", DOC_FILENAME(gd));

    int err = editorconfig_parse(DOC_FILENAME(gd), eh);

    if (err == EDITORCONFIG_PARSE_NOT_FULL_PATH) {
        // ignore full path error
        err = 0;
    }

    if (err != 0) {
        editorconfig_handle_destroy(eh);
        return err;
    }

    // get settings from config
    const int n = editorconfig_handle_get_name_value_count(eh);

    for (int k = 0; k < n; k ++) {
        const char *name;
        const char *value;

        editorconfig_handle_get_name_value(eh, k, &name, &value);

        if (strcmp(name, "indent_style") == 0) {
            ec_conf.indent_style = value;

        } else if (strcmp(name, "tab_width") == 0) {
            ec_conf.tab_width = atoi(value);

        } else if (strcmp(name, "indent_size") == 0) {
            int value_i = atoi(value);

            if (strcmp(value, "tab") == 0)
                ec_conf.indent_size = INDENT_SIZE_TAB;
            else if (value_i > 0)
                ec_conf.indent_size = value_i;

        } else if (strcmp(name, "end_of_line") == 0) {
            ec_conf.end_of_line = value;

        } else if (strcmp(name, "max_line_length") == 0) {
            ec_conf.max_line_length = atoi(value);
        }
    }

    // apply settings

    if (ec_conf.indent_style) {
        if (strcmp(ec_conf.indent_style, "tab") == 0) {
            editor_set_indent_type(gd->editor, GEANY_INDENT_TYPE_TABS);

        } else if (strcmp(ec_conf.indent_style, "space") == 0) {
            editor_set_indent_type(gd->editor, GEANY_INDENT_TYPE_SPACES);
        }
    }

    if (ec_conf.indent_size > 0) {
        editor_set_indent_width(gd->editor, ec_conf.indent_size);

        // set the tab width here, so that this could be overrided if there is
        // non-negative tab_width
        scintilla_send_message(sci, SCI_SETTABWIDTH,
                               (uptr_t)ec_conf.indent_size, 0);
    }

    if (ec_conf.tab_width > 0)
        scintilla_send_message(sci, SCI_SETTABWIDTH, (uptr_t)ec_conf.tab_width,
                               0);

    if (ec_conf.indent_size == INDENT_SIZE_TAB) {
        int cur_tabwidth = scintilla_send_message(sci, SCI_GETTABWIDTH, 0, 0);

        // since "indent_size" was equal to "tab", indent width should be set
        // to current tab width
        editor_set_indent_width(gd->editor, cur_tabwidth);
    }

    // set end-of-line type
    if (ec_conf.end_of_line) {
        if (strcmp(ec_conf.end_of_line, "lf") == 0) {
            scintilla_send_message(sci, SCI_SETEOLMODE, (uptr_t)SC_EOL_LF, 0);

        } else if (strcmp(ec_conf.end_of_line, "crlf") == 0) {
            scintilla_send_message(sci, SCI_SETEOLMODE, (uptr_t)SC_EOL_CRLF, 0);

        } else if (strcmp(ec_conf.end_of_line, "cr") == 0) {
            scintilla_send_message(sci, SCI_SETEOLMODE, (uptr_t)SC_EOL_CR, 0);
        }
    }

    // set maximum line length indicator
    if (ec_conf.max_line_length > 0) {
        scintilla_send_message(sci, SCI_SETEDGECOLUMN,
                               (uptr_t)ec_conf.max_line_length, 0);
    }

    editorconfig_handle_destroy(eh);

    return 0;
}

static void
show_error_message(void)
{
    dialogs_show_msgbox(GTK_MESSAGE_ERROR, "Failed to reload EditorConfig.");
}

static void
menu_item_reload_editorconfig_cb(GtkMenuItem *menuitem, gpointer user_data)
{
    TRACE("menu_item_reload_editorconfig_cb\n");

    GeanyDocument *doc = document_get_current();
    if (!doc)
        return;

    if (load_editorconfig(doc) != 0)
        show_error_message();
}

static void
on_document_open(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
    TRACE("on_document_open\n");

    if (!doc)
        return;

    if (load_editorconfig(doc) != 0)
        show_error_message();
}

static void
on_document_reload(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
    TRACE("on_document_reload\n");

    if (!doc)
        return;

    if (load_editorconfig(doc) != 0)
        show_error_message();
}

static void
on_document_save(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
    TRACE("on_document_save\n");

    if (!doc)
        return;

    if (load_editorconfig(doc) != 0)
        show_error_message();
}

static void
on_geany_startup_complete(GObject *obj, gpointer user_data)
{
    TRACE("on_geany_startup_complete\n");

    GeanyPlugin *plugin = user_data;
    GeanyData *geany_data = plugin->geany_data; // foreach_document uses it
    int i;

    // load EditorConfig for each GeanyDocument on startup
    foreach_document (i) {
        GeanyDocument *doc = documents[i];
        if (load_editorconfig(doc) != 0)
            show_error_message();
    }
}

static gboolean
editorconfig_plugin_init(GeanyPlugin *plugin, gpointer pdata)
{
    TRACE("editorconfig_plugin_init\n");

    // Create a new menu item and show it
    menu_item_reload_editorconfig =
        gtk_menu_item_new_with_mnemonic("Reload EditorConfig");

    gtk_widget_show(menu_item_reload_editorconfig);

    // attach the new menu item to the Tools menu
    gtk_container_add(
        GTK_CONTAINER(plugin->geany_data->main_widgets->tools_menu),
        menu_item_reload_editorconfig);

    // Connect the menu item with a callback function
    // which is called when the item is clicked
    g_signal_connect(menu_item_reload_editorconfig, "activate",
                     G_CALLBACK(menu_item_reload_editorconfig_cb), NULL);

    plugin_signal_connect(plugin, NULL, "geany-startup-complete", TRUE,
                          G_CALLBACK(on_geany_startup_complete), plugin);

    plugin_signal_connect(plugin, NULL, "document-open", TRUE,
                          G_CALLBACK(on_document_open), NULL);

    plugin_signal_connect(plugin, NULL, "document-reload", TRUE,
                          G_CALLBACK(on_document_reload), NULL);

    plugin_signal_connect(plugin, NULL, "document-save", TRUE,
                          G_CALLBACK(on_document_save), NULL);

    return TRUE;
}

static void
editorconfig_plugin_cleanup(GeanyPlugin *plugin, gpointer pdata)
{
    TRACE("editorconfig_plugin_destroy\n");

    gtk_widget_destroy(menu_item_reload_editorconfig);
}

G_MODULE_EXPORT
void
geany_load_module(GeanyPlugin *plugin)
{
    plugin->info->name = "EditorConfig";
    plugin->info->description = "EditorConfig Plugin for Geany";
    plugin->info->version = "0.2";
    plugin->info->author = "http://editorconfig.org"; // TODO: ?

    plugin->funcs->init = editorconfig_plugin_init;
    plugin->funcs->cleanup = editorconfig_plugin_cleanup;

    if (GEANY_PLUGIN_REGISTER(plugin, 225))
        return;
}
