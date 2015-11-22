/*
 ============================================================================
 Name        : ImageProcessor.c
 Author      : Nabheet Sandhu
 Version     :
 Copyright   : &copy; This is my copyright ...
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <gtk/gtk.h>
#include <gif_lib.h>
#include <cairo.h>
#include <queue>
#include "ProcessImage.h"
#include "ProcessGif.h"
#include "ProcessJpg.h"
#include "OcrNet.h"

cv::Mat *image = NULL;
std::map< int, std::set<ImageRegion> >* image_lines = NULL;
OcrNet *net = NULL;
char *characters;

// Signature definition
gboolean on_draw_area_expose (GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
void ImageHandler(char file_name[]);
int ProcessImageExtension(char file_name[]);

gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	return FALSE;
}

void destroy(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

std::string char_set = "ABCD";
const int num_rows = 5;
const int num_cols = 5;
const int num_hidden = 10;
const int num_output = char_set.length();

int main(int argc, char* argv[]) {
	GtkWidget *window = NULL, *draw_area = NULL, *scroll_window = NULL;
	GdkScreen* screen = NULL;
	int screen_width, screen_height;

//	char file_name[] = "/home/nabheet/Projects/ImageProcessor/images/test.gif";
//	char file_name[] = "/home/nabheet/Projects/ImageProcessor/images/ansi-ocr-cropped.gif";
//	char file_name[] = "/home/nabheet/Projects/ImageProcessor/images/OCR-A.gif";
	char file_name[] = "/home/nabheet/Projects/ImageProcessor/images/ansi-ocr.gif";
//	char file_name[] = "/home/nabheet/Projects/ImageProcessor/images/ocr-percent.gif";
//	char file_name[] = "/home/nabheet/Projects/ImageProcessor/images/IMG_0159.JPG";
//	char file_name[] = "/home/nabheet/Projects/ImageProcessor/images/IMG_0156.JPG";

	ImageHandler(file_name);

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	draw_area = gtk_drawing_area_new();
	scroll_window = gtk_scrolled_window_new(NULL, NULL);
	screen = gtk_window_get_screen(GTK_WINDOW(window));

	screen_width = gdk_screen_get_width(screen);
	screen_height = gdk_screen_get_height(screen);

	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

	gtk_container_set_border_width(GTK_CONTAINER (window), 10);
	gtk_window_set_default_size(GTK_WINDOW(window), screen_width, screen_height);
	gtk_widget_set_size_request(draw_area, image->cols, image->rows);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll_window), draw_area);
	gtk_container_add(GTK_CONTAINER (window), scroll_window);
	gtk_signal_connect(GTK_OBJECT (draw_area), "expose-event", GTK_SIGNAL_FUNC (on_draw_area_expose), NULL);

	gtk_widget_show_all(window);

	gtk_main();

	if(net != NULL)
		delete net;

	if(image_lines != NULL && image_lines->size() > 0)
		image_lines->clear();

	delete image_lines;
	delete [] characters;

	ProcessImage::release_image(&image);

	return EXIT_SUCCESS;
}

gboolean on_draw_area_expose (GtkWidget *widget,
		 GdkEventExpose *event,
		 gpointer user_data)
{
	std::map<int, std::set<ImageRegion> >::iterator image_lines_it;
	std::set<ImageRegion> regions;
	std::set<ImageRegion>::iterator regions_it;
	double x = 0, y = 0, width = 0, height = 0;
	int line_num = 1, item_num = 1, val_num = 0;
	cairo_t *cr = gdk_cairo_create(widget->window);
	unsigned char *image_buffer = image->data;
	char *value = new char[2];

	value[1] = '\0';

	gdk_draw_gray_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
			0, 0, image->cols, image->rows, GDK_RGB_DITHER_MAX,
			image_buffer, ((image->cols) * image->channels()));

	cairo_select_font_face (cr, "Georgia", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, 12);

	for(image_lines_it = image_lines->begin(); image_lines_it != image_lines->end(); image_lines_it++, line_num++)
	{
		std::stringstream line_ss;

		line_ss << line_num;

		regions = (std::set<ImageRegion>) image_lines_it->second;

		for(regions_it = regions.begin(), item_num = 1; regions_it != regions.end(); regions_it++, item_num++, val_num++)
		{
			std::stringstream item_ss;
			std::string s;

			x = regions_it->get_min_x();
			y = regions_it->get_min_y();
			width = regions_it->get_max_x() - regions_it->get_min_x();
			height = regions_it->get_max_y() - regions_it->get_min_y();

			item_ss << item_num;

			*value = characters[val_num];

			s = "(" + line_ss.str() + "," + item_ss.str() + ")=" + value;

			cairo_move_to(cr, x, y);
			cairo_set_source_rgb(cr, 0.0, 0.0, 255.0);
			cairo_show_text (cr, s.c_str());

			cairo_set_source_rgb(cr, 255.0, 0.0, 0.0);
			cairo_rectangle(cr, x, y, width, height); //cr ref, x, y, width, height
		}
	}

	cairo_stroke(cr);

	delete [] value;

	return TRUE;
}

void ImageHandler(char file_name[])
{
	std::map<int, std::set<ImageRegion> >::iterator image_lines_it;
	std::set<ImageRegion> regions;
	std::set<ImageRegion>::iterator regions_it;
	std::queue< char > char_queue;
	char value;

	ProcessImage *process_image = NULL;

	net = new OcrNet(num_output, num_rows, num_cols, num_hidden, num_output, 10000000, 0.0001);

	switch(ProcessImageExtension(file_name))
	{
	case 0:
		process_image = new ProcessGif;
		break;
	case 1:
		process_image = new ProcessJpg;
		break;
	default:
		std::cerr << "Unknown extension for input file" << std::endl;
		exit(1);
		break;
	}

	process_image->read_image(file_name, &image);
	image_lines = process_image->find_regions(*image);

	net->Train(*image, *image_lines, char_set);

	for(image_lines_it = image_lines->begin(); image_lines_it != image_lines->end(); image_lines_it++)
	{
		regions = (std::set<ImageRegion>) image_lines_it->second;

		for(regions_it = regions.begin(); regions_it != regions.end(); regions_it++)
		{
			value = net->Run(*image, *regions_it);
			char_queue.push(value);
		}
	}

	characters = new char[char_queue.size() + 1];

	characters[char_queue.size()] = '\0';

	for(int i = 0; char_queue.size() > 0; i++)
	{
		characters[i] = char_queue.front();
		char_queue.pop();
	}

	delete process_image;
}

int ProcessImageExtension(char file_name[])
{
	int length = 1, last_dot_index = 0, ret_val = -1;

	for(; file_name[length-1] != '\0'; length++)
	{
		if(file_name[length-1] == '.')
			last_dot_index = length - 1;
	}

	//.gif = 0
	//.GIF = 0
	//.jpeg = 1
	//.JPEG = 1
	//.jpg = 1
	//.JPG = 1
	switch(file_name[last_dot_index+1])
	{
	case 'G':
	case 'g':
		ret_val = 0;
		break;
	case 'j':
	case 'J':
		ret_val = 1;
		break;
	default:
		break;
	}

	return ret_val;
}
