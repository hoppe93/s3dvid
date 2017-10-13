/* Camera manager */

#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "s3d.h"

double **camera_image;
size_t camera_pixelsi, camera_pixelsj;
double ehat1[3], ehat2[3], cnormal[3], cloc[3], tanvisangI;

#pragma omp threadprivate(camera_image,ehat1,ehat2,cnormal,cloc)

/**
 * Initialize camera image generator globally,
 * not just for one thread.
 */
void camera_init(size_t pixelsi, size_t pixelsj, double visang) {
	/* Inverse of tangent of half vision angle */
	tanvisangI = 1.0 / tan(visang/2.0);

	camera_pixelsi = pixelsi;
	camera_pixelsj = pixelsj;
}

/**
 * Initialize one thread. This function
 * sets the camera settings for that thread.
 *
 * location: Camera position relative device center.
 * direction: Camera viewing direction (normal vector, not necessarily normalized).
 */
void camera_init_local(double location[3], double direction[3]) {
	double n;
	/* Camera location */
	cloc[0] = location[0];
	cloc[1] = location[1];
	cloc[2] = location[2];

	/* Viewing direction */
	n = hypot(direction[0], hypot(direction[1], direction[2]));
	cnormal[0] = direction[0] / n;
	cnormal[1] = direction[1] / n;
	cnormal[2] = direction[2] / n;
	
	/* Compute camera basis */
	if (cnormal[1] == 0) {
		ehat1[0] = 0;
		ehat1[1] = 1;
		ehat1[0] = 0;
	} else {
		n = 1 / sqrt(cnormal[0]*cnormal[0] + cnormal[1]*cnormal[1]);
		ehat1[0] = n * cnormal[1];
		ehat1[1] =-n * cnormal[0];
		ehat1[2] = 0;
	}

	ehat2[0] = ehat1[1]*cnormal[2] - ehat1[2]*cnormal[1];
	ehat2[1] = ehat1[2]*cnormal[0] - ehat1[0]*cnormal[2];
	ehat2[2] = ehat1[0]*cnormal[1] - ehat1[1]*cnormal[0];
}

/**
 * Free memory for current image.
 */
void camera_destroy_image(void) {
	free(camera_image[0]);
	free(camera_image);
}
/**
 * Allocate memory for a new image.
 */
void camera_new_image(void) {
	size_t i, j;
	camera_image = malloc(sizeof(double*)*camera_pixelsi);
	camera_image[0] = malloc(sizeof(double)*camera_pixelsi*camera_pixelsj);
	for (i = 0; i < camera_pixelsi; i++) {
		if (i > 0) camera_image[i] = camera_image[i-1] + camera_pixelsj;

		for (j = 0; j < camera_pixelsj; j++) {
			camera_image[i][j] = 0.0;
		}
	}
}

/**
 * Clear the current image
 */
void camera_clear_image(void) {
	size_t i, j;
	for (i = 0; i < camera_pixelsi; i++) {
		for (j = 0; j < camera_pixelsj; j++) {
			camera_image[i][j] = 0.0;
		}
	}
}

/**
 * Generate a camera image
 */
double **camera_generate(s3d_t *s) {
	size_t i, j, k;
	long long signed int I, J;
	double dx = (s->xmax-s->xmin)/(s->pixels-1),
		   dy = (s->ymax-s->ymin)/(s->pixels-1),
		   dz = (s->zmax-s->zmin)/(s->pixels-1),
		   xmin = s->xmin,
		   ymin = s->ymin,
		   zmin = s->zmin,
		   idx, jdy, kdz,
		   npi2 = camera_pixelsi * 0.5,
		   npj2 = camera_pixelsj * 0.5,
		   Li, f, q1, q2,
		   x[3], rcp[3], q[3];
	
	for (i=0, idx=0; i < s->pixels; i++, idx+=dx) {
		for (j=0, jdy=0; j < s->pixels; j++, jdy+=dy) {
			for (k=0, kdz=0; k < s->pixels; k++, kdz+=dz) {
				/* Ignore empty elements */
				if (s->data[i][j][k] == 0) continue;

				x[0] = xmin+idx;
				x[1] = ymin+jdy;
				x[2] = zmin+kdz;

				rcp[0] = x[0]-cloc[0];
				rcp[1] = x[1]-cloc[1];
				rcp[2] = x[2]-cloc[2];

				Li = 1.0 / hypot(rcp[0], hypot(rcp[1], rcp[2]));
				f = cnormal[0]*rcp[0] + cnormal[1]*rcp[1] + cnormal[2]*rcp[2];

				q[0] = rcp[0]-f*cnormal[0];
				q[1] = rcp[1]-f*cnormal[1];
				q[2] = rcp[2]-f*cnormal[2];

				q1 = ehat1[0]*q[0] + ehat1[1]*q[1] + ehat1[2]*q[2];
				q2 = ehat2[0]*q[0] + ehat2[1]*q[1] + ehat2[2]*q[2];

				I = (long long signed int)(npi2 * (q2*tanvisangI*Li + 1));
				J = (long long signed int)(npj2 * (q1*tanvisangI*Li + 1));

				if (I >= 0 && I < (long long signed)camera_pixelsi &&
					J >= 0 && J < (long long signed)camera_pixelsj)
					camera_image[I][J] += s->data[i][j][k];
			}
		}
	}

	return camera_image;
}

void camera_get_extents(s3d_t *s) {
	size_t i, j, k;
	double dx = (s->xmax-s->xmin)/(s->pixels-1),
		   dy = (s->ymax-s->ymin)/(s->pixels-1),
		   dz = (s->zmax-s->zmin)/(s->pixels-1),
		   xmin, ymin, zmin, xmax, ymax, zmax,
		   idx, jdy, kdz, x, y, z;
	
	xmin = NAN, xmax = NAN;
	ymin = NAN, ymax = NAN;
	zmin = NAN, zmax = NAN;

	for (i=0, idx=0; i < s->pixels; i++, idx+=dx) {
		for (j=0, jdy=0; j < s->pixels; j++, jdy+=dy) {
			for (k=0, kdz=0; k < s->pixels; k++, kdz+=dz) {
				if (s->data[i][j][k] == 0) continue;

				x = s->xmin + idx;
				y = s->ymin + jdy;
				z = s->zmin + kdz;

					 if (isnan(xmin) || x < xmin) xmin = x;
				else if (isnan(xmax) || x > xmax) xmax = x;
					 if (isnan(ymin) || y < ymin) ymin = y;
				else if (isnan(ymax) || y > ymax) ymax = y;
					 if (isnan(zmin) || z < zmin) zmin = z;
				else if (isnan(zmax) || z > zmax) zmax = z;
			}
		}
	}

	printf("-------------------------------\n");
	printf("SURFACE-OF-VISIBILITY EXTENTS\n\n");
	printf("  xmin = %2.3f,  xmax = %2.3f\n", xmin, xmax);
	printf("  ymin = %2.3f,  ymax = %2.3f\n", ymin, ymax);
	printf("  zmin = %2.3f,  zmax = %2.3f\n", zmin, zmax);
	printf("-------------------------------\n\n");
}
