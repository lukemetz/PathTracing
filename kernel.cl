__kernel void simple_demo(__global int *src, __global int *dst, int factor)
{
	int i = get_global_id(0);
	dst[i] = src[i] * factor;
}
