#include "general_utils.h"
#include <time.h>

float clamp(float x)
{
	return x<0 ? 0 : x>1 ? 1 : x;
}

int to_int(float val)
{
	return (int) (pow(clamp(val),1/2.2)*255+.5);
}

char *get_text_from_file(char *filename)
{
	//Hack to get kernel to fully recompile
	int hack_size = 5;
	FILE *f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	long pos = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *program_source = calloc(pos+hack_size, sizeof(char));
	fread(program_source, pos-1, 1, f);
	fclose(f);
	//Add random chars to force kernel to recompile fully with includes
	program_source[pos-1] = '/';
	program_source[pos] = '/';
	srand ( time(NULL) );
	program_source[pos+1] = rand()%255;
	program_source[pos+2] = rand()%255;
	program_source[pos+3] = rand()%255;
	program_source[pos+4] = '\0';
	return program_source;
}

void save_to_file(int width, int height, float *out_r, float *out_g, float *out_b)
{
	//create the ppm file
	FILE *fout = fopen("image.ppm", "w");
	//sets width and height of ppm
	fprintf(fout, "P3\n%d %d\n%d\n", width, height, 255);
	//iterate through each pixel
	for (int i=0; i<width*height; i++) {
	//appends the next pixel to the ppm file (it knows where in the ppm to put it)
		fprintf(fout, "%d %d %d ", to_int(out_r[i]), to_int(out_g[i]), to_int(out_b[i]));
	}
}