#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Defines lifted from pi pico examples for SSD1306 OLED drivers:
// https://github.com/raspberrypi/pico-examples/tree/master/i2c/ssd1306_i2c
#define OLED_HEIGHT (32)
#define OLED_WIDTH (128)
#define OLED_PAGE_HEIGHT (8)
#define OLED_NUM_PAGES OLED_HEIGHT / OLED_PAGE_HEIGHT
#define OLED_BUF_LEN (OLED_NUM_PAGES * OLED_WIDTH)

// Add a 'header' to the output, just for debug and later for frame count
#define HEADER_LEN (sizeof(uint32_t))

static uint32_t CountImagePathPadding(const char* pszPath, uint8_t* pui8PaddingStart)
{
	// Iterate backwards looking for continuous '#'
	uint32_t ui32PaddingCount = 0;
	uint32_t i = strlen(pszPath);

	while (i--)
	{
		if (pszPath[i] == '#')
		{
			ui32PaddingCount++;
			*pui8PaddingStart = i;
		}
		else if (pszPath[i + 1] == '#')
		{
			break; // this is the first non-'#' after a '#', stop searching
		}
	}

	return ui32PaddingCount;
}

// return false on error
static bool LoadImage(const char* pszPath, uint8_t* pui8Output)
{
	// Data from the loaded palette image
	int iWidth = 0;
	int iHeight = 0;
	int iNumChannels = 0;
	uint8_t* pui8Image = stbi_load(
		pszPath,
		&iWidth,
		&iHeight,
		&iNumChannels,
		0
	);

	if (pui8Image == NULL)
	{
		printf("Error: Image %s failed to load\n", pszPath);
		return false;
	}

	printf(
		"Loaded %i * %i image with %i channels\n",
		iWidth,
		iHeight,
		iNumChannels
	);

	if ((iWidth != OLED_WIDTH) || (iHeight != OLED_HEIGHT))
	{
		printf(
			"Error: Loaded image dimensions are is not %u x %u\n",
			OLED_WIDTH,
			OLED_HEIGHT
		);
		goto FAILED_LoadImage;
	}

	// Each byte in the output image represents a 1x8 'column' of pixels
	{
		const uint32_t ui32ColumnStride = (OLED_WIDTH * iNumChannels);
		const uint32_t ui32PageStride = (OLED_PAGE_HEIGHT * ui32ColumnStride);

		for (uint32_t ui32Page = 0; ui32Page < OLED_NUM_PAGES; ++ui32Page)
		for (uint32_t ui32Column = 0; ui32Column < OLED_WIDTH; ++ui32Column)
		{
			// For each pixel in the column
			// Test if first channel is non-zero
			// Set the corresponding bit
			uint8_t ui8ColumnVal = 0x00;
			const uint32_t ui32Offset = (
				(ui32Page * ui32PageStride) + (ui32Column * iNumChannels)
			);

			for (uint8_t ui8Row = 0; ui8Row < 8; ++ui8Row)
			{
				if (pui8Image[ui32Offset + (ui8Row * ui32ColumnStride)] != 0x00)
				{
					ui8ColumnVal |= (0x1 << ui8Row);
				}
			}

			*pui8Output++ = ui8ColumnVal;
		}
	}

	stbi_image_free(pui8Image);
	return true;

FAILED_LoadImage:
	stbi_image_free(pui8Image);
	return false;
}

int main (int argc, char * argv[])
{
	if (argc != 2)
	{
		printf("Error: No image path provided\n");
		return 0;
	}

	char acPath[256];
	// We can't sprintf straight into the path string itself, as sprintf
	// will always put a terminating char in. instead, sprintf the numbers
	// into this buffer, and memcpy back to the path.
	uint8_t ui8PaddingStartIndex = 0;
	uint32_t ui32NumPaddingChars = 0;
	char acPaddingBuf[256];

	uint32_t ui32Count = 0;
	uint32_t ui32First = 0;

	uint32_t ui32OutputSizeInBytes = HEADER_LEN;
	uint8_t* pui8OutputData;
	uint8_t* pui8WriteHead;

	// Take a copy of path string
	strcpy(acPath, argv[1]);

	// Find how many '#' chars there are, and their index in the path
	ui32NumPaddingChars = CountImagePathPadding(
		acPath,
		&ui8PaddingStartIndex
	);

	if (ui32NumPaddingChars > 0) // process image sequence
	{
		// Test which images exist, find first and last
		const uint32_t ui32MaxImages = pow(10, ui32NumPaddingChars);
		for (uint32_t i = 0; i < ui32MaxImages; ++i)
		{
			sprintf(acPaddingBuf, "%0*d", ui32NumPaddingChars, i);
			memcpy(
				&acPath[ui8PaddingStartIndex],
				acPaddingBuf,
				ui32NumPaddingChars
			);

			if (access(acPath, R_OK) != 0) // file doesn't exist
			{
				if (ui32Count > 0) break;
			}
			// file exists, it's the first we've found
			else if (ui32Count++ == 0)
			{
				ui32First = i;
			}
		}

		if (ui32Count < 1)
		{
			printf(
				"Error: This looks like an image sequence,"
				"but no frames were found\n"
			);
			return 0;
		}

		printf("first %u, count %u\n", ui32First, ui32Count);
	}
	else // process single image
	{
		ui32Count = 1;
	}

	// allocate the output buffer
	ui32OutputSizeInBytes += (OLED_BUF_LEN * ui32Count);
	pui8OutputData = (uint8_t*)malloc(ui32OutputSizeInBytes);
	pui8WriteHead = pui8OutputData;

	// write the header, which is just the num frames
	memcpy(pui8WriteHead, &ui32Count, HEADER_LEN);
	pui8WriteHead += HEADER_LEN;

	// write the image data
	for (uint32_t i = ui32First; i < ui32Count; ++i)
	{
		sprintf(acPaddingBuf, "%0*d", ui32NumPaddingChars, i);
		memcpy(
			&acPath[ui8PaddingStartIndex],
			acPaddingBuf,
			ui32NumPaddingChars
		);

		if (!LoadImage(acPath, pui8WriteHead))
		{
			printf("Error writing frame\n");
			goto FAILED_WriteImage;
		}

		pui8WriteHead += OLED_BUF_LEN;
	}

	// write image to file
	{
		FILE* pFile = fopen("test.bin", "w");
		if (!pFile)
		{
			printf("Error writing binary\n");
			goto FAILED_WriteImage;
		}

		fwrite(pui8OutputData, sizeof(uint8_t), ui32OutputSizeInBytes, pFile);
		fclose (pFile);
	}

	printf("%u bytes written\n", ui32OutputSizeInBytes);

	free(pui8OutputData);

	return 0;

FAILED_WriteImage:
	free(pui8OutputData);
	return false;
}
