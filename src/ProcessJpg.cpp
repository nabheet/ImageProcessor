/*
 * ProcessJpg.cpp
 *
 *  Created on: Mar 13, 2011
 *      Author: nabheet
 */

#include "ProcessJpg.h"

void ProcessJpg::read_image(char file_name[], cv::Mat **image)
{
	FILE *file = NULL;
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	ExifShort exif_orientation = read_exif_info(file_name);
	unsigned char *image_buffer, *ptr;
	int components = 0, image_height = 0, image_width = 0;

	if ((file = fopen(file_name, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", file_name);
		exit(1);
	}

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo, file);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);

	image_width = cinfo.image_width;
	image_height = cinfo.image_height;
	components = cinfo.output_components;

	image_buffer = new unsigned char[image_width * image_height * components];

	(*image) = new cv::Mat(cv::Size(image_width, image_height), CV_8UC3, image_buffer);

	image_buffer = (*image)->data;

	ptr = image_buffer;

	for(uint i=0; (i < cinfo.output_height); i++, ptr += (components * cinfo.output_width))
	{
		jpeg_read_scanlines(&cinfo, (JSAMPLE **) &ptr, 1);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	fclose(file);

	fix_orientation(exif_orientation, image);
	convert_to_binary(image);
}

ExifShort ProcessJpg::read_exif_info(char file_name[])
{
	ExifData * data = exif_data_new_from_file(file_name);

	ExifEntry *entry = exif_content_get_entry(data->ifd[0],EXIF_TAG_ORIENTATION);

	ExifShort ret_val = exif_get_short(entry->data, exif_data_get_byte_order(data));

	exif_data_free(data);

	return ret_val;
}

void ProcessJpg::fix_orientation(ExifShort orientation, cv::Mat **image)
{
	switch(orientation)
	{
	case 6:
		transpose(image);
		flip(image, 1);
		break;
	default:
		break;
	}
}
