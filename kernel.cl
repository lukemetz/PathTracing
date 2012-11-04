
typedef struct 
{
  float3 origin;
  float3 direction;
} Ray;

float random()
{
  return .5; 
}

inline float2 tent_distribution()
{
  float r = 2 * random();
  float2 ret;
  if (r < 1) {
    ret.x = sqrt(r)-1;
  } else {
    ret.x = 1-sqrt(2-r);
  }
  
  r = 2 * random();
  if (r < 1) {
    ret.y = sqrt(r)-1;
  } else {
    ret.y = 1-sqrt(2-r);
  }
  return ret;
}

float3 radiance(Ray * ray, int depth)
{
  float3 ret = {.5,.5,.5};

  return ret;
}

__kernel void simple_demo(__global int *src, __global int *dst, int factor, int width, int height)
{
  float3 origin = {50.0f, 52.0f, 295.6f};
  float3 direction = {0.0f, -.042612f, -1.0f};
  Ray cam = {origin, normalize(direction)};
	int x = get_global_id(0);
  int y = get_global_id(1);
  float3 cx = width * .5135f / height;
  float3 cy = normalize( cross( cx, cam.direction ) ) * .5135f;
  
  int samps = 10;

  float2 pixel = {x,y};
  for (int sy = 0; sy < 2; ++sy) {
    int i=(height - y - 1) * width + x;
    for (int sx = 0; sx < 2; ++sx) {
      float3 sub_radiance = 0;
      for (int s=0; s<samps; s++){
        float4 r = 0;
        float2 delta = tent_distribution();

        float3 d = cx * ( ( ( sx + .5f + delta.x) / 2 + pixel.x) 
              / width) - .5f +
                   cy * ( ( ( sy + .5f + delta.y) / 2 + pixel.y) 
              / height) - .5f + cam.direction;

        Ray ray = { cam.origin + d * 140, normalize( d ) };
        sub_radiance += radiance( &ray, 0 ) * ( 1.0f / samps);
      }
    }
  }
	dst[x*width+y] = x * 10 + y;

}
