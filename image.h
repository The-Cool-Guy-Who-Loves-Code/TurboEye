#ifndef IMAGE_H
#define IMAGE_H

#include <iostream>
#include <string>
#include <vector>
#include <future>
#include <cmath>
#include <algorithm>
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>
#include "LodePng/lodepng.h"
#include <algorithm>

//#define DEBUG


void write_png(const char* filename, const unsigned int* image, int width, int height);
void read_png(const char* filename, int width, int height, unsigned int* image);
#endif