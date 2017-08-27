#ifndef _CAMERA_H
#define _CAMERA_H

#include "s3d.h"

void camera_init(size_t, size_t, double);
void camera_init_local(double[3], double[3]);
void camera_destroy_image(void);
void camera_new_image(void);
void camera_clear_image(void);
double **camera_generate(s3d_t*);

#endif/*_CAMERA_H*/
