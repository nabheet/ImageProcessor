/*
 * TessHandler.h
 *
 *  Created on: Aug 18, 2012
 *      Author: nabheet
 */

#ifndef TESSHANDLER_H_
#define TESSHANDLER_H_

#include <tesseract/baseapi.h>
#include <tesseract/ltrresultiterator.h>
#include <tesseract/resultiterator.h>
#include <leptonica/alltypes.h>
#include <cv.h>
#include <cvaux.h>

class TessHandler {
public:
	TessHandler();
	virtual ~TessHandler();
	void DoWork(cv::Mat &image);
};

#endif /* TESSHANDLER_H_ */
