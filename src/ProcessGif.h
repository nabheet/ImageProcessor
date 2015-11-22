/*
 * ProcessGif.h
 *
 *  Created on: Mar 11, 2011
 *      Author: nabheet
 */

#ifndef PROCESSGIF_H_
#define PROCESSGIF_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gif_lib.h>
#include "ProcessImage.h"

#define PROGRAM_NAME	"ImageProcessor"

class ProcessGif : public ProcessImage
{
public:
	/* virtual */ void read_image(char file_name[], cv::Mat **image);
};

#endif /* PROCESSGIF_H_ */
