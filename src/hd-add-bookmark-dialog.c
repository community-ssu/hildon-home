/*
 * This file is part of hildon-desktop
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

#include <glib/gi18n.h>

#include "hd-bookmark-manager.h"

#include "hd-add-bookmark-dialog.h"

/* Pixel sizes */
#define ADD_BOOKMARK_DIALOG_WIDTH 342
#define ADD_BOOKMARK_DIALOG_HEIGHT 80

#define ADD_BOOKMARK_DIALOG_CLOSE  43
#define ADD_BOOKMARK_DIALOG_ICON  24

#define MARGIN_DEFAULT 8
#define MARGIN_HALF 4

/* Timeout in seconds */
#define ADD_BOOKMARK_DIALOG_PREVIEW_TIMEOUT 4

enum
{
  COL_NAME,
  COL_DIALOG,
  NUM_COLS
};

#define HD_ADD_BOOKMARK_DIALOG_GET_PRIVATE(object) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((object), HD_TYPE_ADD_BOOKMARK_DIALOG, HDAddBookmarkDialogPrivate))

struct _HDAddBookmarkDialogPrivate
{
  GtkWidget *selector;
};

G_DEFINE_TYPE (HDAddBookmarkDialog, hd_add_bookmark_dialog, HILDON_TYPE_PICKER_DIALOG);

static void
hd_add_bookmark_dialog_dispose (GObject *object)
{
  G_OBJECT_CLASS (hd_add_bookmark_dialog_parent_class)->dispose (object);
}

static void
hd_add_bookmark_dialog_finalize (GObject *object)
{
  /* HDAddBookmarkDialogPrivate *priv = HD_ADD_BOOKMARK_DIALOG (object)->priv; */

  G_OBJECT_CLASS (hd_add_bookmark_dialog_parent_class)->finalize (object);
}

static void
hd_add_bookmark_dialog_class_init (HDAddBookmarkDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = hd_add_bookmark_dialog_dispose;
  object_class->finalize = hd_add_bookmark_dialog_finalize;

  g_type_class_add_private (klass, sizeof (HDAddBookmarkDialogPrivate));
}

static void
response_cb (HDAddBookmarkDialog *dialog,
             gint             response_id,
             gpointer         data)
{
  HDAddBookmarkDialogPrivate *priv = dialog->priv;

  g_signal_handlers_disconnect_by_func (dialog, response_cb, data);

  g_debug ("response_cb called %d", response_id);

  /* Check if an item was selected */
  if (response_id == GTK_RESPONSE_OK)
    {
      GtkTreeIter iter;

      g_debug ("New bookmark was selected");

      if (hildon_touch_selector_get_selected (HILDON_TOUCH_SELECTOR (priv->selector),
                                              0,
                                              &iter))
        {
          hd_bookmark_manager_install_bookmark (hd_bookmark_manager_get (),
                                            &iter);
        }
    }
}

static void
hd_add_bookmark_dialog_init (HDAddBookmarkDialog *dialog)
{
  HDAddBookmarkDialogPrivate *priv = HD_ADD_BOOKMARK_DIALOG_GET_PRIVATE (dialog);
  HildonTouchSelectorColumn *column;
  GtkCellRenderer *renderer;

  dialog->priv = priv;

  /* Set dialog title */
  gtk_window_set_title (GTK_WINDOW (dialog), _("home_ti_select_bookmark"));

  /* */
  priv->selector = g_object_ref (hildon_touch_selector_new ());

  /* One Column */
  column = hildon_touch_selector_append_column (HILDON_TOUCH_SELECTOR (priv->selector),
                                                hd_bookmark_manager_get_model (hd_bookmark_manager_get ()),
                                                NULL);

  /* Add the icon renderer */
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column),
                              renderer,
                              FALSE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (column),
                                 renderer,
                                 "pixbuf", 3);

  /* Add the label renderer */
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column),
                              renderer,
                              FALSE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (column),
                                 renderer,
                                 "text", 0);

  hildon_picker_dialog_set_selector (HILDON_PICKER_DIALOG (dialog),
                                     HILDON_TOUCH_SELECTOR (priv->selector));

  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK (response_cb), NULL);
}

GtkWidget *
hd_add_bookmark_dialog_new (void)
{
  GtkWidget *window;

  window = g_object_new (HD_TYPE_ADD_BOOKMARK_DIALOG,
                         NULL);

  return window;
}

