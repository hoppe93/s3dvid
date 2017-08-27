#ifndef _S3D_H
#define _S3D_H

#include <stdlib.h>

typedef struct {
	double ***data;
	double xmin, xmax,
		   ymin, ymax,
		   zmin, zmax;
	size_t pixels;
} s3d_t;

void s3d_center(s3d_t*, double[3]);
s3d_t *loads3d(const char*);

#endif/*_S3D_H*/
