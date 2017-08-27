/* Handle S3D loading */

#include <mat.h>
#include "s3d.h"

void s3d_center(s3d_t *s, double cp[3]) {
	cp[0] = (s->xmax+s->xmin) * 0.5;
	cp[1] = (s->ymax+s->ymin) * 0.5;
	cp[2] = (s->zmax+s->zmin) * 0.5;
}

double ***get_image(MATFile *mfp, const char *name, size_t pixels) {
	double *ptr, ***img, **tmp1;
	mxArray *arr;
	size_t m, n, pixels2 = pixels*pixels, pixels3 = pixels2*pixels,
		   i, j;

	arr = matGetVariable(mfp, name);
	if (arr == NULL || mxIsEmpty(arr)) {
		fprintf(stderr, "ERROR: Variable '%s' does not exist in the S3D file.\n", name);
		return NULL;
	}

	m = mxGetM(arr);
	n = mxGetN(arr);

	ptr = mxGetPr(arr);
	if (!((m == 1 && n == pixels3) || (n == 1 && m != pixels3))) {
		fprintf(stderr, "ERROR: Invalid dimensions of S3D image (m = %zu, n = %zu, pixels^3 = %zu).\n", m, n, pixels3);
		return NULL;
	}

	img = malloc(sizeof(double**)*pixels);
	tmp1 = malloc(sizeof(double*)*pixels*pixels);
	for (i = 0; i < pixels; i++) {
		img[i] = tmp1 + i*pixels;
		for (j = 0; j < pixels; j++) {
			img[i][j] = ptr + i*pixels2 + j*pixels;
		}
	}

	/* We don't destroy the mxArray, to avoid
	 * taking up twice as much memory. */
	/*mxDestroyArray(arr);*/

	return img;
}
double get_scalar(MATFile *mfp, const char *name) {
	mxArray *arr;

	arr = matGetVariable(mfp, name);
	if (arr == NULL || mxIsEmpty(arr)) {
		fprintf(stderr, "ERROR: Variable '%s' does not exist in the S3D file.\n", name);
		exit(EXIT_FAILURE);
	}
	if (!mxIsScalar(arr)) {
		fprintf(stderr, "ERROR: Unrecognized type of '%s'.\n", name);
		exit(EXIT_FAILURE);
	}

	return mxGetScalar(arr);
}
s3d_t *loads3d(const char *filename) {
	MATFile *mfp;
	s3d_t *s;

	mfp = matOpen(filename, "r");
	if (mfp == NULL) {
		fprintf(stderr, "ERROR: Unable to open S3D file: %s.\n", filename);
		return NULL;
	}

	s = malloc(sizeof(s3d_t));

	/* pixels */
	s->pixels = (size_t)get_scalar(mfp, "pixels");

	/* Bounds */
	s->xmin = get_scalar(mfp, "xmin");
	s->xmax = get_scalar(mfp, "xmax");
	s->ymin = get_scalar(mfp, "ymin");
	s->ymax = get_scalar(mfp, "ymax");
	s->zmin = get_scalar(mfp, "zmin");
	s->zmax = get_scalar(mfp, "zmax");

	/* Data */
	s->data = get_image(mfp, "image", s->pixels);

	matClose(mfp);

	return s;
}

