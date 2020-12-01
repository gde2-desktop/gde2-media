/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2014 Michal Ratajsky <michal.ratajsky@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <libgde2mixer/gde2mixer.h>

#include "gvc-utils.h"

/* libcanberra requires a PulseAudio channel name to be given to its
 * CA_PROP_CANBERRA_FORCE_CHANNEL property.
 *
 * The strings here are copied from PulseAudio source code to avoid depending
 * on libpulse. */
static const gchar *pulse_position[GDE2_MIXER_CHANNEL_MAX] = {
        [GDE2_MIXER_CHANNEL_MONO] = "mono",
        [GDE2_MIXER_CHANNEL_FRONT_LEFT] = "front-left",
        [GDE2_MIXER_CHANNEL_FRONT_RIGHT] = "front-right",
        [GDE2_MIXER_CHANNEL_FRONT_CENTER] = "front-center",
        [GDE2_MIXER_CHANNEL_LFE] = "lfe",
        [GDE2_MIXER_CHANNEL_BACK_LEFT] = "rear-left",
        [GDE2_MIXER_CHANNEL_BACK_RIGHT] = "rear-right",
        [GDE2_MIXER_CHANNEL_BACK_CENTER] = "rear-center",
        [GDE2_MIXER_CHANNEL_FRONT_LEFT_CENTER] = "front-left-of-center",
        [GDE2_MIXER_CHANNEL_FRONT_RIGHT_CENTER] = "front-right-of-center",
        [GDE2_MIXER_CHANNEL_SIDE_LEFT] = "side-left",
        [GDE2_MIXER_CHANNEL_SIDE_RIGHT] = "side-right",
        [GDE2_MIXER_CHANNEL_TOP_FRONT_LEFT] = "top-front-left",
        [GDE2_MIXER_CHANNEL_TOP_FRONT_RIGHT] = "top-front-right",
        [GDE2_MIXER_CHANNEL_TOP_FRONT_CENTER] = "top-front-center",
        [GDE2_MIXER_CHANNEL_TOP_CENTER] = "top-center",
        [GDE2_MIXER_CHANNEL_TOP_BACK_LEFT] = "top-rear-left",
        [GDE2_MIXER_CHANNEL_TOP_BACK_RIGHT] = "top-rear-right",
        [GDE2_MIXER_CHANNEL_TOP_BACK_CENTER] = "top-rear-center"
};

static const gchar *pretty_position[GDE2_MIXER_CHANNEL_MAX] = {
        [GDE2_MIXER_CHANNEL_UNKNOWN] = N_("Unknown"),
        /* Speaker channel names */
        [GDE2_MIXER_CHANNEL_MONO] = N_("Mono"),
        [GDE2_MIXER_CHANNEL_FRONT_LEFT] = N_("Front Left"),
        [GDE2_MIXER_CHANNEL_FRONT_RIGHT] = N_("Front Right"),
        [GDE2_MIXER_CHANNEL_FRONT_CENTER] = N_("Front Center"),
        [GDE2_MIXER_CHANNEL_LFE] = N_("LFE"),
        [GDE2_MIXER_CHANNEL_BACK_LEFT] = N_("Rear Left"),
        [GDE2_MIXER_CHANNEL_BACK_RIGHT] = N_("Rear Right"),
        [GDE2_MIXER_CHANNEL_BACK_CENTER] = N_("Rear Center"),
        [GDE2_MIXER_CHANNEL_FRONT_LEFT_CENTER] = N_("Front Left of Center"),
        [GDE2_MIXER_CHANNEL_FRONT_RIGHT_CENTER] = N_("Front Right of Center"),
        [GDE2_MIXER_CHANNEL_SIDE_LEFT] = N_("Side Left"),
        [GDE2_MIXER_CHANNEL_SIDE_RIGHT] = N_("Side Right"),
        [GDE2_MIXER_CHANNEL_TOP_FRONT_LEFT] = N_("Top Front Left"),
        [GDE2_MIXER_CHANNEL_TOP_FRONT_RIGHT] = N_("Top Front Right"),
        [GDE2_MIXER_CHANNEL_TOP_FRONT_CENTER] = N_("Top Front Center"),
        [GDE2_MIXER_CHANNEL_TOP_CENTER] = N_("Top Center"),
        [GDE2_MIXER_CHANNEL_TOP_BACK_LEFT] = N_("Top Rear Left"),
        [GDE2_MIXER_CHANNEL_TOP_BACK_RIGHT] = N_("Top Rear Right"),
        [GDE2_MIXER_CHANNEL_TOP_BACK_CENTER] = N_("Top Rear Center")
};

const gchar *
gvc_channel_position_to_pulse_string (Gde2MixerChannelPosition position)
{
        g_return_val_if_fail (position >= 0 && position < GDE2_MIXER_CHANNEL_MAX, NULL);

        return pulse_position[position];
}

const gchar *
gvc_channel_position_to_pretty_string (Gde2MixerChannelPosition position)
{
        g_return_val_if_fail (position >= 0 && position < GDE2_MIXER_CHANNEL_MAX, NULL);

        return pretty_position[position];
}

const gchar *
gvc_channel_map_to_pretty_string (Gde2MixerStreamControl *control)
{
        g_return_val_if_fail (GDE2_MIXER_IS_STREAM_CONTROL (control), NULL);

#define HAS_POSITION(p) (gde2_mixer_stream_control_has_channel_position (control, (p)))

        /* Modeled after PulseAudio 5.0, probably could be extended with other combinations */
        switch (gde2_mixer_stream_control_get_num_channels (control)) {
        case 1:
                if (HAS_POSITION (GDE2_MIXER_CHANNEL_MONO))
                        return _("Mono");
                break;
        case 2:
                if (HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_LEFT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_RIGHT))
                        return _("Stereo");
                break;
        case 4:
                if (HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_LEFT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_RIGHT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_BACK_LEFT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_BACK_RIGHT))
                        return _("Surround 4.0");
                break;
        case 5:
                if (HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_LEFT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_RIGHT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_BACK_LEFT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_BACK_RIGHT)) {
                        if (HAS_POSITION (GDE2_MIXER_CHANNEL_LFE))
                                return _("Surround 4.1");
                        if (HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_CENTER))
                                return _("Surround 5.0");
                }
                break;
        case 6:
                if (HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_LEFT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_RIGHT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_CENTER) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_BACK_LEFT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_BACK_RIGHT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_LFE))
                        return _("Surround 5.1");
                break;
        case 8:
                if (HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_LEFT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_RIGHT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_FRONT_CENTER) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_BACK_LEFT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_BACK_RIGHT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_SIDE_LEFT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_SIDE_RIGHT) &&
                    HAS_POSITION (GDE2_MIXER_CHANNEL_LFE))
                        return _("Surround 7.1");
                break;
        }

#undef HAS_POSITION

        return NULL;
}
