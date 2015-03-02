/* ui_threshold.h
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

#define UI_THRESHOLD_COLOR_STRIP_WIDTH 16
#define UI_THRESHOLD_COLOR_STRIP_HEIGHT 300
#define UI_THRESHOLD_BAR_GRAPH_WIDTH 128
#define UI_THRESHOLD_TRIANGLE_WIDTH 15
#define UI_THRESHOLD_TRIANGLE_HEIGHT 15

/* ui_threshold data structures */

typedef enum {
  MAX_ARROW, MAX_ABSOLUTE, MAX_PERCENT, 
  MIN_ARROW, MIN_ABSOLUTE, MIN_PERCENT
} which_threshold_widget_t;

typedef struct ui_threshold_t {
  GnomeApp * app; /* pointer to the threshold window for this study */
  GnomeCanvas * color_strip;
  GnomeCanvasItem * color_strip_image;
  GnomeCanvasItem * min_arrow;
  GnomeCanvasItem * max_arrow;
  GnomeCanvasItem * bar_graph_item;
  GtkWidget * name_label;
  GtkWidget * max_percentage;
  GtkWidget * max_absolute;
  GtkWidget * min_percentage;
  GtkWidget * min_absolute;
  GtkWidget * color_table_menu;
  volume_t * volume; /* what volume this threshold corresponds to */
  guint reference_count;
} ui_threshold_t;

/* external functions */
ui_threshold_t * ui_threshold_free(ui_threshold_t * ui_threshold);
ui_threshold_t * ui_threshold_init(void);
void ui_threshold_update_entries(ui_threshold_t * ui_threshold);

