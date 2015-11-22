/*
 * ImagePoint.h
 *
 *  Created on: Mar 17, 2011
 *      Author: nabheet
 */

#ifndef IMAGEPOINT_H_
#define IMAGEPOINT_H_

#include <cmath>
#include <iostream>

class ImagePoint
{
public:
	int get_x();
	int get_y();
	void set_x(int x);
	void set_y(int y);
	int distance(ImagePoint a);
	bool operator<( const ImagePoint& rhs ) const;
	friend std::ostream& operator<< (std::ostream &out, ImagePoint &point);
private:
	int X;
	int Y;
};

#endif /* IMAGEPOINT_H_ */
