/*
 * ImageRegion.c
 *
 *  Created on: Mar 11, 2011
 *      Author: nabheet
 */

#include "ImageRegion.h"

int ImageRegion::get_max_x() const
{
	return MaxX;
}

int ImageRegion::get_max_y() const
{
	return MaxY;
}

int ImageRegion::get_min_x() const
{
	return MinX;
}

int ImageRegion::get_min_y() const
{
	return MinY;
}

ImageRegion::ImageRegion()
{
	ImageRegion(0, 0);
}

ImageRegion::~ImageRegion()
{
	Pixels.clear();
}

ImageRegion::ImageRegion(int x, int y)
{
	ImagePoint point;

	point.set_x(x);
	point.set_y(y);

	//COPIES (not reference) the point to the end of the list
	Pixels.push_back(point);

	MinX = MaxX = x;
	MinY = MaxY = y;
}

void ImageRegion::add(int x, int y)
{
	ImagePoint point;

	point.set_x(x);
	point.set_y(y);

	Pixels.push_back(point);

	MinX = std::min(MinX, x);
	MaxX = std::max(MaxX, x);
	MinY = std::min(MinY, y);
	MaxY = std::max(MaxY, y);
}

void ImageRegion::merge(ImageRegion *region)
{
	Pixels.merge(region->Pixels);

	MinX = std::min(MinX, region->MinX);
	MaxX = std::max(MaxX, region->MaxX);
	MinY = std::min(MinY, region->MinY);
	MaxY = std::max(MaxY, region->MaxY);
}

int ImageRegion::area() const
{
	return (MaxX - MinX) * (MaxY - MinY);
}

int ImageRegion::get_pixel_count() const
{
	return Pixels.size();
}

ImagePoint ImageRegion::center()
{
	Center.set_x(MinX + (MaxX - MinX) / 2);
	Center.set_y(MinY + (MaxY - MinY) / 2);

	return Center;
}

bool ImageRegion::operator<( const ImageRegion& rhs ) const
{
	int lhs_height = (MaxY - MinY), rhs_height = (rhs.MaxY - rhs.MinY);

	int half_height = std::max(lhs_height, rhs_height) / 2;

	return (MaxY == rhs.MaxY)
						? (MinX < rhs.MinX)
						: (MaxY < rhs.MaxY)
									  ? (((MaxY + half_height) < rhs.MaxY)
											  ? true
											  : (MinX < rhs.MinX))
									   : ((MaxY - half_height) < rhs.MaxY)
													      ? MinX < rhs.MinX
													      : false;
}

std::ostream& operator<<(std::ostream &out, ImageRegion &region)
{
	out << "Image Region --> (MinX, MinY) - (MaxX, MaxY) = "
			<< "(" << region.MinX << "," << region.MinY << ") - ("
			<< region.MaxX << "," << region.MaxY << ")";
	return out;
}
