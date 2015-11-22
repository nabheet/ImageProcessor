/*
 * OcrNet.cpp
 *
 *  Created on: Apr 3, 2011
 *      Author: nabheet
 */

#include "OcrNet.h"

OcrNet::OcrNet(int data_set_size, int num_rows, int num_columns, int num_hidden, int num_output, int max_epochs, float error)
{
	const int num_layers = 3;

	DataSetSize = data_set_size;
	Rows = num_rows;
	Cols = num_columns;
	MaxEpochs = max_epochs;
	Error = error;
	Ann = new struct fann* [DataSetSize];

	for(int i=0; i < DataSetSize; i++)
	{
		Ann[i] = fann_create_standard(num_layers, Rows * Cols, num_hidden, num_output);
		fann_set_activation_function_hidden(Ann[i], FANN_SIGMOID_SYMMETRIC);
		fann_set_activation_function_output(Ann[i], FANN_SIGMOID_SYMMETRIC);
	}

	//	fann_set_activation_steepness_hidden(Ann, 1);
	//	fann_set_activation_steepness_output(Ann, 1);
	//	fann_set_learning_rate(Ann, 0.01);
	//	fann_set_learning_momentum(Ann, 0.7);
	//	fann_set_rprop_increase_factor(Ann, 5);
	//	fann_set_rprop_decrease_factor(Ann, 0.007);
	//	fann_set_rprop_delta_max(Ann, 17);
	//	fann_set_rprop_delta_zero(Ann, 0.01);
}

OcrNet::~OcrNet()
{
	for(int i=0; i < DataSetSize; i++)
	{
		fann_destroy(Ann[i]);
	}

	delete [] Ann;
}

void OcrNet::Train(cv::Mat &image, std::map< int, std::set<ImageRegion> > &image_lines, std::string &char_set)
{
	std::map<int, std::set<ImageRegion> >::iterator image_lines_it;
	std::set<ImageRegion> regions;
	std::set<ImageRegion>::iterator regions_it;
	unsigned int char_count = 0, char_length = 0, repeat = 2, index = 0;
	struct fann_train_data **data;
	fann_type **inputs, **outputs;

	char_length = char_set.length();
	data = new struct fann_train_data* [char_length];

	for(uint j=0; j < char_length; j++)
	{
		data[j] = new struct fann_train_data;
		data[j]->num_data = repeat;
		data[j]->num_input = Ann[j]->num_input;
		data[j]->num_output = Ann[j]->num_output;
		inputs = new fann_type*[data[j]->num_data];
		outputs = new fann_type*[data[j]->num_data];
		data[j]->input = inputs;
		data[j]->output = outputs;

		for(uint i = 0; i < repeat; i++, char_count = 0)
		{
			for(image_lines_it = image_lines.begin();
					char_count < char_length && image_lines_it != image_lines.end();
					image_lines_it++, char_count++)
			{
				regions = (std::set<ImageRegion>) image_lines_it->second;

				for(regions_it = regions.begin();
						char_count < char_length && regions_it != regions.end();
						regions_it++, char_count++)
				{
					index = i * char_length + char_count;
					inputs[index] = new fann_type[data->num_input];
					outputs[index] = new fann_type[data->num_output];
					CreateInput(inputs[index], data->num_input, image, *regions_it);
					CreateOutput(outputs[index], data->num_output, char_count);

					std::cout << "Repeat Index: " << i << ", index: " << index << std::endl;
				}
			}
		}

		fann_randomize_weights(Ann[j], -0.001, 0.0);
		//	fann_init_weights(Ann, data);
		fann_train_on_data(Ann[j], &data[j], MaxEpochs, 10, Error);

		for(unsigned int i=0; i < data[j]->num_data; i++)
		{
			delete [] inputs[i];
			delete [] outputs[i];
		}
	}

	delete data;
	delete [] inputs;
	delete [] outputs;
}

char OcrNet::Run(cv::Mat &image, ImageRegion image_region)
{
	fann_type *input, *output;
	char ret_val = 'A';
	float max_val = LOWER_VAL;
	int max_val_index = -1;

	input = new fann_type[Ann[0]->num_input];

	CreateInput(input, Ann[0]->num_input, image, image_region);

	for(int j=0; j < DataSetSize; j++)
	{
		output = fann_run(Ann[j], input);

		std::cout << std::endl << "Printing Output:" << std::endl;

		for(unsigned int i=0; i < Ann[j]->num_output; i++)
		{
			float value = output[i];

			std::cout << value << " ";

			if(value >= max_val)
			{
				max_val = value;
				max_val_index = i;
			}
		}
	}

	std::cout << std::endl;

	ret_val += max_val_index;

	return ret_val;
}

void OcrNet::CreateOutput(fann_type output[], unsigned int num_output, unsigned int index)
{
//	std::cout << std::endl << "Printing Training Output:" << std::endl;

	for(unsigned int i=0; i < num_output; i++)
	{
		output[i] = (i == index) ? UPPER_VAL : LOWER_VAL;

//		std::cout << output[i] << " ";
	}

//	std::cout << std::endl;
}

void OcrNet::CreateInput(fann_type input[], int num_input, cv::Mat &image, ImageRegion region)
{
	int min_x = 0, max_x = 0, min_y = 0, max_y = 0;
	double ratio_x = 0.0, ratio_y = 0.0;

	min_x = region.get_min_x();
	min_y = region.get_min_y();
	max_x = region.get_max_x();
	max_y = region.get_max_y();

	ratio_x = ((double) (max_x - min_x)) / ((double) Cols);
	ratio_y = ((double) (max_y - min_y)) / ((double) Rows);

//	std::cout << std::endl << "Input Elements:" << std::endl;

	for(int j=0; j < Rows; j++)
	{
		for(int i=0; i < Cols; i++)
		{
			input[j + i] = DownSampleRegion(image, region, i, j, ratio_x, ratio_y);
//			std::cout << input[j + i] << " ";
		}

//		std::cout << std::endl;
	}
}

float OcrNet::DownSampleRegion(cv::Mat &image, ImageRegion region, int x, int y, double ratio_x, double ratio_y)
{
	int min_x = 0, max_x = 0, min_y = 0, max_y = 0,
			begin_x = 0, begin_y = 0, end_x = 0, end_y = 0;
	unsigned char value;

	min_x = region.get_min_x();
	min_y = region.get_min_y();
	max_x = region.get_max_x();
	max_y = region.get_max_y();

	for(begin_y = (min_y + (y * ratio_y)), end_y = (min_y + ((y + 1) * ratio_y));
			begin_y <= end_y; begin_y++)
	{
		for(begin_x = (min_x + (x * ratio_x)), end_x = (min_x + ((x + 1) * ratio_x));
				begin_x <= end_x; begin_x++)
		{
			value = image.at<unsigned char>(begin_y, begin_x);

			if(value == 0)
				return UPPER_VAL;
		}
	}

	return LOWER_VAL;
}
