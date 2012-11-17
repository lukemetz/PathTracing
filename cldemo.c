/*
 *  Path Tracing renderer
 *
 *  Based on:
 *  Simple OpenCL demo program
 *  Copyright (C) 2009  Clifford Wolf <clifford@clifford.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <CL/cl.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <math.h>

#define CL_CHECK(_expr)                                                         \
 do {                                                                         \
 	cl_int _err = _expr;                                                       \
 	if (_err == CL_SUCCESS)                                                    \
 		break;                                                                   \
 	fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err);   \
 	abort();                                                                   \
 } while (0)

#define CL_CHECK_ERR(_expr)                                                     \
 ({                                                                           \
 	cl_int _err = CL_INVALID_VALUE;                                            \
 	typeof(_expr) _ret = _expr;                                                \
 	if (_err != CL_SUCCESS) {                                                  \
 		fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err); \
 		abort();                                                                 \
 	}                                                                          \
 	_ret;                                                                      \
 })

 void pfn_notify(const char *errinfo, const void *private_info, size_t cb, void *user_data)
 {
 	fprintf(stderr, "OpenCL Error (via pfn_notify): %s\n", errinfo);
 }

 float clamp(float x)
 {
 	return x<0 ? 0 : x>1 ? 1 : x;
 }

 int to_int(float val)
 {
 	return (int) (pow(clamp(val),1/2.2)*255+.5);
 }

 int main(int argc, char **argv)
 {
 	cl_platform_id platforms[100];
 	cl_uint platforms_n = 0;
 	CL_CHECK(clGetPlatformIDs(100, platforms, &platforms_n));

 	printf("=== %d OpenCL platform(s) found: ===\n", platforms_n);
 	for (int i=0; i<platforms_n; i++){
 		char buffer[10240];
 		printf("  -- %d --\n", i);
 		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_PROFILE, 10240, buffer, NULL));
 		printf("  PROFILE = %s\n", buffer);
 		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, 10240, buffer, NULL));
 		printf("  VERSION = %s\n", buffer);
 		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 10240, buffer, NULL));
 		printf("  NAME = %s\n", buffer);
 		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, 10240, buffer, NULL));
 		printf("  VENDOR = %s\n", buffer);
 		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, 10240, buffer, NULL));
 		printf("  EXTENSIONS = %s\n", buffer);
 	}

 	if (platforms_n == 0)
 		return 1;

 	cl_device_id devices[100];
 	cl_uint devices_n = 0;
	// CL_CHECK(clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 100, devices, &devices_n));
 	CL_CHECK(clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 100, devices, &devices_n));

 	printf("=== %d OpenCL device(s) found on platform:\n", platforms_n);
 	for (int i=0; i<devices_n; i++)
 	{
 		char buffer[10240];
 		cl_uint buf_uint;
 		cl_ulong buf_ulong;
 		printf("  -- %d --\n", i);
 		CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(buffer), buffer, NULL));
 		printf("  DEVICE_NAME = %s\n", buffer);
 		CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL));
 		printf("  DEVICE_VENDOR = %s\n", buffer);
 		CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL));
 		printf("  DEVICE_VERSION = %s\n", buffer);
 		CL_CHECK(clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL));
 		printf("  DRIVER_VERSION = %s\n", buffer);
 		CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(buf_uint), &buf_uint, NULL));
 		printf("  DEVICE_MAX_COMPUTE_UNITS = %u\n", (unsigned int)buf_uint);
 		CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(buf_uint), &buf_uint, NULL));
 		printf("  DEVICE_MAX_CLOCK_FREQUENCY = %u\n", (unsigned int)buf_uint);
 		CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(buf_ulong), &buf_ulong, NULL));
 		printf("  DEVICE_GLOBAL_MEM_SIZE = %llu\n", (unsigned long long)buf_ulong);
 	}

 	if (devices_n == 0)
 		return 1;

 	cl_context context;
 	context = CL_CHECK_ERR(clCreateContext(NULL, 1, devices, &pfn_notify, NULL, &_err));

  //dumps contents of kernel.cl into a string (don't need to edit this)
 	FILE *f = fopen("kernel.cl", "rb");
 	fseek(f, 0, SEEK_END);
 	long pos = ftell(f);
 	fseek(f, 0, SEEK_SET);
 	char *program_source = malloc(pos);
 	fread(program_source, pos, 1, f);
 	fclose(f);

 	cl_program program;

 	program = CL_CHECK_ERR(clCreateProgramWithSource(context, 1, (const char **)&program_source, NULL, &_err));
 	if (clBuildProgram(program, 1, devices, "", NULL, NULL) != CL_SUCCESS) {
 		char buffer[10240];
 		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
 		fprintf(stderr, "CL Compilation failed:\n%s", buffer);
 		abort();
 	}
 	CL_CHECK(clUnloadCompiler());

 	int width = 320;
 	int height = 240;

 	cl_mem random_seeds;
 	cl_mem output_r;
 	cl_mem output_g;
 	cl_mem output_b;

 	random_seeds = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int)*width*height, NULL, &_err));
 	output_r = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*width*height, NULL, &_err));
 	output_g = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*width*height, NULL, &_err));
 	output_b = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*width*height, NULL, &_err));


	//create an instance of the kernel
 	cl_kernel kernel;

	//inputs are for the path_trace function of the kernel
 	kernel = CL_CHECK_ERR(clCreateKernel(program, "path_trace", &_err));

	//sets the arguments of path_trace in order	
	CL_CHECK(clSetKernelArg(kernel, 0, sizeof(random_seeds), &random_seeds));
 	CL_CHECK(clSetKernelArg(kernel, 1, sizeof(output_r), &output_r));
 	CL_CHECK(clSetKernelArg(kernel, 2, sizeof(output_g), &output_g));
 	CL_CHECK(clSetKernelArg(kernel, 3, sizeof(output_b), &output_b));
 	CL_CHECK(clSetKernelArg(kernel, 4, sizeof(width), &width));
 	CL_CHECK(clSetKernelArg(kernel, 5, sizeof(height), &height));

 	cl_command_queue queue;
 	queue = CL_CHECK_ERR(clCreateCommandQueue(context, devices[0], 0, &_err));
 	
 	//set up random seeds buffer


 	for (int i=0; i < width*height; ++i) {
 		int seed = rand();
 		CL_CHECK(clEnqueueWriteBuffer(queue, random_seeds, CL_TRUE, i*sizeof(int), sizeof(int), &seed, 0, NULL, NULL));
 	}

 	cl_event kernel_completion;

  	//this work group is two dimensional
 	int work_group_size = 2;
	//the actual size of the work group is widthxheight
 	size_t global_work_size[2] = { width, height};

	//this creates the work group
 	CL_CHECK(clEnqueueNDRangeKernel(queue, kernel, work_group_size, NULL, global_work_size, NULL, 0, NULL, &kernel_completion));

	//kernel either runs when you call clWaitForEvents (below) or it runs when you call 
	//clEnqueueNDRangeKernel (above), we are not really sure.
 	CL_CHECK(clWaitForEvents(1, &kernel_completion));
 	CL_CHECK(clReleaseEvent(kernel_completion));


  	//The following junk creates the image
 	printf("Result:");

  	//create the ppm file
 	FILE *fout = fopen("image.ppm", "w");

  	//sets width and height of ppm
 	fprintf(fout, "P3\n%d %d\n%d\n", width, height, 255);

	//read the output buffers
 	size_t size = width*height*sizeof(float);
 	float *out_r = (float *)malloc(size);
 	float *out_g = (float *)malloc(size);
 	float *out_b = (float *)malloc(size);
 	CL_CHECK(clEnqueueReadBuffer(queue, output_r, CL_TRUE, 0, size, out_r, 0, NULL, NULL));
 	CL_CHECK(clEnqueueReadBuffer(queue, output_g, CL_TRUE, 0, size, out_g, 0, NULL, NULL));
 	CL_CHECK(clEnqueueReadBuffer(queue, output_b, CL_TRUE, 0, size, out_b, 0, NULL, NULL));
	//iterate through each pixel
 	for (int i=0; i<width*height; i++) {
		//appends the next pixel to the ppm file (it knows where in the ppm to put it)
 		fprintf(fout, "%d %d %d ", to_int(out_r[i]), to_int(out_g[i]), to_int(out_b[i]));
 	}


	//boiler plate stuff, you can ignore it (but don't touch!)

 	printf("\n");

 	CL_CHECK(clReleaseMemObject(output_r));
 	CL_CHECK(clReleaseMemObject(output_g));
 	CL_CHECK(clReleaseMemObject(output_b));

 	CL_CHECK(clReleaseKernel(kernel));
 	CL_CHECK(clReleaseProgram(program));
 	CL_CHECK(clReleaseContext(context));
 	free(program_source);
 	return 0;
 }


