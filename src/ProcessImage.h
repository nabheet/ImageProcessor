/*
 * ProcessImage.h
 *
 *  Created on: Mar 14, 2011
 *      Author: nabheet
 */

#ifndef PROCESSIMAGE_H_
#define PROCESSIMAGE_H_

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <cv.h>
#include <cvaux.h>
#include "ImageRegion.h"

class ProcessImage {
public:
	virtual void read_image(char file_name[], cv::Mat **image) = 0;
	static void release_image(cv::Mat **image);
	std::map< int, std::set<ImageRegion> >* find_regions(cv::Mat &image);
protected:
	void convert_to_binary(cv::Mat **image);
	void transpose(cv::Mat **image);
	void flip(cv::Mat **image, int flip_mode);
private:
	std::map< int, std::set<ImageRegion> >* find_lines(std::map<int, ImageRegion>* regions, int image_area);
	void merge_regions(std::set<ImageRegion>* line, std::set<ImageRegion>* line_temp);
};

#endif /* PROCESSIMAGE_H_ */
