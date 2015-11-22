/*
 * ProcessJpg.h
 *
 *  Created on: Mar 13, 2011
 *      Author: nabheet
 */

#ifndef PROCESSJPG_H_
#define PROCESSJPG_H_

#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include <exif-data.h>
#include "ProcessImage.h"

class ProcessJpg : public ProcessImage
{
public:
	/* virtual */ void read_image(char file_name[], cv::Mat **image);
private:
	ExifShort read_exif_info(char file_name[]);
	void fix_orientation(ExifShort orientation, cv::Mat **image);
};

#endif /* PROCESSJPG_H_ */
