/*
 * This file is part of hildon-home
 *
 * Copyright (C) 2008 Nokia Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <gdk/gdkx.h>

#include <X11/X.h>
#include <X11/Xatom.h>

#include "hd-incoming-event-window.h"

/* Pixel sizes */
#define INCOMING_EVENT_WINDOW_WIDTH 342
#define INCOMING_EVENT_WINDOW_HEIGHT 80

#define INCOMING_EVENT_WINDOW_CLOSE  43
#define INCOMING_EVENT_WINDOW_ICON  24

#define MARGIN_DEFAULT 8
#define MARGIN_HALF 4

/* Timeout in seconds */
#define INCOMING_EVENT_WINDOW_PREVIEW_TIMEOUT 4

#define HD_INCOMING_EVENT_WINDOW_GET_PRIVATE(object) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((object), HD_TYPE_INCOMING_EVENT_WINDOW, HDIncomingEventWindowPrivate))

enum
{
  PROP_0,
  PROP_PREVIEW,
  PROP_ICON,
  PROP_TITLE,
  PROP_TIME,
  PROP_MESSAGE
};

enum {
    RESPONSE,
    N_SIGNALS
};

static guint signals[N_SIGNALS];  

struct _HDIncomingEventWindowPrivate
{
  gboolean preview;

  GtkWidget *icon;
  GtkWidget *title;
  GtkWidget *time_label;
  GtkWidget *cbox;
  GtkWidget *message;

  time_t time;

  guint timeout_id;
};

G_DEFINE_TYPE (HDIncomingEventWindow, hd_incoming_event_window, GTK_TYPE_WINDOW);

static gboolean
hd_incoming_event_window_timeout (HDIncomingEventWindow *window)
{
  HDIncomingEventWindowPrivate *priv = window->priv;

  GDK_THREADS_ENTER ();

  priv->timeout_id = 0;

  g_signal_emit (window, signals[RESPONSE], 0, GTK_RESPONSE_DELETE_EVENT);

  GDK_THREADS_LEAVE ();

  return FALSE;
}

static gboolean
hd_incoming_event_window_map_event (GtkWidget   *widget,
                                    GdkEventAny *event)
{
  HDIncomingEventWindowPrivate *priv = HD_INCOMING_EVENT_WINDOW (widget)->priv;
  gboolean result = FALSE;

  if (GTK_WIDGET_CLASS (hd_incoming_event_window_parent_class)->map_event)
    result = GTK_WIDGET_CLASS (hd_incoming_event_window_parent_class)->map_event (widget,
                                                                                  event);

  if (priv->preview)
    {
      priv->timeout_id = g_timeout_add_seconds (INCOMING_EVENT_WINDOW_PREVIEW_TIMEOUT,
                                                (GSourceFunc) hd_incoming_event_window_timeout,
                                                widget);
    }

  return result;
}

static gboolean
hd_incoming_event_window_delete_event (GtkWidget   *widget,
                                       GdkEventAny *event)
{
  HDIncomingEventWindowPrivate *priv = HD_INCOMING_EVENT_WINDOW (widget)->priv;

  if (priv->timeout_id)
    {
      g_source_remove (priv->timeout_id);
      priv->timeout_id = 0;
    }

  g_signal_emit (widget, signals[RESPONSE], 0, GTK_RESPONSE_DELETE_EVENT);

  return TRUE;
}

static gboolean
hd_incoming_event_window_button_press_event (GtkWidget      *widget,
                                             GdkEventButton *event)
{
  HDIncomingEventWindowPrivate *priv = HD_INCOMING_EVENT_WINDOW (widget)->priv;

  if (priv->timeout_id)
    {
      g_source_remove (priv->timeout_id);
      priv->timeout_id = 0;
    }

  /* Emit the ::response signal */
  g_signal_emit (widget, signals[RESPONSE], 0, GTK_RESPONSE_OK);

  return TRUE;
}

static void
hd_incoming_event_window_realize (GtkWidget *widget)
{
  HDIncomingEventWindowPrivate *priv = HD_INCOMING_EVENT_WINDOW (widget)->priv;
  GdkDisplay *display;
  Atom atom;
  const gchar *notification_type;

  GTK_WIDGET_CLASS (hd_incoming_event_window_parent_class)->realize (widget);

  /* Notification window */
  gdk_window_set_type_hint (widget->window, GDK_WINDOW_TYPE_HINT_NOTIFICATION);

  /* Set the _NET_WM_WINDOW_TYPE property to _HILDON_WM_WINDOW_TYPE_HOME_APPLET */
  display = gdk_drawable_get_display (widget->window);
  atom = gdk_x11_get_xatom_by_name_for_display (display,
                                                "_HILDON_NOTIFICATION_TYPE");

  if (priv->preview)
    notification_type = "_HILDON_NOTIFICATION_TYPE_PREVIEW";
  else
    notification_type = "_HILDON_NOTIFICATION_TYPE_INCOMING_EVENT";

  XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window),
                   GDK_WINDOW_XID (widget->window),
                   atom, XA_STRING, 8, PropModeReplace,
                   (guchar *) notification_type,
                   strlen (notification_type));
}

static void
hd_incoming_event_window_size_request (GtkWidget      *widget,
                                       GtkRequisition *requisition)
{
  GTK_WIDGET_CLASS (hd_incoming_event_window_parent_class)->size_request (widget, requisition);

  requisition->width = INCOMING_EVENT_WINDOW_WIDTH;
  requisition->height = INCOMING_EVENT_WINDOW_HEIGHT;
}

static void
hd_incoming_event_window_dispose (GObject *object)
{
  HDIncomingEventWindowPrivate *priv = HD_INCOMING_EVENT_WINDOW (object)->priv;

  if (priv->timeout_id)
    {
      g_source_remove (priv->timeout_id);
      priv->timeout_id = 0;
    }

  G_OBJECT_CLASS (hd_incoming_event_window_parent_class)->dispose (object);
}

static void
hd_incoming_event_window_finalize (GObject *object)
{
  /* HDIncomingEventWindowPrivate *priv = HD_INCOMING_EVENT_WINDOW (object)->priv; */

  G_OBJECT_CLASS (hd_incoming_event_window_parent_class)->finalize (object);
}

static void
hd_incoming_event_window_get_property (GObject      *object,
                                       guint         prop_id,
                                       GValue       *value,
                                       GParamSpec   *pspec)
{
  HDIncomingEventWindowPrivate *priv = HD_INCOMING_EVENT_WINDOW (object)->priv;

  switch (prop_id)
    {
    case PROP_PREVIEW:
      g_value_set_boolean (value, priv->preview);
      break;

    case PROP_ICON:
        {
          const gchar *icon_name;
          
          gtk_image_get_icon_name (GTK_IMAGE (priv->icon), &icon_name, NULL);

          g_value_set_string (value, icon_name);
        }
      break;

    case PROP_TITLE:
      g_value_set_string (value, gtk_label_get_label (GTK_LABEL (priv->title)));
      break;

    case PROP_TIME:
      g_value_set_int64 (value, priv->time);
      break;

    case PROP_MESSAGE:
      g_value_set_string (value, gtk_label_get_label (GTK_LABEL (priv->message)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
hd_incoming_event_window_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  HDIncomingEventWindowPrivate *priv = HD_INCOMING_EVENT_WINDOW (object)->priv;

  switch (prop_id)
    {
    case PROP_PREVIEW:
      priv->preview = g_value_get_boolean (value);
      /* Close button is not shown in preview windows */
      if (priv->preview)
        gtk_widget_hide (priv->cbox);
      else
        gtk_widget_show (priv->cbox);
      break;

    case PROP_ICON:
      gtk_image_set_from_icon_name (GTK_IMAGE (priv->icon),
                                    g_value_get_string (value),
                                    0);
      break;

    case PROP_TITLE:
      gtk_label_set_text (GTK_LABEL (priv->title), g_value_get_string (value));
      break;

    case PROP_TIME:
        {
          gchar buf[20] = "";

          priv->time = g_value_get_int64 (value);
          if (priv->time >= 0)
            strftime (buf, 20, "%H:%M", localtime (&(priv->time)));
          gtk_label_set_text (GTK_LABEL (priv->time_label), buf);
        }
      break;

    case PROP_MESSAGE:
      gtk_label_set_text (GTK_LABEL (priv->message), g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
hd_incoming_event_window_class_init (HDIncomingEventWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  widget_class->button_press_event = hd_incoming_event_window_button_press_event;
  widget_class->delete_event = hd_incoming_event_window_delete_event;
  widget_class->map_event = hd_incoming_event_window_map_event;
  widget_class->realize = hd_incoming_event_window_realize;
  widget_class->size_request = hd_incoming_event_window_size_request;

  object_class->dispose = hd_incoming_event_window_dispose;
  object_class->finalize = hd_incoming_event_window_finalize;
  object_class->get_property = hd_incoming_event_window_get_property;
  object_class->set_property = hd_incoming_event_window_set_property;

  signals[RESPONSE] = g_signal_new ("response",
                                    G_OBJECT_CLASS_TYPE (klass),
                                    G_SIGNAL_RUN_LAST,
                                    G_STRUCT_OFFSET (HDIncomingEventWindowClass, response),
                                    NULL, NULL,
                                    g_cclosure_marshal_VOID__INT,
                                    G_TYPE_NONE, 1,
                                    G_TYPE_INT);

  g_object_class_install_property (object_class,
                                   PROP_PREVIEW,
                                   g_param_spec_boolean ("preview",
                                                         "Preview",
                                                         "If the window is a preview window",
                                                         FALSE,
                                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class,
                                   PROP_ICON,
                                   g_param_spec_string ("icon",
                                                        "Icon",
                                                        "The icon-name of the incoming event",
                                                        NULL,
                                                        G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_TITLE,
                                   g_param_spec_string ("title",
                                                        "Title",
                                                        "The title of the incoming event",
                                                        NULL,
                                                        G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                                   PROP_TIME,
                                   g_param_spec_int64 ("time",
                                                       "Time",
                                                       "The time of the incoming event (time_t)",
                                                       G_MININT64,
                                                       G_MAXINT64,
                                                       -1,
                                                       G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_MESSAGE,
                                   g_param_spec_string ("message",
                                                        "Message",
                                                        "The message of the incoming event",
                                                        NULL,
                                                        G_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (HDIncomingEventWindowPrivate));
}

static void
hd_incoming_event_window_init (HDIncomingEventWindow *window)
{
  HDIncomingEventWindowPrivate *priv = HD_INCOMING_EVENT_WINDOW_GET_PRIVATE (window);
  GtkWidget *vbox, *hbox, *title_box, *message_box, *fbox, *hsep;
  GtkSizeGroup *icon_size_group;

  window->priv = priv;

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);

  hbox = gtk_hbox_new (FALSE, MARGIN_DEFAULT);
  gtk_widget_show (hbox);

  title_box = gtk_hbox_new (FALSE, MARGIN_HALF);
  gtk_widget_show (title_box);

  message_box = gtk_hbox_new (FALSE, MARGIN_DEFAULT);
  gtk_widget_show (message_box);

  icon_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  priv->icon = gtk_image_new ();
  gtk_widget_show (priv->icon);
  gtk_image_set_pixel_size (GTK_IMAGE (priv->icon), INCOMING_EVENT_WINDOW_ICON);
  gtk_widget_set_size_request (priv->icon, INCOMING_EVENT_WINDOW_ICON, INCOMING_EVENT_WINDOW_ICON);
  gtk_size_group_add_widget (icon_size_group, priv->icon);

  /* fill box for the left empty space in the message row */
  fbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (fbox);
  gtk_size_group_add_widget (icon_size_group, fbox);

  priv->title = gtk_label_new (NULL);
  gtk_widget_show (priv->title);
  gtk_misc_set_alignment (GTK_MISC (priv->title), 0.0, 0.5);

  priv->time_label = gtk_label_new (NULL);
  gtk_widget_show (priv->time_label);

  /* fill box for the close button in the title row */
  priv->cbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (priv->cbox);
  gtk_widget_set_size_request (priv->cbox, INCOMING_EVENT_WINDOW_CLOSE, -1);

  priv->message = gtk_label_new (NULL);
  gtk_widget_show (priv->message);
  gtk_misc_set_alignment (GTK_MISC (priv->message), 0.0, 0.5);

  hsep = gtk_hseparator_new ();
  gtk_widget_show (hsep);

  /* Pack containers */
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hsep, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), message_box, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), priv->icon, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), title_box, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (title_box), priv->title, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (title_box), priv->time_label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (title_box), priv->cbox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (message_box), fbox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (message_box), priv->message, TRUE, TRUE, 0);

  /* Enable handling of button press events */
  gtk_widget_add_events (GTK_WIDGET (window), GDK_BUTTON_PRESS_MASK);
}

GtkWidget *
hd_incoming_event_window_new (gboolean     preview,
                              const gchar *summary,
                              const gchar *body,
                              time_t       time,
                              const gchar *icon)
{
  GtkWidget *window;

  window = g_object_new (HD_TYPE_INCOMING_EVENT_WINDOW,
                         "preview", preview,
                         "title", summary,
                         "message", body,
                         "time", (gint64) time,
                         "icon", icon,
                         NULL);

  return window;
}
