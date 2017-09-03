/* Write PNG with colors according to GeriMap */

#include <math.h>
#include <png.h>
#include <stdint.h>
#include <stdlib.h>
#include "s3dpng.h"

const int GERIMAP_COLORS=9;
uint8_t GERIMAP[9][3] = {
	{0,0,0},
	{38,38,128},
	{76,38,191},
	{153,51,128},
	{255,64,38},
	{230,128,0},
	{230,191,26},
	{230,230,128},
	{255,255,255}
};

double bitmap_threshold = 1;

void set_png_threshold(double mx) {
	bitmap_threshold = mx;
}

/**
 * Convert a scalar image to a bitmap.
 * Uses GeriMap as colormap.
 */
bitmap_t *img2bitmap(double **img, size_t width, size_t height) {
	long long signed int i, j, index;
	int gmil;
	double gmi, gmif;
	bitmap_t *bmp;

	bmp = malloc(sizeof(bitmap_t));
	bmp->pixels = malloc(sizeof(pixel_t)*width*height);
	bmp->width = width;
	bmp->height = height;

	/* Generate bitmap image */
	for (i = width-1, index = 0; i >= 0; i--) {
		for (j = 0; j < (long long signed int)height; j++, index++) {
			gmi = (img[i][j]/bitmap_threshold * (GERIMAP_COLORS-1));
			gmil = floor(gmi);
			if (gmil >= GERIMAP_COLORS-1) {
				bmp->pixels[index].red   = GERIMAP[GERIMAP_COLORS-1][0];
				bmp->pixels[index].green = GERIMAP[GERIMAP_COLORS-1][1];
				bmp->pixels[index].blue  = GERIMAP[GERIMAP_COLORS-1][2];
			} else {
				gmif = gmi - (double)gmil;

				bmp->pixels[index].red   = GERIMAP[gmil][0]+(GERIMAP[gmil+1][0]-GERIMAP[gmil][0])*gmif;
				bmp->pixels[index].green = GERIMAP[gmil][1]+(GERIMAP[gmil+1][1]-GERIMAP[gmil][1])*gmif;
				bmp->pixels[index].blue  = GERIMAP[gmil][2]+(GERIMAP[gmil+1][2]-GERIMAP[gmil][2])*gmif;
			}
		}
	}

	return bmp;
}

pixel_t *pixel_at(bitmap_t *bmp, size_t x, size_t y) {
	return bmp->pixels + bmp->width*y + x;
}

int saveimg(double **img, size_t width, size_t height, const char *name) {
	int s;
	bitmap_t *bmp = img2bitmap(img, width, height);
	s = savepng(bmp, name);

	free(bmp->pixels);
	free(bmp);

	return s;
}
int savepng(bitmap_t *bmp, const char *name) {
	FILE *f;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;

	size_t x, y;
	png_byte **row_pointers = NULL;
	int pixel_size = 3;
	int depth = 8;

	f = fopen(name, "wb");
	if (!f) {
		perror("ERROR");
		fprintf(stderr, "ERROR: Unable to create PNG file.\n");
		return -1;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "ERROR: Unable to create PNG 'write struct'.\n");
		fclose(f);
		return -1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "ERROR: Unable to create PNG 'info struct'.\n");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(f);
		return -1;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "ERROR: Unable to write PNG.\n");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(f);
		return -1;
	}

	png_set_IHDR(
		png_ptr,
		info_ptr,
		bmp->width,
		bmp->height,
		depth,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);

	/* Initialize rows of PNG */
	row_pointers = png_malloc(png_ptr, bmp->height * sizeof(png_byte*));
	for (y = 0; y < bmp->height; y++) {
		png_byte *row = png_malloc(png_ptr, sizeof(uint8_t)*bmp->width*pixel_size);
		row_pointers[y] = row;
		for (x = 0; x < bmp->width; x++) {
			pixel_t *pixel = pixel_at(bmp, x, y);
			*row++ = pixel->red;
			*row++ = pixel->green;
			*row++ = pixel->blue;
		}
	}

	png_init_io(png_ptr, f);
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	/* Dealloc */
	for (y = 0; y < bmp->height; y++)
		png_free(png_ptr, row_pointers[y]);
	png_free(png_ptr, row_pointers);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(f);
	return 0;
}

