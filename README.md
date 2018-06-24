s3dvid
======
A simple tool for visualizing space3d sycout output from
[SOFT](https://github.com/hoppe93/SOFT).

Compilation
-----------
This program requires the following software to be available on your system:

- [libpng](http://www.libpng.org/pub/png/libpng.html)
- [MATLAB](https://se.mathworks.com/products/matlab.html) (version R2006b or later)
- [OpenMP](https://www.openmp.org/)
- [CMake](https://cmake.org/)

To generate build files for the program, create a directory called `build` in
the top directory, and then run `cmake ../` in the newly created directory:
```bash
$ mkdir build && cd build && cmake ../
```
When `cmake` finishes, successfully, run `make` to compile all files:
```bash
$ make
```
Once compilation finishes, an executable file called `s3dvid` should exist under
`build/src/`.

Running
-------
This program reads settings from `stdin` in order, and therefore settings can
be specified either directly in the terminal by the user, or they can be put
in a text file (with one setting per line) which is put on `stdin` when
invoking the program:
```bash
$ build/src/s3dvid < mysettings.txt
```
The settings that must be specified are (in order)
1. Name of the input S3D file (output of SOFT).
2. Name template of the output file(s). A number and file extension (`.png`) will be appended to this name.
3. Number of frames per second in the output video.
4. Length of video (in seconds).
5. Frame height (in pixels).
6. Frame width (in pixels).
7. Vision angle of the camera to emulate.
8. Location of the camera to emulate. Coordinates should be separated by spaces (e.g. `1 0 0`).
9. Viewing direction of the camera to emulate. Coordinates should be separated by spaces (e.g. `0 1 0`).
10. Axis around which to rotate the camera view.
11. Intensity threshold of images. If `1`, each pixel of every frame will be normalized to the value of that frame's brightest pixel. If, for example, `0.5` is instead specified, each pixel of every frame will be normalized instead to half the value of the brightest pixel of that frame.

An example of a settings file which can be passed to the program:
```
data/s3d.mat
frames/frame
24
5
900
900
0.8
1.55 -2.5 0
0 1 0
0 0 1
0.5
```
Note that this very simple input format does *not* permit comments to be given
in the file.

Generating video
----------------
Despite having "video" in it's name, this program does not generate actual video
files. It instead generates individual PNG frames that can be stitched together
into a video. There are several ways to do this, but one fairly good way is to
use [ffmpeg](https://www.ffmpeg.org/):
```bash
$ ffmpeg -r 15 -f image2 -s 3840x2160 -i frames/frame%d.png -vcodec libx264 -crf 25 -pix_fmt yuv420p video.mp4
```
This will generate an MPEG-4 encoded video with a framerate of 15 FPS, where
each frame has dimensions 3840 (width) by 2160 (height) pixels using the 
`libx264` codec and `yuv420p` pixel format. Input files are all files located
under the directory `frames/` with filenames starting with `frame`, followed
by an integer, followed by the extension `.png` (files are read in order
according to their number). The output filename is `video.mp4`. The switch
`-f image2` specifies that we're inputting a sequence of images.
