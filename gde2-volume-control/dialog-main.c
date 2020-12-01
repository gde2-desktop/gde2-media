/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2008 Red Hat, Inc.
 * Copyright (C) 2014 Michal Ratajsky <michal.ratajsky@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <libintl.h>
#include <unique/uniqueapp.h>
#include <libgde2mixer/gde2mixer.h>

#include "gvc-mixer-dialog.h"

#define DIALOG_POPUP_TIMEOUT 3

static guint       popup_id = 0;
static gboolean    debug = FALSE;
static gboolean    show_version = FALSE;

static gchar      *page = NULL;
static GtkWidget  *app_dialog = NULL;
static GtkWidget  *warning_dialog = NULL;

static void
on_dialog_response (GtkDialog *dialog, guint response_id, gpointer data)
{
        gboolean destroy = GPOINTER_TO_INT (data);

        if (destroy == TRUE)
                gtk_widget_destroy (GTK_WIDGET (dialog));

        gtk_main_quit ();
}

static void
on_dialog_close (GtkDialog *dialog, gpointer data)
{
        gboolean destroy = GPOINTER_TO_INT (data);

        if (destroy == TRUE)
                gtk_widget_destroy (GTK_WIDGET (dialog));

        gtk_main_quit ();
}

static UniqueResponse
on_app_message_received (UniqueApp         *app,
                         int                command,
                         UniqueMessageData *message_data,
                         guint              time_,
                         gpointer           user_data)
{
        gtk_window_present (GTK_WINDOW (user_data));

        return UNIQUE_RESPONSE_OK;
}

static void
remove_warning_dialog (void)
{
        if (popup_id != 0) {
                g_source_remove (popup_id);
                popup_id = 0;
        }

        g_clear_pointer (&warning_dialog, gtk_widget_destroy);
}

static void
context_ready (Gde2MixerContext *context, UniqueApp *app)
{
	/* The dialog might be already created, e.g. when reconnected
	 * to a sound server */
        if (app_dialog != NULL)
                return;

        app_dialog = GTK_WIDGET (gvc_mixer_dialog_new (context));

        g_signal_connect (G_OBJECT (app_dialog),
                          "response",
                          G_CALLBACK (on_dialog_response),
                          GINT_TO_POINTER (FALSE));
        g_signal_connect (G_OBJECT (app_dialog),
                          "close",
                          G_CALLBACK (on_dialog_close),
                          GINT_TO_POINTER (FALSE));

        gvc_mixer_dialog_set_page (GVC_MIXER_DIALOG (app_dialog), page);

        g_signal_connect (G_OBJECT (app),
                          "message-received",
                          G_CALLBACK (on_app_message_received),
                          app_dialog);

        gtk_widget_show (app_dialog);
}

static void
on_context_state_notify (Gde2MixerContext *context,
                         GParamSpec       *pspec,
                         UniqueApp        *app)
{
        Gde2MixerState state = gde2_mixer_context_get_state (context);

        if (state == GDE2_MIXER_STATE_READY) {
                remove_warning_dialog ();
                context_ready (context, app);
        }
        else if (state == GDE2_MIXER_STATE_FAILED) {
                GtkWidget *dialog;

                remove_warning_dialog ();

                dialog = gtk_message_dialog_new (GTK_WINDOW (app_dialog),
                                                 0,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_CLOSE,
                                                 _("Sound system is not available"));

                g_signal_connect (G_OBJECT (dialog),
                                  "response",
                                  G_CALLBACK (on_dialog_response),
                                  GINT_TO_POINTER (TRUE));
                g_signal_connect (G_OBJECT (dialog),
                                  "close",
                                  G_CALLBACK (on_dialog_close),
                                  GINT_TO_POINTER (TRUE));

                gtk_widget_show (dialog);
        }
}

static gboolean
dialog_popup_timeout (gpointer data)
{
	warning_dialog = gtk_message_dialog_new (GTK_WINDOW (app_dialog),
	                                         0,
	                                         GTK_MESSAGE_INFO,
	                                         GTK_BUTTONS_CANCEL,
	                                         _("Waiting for sound system to respond"));

	g_signal_connect (G_OBJECT (warning_dialog),
	                  "response",
                      G_CALLBACK (on_dialog_response),
                      GINT_TO_POINTER (TRUE));
	g_signal_connect (G_OBJECT (warning_dialog),
	                  "close",
                      G_CALLBACK (on_dialog_close),
                      GINT_TO_POINTER (TRUE));

	gtk_widget_show (warning_dialog);

	return FALSE;
}

int
main (int argc, char **argv)
{
        GError           *error = NULL;
        gchar            *backend = NULL;
        Gde2MixerContext *context;
        UniqueApp        *app;
        GOptionEntry      entries[] = {
                { "backend", 'b', 0, G_OPTION_ARG_STRING, &backend, N_("Sound system backend"), "pulse|alsa|oss|null" },
                { "debug",   'd', 0, G_OPTION_ARG_NONE,   &debug, N_("Enable debug"), NULL },
                { "page",    'p', 0, G_OPTION_ARG_STRING, &page, N_("Startup page"), "effects|hardware|input|output|applications" },
                { "version", 'v', 0, G_OPTION_ARG_NONE,   &show_version, N_("Version of this application"), NULL },
                { NULL }
        };

        bindtextdomain (GETTEXT_PACKAGE, LOCALE_DIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);

        gtk_init_with_args (&argc, &argv,
                            _(" — GDE2 Volume Control"),
                            entries, GETTEXT_PACKAGE,
                            &error);

        if (error != NULL) {
                g_warning ("%s", error->message);
                return 1;
        }
        if (show_version == TRUE) {
                g_print ("%s %s\n", argv[0], VERSION);
                return 0;
        }
        if (debug == TRUE) {
                g_setenv ("G_MESSAGES_DEBUG", "all", FALSE);
        }

        app = unique_app_new (GVC_DIALOG_DBUS_NAME, NULL);

        if (unique_app_is_running (app) == TRUE) {
                unique_app_send_message (app, UNIQUE_ACTIVATE, NULL);
                return 0;
        }
        if (gde2_mixer_init () == FALSE) {
                g_warning ("libgde2mixer initialization failed, exiting");
                return 1;
        }

        context = gde2_mixer_context_new ();

        if (backend != NULL) {
                if (strcmp (backend, "pulse") == 0)
                        gde2_mixer_context_set_backend_type (context, GDE2_MIXER_BACKEND_PULSEAUDIO);
                else if (strcmp (backend, "alsa") == 0)
                        gde2_mixer_context_set_backend_type (context, GDE2_MIXER_BACKEND_ALSA);
                else if (strcmp (backend, "oss") == 0)
                        gde2_mixer_context_set_backend_type (context, GDE2_MIXER_BACKEND_OSS);
                else if (strcmp (backend, "null") == 0)
                        gde2_mixer_context_set_backend_type (context, GDE2_MIXER_BACKEND_NULL);
                else {
                        g_warning ("Invalid backend: %s", backend);
                        g_object_unref (context);
                        g_object_unref (app);
                        g_free (backend);
                        return 1;
                }

                g_free (backend);
        }

        gde2_mixer_context_set_app_name (context, _("Volume Control"));
        gde2_mixer_context_set_app_id (context, GVC_DIALOG_DBUS_NAME);
        gde2_mixer_context_set_app_version (context, VERSION);
        gde2_mixer_context_set_app_icon (context, "multimedia-volume-control");

        g_signal_connect (G_OBJECT (context),
                          "notify::state",
                          G_CALLBACK (on_context_state_notify),
                          app);

        gde2_mixer_context_open (context);

        if (gde2_mixer_context_get_state (context) == GDE2_MIXER_STATE_CONNECTING) {
		   popup_id = g_timeout_add_seconds (DIALOG_POPUP_TIMEOUT,
                                                 dialog_popup_timeout,
                                                 NULL);
        }

        gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (),
                                           ICON_DATA_DIR);

        gtk_window_set_default_icon_name ("multimedia-volume-control");

        gtk_main ();

        g_object_unref (context);
        g_object_unref (app);

        return 0;
}
