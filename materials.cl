#include "types.h"

inline void diffuse(Ray *ray, float3 normal, float3 hit_pos, unsigned int *seed)
{
  float3 normal_l;
  if (dot(normal, ray->direction) < 0) {
    normal_l = normal;
  } else {
    normal_l = -1*normal;
  }
  float r1 = 2*M_PI*random(seed);
  float r2 = random(seed);
  float r2_sqrt = sqrt(r2);

  //Create orthonormal basis uvw
  float3 w=normal_l;
  float3 u = {0,0,0};
  if (fabs(w.x) > .1) {
    u.y = 1;
  } else {
    u.x = 1;
  }
  u = normalize(cross(u, w));
  float3 v = cross(w,u);

  float3 d = normalize(u*cos(r1)*r2_sqrt + v*sin(r1)*r2_sqrt + w*sqrt(1-r2));
  ray->origin = hit_pos;
  ray->direction = d;
}

inline void reflect(Ray *ray, float3 normal, float3 hit_pos, unsigned int *seed)
{
  ray->origin = hit_pos;
  ray->direction = ray->direction-normal*2*dot(normal, ray->direction);
}