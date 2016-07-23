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

/* indent_size = -1000 means indent_size = tab*/
#define INDENT_SIZE_TAB (-1000)

//~ static GeanyPlugin *geany_plugin;
//~ static GeanyData *geany_data;
//~ static GeanyFunctions *geany_functions;

/* Reload EditorConfig menu item */
static GtkWidget *menu_item_reload_editorconfig;

//~ static void
//~ on_document_open(GObject *obj, GeanyDocument *gd, gpointer user_data);

//~ static void
//~ on_geany_startup_complete(GObject *obj, gpointer user_data);

//~ /* plugin signals */
//~ PluginCallback plugin_callbacks2[] = {
    //~ {"document-open", (GCallback)&on_document_open, TRUE, NULL},
    //~ {"geany-startup-complete", (GCallback)&on_geany_startup_complete, TRUE,
     //~ NULL},
    //~ {NULL, NULL, FALSE, NULL}};

static int
load_editorconfig(const GeanyDocument *gd)
{
    struct {
        const char *indent_style;
        int indent_size;
        int tab_width;
        const char *end_of_line;
    } ecConf; /* obtained EditorConfig settings will be here */

    int i;
    editorconfig_handle eh = editorconfig_handle_init();
    int err_num;
    int name_value_count;
    ScintillaObject *sci = gd->editor->sci;

    memset(&ecConf, 0, sizeof(ecConf));

    /* start parsing */
    err_num = editorconfig_parse(DOC_FILENAME(gd), eh);
    if (err_num != 0 && err_num != EDITORCONFIG_PARSE_NOT_FULL_PATH) {
        /* Ignore full path error, whose error code is
         * EDITORCONFIG_PARSE_NOT_FULL_PATH */
        editorconfig_handle_destroy(eh);
        return err_num;
    }

    /* apply the settings */
    name_value_count = editorconfig_handle_get_name_value_count(eh);

    /* get settings */
    for (i = 0; i < name_value_count; ++i) {
        const char *name;
        const char *value;

        editorconfig_handle_get_name_value(eh, i, &name, &value);

        if (!strcmp(name, "indent_style")) {
            ecConf.indent_style = value;

        } else if (!strcmp(name, "tab_width")) {
            ecConf.tab_width = atoi(value);

        } else if (!strcmp(name, "indent_size")) {

            int value_i = atoi(value);

            if (!strcmp(value, "tab"))
                ecConf.indent_size = INDENT_SIZE_TAB;
            else if (value_i > 0)
                ecConf.indent_size = value_i;

        } else if (!strcmp(name, "end_of_line")) {
            ecConf.end_of_line = value;
        }
    }

    if (ecConf.indent_style) {
        if (!strcmp(ecConf.indent_style, "tab"))
            scintilla_send_message(sci, SCI_SETUSETABS, (uptr_t)1, 0);
        else if (!strcmp(ecConf.indent_style, "space"))
            scintilla_send_message(sci, SCI_SETUSETABS, (uptr_t)0, 0);
    }

    if (ecConf.indent_size > 0) {
        scintilla_send_message(sci, SCI_SETINDENT, (uptr_t)ecConf.indent_size,
                               0);

        /*
         * We set the tab width here, so that this could be overrided then
         * if ecConf.tab_wdith > 0
         */
        scintilla_send_message(sci, SCI_SETTABWIDTH, (uptr_t)ecConf.indent_size,
                               0);
    }

    if (ecConf.tab_width > 0)
        scintilla_send_message(sci, SCI_SETTABWIDTH, (uptr_t)ecConf.tab_width,
                               0);

    if (ecConf.indent_size == INDENT_SIZE_TAB) {
        int cur_tabwidth = scintilla_send_message(sci, SCI_GETTABWIDTH, 0, 0);

        /* set indent_size to tab_width here */
        scintilla_send_message(sci, SCI_SETINDENT, (uptr_t)cur_tabwidth, 0);
    }

    /* set eol */
    if (ecConf.end_of_line) {
        if (!strcmp(ecConf.end_of_line, "lf")) {
            scintilla_send_message(sci, SCI_SETEOLMODE, (uptr_t)SC_EOL_LF, 0);

        } else if (!strcmp(ecConf.end_of_line, "crlf")) {
            scintilla_send_message(sci, SCI_SETEOLMODE, (uptr_t)SC_EOL_CRLF, 0);

        } else if (!strcmp(ecConf.end_of_line, "cr")) {
            scintilla_send_message(sci, SCI_SETEOLMODE, (uptr_t)SC_EOL_CR, 0);
        }
    }

    editorconfig_handle_destroy(eh);

    return 0;
}

/*
 * Reload EditorConfig menu call back
 */
static void
menu_item_reload_editorconfig_cb(GtkMenuItem *menuitem,
                                 gpointer user_data)
{
    int err_num;
    GeanyDocument *gd = document_get_current();

    /* if gd is NULL, do nothing */
    if (!gd)
        return;

    /* reload EditorConfig */
    err_num = load_editorconfig(gd);
    if (err_num != 0) {
        dialogs_show_msgbox(GTK_MESSAGE_ERROR,
                            "Failed to reload EditorConfig.");
    }
}

//~ static void
//~ on_document_open(GObject *obj, GeanyDocument *gd, gpointer user_data)
//~ {
    //~ int err_num;

    //~ if (!gd)
        //~ return;

    //~ /* reload EditorConfig */
    //~ err_num = load_editorconfig(gd);
    //~ if (err_num != 0) {
        //~ dialogs_show_msgbox(GTK_MESSAGE_ERROR,
                            //~ "Failed to reload EditorConfig.");
    //~ }
//~ }

//~ static void
//~ on_geany_startup_complete(GObject *obj, gpointer user_data)
//~ {
    //~ int i;

    //~ /* load EditorConfig for each GeanyDocument on startup */
    //~ foreach_document (i) {
        //~ int err = load_editorconfig(documents[i]);
        //~ if (err != 0)
            //~ dialogs_show_msgbox(GTK_MESSAGE_ERROR,
                                //~ "Failed to reload EditorConfig.");
    //~ }
//~ }

static gboolean
editorconfig_plugin_init(GeanyPlugin *plugin, gpointer pdata)
{
    printf("editorconfig_plugin_init\n");

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

    return TRUE;
}

static void
editorconfig_plugin_cleanup(GeanyPlugin *plugin, gpointer pdata)
{
    printf("editorconfig_plugin_destroy\n");
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
