/* Draw axis on image */

#include <math.h>
#include "s3dpng.h"

/* Percentage of the total image dimensions where
 * the axes should originate from. */
#define ORIGIN_PERCENTAGE 0.05
/* Real length of vector (in percentage of image height)
 * corresponding to a unit vector. */
#define VECTOR_NORMALIZED_LENGTH 0.1

void axes_project(double axis[3], double e1[3], double e2[3], double ret[2]) {
	ret[0] = axis[0]*e1[0] + axis[1]*e1[1] + axis[2]*e1[2],
	ret[1] = axis[0]*e2[0] + axis[1]*e2[1] + axis[2]*e2[2];
}

/**
 * Draw a single axis px, projected on the
 * camera plane, on the bitmap image.
 *
 * img: Bitmap image to draw on
 * px:  Projection of axis vector on camera plane (e1-e2)
 */
void draw_axis(bitmap_t *img, double px[2]) {
	/* Define origin pixel */
	size_t opi = img->height * ORIGIN_PERCENTAGE;
	size_t opj = img->width * ORIGIN_PERCENTAGE;

	double di = (img->height * VECTOR_NORMALIZED_LENGTH * px[0]),
		   dj = (img->width  * VECTOR_NORMALIZED_LENGTH * px[1]);
	size_t i = (size_t)di, j = (size_t)dj;

	double k;
	if (j == opj) {	/* Vertical line */
		/* ... */
	} else {
		k = (di-opi)/(dj-opj);
	}
}

/**
 * Draw X, Y and Z axes over the image.
 *
 * img: Image to draw the axes over
 * pixelsi: Number of rows in 'img'
 * pixelsj: Number of cols in 'img'
 * loc: Camera location vector
 * dir: Camera direction vector
 */
void draw_axes(bitmap_t *img, double loc[3], double dir[3]) {
	double e1[3], e2[3], n,
		   px[2], py[2], pz[2];

	/* Compute camera plane vectors */
	if (loc[1] == 0) {
		e1[0] = 0;
		e1[1] = 1;
		e1[2] = 0;
	} else {
		n = hypot(loc[0], loc[1]);
		e1[0] = loc[1] / n;
		e1[1] = loc[0] / n;
		e1[2] = 0;
	}

	e2[0] = e1[1]*loc[2] - e1[2]*loc[1];
	e2[1] = e1[2]*loc[0] - e1[0]*loc[2];
	e2[2] = e1[0]*loc[1] - e1[1]*loc[0];

	/* Project axes */
	double xhat[3] = {1.0,0.0,0.0},
		   yhat[3] = {0.0,1.0,0.0},
		   zhat[3] = {0.0,0.0,1.0};

	axes_project(xhat, e1, e2, px);
	axes_project(yhat, e1, e2, py);
	axes_project(zhat, e1, e2, pz);

	draw_axis(img, px);
	draw_axis(img, py);
	draw_axis(img, pz);
}
