#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

float clamp(float x);
int to_int(float val);
char *get_text_from_file(char *filename);
void save_to_file(int width, int height, float *out_r, float *out_g, float *out_b);