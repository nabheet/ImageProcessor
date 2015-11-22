/*
 * TessHandler.cpp
 *
 *  Created on: Aug 18, 2012
 *      Author: nabheet
 */

#include "TessHandler.h"

TessHandler::TessHandler() {
	// TODO Auto-generated constructor stub

}

TessHandler::~TessHandler() {
	// TODO Auto-generated destructor stub
}

void TessHandler::DoWork(cv::Mat &image)
{
	tesseract::TessBaseAPI *tessApi;

	tessApi = new tesseract::TessBaseAPI();

	tessApi->Init(NULL, "eng");

	tessApi->SetImage(image.data, image.cols, image.rows, image.channels(), ((image.cols) * image.channels()));

	tessApi->Recognize(NULL);

	tesseract::ResultIterator* ri = tessApi->GetIterator();
	tesseract::ChoiceIterator* ci;

	if(ri != 0)
	{
		do
		{
			const char* symbol = ri->GetUTF8Text(tesseract::RIL_TEXTLINE);

			if(symbol != 0)
			{
				float conf = ri->Confidence(tesseract::RIL_TEXTLINE);
				std::cout << "Symbol --> " << symbol << ", Confidence --> " << conf << std::endl;

				const tesseract::ResultIterator itr = *ri;
				ci = new tesseract::ChoiceIterator(itr);

				do
				{
					const char* choice = ci->GetUTF8Text();

					if(choice != 0)
					{
						std::cout << "\t\tChoice --> " << choice << ", Choice Confidence --> " << ci->Confidence() << std::endl;
						delete[] choice;
					}
				}
				while(ci->Next());

				delete ci;
			}

			delete[] symbol;
		}
		while((ri->Next(tesseract::RIL_TEXTLINE)));
	}

	delete tessApi;
}

