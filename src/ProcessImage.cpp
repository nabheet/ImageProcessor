/*
 * ProcessImage.cpp
 *
 *  Created on: Mar 15, 2011
 *      Author: nabheet
 */

#include "ProcessImage.h"

void ProcessImage::release_image(cv::Mat **image)
{
	delete (*image);
}

void ProcessImage::convert_to_binary(cv::Mat **image)
{
	cv::Mat *im_gray, *im_binary;

	if((*image)->channels() > 1)
	{
		im_gray = new cv::Mat(cv::Size((*image)->cols, (*image)->rows), CV_8UC1);
		cv::cvtColor(*(*image), *im_gray, CV_RGB2GRAY);
		release_image(image);
		(*image) = im_gray;
	}

	im_binary = new cv::Mat(cv::Size((*image)->cols, (*image)->rows), CV_8UC1);
//	cv::adaptiveThreshold(*image, im_binary, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 7, 5);
	cv::threshold(*(*image), *im_binary, 128, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	release_image(image);
	(*image) = im_binary;
}

void ProcessImage::transpose(cv::Mat **image)
{
	cv::Mat *transposed_image = NULL;

	transposed_image = new cv::Mat(cv::Size((*image)->rows, (*image)->cols), (*image)->depth());

	cv::transpose(**image, *transposed_image);

	release_image(image);

	(*image) = transposed_image;
}

void ProcessImage::flip(cv::Mat **image, int flip_mode)
{
	cv::Mat *flipped = new cv::Mat(cv::Size((*image)->cols, (*image)->rows), (*image)->depth());
	/*
	 * flip_mode = 0 means flipping around x-axis, flip_mode > 0 (e.g. 1)
	 * means flipping around y-axis and flip_mode < 0 (e.g. -1) means
	 * flipping around both axises.
	*/
	cv::flip(**image, *flipped, flip_mode);

	release_image(image);

	*image = flipped;
}

std::map< int, std::set<ImageRegion> >* ProcessImage::find_regions(cv::Mat &image)
{
	std::map<int, ImageRegion>* regions_ptr = new std::map<int, ImageRegion>();
	std::map<int, ImageRegion>::iterator regions_it;
	std::map< int, std::set<ImageRegion> >* ret_val;
	std::set<int> smallest_region;
	int n_regions = 0, new_region = 0, x = -1, y = -1;
	std::map< int, std::set<int> > equivalences;
	std::map< int, std::set<int> >::iterator equivalences_it;
	unsigned char* pixel, *image_buffer;
	int image_width, image_height, components;

	image_width = image.cols;
	image_height = image.rows;
	components = image.channels();
	image_buffer = image.data;

	std::vector< std::vector<int> > pixel_region(image_width, std::vector<int>(image_height, 0));

	for(y = 0; y < image_height; y++)
	{
		for(x = 0; x < image_width; x++)
		{
			 pixel = image_buffer + (y * image_width * components) + (x * components);

			 // Red != 0 && Green != 0 && Blue != 0 --> Not Black --> Is Background
			 if((*(pixel) != 0) && (*(pixel + 1) != 0) && (*(pixel + 2) != 0))
			 {
				 // Pixel is background so ignore it
				 continue;
			 }

			 // Pixel is not back ground, check for regions
			 int region_ne = x + 1 < image_width && y > 0 ? pixel_region[x + 1][y - 1] : 0;
			 int region_n = y > 0 ? pixel_region[x][y - 1] : 0;
			 int region_nw = x > 0 && y > 0 ? pixel_region[x - 1][y - 1] : 0;
			 int region_w = x > 0 ? pixel_region[x - 1][y] : 0;

			 int max_region = std::max(std::max(std::max(region_ne, region_n), region_nw), region_w);

			 smallest_region.clear();

			 if(max_region > 0)
			 {
				 if(region_ne != 0)
					 smallest_region.insert(region_ne);

				 if(region_n != 0)
					 smallest_region.insert(region_n);

				 if(region_nw != 0)
					 smallest_region.insert(region_nw);

				 if(region_w != 0)
					 smallest_region.insert(region_w);

				 new_region = *(smallest_region.begin());

				 if(max_region > new_region)
				 {
					 equivalences_it = equivalences.find(max_region);
					 if(equivalences_it != equivalences.end())
					 {
						 ((std::set<int>)(equivalences_it->second)).insert(new_region);
					 }
					 else
					 {
						 std::set<int> new_region_set;
						 new_region_set.insert(new_region);
						 equivalences.insert(std::pair<int, std::set<int> >(max_region, new_region_set));
					 }
				 }
			 }
			 else
			 {
				 n_regions++;
				 new_region = n_regions;
			 }

			 pixel_region[x][y] = new_region;
		}
	}

	// Scan image again, assigning all equivalent regions the same region value.
	for (y = 0; y < image_height; y++)
	{
		for (x = 0; x < image_width; x++)

		{
			int r = pixel_region[x][y];

			if (r > 0)
			{
				equivalences_it = equivalences.find(r);

				while(equivalences_it != equivalences.end())
				{
					r = *(((std::set<int>)(equivalences_it->second)).begin());
					equivalences_it = equivalences.find(r);
				}

				regions_it = regions_ptr->find(r);

				if (regions_it != regions_ptr->end())
				{
					(regions_it->second).add(x, y);
				}
				else
				{
					regions_ptr->insert(std::pair<int, ImageRegion>(r, ImageRegion(x, y)));
				}
			}
		}
	}

	ret_val = find_lines(regions_ptr, image_width * image_height);

	regions_ptr->clear();

	delete regions_ptr;

	return ret_val;
}

std::map< int, std::set<ImageRegion> >* ProcessImage::find_lines(std::map<int, ImageRegion>* regions, int image_area)
{
	std::map<int, ImageRegion>::iterator regions_it_i;
	std::map< int, std::set<ImageRegion> >* image_lines = new std::map< int, std::set<ImageRegion> >;
	std::set<ImageRegion>* image_line = new std::set<ImageRegion>, *image_line_temp = new std::set<ImageRegion>;
	ImageRegion* image_region;
	int line_num = 0, total_y = 0, num_items = 0, total_height = 0,
			center_y = 0, min_y = 0, avg_y = 0, avg_height = 0,
			max_y = 0, height = 0, max_height = 0;

	for (regions_it_i = regions->begin(), line_num = 1; regions_it_i != regions->end(); regions_it_i++)
	{
		image_region = &(regions_it_i->second);

		if((image_region->area() < (image_area / 800000))
				|| (image_region->area() > (image_area / 100)))
			continue;

		num_items++;

		min_y = image_region->get_min_y();
		max_y = image_region->get_max_y();
		height = max_y - min_y;
		center_y = min_y + height / 2;

		avg_y = (total_y + center_y) / num_items;

		avg_height = (total_height + height) / num_items;

		max_height = std::max(height, avg_height);

		//do I need new line
		if ((center_y <= (avg_y + max_height)) && (center_y >= (avg_y - max_height)))
		{
			// no - add to existing line
			image_line->insert(*image_region);
		}
		else
		{
			// yes - add existing line to image_lines and instantiate new line
			merge_regions(image_line, image_line_temp);
			image_lines->insert(std::pair<int, std::set<ImageRegion> >(line_num, *image_line_temp));
			line_num++;
			image_line->clear();
			image_line_temp->clear();
			image_line->insert(*image_region);
			total_height = 0;
			total_y = 0;
			num_items = 1;
		}

		total_y += center_y;
		total_height += height;
	}

	// add the last line
	merge_regions(image_line, image_line_temp);
	image_lines->insert(std::pair<int, std::set<ImageRegion> >(line_num, *image_line_temp));
	line_num++;

	delete image_line;
	delete image_line_temp;

	return image_lines;
}

void ProcessImage::merge_regions(std::set<ImageRegion>* line, std::set<ImageRegion>* line_temp)
{
	ImageRegion region_1, region_2;
	std::set<ImageRegion> processed;
	std::set<ImageRegion>::iterator line_it_1, line_it_2;
	int center_x_1 = 0, max_width = 0, min_x_1 = 0, max_x_1 = 0, width_1 = 0,
			center_x_2 = 0, min_x_2 = 0, max_x_2 = 0, width_2 = 0;

	for(line_it_1 = line->begin(); line_it_1 != line->end(); line_it_1++)
	{
		region_1 = *line_it_1;

		if(processed.find(region_1) != processed.end())
			continue;

		min_x_1 = region_1.get_min_x();
		max_x_1 = region_1.get_max_x();
		width_1 = max_x_1 - min_x_1;
		center_x_1 = min_x_1 + width_1 / 2;

		for(line_it_2 = line_it_1, line_it_2++; line_it_2 != line->end(); line_it_2++)
		{
			region_2 = *line_it_2;
			min_x_2 = region_2.get_min_x();
			max_x_2 = region_2.get_max_x();
			width_2 = max_x_2 - min_x_2;
			center_x_2 = min_x_2 + width_2 / 2;

			max_width = std::max(width_1, width_2) / 2;

			if((center_x_2 >= (center_x_1 - max_width))
					&& (center_x_2 <= (center_x_1 + max_width)))
			{
				region_1.merge(&region_2);
				line_temp->erase(region_1);
				line_temp->insert(region_1);
				processed.insert(region_2);
				region_2.merge(&region_1);
				processed.insert(region_2);

				min_x_1 = region_1.get_min_x();
				max_x_1 = region_1.get_max_x();
				width_1 = max_x_1 - min_x_1;
				center_x_1 = min_x_1 + width_1 / 2;
			}
		}

		if(processed.find(region_1) == processed.end())
			line_temp->insert(region_1);
	}
}
