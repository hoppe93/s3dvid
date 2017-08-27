#ifndef _S3DPNG_H
#define _S3DPNG_H

#include <stdint.h>

typedef struct {
	uint8_t red, green, blue;
} pixel_t;
typedef struct {
	pixel_t *pixels;
	size_t width, height;
} bitmap_t;

void set_png_threshold(double);
int saveimg(double**, size_t, size_t, const char*);
int savepng(bitmap_t*, const char*);

#endif/*_S3DPNG_H*/
