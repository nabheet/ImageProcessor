/*
 * ImageRegion.h
 *
 *  Created on: Mar 11, 2011
 *      Author: nabheet
 */

#ifndef IMAGEREGION_H_
#define IMAGEREGION_H_

#include <list>
#include <iostream>
#include "ImagePoint.h"

class ImageRegion
{
public:
	ImageRegion();
	ImageRegion(int x, int y);
	~ImageRegion();
	void add(int x, int y);
	void merge(ImageRegion *region);
	bool operator<( const ImageRegion& rhs ) const;
	friend std::ostream& operator<<(std::ostream &out, ImageRegion &region);
	int area() const;
	ImagePoint center();
	int get_max_x() const;
	int get_max_y() const;
	int get_min_x() const;
	int get_min_y() const;
	int get_pixel_count() const;

private:
	std::list<ImagePoint> Pixels;
	ImagePoint Center;
	int MinX;
	int MinY;
	int MaxX;
	int MaxY;
};

#endif /* IMAGEREGION_H_ */
