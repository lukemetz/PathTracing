__kernel void simple_demo(__global int *src, __global int *dst, int factor, int width, int height)
{
	int i = get_global_id(0);
  int j = get_global_id(1);

	dst[i*width+j] = i*10+j;
}
