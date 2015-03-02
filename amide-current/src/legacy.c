/* legacy.c - for loading in .xif files previous to the 2.0 format,
 * these files were generated by amide versions previous to 0.7.0
 *
 *
 * Part of amide - Amide's a Medical Image Dataset Examiner
 * Copyright (C) 2000-2009 Andy Loening
 *
 * Author: Andy Loening <loening@alum.mit.edu>
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


#include "legacy.h"

/* external variables */
static gchar * roi_type_names[] = {
  "Ellipsoid", 
  "Elliptic Cylinder", 
  "Box",
  "2D Isocontour",
  "3D Isocontour"
};
#define NUM_ROI_TYPES 5

static gchar * modality_names[] = {
  "PET",
  "SPECT", 
  "CT", 
  "MRI", 
  "Other"
};

#define NUM_MODALITIES 5

static gchar * threshold_names[] = {
  "per slice", 
  "per frame", 
  "interpolated between frames",
  "global"
};

#define LEGACY_COLOR_TABLE_NUM 22
gchar * color_table_legacy_names[] = {
  "black/white linear", 
  "white/black linear", 
  "black/white/black",
  "white/black/white",
  "red temperature", 
  "inverse red temp.", 
  "blue temperature", 
  "inv. blue temp.", 
  "green temperature", 
  "inv. green temp.", 
  "hot metal", 
  "inv. hot metal", 
  "hot blue", 
  "inverse hot blue", 
  "hot green", 
  "inverse hot green", 
  "spectrum", 
  "inverse spectrum", 
  "NIH + white", 
  "inv. NIH + white", 
  "NIH",
  "inverse NIH"
};

#define NUM_THRESHOLDS 4


static AmitkVoxel voxel3d_read_xml(xmlNodePtr nodes, gchar * descriptor, gchar **perror_buf) {

  gchar * temp_string;
  AmitkVoxel return_vp;
  gint x,y,z;
  gint error=0;

  return_vp = zero_voxel;

  temp_string = xml_get_string(nodes, descriptor);

  if (temp_string != NULL) {

    /* convert to a voxel */
    error = sscanf(temp_string,"%d\t%d\t%d", &x,&y,&z);
    g_free(temp_string);
    
    return_vp.x = x;
    return_vp.y = y;
    return_vp.z = z;
    return_vp.g = 1;
    return_vp.t = 1;

  } 

  if ((temp_string == NULL) || (error == EOF)) {
    return_vp.x = return_vp.y = return_vp.z = return_vp.g = return_vp.t = 0;
    amitk_append_str_with_newline(perror_buf,"Couldn't read value for %s, substituting [%d %d %d %d %d]",
				  descriptor, return_vp.x, return_vp.y, return_vp.z, return_vp.g, return_vp.t);
  }

  return return_vp;
}


/* function to load in a raw_data structure from a legacy file */
static AmitkRawData * data_set_load_xml(gchar * data_set_xml_filename, gchar **perror_buf) {

  xmlDocPtr doc;
  AmitkRawData * new_data_set;
  xmlNodePtr nodes;
  AmitkRawFormat i_raw_data_format, raw_data_format;
  gchar * temp_string;
  gchar * data_set_raw_filename;
  AmitkVoxel dim;

  /* parse the xml file */
  if ((doc = xmlParseFile(data_set_xml_filename)) == NULL) {
    amitk_append_str_with_newline(perror_buf,"Couldn't Parse AMIDE data_set xml file %s",
				  data_set_xml_filename);
    return NULL;
  }

  /* get the root of our document */
  if ((nodes = xmlDocGetRootElement(doc)) == NULL) {
    amitk_append_str_with_newline(perror_buf,"Data Set xml file doesn't appear to have a root: %s", 
				  data_set_xml_filename);
    return NULL;
  }

  /* get the document tree */
  nodes = nodes->children;

  /* figure out the data format */
  temp_string = xml_get_string(nodes, "raw_data_format");
#if (G_BYTE_ORDER == G_BIG_ENDIAN)
  raw_data_format = AMITK_RAW_FORMAT_DOUBLE_64_BE;
#else /* (G_BYTE_ORDER == G_LITTLE_ENDIAN) */
  raw_data_format = AMITK_RAW_FORMAT_DOUBLE_64_LE;
#endif
  if (temp_string != NULL)
    for (i_raw_data_format=0; i_raw_data_format < AMITK_RAW_FORMAT_NUM; i_raw_data_format++) 
      if (g_ascii_strcasecmp(temp_string, amitk_raw_format_legacy_names[i_raw_data_format]) == 0)
	raw_data_format = i_raw_data_format;
  g_free(temp_string);

  /* get the rest of the parameters */
  dim = amitk_voxel_read_xml(nodes, "dim", perror_buf);

  /* get the name of our associated data file */
  data_set_raw_filename = xml_get_string(nodes, "raw_data_file");

  /* now load in the raw data */
#ifdef AMIDE_DEBUG
  g_print("reading data from file %s\n", data_set_raw_filename);
#endif
  new_data_set = amitk_raw_data_import_raw_file(data_set_raw_filename, NULL, raw_data_format, 
						dim, 0, NULL, NULL);
   
  /* and we're done */
  g_free(data_set_raw_filename);
  xmlFreeDoc(doc);

  return new_data_set;
}

/* function to load in an alignment point xml file */
static AmitkFiducialMark * align_pt_load_xml(gchar * pt_xml_filename, gchar **perror_buf, AmitkSpace *space) {

  xmlDocPtr doc;
  AmitkFiducialMark * new_pt;
  AmitkPoint point;
  xmlNodePtr nodes;
  gchar * temp_string;


  /* parse the xml file */
  if ((doc = xmlParseFile(pt_xml_filename)) == NULL) {
    amitk_append_str_with_newline(perror_buf, "Couldn't Parse AMIDE alignment point xml file %s",
				  pt_xml_filename);
    return NULL;
  }

  /* get the root of our document */
  if ((nodes = xmlDocGetRootElement(doc)) == NULL) {
    amitk_append_str_with_newline(perror_buf,"AMIDE alignment point xml file doesn't appear to have a root: %s",
				  pt_xml_filename);
    return NULL;
  }

  new_pt = amitk_fiducial_mark_new();

  /* get the name */
  temp_string = xml_get_string(nodes->children, "text");
  if (temp_string != NULL) {
    amitk_object_set_name(AMITK_OBJECT(new_pt), temp_string);
    g_free(temp_string);
  }

  /* get the document tree */
  nodes = nodes->children;

  /* previous to version xif version 2.0, points were defined only with respect to their 
     parent's space*/
  amitk_space_copy_in_place(AMITK_SPACE(new_pt), space);

  /* the "point" option was eliminated in version 0.7.11, just 
     using the space's offset instead */
  point = amitk_point_read_xml(nodes, "point", perror_buf);
  point = amitk_space_s2b(AMITK_SPACE(new_pt), point);
  amitk_space_set_offset(AMITK_SPACE(new_pt), point);


  /* and we're done */
  xmlFreeDoc(doc);
  
  return new_pt;
}


/* function to load in a list of alignment point xml nodes */
static GList * align_pts_load_xml(xmlNodePtr node_list, gchar **perror_buf, AmitkSpace * space) {

  gchar * file_name;
  GList * new_pts;
  AmitkFiducialMark * new_pt;

  if (node_list != NULL) {
    /* first, recurse on through the list */
    new_pts = align_pts_load_xml(node_list->next, perror_buf, space);

    /* load in this node */
    file_name = xml_get_string(node_list->children, "text");
    new_pt = align_pt_load_xml(file_name, perror_buf, space);
    new_pts = g_list_prepend(new_pts, new_pt);
    g_free(file_name);

  } else
    new_pts = NULL;

  return new_pts;
}



/* function to load in a volume xml file */
static AmitkDataSet * volume_load_xml(gchar * volume_xml_filename, AmitkInterpolation interpolation, gchar ** perror_buf) {

  xmlDocPtr doc;
  AmitkDataSet * new_volume;
  xmlNodePtr nodes;
  xmlNodePtr pts_nodes;
  AmitkModality i_modality;
  AmitkColorTable i_color_table;
  AmitkThresholding i_thresholding;
  gchar * temp_string;
  gchar * scan_date;
  gchar * data_set_xml_filename;
  gchar * internal_scaling_xml_filename;
  GList * align_pts;
  AmitkSpace * space;
  AmitkViewMode i_view_mode;

  /* parse the xml file */
  if ((doc = xmlParseFile(volume_xml_filename)) == NULL) {
    amitk_append_str_with_newline(perror_buf,"Couldn't Parse AMIDE volume xml file %s",
				  volume_xml_filename);
    return NULL;
  }

  /* get the root of our document */
  if ((nodes = xmlDocGetRootElement(doc)) == NULL) {
    amitk_append_str_with_newline(perror_buf,"AMIDE volume xml file doesn't appear to have a root: %s", 
				  volume_xml_filename);
    return NULL;
  }

  new_volume = amitk_data_set_new(NULL, -1);
  new_volume->interpolation = interpolation;

  /* get the volume name */
  temp_string = xml_get_string(nodes->children, "text");
  if (temp_string != NULL) {
    amitk_object_set_name(AMITK_OBJECT(new_volume),temp_string);
    g_free(temp_string);
  }

  /* get the document tree */
  nodes = nodes->children;

  /* get the date the scan was made */
  scan_date = xml_get_string(nodes, "scan_date");
  amitk_data_set_set_scan_date(new_volume, scan_date);
  g_free(scan_date);

  /* figure out the modality */
  temp_string = xml_get_string(nodes, "modality");
  if (temp_string != NULL)
    for (i_modality=0; i_modality < NUM_MODALITIES; i_modality++) 
      if (g_ascii_strcasecmp(temp_string, modality_names[i_modality]) == 0)
	new_volume->modality = i_modality;
  g_free(temp_string);

  /* figure out the color table */
  temp_string = xml_get_string(nodes, "color_table");
  if (temp_string != NULL)
    for (i_color_table=0; i_color_table < LEGACY_COLOR_TABLE_NUM; i_color_table++) 
      if (g_ascii_strcasecmp(temp_string, color_table_legacy_names[i_color_table]) == 0)
	for (i_view_mode=0; i_view_mode < AMITK_VIEW_MODE_NUM; i_view_mode++)
	  new_volume->color_table[i_view_mode] = i_color_table;
  g_free(temp_string);

  /* load in our data set */
  data_set_xml_filename = xml_get_string(nodes, "data_set_file");
  internal_scaling_xml_filename = xml_get_string(nodes, "internal_scaling_file");
  if ((data_set_xml_filename != NULL) && (internal_scaling_xml_filename != NULL)) {
    new_volume->raw_data = data_set_load_xml(data_set_xml_filename, perror_buf);

    if (new_volume->internal_scaling_factor != NULL) {
      g_object_unref(new_volume->internal_scaling_factor);
      new_volume->internal_scaling_factor = NULL;
    }
    new_volume->internal_scaling_factor = data_set_load_xml(internal_scaling_xml_filename, perror_buf);

    /* the type of internal_scaling has been changed to double
       as of amide 0.7.1 */
    if (new_volume->internal_scaling_factor->format != AMITK_FORMAT_DOUBLE) {
      AmitkRawData * old_scaling;
      AmitkVoxel i;

      amitk_append_str_with_newline(perror_buf,"wrong type found on internal scaling, converting to double");
      old_scaling = new_volume->internal_scaling_factor;

      new_volume->internal_scaling_factor = amitk_raw_data_new_with_data(AMITK_FORMAT_DOUBLE, old_scaling->dim);
      if (new_volume->internal_scaling_factor == NULL) {
	amitk_append_str_with_newline(perror_buf,"couldn't allocate memory space for the new scaling structure");
	amitk_object_unref(new_volume);
	return NULL;
      }
      
      for (i.t=0; i.t<new_volume->internal_scaling_factor->dim.t; i.t++)
	for (i.g=0; i.g<new_volume->internal_scaling_factor->dim.g; i.g++)
	  for (i.z=0; i.z<new_volume->internal_scaling_factor->dim.z; i.z++)
	    for (i.y=0; i.y<new_volume->internal_scaling_factor->dim.y; i.y++)
	      for (i.x=0; i.x<new_volume->internal_scaling_factor->dim.x; i.x++)
		AMITK_RAW_DATA_DOUBLE_SET_CONTENT(new_volume->internal_scaling_factor,i) = 
		  amitk_raw_data_get_value(old_scaling, i);
      
      g_object_unref(old_scaling);
    }


    /* parameters that aren't in older versions and default values aren't good enough*/
    amitk_data_set_set_scale_factor(new_volume, xml_get_data(nodes, "external_scaling", perror_buf));

  } else {
    /* ---- legacy cruft previous to .xif version 1.4 ----- */

    gchar * raw_data_filename;
    AmitkRawFormat i_raw_data_format, raw_data_format;
    AmitkVoxel temp_dim;

    amitk_append_str_with_newline(perror_buf,"no data_set file, will continue with the assumption of a .xif format previous to 1.4");

    /* get the name of our associated data file */
    raw_data_filename = xml_get_string(nodes, "raw_data");

    /* and figure out the data format */
    temp_string = xml_get_string(nodes, "data_format");
#if (G_BYTE_ORDER == G_BIG_ENDIAN)
    raw_data_format = AMITK_RAW_FORMAT_DOUBLE_64_BE; 
#else /* (G_BYTE_ORDER == G_LITTLE_ENDIAN) */
    raw_data_format = AMITK_RAW_FORMAT_DOUBLE_64_LE; 
#endif
    if (temp_string != NULL)
      for (i_raw_data_format=0; i_raw_data_format < AMITK_RAW_FORMAT_NUM; i_raw_data_format++) 
	if (g_ascii_strcasecmp(temp_string, amitk_raw_format_legacy_names[i_raw_data_format]) == 0)
	  raw_data_format = i_raw_data_format;
    g_free(temp_string);

    temp_dim = voxel3d_read_xml(nodes, "dim", perror_buf);
    temp_dim.t = xml_get_int(nodes, "num_frames", perror_buf);
    amitk_data_set_set_scale_factor(new_volume,  xml_get_data(nodes, "conversion", perror_buf));
    
    /* now load in the raw data */
    new_volume->raw_data = amitk_raw_data_import_raw_file(raw_data_filename, NULL, raw_data_format, 
							  temp_dim, 0, NULL, NULL);
    
    g_free(raw_data_filename);
    /* -------- end legacy cruft -------- */
  }

  /* figure out the rest of the parameters */
  new_volume->voxel_size = amitk_point_read_xml(nodes, "voxel_size", perror_buf);
  new_volume->scan_start = xml_get_time(nodes, "scan_start", perror_buf);
  new_volume->frame_duration = xml_get_times(nodes, "frame_duration", 
					     AMITK_DATA_SET_NUM_FRAMES(new_volume),
					     perror_buf);
  new_volume->threshold_max[0] =  xml_get_data(nodes, "threshold_max", perror_buf);
  new_volume->threshold_min[0] =  xml_get_data(nodes, "threshold_min", perror_buf);
  new_volume->threshold_max[1] =  xml_get_data(nodes, "threshold_max_1", perror_buf);
  new_volume->threshold_min[1] =  xml_get_data(nodes, "threshold_min_1", perror_buf);
  new_volume->threshold_ref_frame[0] = xml_get_int(nodes,"threshold_ref_frame_0", perror_buf);
  new_volume->threshold_ref_frame[1] = xml_get_int(nodes,"threshold_ref_frame_1", perror_buf);

  space = amitk_space_read_xml(nodes, "coord_frame", perror_buf);
  amitk_space_copy_in_place(AMITK_SPACE(new_volume), space);
  g_object_unref(space);

  /* figure out the thresholding */
  temp_string = xml_get_string(nodes, "threshold_type");
  if (temp_string != NULL)
    for (i_thresholding=0; i_thresholding < NUM_THRESHOLDS; i_thresholding++) 
      if (g_ascii_strcasecmp(temp_string, threshold_names[i_thresholding]) == 0)
	new_volume->thresholding = i_thresholding;
  g_free(temp_string);

  /* figure out the scaling type */
  if (new_volume->internal_scaling_factor->dim.z > 1) 
    new_volume->scaling_type = AMITK_SCALING_TYPE_2D;
  else if (new_volume->internal_scaling_factor->dim.t > 1)
    new_volume->scaling_type = AMITK_SCALING_TYPE_1D;
  else 
    new_volume->scaling_type = AMITK_SCALING_TYPE_0D;

  /* recalc the temporary parameters */
  amitk_data_set_calc_far_corner(new_volume);

  /* and load in any alignment points */
  pts_nodes = xml_get_node(nodes, "Alignment_points");
  if (pts_nodes != NULL) {
    pts_nodes = pts_nodes->children;
    if (pts_nodes != NULL) {
      align_pts = align_pts_load_xml(pts_nodes, perror_buf, AMITK_SPACE(new_volume));
      amitk_object_add_children(AMITK_OBJECT(new_volume), align_pts);
      amitk_objects_unref(align_pts);
    }
  }

  /* and we're done */
  xmlFreeDoc(doc);
  
  return new_volume;
}


/* function to load in a list of volume xml nodes */
static GList * volumes_load_xml(xmlNodePtr node_list, AmitkInterpolation interpolation, gchar **perror_buf) {

  gchar * file_name;
  GList * new_data_sets;
  AmitkDataSet * new_ds;

  if (node_list != NULL) {
    /* first, recurse on through the list */
    new_data_sets = volumes_load_xml(node_list->next, interpolation, perror_buf);

    /* load in this node */
    file_name = xml_get_string(node_list->children, "text");
    new_ds = volume_load_xml(file_name, interpolation, perror_buf);
    new_data_sets = g_list_prepend(new_data_sets, new_ds);
    g_free(file_name);

  } else
    new_data_sets = NULL;

  return new_data_sets;
}


/* function to load in an ROI xml file */
AmitkRoi * roi_load_xml(gchar * roi_xml_filename, gchar **perror_buf) {

  xmlDocPtr doc;
  AmitkRoi * new_roi;
  xmlNodePtr nodes;
  AmitkRoiType i_roi_type;
  gchar * temp_string;
  gchar * isocontour_xml_filename;
  AmitkSpace * space;


  /* parse the xml file */
  if ((doc = xmlParseFile(roi_xml_filename)) == NULL) {
    amitk_append_str_with_newline(perror_buf,"Couldn't Parse AMIDE ROI xml file %s",roi_xml_filename);
    return NULL;
  }

  /* get the root of our document */
  if ((nodes = xmlDocGetRootElement(doc)) == NULL) {
    amitk_append_str_with_newline(perror_buf,"AMIDE ROI xml file doesn't appear to have a root: %s", roi_xml_filename);
    return NULL;
  }
  
  new_roi = amitk_roi_new(0);

  /* get the roi name */
  temp_string = xml_get_string(nodes->children, "text");
  if (temp_string != NULL) {
    amitk_object_set_name(AMITK_OBJECT(new_roi),temp_string);
    g_free(temp_string);
  }

  /* get the document tree */
  nodes = nodes->children;

  /* figure out the type */
  temp_string = xml_get_string(nodes, "type");
  if (temp_string != NULL)
    for (i_roi_type=0; i_roi_type < NUM_ROI_TYPES; i_roi_type++) 
      if (g_ascii_strcasecmp(temp_string, roi_type_names[i_roi_type]) == 0)
	new_roi->type = i_roi_type;
  g_free(temp_string);

  /* and figure out the rest of the parameters */
  space = amitk_space_read_xml(nodes, "coord_frame", perror_buf);
  amitk_space_copy_in_place(AMITK_SPACE(new_roi), space);
  g_object_unref(space);

  amitk_volume_set_corner(AMITK_VOLUME(new_roi), amitk_point_read_xml(nodes, "corner", perror_buf));

  /* isocontour specific stuff */
  if (AMITK_ROI_TYPE_ISOCONTOUR(new_roi)) {
    new_roi->voxel_size = amitk_point_read_xml(nodes, "voxel_size", perror_buf);
    new_roi->isocontour_min_value = xml_get_real(nodes, "isocontour_value", perror_buf);

    isocontour_xml_filename = xml_get_string(nodes, "isocontour_file");
    if (isocontour_xml_filename != NULL)
      new_roi->map_data = data_set_load_xml(isocontour_xml_filename, perror_buf);
  }

  /* children were never used */

  /* make sure to mark the roi as undrawn if needed */
  if (AMITK_ROI_TYPE_ISOCONTOUR(new_roi)) {
    if (new_roi->map_data == NULL) 
      AMITK_VOLUME(new_roi)->valid = FALSE;
  } else {
    if (POINT_EQUAL(AMITK_VOLUME_CORNER(new_roi), zero_point)) {
      AMITK_VOLUME(new_roi)->valid = FALSE;
    }
  }
   
  /* and we're done */
  xmlFreeDoc(doc);

  return new_roi;
}


/* function to load in a list of ROI xml nodes */
static GList * rois_load_xml(xmlNodePtr node_list, gchar **perror_buf) {

  gchar * roi_xml_filename;
  GList * new_rois;
  AmitkRoi * new_roi;

  if (node_list != NULL) {
    /* first, recurse on through the list */
    new_rois = rois_load_xml(node_list->next, perror_buf);

    /* load in this node */
    roi_xml_filename = xml_get_string(node_list->children, "text");
    new_roi = roi_load_xml(roi_xml_filename, perror_buf);
    new_rois = g_list_prepend(new_rois, new_roi);
    g_free(roi_xml_filename);
  } else
    new_rois = NULL;

  return new_rois;
}

AmitkStudy * legacy_load_xml(gchar ** perror_buf) {

  AmitkStudy * study = NULL;
  AmitkSpace * space;
  AmitkPoint view_center;
  AmitkInterpolation interpolation;
  xmlDocPtr doc;
  xmlNodePtr nodes;
  xmlNodePtr object_nodes;
  gchar * temp_string;
  gchar * file_version;
  gchar * creation_date;
  GList * objects;
  

  /* warn that this is an old file version */
  amitk_append_str_with_newline(perror_buf,"A .xif file previous to file version 2.0 found.\n"
				"Invoking legacy loader, please resave file as soon as possible");


  /* parse the xml file */
  if ((doc = xmlParseFile("Study.xml")) == NULL) {
    amitk_append_str_with_newline(perror_buf,"Couldn't Parse AMIDE xml file:\n\tStudy.xml");
    return NULL;
  }

  /* get the root of our document */
  if ((nodes = xmlDocGetRootElement(doc)) == NULL) {
    amitk_append_str_with_newline(perror_buf,"AMIDE xml file doesn't appear to have a root:\n\tStudy.xml");
    return NULL;
  }

  study = amitk_study_new(NULL);

  /* get the study name */
  temp_string = xml_get_string(nodes->children, "text");
  if (temp_string != NULL) {
    amitk_object_set_name(AMITK_OBJECT(study),temp_string);
    g_free(temp_string);
  }

  /* get the document tree */
  nodes = nodes->children;

  /* get the version of the data file */
  file_version = xml_get_string(nodes, "AMIDE_Data_File_Version");


  /* get the creation date of the study */
  creation_date = xml_get_string(nodes, "creation_date");
  amitk_study_set_creation_date(study, creation_date);
  g_free(creation_date);

  /* get our study parameters */
  space = amitk_space_read_xml(nodes, "coord_frame", perror_buf);
  amitk_space_copy_in_place(AMITK_SPACE(study), space);
  g_object_unref(space);

  /* figure out the interpolation */
  temp_string = xml_get_string(nodes, "interpolation");
  interpolation = AMITK_INTERPOLATION_NEAREST_NEIGHBOR;
  if (temp_string != NULL)
    if (g_ascii_strcasecmp(temp_string, "Trilinear") == 0)
      interpolation = AMITK_INTERPOLATION_TRILINEAR;
  g_free(temp_string);

  /* load in the volumes */
  object_nodes = xml_get_node(nodes, "Volumes");
  object_nodes = object_nodes->children;
  objects = volumes_load_xml(object_nodes, interpolation, perror_buf);
  amitk_object_add_children(AMITK_OBJECT(study), objects);
  amitk_objects_unref(objects);

  /* load in the rois */
  object_nodes = xml_get_node(nodes, "ROIs");
  object_nodes = object_nodes->children;
  objects = rois_load_xml(object_nodes, perror_buf);
  amitk_object_add_children(AMITK_OBJECT(study), objects);
  amitk_objects_unref(objects);

  /* get our view parameters */
  view_center = amitk_point_read_xml(nodes, "view_center", perror_buf);
  amitk_study_set_view_center(study, amitk_space_s2b(AMITK_SPACE(study), view_center));
  amitk_study_set_view_thickness(study, xml_get_real(nodes, "view_thickness", perror_buf));
  amitk_study_set_view_start_time(study, xml_get_time(nodes, "view_time", perror_buf));
  amitk_study_set_view_duration(study, xml_get_time(nodes, "view_duration", perror_buf));
  amitk_study_set_zoom(study, xml_get_real(nodes, "zoom", perror_buf));
 
  /* sanity check */
  if (AMITK_STUDY_ZOOM(study) < EPSILON) {
    amitk_append_str_with_newline(perror_buf,"inappropriate zoom (%5.3f) for study, reseting to 1.0",
				  AMITK_STUDY_ZOOM(study));
    amitk_study_set_zoom(study, 1.0);
  }

  /* and we're done */
  xmlFreeDoc(doc);
    
  /* legacy cruft, rip out at some point in the future */
  /* compensate for errors in old versions of amide */
  if (g_ascii_strcasecmp(file_version, "1.3") < 0) {
    GList * objects;
    AmitkObject * object;
    AmitkPoint new_axes[AMITK_AXIS_NUM];
    AmitkPoint new_offset;
    AmitkAxis i_axis;

    amitk_append_str_with_newline(perror_buf,"detected file version previous to 1.3, compensating for coordinate errors");

    view_center = AMITK_STUDY_VIEW_CENTER(study);
    view_center.y = -view_center.y;
    amitk_study_set_view_center(study,view_center);

    objects = AMITK_OBJECT_CHILDREN(study);
    while (objects != NULL) {
      object = objects->data;


      for (i_axis=0;i_axis<AMITK_AXIS_NUM;i_axis++)
	new_axes[i_axis] = amitk_space_get_axis(AMITK_SPACE(object), i_axis);
      new_axes[AMITK_AXIS_X].y = -new_axes[AMITK_AXIS_X].y;
      new_axes[AMITK_AXIS_Y].y = -new_axes[AMITK_AXIS_Y].y;
      new_axes[AMITK_AXIS_Z].y = -new_axes[AMITK_AXIS_Z].y;
      amitk_space_set_axes(AMITK_SPACE(object), new_axes, AMITK_SPACE_OFFSET(object));

      new_offset = AMITK_SPACE_OFFSET(object);
      new_offset.y = -new_offset.y;
      amitk_space_set_offset(AMITK_SPACE(object), new_offset);

      objects = objects->next;
    }

  }

  /* freeing up anything we haven't freed yet */
  g_free(file_version);


  return study;

}


