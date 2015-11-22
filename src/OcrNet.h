/*
 * OcrNet.h
 *
 *  Created on: Apr 3, 2011
 *      Author: nabheet
 */

#ifndef OCRNET_H_
#define OCRNET_H_

#include <sstream>
#include <set>
#include <map>
#include <cv.h>
#include <fann.h>
#include "ImageRegion.h"

class OcrNet
{
public:
	OcrNet(int data_set_size, int num_rows, int num_columns, int num_hidden, int num_output, int max_epochs, float error);
	~OcrNet();
	void Train(cv::Mat &image, std::map< int, std::set<ImageRegion> > &image_lines, std::string &char_set);
	char Run(cv::Mat &image, ImageRegion image_region);
private:
	void CreateTrainData(unsigned int index, unsigned int num_input, unsigned int num_output, fann_type * input_data, fann_type * output_data);
	void CreateOutput(fann_type output[], unsigned int num_output, unsigned int index);
	void CreateInput(fann_type input[], int num_input, cv::Mat &image, ImageRegion region);
	float DownSampleRegion(cv::Mat &image, ImageRegion region, int x, int y, double ratioX, double ratioY);
	struct fann **Ann;
	std::string Input;
	int Rows, Cols;
	unsigned int MaxEpochs;
	float Error;
	int DataSetSize;
};

const float UPPER_VAL = 1.0f;
const float LOWER_VAL = -1.0f;

#endif /* OCRNET_H_ */
