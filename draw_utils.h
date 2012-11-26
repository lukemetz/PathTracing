#pragma once
#include <GL/glew.h>
#include <GL/glfw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "general_utils.h"

void make_window(int width, int height);
void navigation(float *origin, float *direction);
void fill_pixels(int *pixels, int width, int height, float *out_r, float *out_g, float *out_b);