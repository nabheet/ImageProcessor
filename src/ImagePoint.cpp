/*
 * ImagePoint.cpp
 *
 *  Created on: Mar 17, 2011
 *      Author: nabheet
 */

#include "ImagePoint.h"

int ImagePoint::get_x()
{
	return X;
}

int ImagePoint::get_y()
{
	return Y;
}

void ImagePoint::set_x(int x)
{
	X = x;
}

void ImagePoint::set_y(int y)
{
	Y = y;
}

int ImagePoint::distance(ImagePoint a)
{
	double x = X - a.get_x();
	double y = Y - a.get_y();
	return round(sqrt(x * x + y * y));
}

bool ImagePoint::operator<( const ImagePoint& rhs ) const
{
	return (Y == rhs.Y) ? (X < rhs.X) : (Y < rhs.Y);
}

std::ostream& operator<<(std::ostream &out, ImagePoint &point)
{
    out << "ImagePoint --> (x,y) = (" << point.get_x() << "," << point.get_y() << ")";
    return out;
}

