__kernel void blend (__global float *out_r,
                     __global float *out_g,
                     __global float *out_b,
                     __global float *prev_r,
                     __global float *prev_g,
                     __global float *prev_b,
                     __global float *current_r,
                     __global float *current_g,
                     __global float *current_b,
                     int width,
                     int height,
                     float blend
                    )
{
	int array_index = get_global_id(0);
	int x = (array_index)%width;
	int y = (array_index)/width;

	out_r[array_index] = prev_r[array_index] * (1-blend) + current_r[array_index] * (blend);
	out_g[array_index] = prev_g[array_index] * (1-blend) + current_g[array_index] * (blend);
	out_b[array_index] = prev_b[array_index] * (1-blend) + current_b[array_index] * (blend);
};