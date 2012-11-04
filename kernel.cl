
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
  float3 ret = {.8,.1,.3};

  return ret;
}

__kernel void path_trace(__global float *out_r, __global float *out_g, __global float *out_b, int width, int height)
{
  float3 origin = {50.0f, 52.0f, 295.6f};
  float3 direction = {0.0f, -.042612f, -1.0f};
  Ray cam = {origin, normalize(direction)};
	int x = get_global_id(0);
  int y = get_global_id(1);
  float3 cx = width * .5135f / height;
  float3 cy = normalize( cross( cx, cam.direction ) ) * .5135f;
  
  int samps = 1;
  
  float3 final_radiance = 0;

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
        final_radiance +=sub_radiance*.25f;// clamp(sub_radiance, 0.f, 1.0f)*.25f;
    }
  }
  out_r[x+width*y] = final_radiance.x;
  out_g[x+width*y] = final_radiance.y;
  out_b[x+width*y] = final_radiance.z;
}
