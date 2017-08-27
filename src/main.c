/* Space3D video generator */

#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "camera.h"
#include "s3d.h"
#include "s3dpng.h"

#define FPS 24
#define VIDEOLENGTH 5
#define PIXELSI 100
#define PIXELSJ 100
#define VISANG 0.8
#define LOCATION {0.0,-1.069,0.0}
#define DIRECTION {0.4,0.5,0.0}
#define ROTATEAXIS {0,0,1}
#define OUTNAME "frames/frame"

#define PI (3.14159265359)

struct settings {
	size_t fps, videolength;
	size_t height, width;
	double visang;
	double location[3], direction[3];
	double rotate_axis[3];
	char *infile, *outfile;
};

struct timespec ticclock;

#pragma omp threadprivate(ticclock)

void tic(void) {
	clock_gettime(CLOCK_REALTIME, &ticclock);
}
double toc(void) {
	struct timespec tocclock;
	if (!clock_gettime(CLOCK_REALTIME, &tocclock)) {
		long ds = tocclock.tv_sec - ticclock.tv_sec;
		long dn = tocclock.tv_nsec - ticclock.tv_nsec;
		return (((double)ds) + ((double)dn)/1e9);
	} else return NAN;
}

/**
 * Read settings
 */
struct settings *read_settings(void) {
	struct settings *s;
	const int strmaxlen = 1024;
	long l;

	s = malloc(sizeof(struct settings));
	s->infile  = malloc(sizeof(char)*(strmaxlen+1));
	s->outfile = malloc(sizeof(char)*(strmaxlen+1));

	printf("Name of input file: ");
	fgets(s->infile, strmaxlen, stdin);
	l = strlen(s->infile);
	s->infile[l-1] = 0;
	printf("(%s)\n", s->infile);

	printf("Name of output file(s): ");
	fgets(s->outfile, strmaxlen, stdin);
	l = strlen(s->outfile);
	s->outfile[l-1] = 0;
	printf("(%s)\n", s->outfile);

	printf("Frames per second: ");
	scanf("%zu", &s->fps);
	printf("(%zu)\n", s->fps);

	printf("Length of video (s): ");
	scanf("%zu", &s->videolength);
	printf("(%zu)\n", s->videolength);

	printf("Frame height: ");
	scanf("%zu", &s->height);
	printf("(%zu)\n", s->height);

	printf("Frame width: ");
	scanf("%zu", &s->width);
	printf("(%zu)\n", s->width);

	printf("Vision angle: ");
	scanf("%lf", &s->visang);
	printf("(%lf)\n", s->visang);

	printf("Camera location: ");
	scanf("%lf %lf %lf", s->location, s->location+1, s->location+2);
	printf("(%lf, %lf, %lf)\n", s->location[0], s->location[1], s->location[2]);

	printf("Camera direction: ");
	scanf("%lf %lf %lf", s->direction, s->direction+1, s->direction+2);
	printf("(%lf, %lf, %lf)\n", s->direction[0], s->direction[1], s->direction[2]);
	
	printf("Axis of camera rotation: ");
	scanf("%lf %lf %lf", s->rotate_axis, s->rotate_axis+1, s->rotate_axis+2);
	printf("(%lf, %lf, %lf)\n", s->rotate_axis[0], s->rotate_axis[1], s->rotate_axis[2]);

	return s;
}

/**
 * Rotate a vector
 */
void rotate2(double angle, double v1[3], double v2[3], double origin[3], double axis[3]) {
	double n, s, c, ux, uy, uz, tx, ty, tz, vx, vy, vz, R[3][3];
	n = hypot(axis[0], hypot(axis[1], axis[2]));
	ux = axis[0] / n;
	uy = axis[1] / n;
	uz = axis[2] / n;

	vx = v1[0]-origin[0];
	vy = v1[1]-origin[1];
	vz = v1[2]-origin[2];
	
	s = sin(angle), c = cos(angle);
	R[0][0] = (c+ux*ux*(1-c));    R[0][1] = (ux*uy*(1-c)-uz*s); R[0][2] = (ux*uz*(1-c)+uy*s);
	R[1][0] = (uy*ux*(1-c)+uz*s); R[1][1] = (c+uy*uy*(1-c));    R[1][2] = (uy*uz*(1-c)+ux*s);
	R[2][0] = (uz*ux*(1-c)+uy*s); R[2][1] = (uz*uy*(1-c)+ux*s); R[2][2] = (c+uz*uz*(1-c));

	tx = R[0][0]*vx + R[0][1]*vy + R[0][2]*vz  + origin[0];
	ty = R[1][0]*vx + R[1][1]*vy + R[1][2]*vz  + origin[1];
	tz = R[2][0]*vx + R[2][1]*vy + R[2][2]*vz  + origin[2];
	v1[0]=tx; v1[1]=ty; v1[2]=tz;

	tx = R[0][0]*v2[0] + R[0][1]*v2[1] + R[0][2]*v2[2];
	ty = R[1][0]*v2[0] + R[1][1]*v2[1] + R[1][2]*v2[2];
	tz = R[2][0]*v2[0] + R[2][1]*v2[1] + R[2][2]*v2[2];
	v2[0]=tx; v2[1]=ty; v2[2]=tz;
}

void find_max_intensity(s3d_t *s, struct settings *set) {
	double **tmpimg, mx = 0.0;
	size_t i, j;

	camera_new_image();
	camera_init_local(set->location, set->direction);
	tmpimg = camera_generate(s);

	for (i = 0; i < set->height; i++) {
		for (j = 0; j < set->width; j++) {
			if (tmpimg[i][j] > mx)
				mx = tmpimg[i][j];
		}
	}

	camera_destroy_image();

	if (mx <= 0) {
		fprintf(stderr, "ERROR: Maximum value of reference image is %e\n", mx);
		exit(EXIT_FAILURE);
	}

	set_png_threshold(mx);
}

void generate_frames(
	s3d_t *s, double *angles, size_t *anglecount, double **anglestart,
	double dangle, struct settings *set, double centerpoint[3]
) {
	size_t j, offset=0;
	int tn = omp_get_thread_num(), mlen = strlen(set->outfile)+20;
	double avg = 0.0;
	double loc[3], dir[3];
	char *outname = malloc(sizeof(char)*mlen);

	/* Compute filename offset */
	for (j = 0; j < (size_t)tn; j++)
		offset += anglecount[j];

	camera_new_image();
	for (j = 0; j < anglecount[tn]; j++) {
		camera_clear_image();

		loc[0] = set->location[0];
		loc[1] = set->location[1];
		loc[2] = set->location[2];
		dir[0] = set->direction[0];
		dir[1] = set->direction[1];
		dir[2] = set->direction[2];

		rotate2(anglestart[tn][j], loc, dir, centerpoint, set->rotate_axis);
		camera_init_local(loc, dir);

		tic();
		double **img = camera_generate(s);
		avg += toc();

		snprintf(outname, mlen, "%s%zu.png", set->outfile, offset+j);
		saveimg(img, set->height, set->width, outname);
	}

	printf("Average time per frame on thread #%d: %.3fms\n", tn, avg*1e3/((double)anglecount[tn]));
}
void divide_among_threads(
	const size_t frames, double dangle, const size_t threads,
	double *angles, double **anglestart, size_t *anglecount
) {
	size_t i;
	angles[0] = 0.0;

	anglestart[0] = angles;
	for (i = 1; i < frames; i++)
		angles[i] = angles[i-1] + dangle;
	
	/* Divide frames among threads */
	if (threads > 1) {
		for (i = 0; i < threads; i++)
			anglecount[i] = frames / threads;
		for (i = 0; i < frames % threads; i++)
			anglecount[i]++;

		for (i = 1; i < threads; i++)
			anglestart[i] = anglestart[i-1] + anglecount[i-1];
	} else anglecount[0] = frames;
}

int main(int argc, char *argv[]) {
	s3d_t *s;
	struct settings *set;
	double centerpoint[3];
	const size_t threads = omp_get_max_threads();
	double *angles, dangle, **anglestart;
	size_t *anglecount;

	set = read_settings();

	s = loads3d(set->infile);
	if (s == NULL) return -1;

	s3d_center(s, centerpoint);

	angles = malloc(sizeof(double)*set->fps);
	anglecount = malloc(sizeof(size_t)*threads);
	anglestart = malloc(sizeof(double*)*threads);
	dangle = 2.0*PI / (double)(set->fps-1);

	divide_among_threads(set->fps, dangle, threads, angles, anglestart, anglecount);

	camera_init(set->height, set->width, set->visang);

	/* Find maximum intensity */
	find_max_intensity(s, set);
	
	#pragma omp parallel
	{
		generate_frames(
			s, angles, anglecount, anglestart, dangle,
			set, centerpoint
		);
	}

	return 0;
}

