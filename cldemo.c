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
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "draw_utils.h"
#include "general_utils.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))

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


void print_cl_platforms(cl_platform_id *platforms, int platforms_n)
{
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
}

void print_cl_devices(cl_device_id *devices, int devices_n)
{
	printf("=== %d OpenCL device(s) found on platform:\n", devices_n);
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

}

void make_clprogram(cl_program *program, char *file, cl_context *context, cl_device_id *devices)
{
	char *program_source = get_text_from_file(file);

	*program = CL_CHECK_ERR(clCreateProgramWithSource(*context, 1, (const char **)&program_source, NULL, &_err));
	printf("Compiling %s \n", file);
	if (clBuildProgram(*program, 1, devices, "", NULL, NULL) != CL_SUCCESS) {
		char buffer[10240];
		clGetProgramBuildInfo(*program, devices[0], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
		fprintf(stderr, "CL Compilation failed:\n%s", buffer);
		abort();
	}
	//CL_CHECK(clUnloadCompiler()); gives compiler warning but probably need
	free(program_source);
}

int main(int argc, char **argv)
{
	bool debug = true;
	cl_platform_id platforms[100];
	cl_uint platforms_n = 0;
	CL_CHECK(clGetPlatformIDs(100, platforms, &platforms_n));

	print_cl_platforms(platforms, platforms_n);

	if (platforms_n == 0) {
		fprintf(stderr, "error: No platforms found :(\n");
		exit(1);
	}

	cl_device_id devices[100];
	cl_uint devices_n = 0;
	CL_CHECK(clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 100, devices, &devices_n));

	print_cl_devices(devices, devices_n);
	if (devices_n == 0) {
		fprintf(stderr, "error: No Devices found :(\n");
		exit(1);
	}

	cl_context context = CL_CHECK_ERR(clCreateContext(NULL, 1, devices, &pfn_notify, NULL, &_err));


	//Set up cl program and kernel
	cl_program path_trace_program;
	make_clprogram(&path_trace_program, "kernel.cl", &context, devices);
	cl_kernel path_kernel = CL_CHECK_ERR(clCreateKernel(path_trace_program, "path_trace", &_err));

	cl_program blend_program;
	make_clprogram(&blend_program, "blend.cl", &context, devices);
	cl_kernel blend_kernel = CL_CHECK_ERR(clCreateKernel(blend_program, "blend", &_err));

	int width = 1024/2;
	int height = 768/2;
	//width=height=100;
	float blend_amount = 0.5f;
	if (debug) {
		make_window(width*2, height);
	} else {
		make_window(width, height);
	}

	//the actual size of the work group is widthxheight
	size_t total_size = width*height*sizeof(float);

	int *pixels = (int *)malloc(sizeof(int)*width*height);

	#define MAX_WORKGROUP 10000
	//Buffers for path_tracing
	cl_mem output_r_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*MAX_WORKGROUP, NULL, &_err));
	cl_mem output_g_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*MAX_WORKGROUP, NULL, &_err));
	cl_mem output_b_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*MAX_WORKGROUP, NULL, &_err));
	cl_mem random_seeds_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int)*MAX_WORKGROUP, NULL, &_err));
	cl_mem input_origin_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*3, NULL, &_err));
	cl_mem input_dir_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*3, NULL, &_err));


	cl_command_queue queue = CL_CHECK_ERR(clCreateCommandQueue(context, devices[0], 0, &_err));

	//Buffers for blending
	cl_mem prev_r_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*total_size, NULL, &_err));
	cl_mem prev_g_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*total_size, NULL, &_err));
	cl_mem prev_b_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*total_size, NULL, &_err));
	cl_mem current_r_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*total_size, NULL, &_err));
	cl_mem current_g_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*total_size, NULL, &_err));
	cl_mem current_b_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*total_size, NULL, &_err));
	cl_mem full_output_r_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*total_size, NULL, &_err));
	cl_mem full_output_g_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*total_size, NULL, &_err));
	cl_mem full_output_b_buf = CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*total_size, NULL, &_err));
	//set up random seeds buffer
	unsigned int seed;
	printf("calculating random seeds \n");
	for (int i=0; i < MAX_WORKGROUP; ++i) {
		seed = rand();//seed*mult;
		CL_CHECK(clEnqueueWriteBuffer(queue, random_seeds_buf, CL_TRUE, i*sizeof(int), sizeof(int), &seed, 0, NULL, NULL));
	}

	printf("done random calculation \n");

	cl_event kernel_completion;

	//this work group is two dimensional
	int work_group_size = 1;
	float *out_r = (float *)calloc(total_size, sizeof(float));
	float *out_g = (float *)calloc(total_size, sizeof(float));
	float *out_b = (float *)calloc(total_size, sizeof(float));

	float *prev_r = (float *)calloc(total_size, sizeof(float));
	float *prev_g = (float *)calloc(total_size, sizeof(float));
	float *prev_b = (float *)calloc(total_size, sizeof(float));

	float *origin_in=(float *)malloc(sizeof(float)*3);
	float *direction_in=(float *)malloc(sizeof(float)*3);

	origin_in[0]=50.0f;
	origin_in[1]=52.0f;
	origin_in[2]=295.6f;

	direction_in[0]=0.0f;
	direction_in[1]=-.042612f;
	direction_in[2]=-1.0f;

	do {
		navigation(origin_in, direction_in);
		CL_CHECK(clEnqueueWriteBuffer(queue, input_origin_buf, CL_TRUE, 0, 3*sizeof(float), origin_in, 0, NULL, NULL));
		CL_CHECK(clEnqueueWriteBuffer(queue, input_dir_buf, CL_TRUE, 0, 3*sizeof(float), direction_in, 0, NULL, NULL));

		clock_t time_after;
		clock_t time_before;
		time_before = clock();

		//sets the arguments of path_trace in order
		CL_CHECK(clSetKernelArg(path_kernel, 0, sizeof(random_seeds_buf), &random_seeds_buf));
		CL_CHECK(clSetKernelArg(path_kernel, 1, sizeof(output_r_buf), &output_r_buf));
		CL_CHECK(clSetKernelArg(path_kernel, 2, sizeof(output_g_buf), &output_g_buf));
		CL_CHECK(clSetKernelArg(path_kernel, 3, sizeof(output_b_buf), &output_b_buf));
		CL_CHECK(clSetKernelArg(path_kernel, 4, sizeof(width), &width));
		CL_CHECK(clSetKernelArg(path_kernel, 5, sizeof(height), &height));

		CL_CHECK(clSetKernelArg(path_kernel, 7, sizeof(input_origin_buf), &input_origin_buf));
		CL_CHECK(clSetKernelArg(path_kernel, 8, sizeof(input_dir_buf), &input_dir_buf));

		int batches = (width*height)/MAX_WORKGROUP+1;
		for(int i=0; i < batches; ++i) {
			size_t workgroup_amount = min(width*height-i*MAX_WORKGROUP, MAX_WORKGROUP);
			//Fix evenly divisible MAX_WORKGROUPs
			if(workgroup_amount <= 0)
				break;

			size_t global_work_size[1] = { workgroup_amount };
			int offset = i*MAX_WORKGROUP;
			CL_CHECK(clSetKernelArg(path_kernel, 6, sizeof(offset), &offset));
			printf("running %d / %d \n", i, batches);
		 	CL_CHECK(clEnqueueNDRangeKernel(queue, path_kernel, work_group_size, NULL, global_work_size, NULL, 0, NULL, &kernel_completion));
			//kernel either runs when you call clWaitForEvents (below) or it runs when you call
		 	CL_CHECK(clWaitForEvents(1, &kernel_completion));

			CL_CHECK(clEnqueueReadBuffer(queue, output_r_buf, CL_TRUE, 0, sizeof(float)*workgroup_amount, out_r+i*MAX_WORKGROUP, 0, NULL, NULL));
			CL_CHECK(clEnqueueReadBuffer(queue, output_g_buf, CL_TRUE, 0, sizeof(float)*workgroup_amount, out_g+i*MAX_WORKGROUP, 0, NULL, NULL));
			CL_CHECK(clEnqueueReadBuffer(queue, output_b_buf, CL_TRUE, 0, sizeof(float)*workgroup_amount, out_b+i*MAX_WORKGROUP, 0, NULL, NULL));
		}
		//Done with path tracing
		printf("Done with tracing \n");
		glClear(GL_COLOR_BUFFER_BIT);
		//move to center self in screen
		if (debug) {
			glRasterPos2i(0,-1);
			fill_pixels(pixels, width, height, out_r, out_g, out_b);
			glDrawPixels(width, height, GL_RGBA,  GL_UNSIGNED_BYTE, pixels);
		}
		//Set up blend input buffers
		CL_CHECK(clEnqueueWriteBuffer(queue, prev_r_buf, CL_TRUE, 0, total_size*sizeof(float), prev_r, 0, NULL, NULL));
				printf("buffer1 set \n");

		CL_CHECK(clEnqueueWriteBuffer(queue, prev_g_buf, CL_TRUE, 0, total_size*sizeof(float), prev_g, 0, NULL, NULL));
				printf("buffer2 set \n");
		CL_CHECK(clEnqueueWriteBuffer(queue, prev_b_buf, CL_TRUE, 0, total_size*sizeof(float), prev_b, 0, NULL, NULL));

		CL_CHECK(clEnqueueWriteBuffer(queue, current_r_buf, CL_TRUE, 0, total_size*sizeof(float), out_r, 0, NULL, NULL));
		CL_CHECK(clEnqueueWriteBuffer(queue, current_g_buf, CL_TRUE, 0, total_size*sizeof(float), out_g, 0, NULL, NULL));
		CL_CHECK(clEnqueueWriteBuffer(queue, current_b_buf, CL_TRUE, 0, total_size*sizeof(float), out_b, 0, NULL, NULL));

		printf("setup buffers \n");
		CL_CHECK(clSetKernelArg(blend_kernel, 0, sizeof(full_output_r_buf), &full_output_r_buf));

		CL_CHECK(clSetKernelArg(blend_kernel, 1, sizeof(full_output_g_buf), &full_output_g_buf));
		CL_CHECK(clSetKernelArg(blend_kernel, 2, sizeof(full_output_b_buf), &full_output_b_buf));

		CL_CHECK(clSetKernelArg(blend_kernel, 3, sizeof(prev_r_buf), &prev_r_buf));
		CL_CHECK(clSetKernelArg(blend_kernel, 4, sizeof(prev_g_buf), &prev_g_buf));
		CL_CHECK(clSetKernelArg(blend_kernel, 5, sizeof(prev_b_buf), &prev_b_buf));
		CL_CHECK(clSetKernelArg(blend_kernel, 6, sizeof(current_r_buf), &current_r_buf));
		CL_CHECK(clSetKernelArg(blend_kernel, 7, sizeof(current_g_buf), &current_g_buf));
		CL_CHECK(clSetKernelArg(blend_kernel, 8, sizeof(current_b_buf), &current_b_buf));
		CL_CHECK(clSetKernelArg(blend_kernel, 9, sizeof(width), &width));
		CL_CHECK(clSetKernelArg(blend_kernel, 10, sizeof(height), &height));
		CL_CHECK(clSetKernelArg(blend_kernel, 11, sizeof(blend_amount), &blend_amount));

		//Run the kernel
		printf("running blend \n");
		size_t global_work_size[1] = { total_size };
		CL_CHECK(clEnqueueNDRangeKernel(queue, blend_kernel, work_group_size, NULL, global_work_size, NULL, 0, NULL, &kernel_completion));
		//kernel either runs when you call clWaitForEvents (below) or it runs when you call
		CL_CHECK(clWaitForEvents(1, &kernel_completion));

		CL_CHECK(clEnqueueReadBuffer(queue, full_output_r_buf, CL_TRUE, 0, sizeof(float)*total_size, out_r, 0, NULL, NULL));
		CL_CHECK(clEnqueueReadBuffer(queue, full_output_g_buf, CL_TRUE, 0, sizeof(float)*total_size, out_g, 0, NULL, NULL));
		CL_CHECK(clEnqueueReadBuffer(queue, full_output_b_buf, CL_TRUE, 0, sizeof(float)*total_size, out_b, 0, NULL, NULL));

		time_after = clock();

		glRasterPos2i(-1,-1);
		fill_pixels(pixels, width, height, out_r, out_g, out_b);
		glDrawPixels(width, height, GL_RGBA,  GL_UNSIGNED_BYTE, pixels);

		float time_diff = ((float)(time_after-time_before))/CLOCKS_PER_SEC;
		float samps_per_second = (50.0f*4*width*height)/time_diff;
		printf("Done in %f seconds at a rate of %fK samples per second \n",time_diff, samps_per_second/1000);
		//save_to_file(width, height, out_r, out_g, out_b);

		//Save current image to previous
		for(int i=0; i < total_size; ++i) {
			prev_r[i] = out_r[i];
			prev_g[i] = out_g[i];
			prev_b[i] = out_b[i];
		}

		glfwSwapBuffers();
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS && glfwGetWindowParam( GLFW_OPENED ) );

	CL_CHECK(clReleaseEvent(kernel_completion));

	//boiler plate stuff, you can ignore it (but don't touch!)
	printf("\n");
	CL_CHECK(clReleaseMemObject(random_seeds_buf));
	CL_CHECK(clReleaseMemObject(output_r_buf));
	CL_CHECK(clReleaseMemObject(output_g_buf));
	CL_CHECK(clReleaseMemObject(output_b_buf));

	CL_CHECK(clReleaseKernel(path_kernel));
	CL_CHECK(clReleaseProgram(path_trace_program));

	CL_CHECK(clReleaseKernel(blend_kernel));
	CL_CHECK(clReleaseProgram(blend_program));

	CL_CHECK(clReleaseContext(context));
	return 0;

}