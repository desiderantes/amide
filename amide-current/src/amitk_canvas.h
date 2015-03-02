/* amitk_canvas.h
 *
 * Part of amide - Amide's a Medical Image Dataset Examiner
 * Copyright (C) 2002 Andy Loening
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


#ifndef __AMITK_CANVAS_H__
#define __AMITK_CANVAS_H__

/* includes we always need with this widget */
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gnome.h>
#include "amitk_study.h"

G_BEGIN_DECLS

#define AMITK_TYPE_CANVAS            (amitk_canvas_get_type ())
#define AMITK_CANVAS(obj)            (GTK_CHECK_CAST ((obj), AMITK_TYPE_CANVAS, AmitkCanvas))
#define AMITK_CANVAS_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), AMITK_TYPE_CANVAS, AmitkCanvasClass))
#define AMITK_IS_CANVAS(obj)         (GTK_CHECK_TYPE ((obj), AMITK_TYPE_CANVAS))
#define AMITK_IS_CANVAS_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), AMITK_TYPE_CANVAS))

#define AMITK_CANVAS_VIEW(obj)       (AMITK_CANVAS(obj)->view)
#define AMITK_CANVAS_PIXBUF(obj)     (AMITK_CANVAS(obj)->pixbuf)



typedef enum {
  AMITK_CANVAS_CROSS_ACTION_HIDE,
  AMITK_CANVAS_CROSS_ACTION_SHOW
} AmitkCanvasCrossAction;

typedef struct _AmitkCanvas             AmitkCanvas;
typedef struct _AmitkCanvasClass        AmitkCanvasClass;


struct _AmitkCanvas
{
  GtkVBox vbox;

  GtkWidget * canvas;
  GtkWidget * label;
  GtkWidget * scrollbar;
  GtkObject * scrollbar_adjustment;
  GnomeCanvasItem * arrows[4];
  gboolean with_arrows;

  AmitkVolume * volume; /* the volume that this canvas slice displays */
  AmitkPoint center; /* in base coordinate space */
  amide_real_t voxel_dim;
  amide_real_t zoom;
  amide_time_t start_time;
  amide_time_t duration;
  AmitkFuseType fuse_type;

  AmitkView view;
  AmitkLayout layout;
  gint roi_width;
  GdkLineStyle line_style;
  AmitkDataSet * active_ds;

  GList * slices;
  gint pixbuf_width, pixbuf_height;
  GnomeCanvasItem * image;
  GdkPixbuf * pixbuf;

  GList * objects;
  GList * object_items;

  guint next_update;
  guint idle_handler_id;
  GList * next_update_items;

  /* cross stuff */
  GnomeCanvasItem * cross[4];
  AmitkCanvasCrossAction next_cross_action;
  AmitkPoint next_cross_center;
  rgba_t next_cross_color;
  amide_real_t next_cross_thickness;

};

struct _AmitkCanvasClass
{
  GtkVBoxClass parent_class;
  
  void (* help_event)                (AmitkCanvas *Canvas,
				      AmitkHelpInfo which_help,
				      AmitkPoint *position,
				      amide_data_t value);
  void (* view_changing)             (AmitkCanvas *Canvas,
				      AmitkPoint *position,
				      amide_real_t thickness);
  void (* view_changed)              (AmitkCanvas *Canvas,
				      AmitkPoint *position,
				      amide_real_t thickness);
  void (* isocontour_3d_changed)     (AmitkCanvas *Canvas,
				      AmitkRoi * roi,
				      AmitkPoint *position);
  void (* erase_volume)              (AmitkCanvas *Canvas,
				      AmitkRoi *roi,
				      gboolean outside);
  void (* new_object)                (AmitkCanvas *Canvas,
				      AmitkObject * parent,
				      AmitkObjectType type,
				      AmitkPoint *position);
};  


GType         amitk_canvas_get_type           (void);
GtkWidget *   amitk_canvas_new                (AmitkStudy * study,
					       AmitkView view, 
					       AmitkLayout layout, 
					       GdkLineStyle line_style,
					       gint roi_width,
					       AmitkDataSet * active_ds,
					       gboolean with_arrows);
void          amitk_canvas_set_layout         (AmitkCanvas * canvas, 
					       AmitkLayout new_layout);
void          amitk_canvas_set_active_data_set(AmitkCanvas * canvas, 
					       AmitkDataSet * active_ds);
void          amitk_canvas_set_line_style     (AmitkCanvas * canvas, 
					       GdkLineStyle new_line_style);
void          amitk_canvas_set_roi_width      (AmitkCanvas * canvas, 
					       gint new_roi_width);
void          amitk_canvas_add_object         (AmitkCanvas * canvas, 
					       AmitkObject * object);
gboolean      amitk_canvas_remove_object      (AmitkCanvas * canvas, 
					       AmitkObject * object);
void          amitk_canvas_update_cross       (AmitkCanvas * canvas, 
					       AmitkCanvasCrossAction action, 
					       AmitkPoint center, 
					       rgba_t color, 
					       amide_real_t thickness);


G_END_DECLS


#endif /* __AMITK_CANVAS_H__ */
