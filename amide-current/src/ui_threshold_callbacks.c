/* ui_threshold_callbacks.c
 *
 * Part of amide - Amide's a Medical Image Dataset Examiner
 * Copyright (C) 2001 Andy Loening
 *
 * Author: Andy Loening <loening@ucla.edu>
 */

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
  02111-1307, USA.
*/

#include "config.h"
#include <gnome.h>
#include "amide.h"
#include "volume.h"
#include "roi.h"
#include "study.h"
#include "rendering.h"
#include "image.h"
#include "ui_threshold.h"
#include "ui_series.h"
#include "ui_roi.h"
#include "ui_volume.h"
#include "ui_study.h"
#include "ui_threshold2.h"
#include "ui_series2.h"
#include "ui_threshold_callbacks.h"


/* function called when the max or min triangle is moved 
 * mostly taken from Pennington's fine book */
gint ui_threshold_callbacks_arrow(GtkWidget* widget, 
				  GdkEvent * event,
				  gpointer data) {

  ui_threshold_t * ui_threshold = data;
  ui_study_t * ui_study;
  gdouble item_x, item_y;
  GdkCursor * cursor;
  volume_data_t temp;
  volume_t * volume;
  which_threshold_widget_t which_threshold_widget;
  
  /* get the location of the event, and convert it to our coordinate system */
  item_x = event->button.x;
  item_y = event->button.y;
  gnome_canvas_item_w2i(GNOME_CANVAS_ITEM(widget)->parent, &item_x, &item_y);

  volume = ui_threshold->volume;

  /* get a pointer to ui_study so we can update the canvas */
  ui_study = gtk_object_get_data(GTK_OBJECT(widget), "ui_study");

  /* figure out which of the arrows triggered the callback */
  which_threshold_widget = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(widget), "type"));

  /* switch on the event which called this */
  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      cursor = gdk_cursor_new(GDK_SB_V_DOUBLE_ARROW);
      gnome_canvas_item_grab(GNOME_CANVAS_ITEM(widget),
			     GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
			     cursor,
			     event->button.time);
      gdk_cursor_destroy(cursor);
      break;

    case GDK_MOTION_NOTIFY:
      if (event->motion.state & GDK_BUTTON1_MASK) {
	temp = ((UI_THRESHOLD_COLOR_STRIP_HEIGHT - item_y) /
		UI_THRESHOLD_COLOR_STRIP_HEIGHT) *
	  (volume->max - volume->min)+volume->min;

	switch (which_threshold_widget) 
	  {
	  case MAX_ARROW:
	    if (temp <= volume->threshold_min) 
	      temp = volume->threshold_min;
	    if (temp < 0.0)
	      temp = 0.0;
	    volume->threshold_max = temp;
	    break;
	  case MIN_ARROW:
	  default:
	    if (temp < volume->min) 
	      temp = volume->min;
	    if (temp >= volume->threshold_max)
	      temp = volume->threshold_max;
	    if (temp >= volume->max)
	      temp = volume->max;
	    volume->threshold_min = temp;
	    break;
	  }

	ui_threshold_update_canvas(ui_study, ui_threshold);
	ui_threshold_update_entries(ui_threshold);   /* reset the entry widgets */
      }
      break;

    case GDK_BUTTON_RELEASE:
      gnome_canvas_item_ungrab(GNOME_CANVAS_ITEM(widget), event->button.time);
      ui_study_update_canvas(ui_study, NUM_VIEWS, REFRESH_IMAGE);
      ui_threshold_update_entries(ui_threshold);   /* reset the entry widgets */
      /* and if we have a series up, update that */
      if (ui_study->series != NULL)
	ui_series_update_canvas_image(ui_study);
      break;

    default:
      break;
    }
      
  return FALSE;
}



void ui_threshold_callbacks_entry(GtkWidget* widget, gpointer data) {

  ui_threshold_t * ui_threshold = data;
  ui_study_t * ui_study;
  gchar * string;
  gint error;
  gdouble temp;
  volume_t * volume;
  which_threshold_widget_t which_threshold_widget;
  gboolean update = FALSE;

  volume = ui_threshold->volume;

  /* get a pointer to ui_study so we can update the canvas */
  ui_study = gtk_object_get_data(GTK_OBJECT(widget), "ui_study");

  /* figure out which of the arrows triggered the callback */
  which_threshold_widget = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(widget), "type"));

  /* get the current entry */
  string = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);

  /* convert it to a floating point */
  error = sscanf(string, "%lf", &temp);
  g_free(string);
    
  /* make sure it's a valid floating point */
  if (!(error == EOF)) {
    switch (which_threshold_widget) 
      {
      case MAX_PERCENT:
	temp = (volume->max-volume->min) * temp/100.0;
      case MAX_ABSOLUTE:
	if ((temp > 0.0) && (temp > volume->threshold_min)) {
	  volume->threshold_max = temp;
	  update = TRUE;
	}
	break;
      case MIN_PERCENT:
	temp = ((volume->max-volume->min) * temp/100.0)+volume->min;
      case MIN_ABSOLUTE:
	if ((temp < volume->max) && (temp >= volume->min) && (temp < volume->threshold_max)) {
	  volume->threshold_min = temp;
	  update = TRUE;
	}
      default:
	break;
      }
  }

  /* if we had a valid change, update the canvases */
  if (update) {
    ui_threshold_update_canvas(ui_study, ui_threshold);
    ui_study_update_canvas(ui_study, NUM_VIEWS, REFRESH_IMAGE);
    /* and if we have a series up, update that */
    if (ui_study->series != NULL) 
    ui_series_update_canvas_image(ui_study);
  }

  /* reset the entry widgets */
  ui_threshold_update_entries(ui_threshold);

  return;
}




/* function to run for a delete_event */
void ui_threshold_callbacks_delete_event(GtkWidget* widget, GdkEvent * event, gpointer data) {

  ui_study_t * ui_study = data;

  /* destroy the widget */
  gtk_widget_destroy(widget);

  /* free the associated data structure */
  ui_study->threshold = ui_threshold_free(ui_study->threshold);

  return;
}