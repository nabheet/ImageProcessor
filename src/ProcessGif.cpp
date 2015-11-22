/*
 * ProcessGif.c
 *
 *  Created on: Mar 11, 2011
 *      Author: nabheet
 */

#include "ProcessGif.h"

void ProcessGif::read_image(char file_name[], cv::Mat **image)
{
	GifFileType *gif_image = NULL;
	GifRowType *ScreenBuffer = NULL;
	int size_of_each_row = 0, BackGround = 0, i, j, Row = 0, Col = 0, Width = 0, Height = 0,
			ImageNum = 0, ExtCode, Count, img_width = 0, img_height = 0,
			InterlacedOffset[] = { 0, 4, 2, 1 }, /* The way Interlaced image should. */
		    InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */;
	ColorMapObject *ColorMap;
	GifRowType GifRow;
	static GifColorType *ColorMapEntry;
	GifRecordType RecordType;
	GifByteType *Extension;
	unsigned char *BufferP, *image_buffer;

	gif_image = DGifOpenFileName(file_name);

	img_height = gif_image->SHeight;
	img_width = gif_image->SWidth;

	image_buffer = new unsigned char[img_height * img_width * 3];

	if ((ScreenBuffer = (GifRowType *) malloc(img_height * sizeof(GifRowType *))) == NULL)
		GIF_EXIT("Failed to allocate memory required, aborted.");

	size_of_each_row = img_width * sizeof(GifPixelType);/* Size in bytes one row.*/
	if ((ScreenBuffer[0] = (GifRowType) malloc(size_of_each_row)) == NULL) /* First row. */
		GIF_EXIT("Failed to allocate memory required, aborted.");

	for (i = 0; i < gif_image->SWidth; i++)  /* Set first row's color to BackGround. */
		ScreenBuffer[0][i] = gif_image->SBackGroundColor;

	for (i = 1; i < img_height; i++)
	{
		/* Allocate the other rows, and set their color to background too: */
		if ((ScreenBuffer[i] = (GifRowType) malloc(size_of_each_row)) == NULL)
			GIF_EXIT("Failed to allocate memory required, aborted.");

		memcpy(ScreenBuffer[i], ScreenBuffer[0], size_of_each_row);
	}

	/* Scan the content of the GIF file and load the image(s) in: */
	do {
		if (DGifGetRecordType(gif_image, &RecordType) == GIF_ERROR)
		{
			PrintGifError();
			exit(EXIT_FAILURE);
		}
		switch (RecordType) {
		case IMAGE_DESC_RECORD_TYPE:
			if (DGifGetImageDesc(gif_image) == GIF_ERROR)
			{
				PrintGifError();
				exit(EXIT_FAILURE);
			}

			Row = gif_image->Image.Top; /* Image Position relative to Screen. */
			Col = gif_image->Image.Left;
			Width = gif_image->Image.Width;
			Height = gif_image->Image.Height;

			if (gif_image->Image.Left + gif_image->Image.Width > gif_image->SWidth ||
					gif_image->Image.Top + gif_image->Image.Height > gif_image->SHeight)
			{
				fprintf(stderr, "Image %d is not confined to screen dimension, aborted.\n",ImageNum);
				exit(EXIT_FAILURE);
			}
			if (gif_image->Image.Interlace) {
				/* Need to perform 4 passes on the images: */
				for (Count = i = 0; i < 4; i++)
					for (j = Row + InterlacedOffset[i]; j < Row + Height; j += InterlacedJumps[i])
					{
						if (DGifGetLine(gif_image, &ScreenBuffer[j][Col], Width) == GIF_ERROR)
						{
							PrintGifError();
							exit(EXIT_FAILURE);
						}
					}
			}
			else {
				for (i = 0; i < Height; i++) {
					if (DGifGetLine(gif_image, &ScreenBuffer[Row++][Col], Width) == GIF_ERROR)
					{
						PrintGifError();
						exit(EXIT_FAILURE);
					}
				}
			}
			break;
		case EXTENSION_RECORD_TYPE:
			/* Skip any extension blocks in file: */
			if (DGifGetExtension(gif_image, &ExtCode, &Extension) == GIF_ERROR)
			{
				PrintGifError();
				exit(EXIT_FAILURE);
			}
			while (Extension != NULL) {
				if (DGifGetExtensionNext(gif_image, &Extension) == GIF_ERROR)
				{
					PrintGifError();
					exit(EXIT_FAILURE);
				}
			}
			break;
		case TERMINATE_RECORD_TYPE:
			break;
		default:		    /* Should be traps by DGifGetRecordType. */
			break;
		}
	} while (RecordType != TERMINATE_RECORD_TYPE);

	BackGround = gif_image->SBackGroundColor;
	ColorMap = (gif_image->Image.ColorMap
			? gif_image->Image.ColorMap
					: gif_image->SColorMap);
	if (ColorMap == NULL)
	{
		fprintf(stderr, "Gif Image does not have a colormap\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0, BufferP = image_buffer; i < img_height; i++)
	{
		GifRow = ScreenBuffer[i];

		for (j = 0; j < img_width; j++) {
			ColorMapEntry = &ColorMap->Colors[GifRow[j]];
			*BufferP++ = ColorMapEntry->Red;
			*BufferP++ = ColorMapEntry->Green;
			*BufferP++ = ColorMapEntry->Blue;
		}

		free(ScreenBuffer[i]);
	}

	free(ScreenBuffer);

	DGifCloseFile(gif_image);

	(*image) = new cv::Mat(cv::Size(img_width, img_height), CV_8UC3, image_buffer);

	convert_to_binary(image);
}
