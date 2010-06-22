/* gtkgui.c
   This file is part of Redshift.

   Redshift is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Redshift is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Redshift.  If not, see <http://www.gnu.org/licenses/>.

   Copyright (c) 2010  Jon Lund Steffensen <jonlst@gmail.com>
*/

#include "common.h"
#include <gtk/gtk.h>
#include "options.h"

//// Modal dialog to pick/search location
//static void setloc_dialog( GtkWidget *widget, gpointer data)
//{
//	GtkWidget *loc_dialog, *table;
//	GtkWidget *image;
//	// Labels and Entries
//	GtkWidget *help_txt,*address_ttl, *address_edt,
//			*lat_ttl,*lat_edt,*lon_ttl,*lon_edt;
//	// Buttons
//	GtkWidget *search, *done;
//
//	loc_dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//	gtk_window_set_title(GTK_WINDOW(loc_dialog), "Location Dialog");
//	gtk_window_set_modal(GTK_WINDOW(loc_dialog), TRUE);
//	gtk_window_set_transient_for(
//			GTK_WINDOW(gtk_widget_get_toplevel(widget)),
//			GTK_WINDOW(loc_dialog));
//
//	table = gtk_table_new(5,5,FALSE);
//	gtk_container_add(GTK_CONTAINER(loc_dialog),table);
//	gtk_container_set_border_width(GTK_CONTAINER(loc_dialog), 5);
//
//	help_txt = gtk_label_new("Enter an address/zip to search for Lat/Lon\n"
//			"Or enter your Latitude/Longitude directly.");
//	gtk_table_attach_defaults(GTK_TABLE(table),help_txt,0,4,0,1);
//	address_ttl = gtk_label_new("Address:");
//	gtk_table_attach_defaults(GTK_TABLE(table),address_ttl,0,1,1,2);
//	address_edt = gtk_entry_new();
//	gtk_table_attach_defaults(GTK_TABLE(table),address_edt,1,4,1,2);
//	search = gtk_button_new_with_label("Search");
//	gtk_table_attach_defaults(GTK_TABLE(table),search,4,5,1,2);
//	lat_ttl = gtk_label_new("Latitude:");
//	gtk_table_attach_defaults(GTK_TABLE(table),lat_ttl,0,1,2,3);
//	lat_edt = gtk_entry_new();
//	gtk_table_attach_defaults(GTK_TABLE(table),lat_edt,1,2,2,3);
//	lon_ttl = gtk_label_new("Longitude:");
//	gtk_table_attach_defaults(GTK_TABLE(table),lon_ttl,2,3,2,3);
//	lon_edt = gtk_entry_new();
//	gtk_table_attach_defaults(GTK_TABLE(table),lon_edt,3,4,2,3);
//	image = gtk_image_new_from_file("staticmap.png");
//	gtk_table_attach_defaults(GTK_TABLE(table),image,0,5,3,4);
//	done = gtk_button_new_with_label("Done");
//	gtk_table_attach_defaults(GTK_TABLE(table),done,4,5,4,5);
//	gtk_widget_show_all(loc_dialog);
//}
//
//// Creates a display item in the form of Title: Text
//void create_disp_item(GtkWidget *ttl, GtkWidget *txt, GtkWidget *tbl,
//		const char ttl_str[], const char txt_str[], int row)
//{
//	const int cs = 1; // Starting column
//	const int rs = 0; // Starting row
//	ttl = gtk_label_new(ttl_str);
//	gtk_misc_set_alignment(GTK_MISC(ttl),0.0,0.5);
//	gtk_table_attach_defaults(GTK_TABLE(tbl),ttl,
//			cs,cs+1,rs+row,rs+row+1);
//	txt = gtk_label_new(txt_str);
//	gtk_misc_set_alignment(GTK_MISC(txt),0.0,0.5);
//	gtk_table_attach_defaults(GTK_TABLE(tbl),txt,
//			cs+1,cs+2,rs+row,rs+row+1);
//}
//
//// Main GUI code
//void redshift_gui(int argc, char *argv[])
//{
//	// Containers
//	GtkWidget *window, *table;
//
//	GtkWidget *sun_pos;
//	GtkWidget *temp_slider;
//
//	// Labels
//	GtkWidget *lat_ttl,*lat_txt,
//		*lon_ttl, *lon_txt,
//		*temp_day_ttl, *temp_day_txt,
//		*temp_night_ttl, *temp_night_txt;
//
//	// Buttons
//	GtkWidget *loc_button, *settings_button;
//
//	const int bufsize=512;
//	char buffer[bufsize];
//
//	gtk_init(&argc, &argv);
//
//	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//	gtk_window_set_title(GTK_WINDOW(window), "Redshift GUI");
//	gtk_window_set_default_size(GTK_WINDOW(window), 400,150);
//	gtk_window_set_gravity(GTK_WINDOW(window), GDK_GRAVITY_SOUTH_EAST);
//	gtk_container_set_border_width(GTK_CONTAINER(window), 5);
//
//	table = gtk_table_new(8,4,FALSE);
//	gtk_container_add(GTK_CONTAINER(window), table);
//	
//	// Information display
//	snprintf(buffer, bufsize, "%.2f", opts->lat);
//	create_disp_item(lat_ttl,lat_txt,table,"Latitude:",buffer,0);
//
//	snprintf(buffer, bufsize, "%.2f", opts->lon);
//	create_disp_item(lon_ttl,lon_txt,table,"Longitude:",buffer,1);
//
//	snprintf(buffer, bufsize, "%d° K", opts->temp_day);
//	create_disp_item(temp_day_ttl,temp_day_txt,table,"Day Temp:",buffer,2);
//
//	snprintf(buffer, bufsize, "%d° K", opts->temp_night);
//	create_disp_item(temp_night_ttl,temp_night_txt,table,"Night Temp:",buffer,3);
//
//	// Configuration Buttons
//	loc_button = gtk_button_new_with_label("Locate");
//	g_signal_connect(G_OBJECT(loc_button), "clicked",G_CALLBACK(setloc_dialog),
//			(gpointer) opts);
//
//	gtk_table_attach_defaults(GTK_TABLE(table),loc_button,3,4,0,1);
//	
//	g_signal_connect_swapped(G_OBJECT(window), "destroy",
//			G_CALLBACK(gtk_main_quit), NULL);
//
//	gtk_widget_show_all(window);
//	gtk_main();
//}
//
int gtk_gui(int argc, char *argv[]){
	return 0;
}
