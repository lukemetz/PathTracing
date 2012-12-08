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
  ray->direction = ray->direction-normal*2*dot(normal, ray->direction); //Ideal reflection
}

inline void refract(Ray *ray, float3 normal, float3 hit_pos, float3 *accumulated_reflectance, unsigned int *seed)
{
    float3 normal_l;
    if (dot(normal, ray->direction) < 0) {
      normal_l = normal;
    } else {
      normal_l = -1*normal;
    }

    float3 new_dir = ray->direction-normal*2*dot(normal, ray->direction);// Ideal dielectric refraction

    bool into = dot(normal, normal_l)>0;// Ray from outside going in?
    float nc=1, nt=1.5;
    float nnt;
    if (into) {
      nnt = nc/nt;
    } else {
      nnt = nt/nc;
    }
    float ddn = dot(ray->direction, normal_l);
    float cos2t = 1 - nnt * nnt * (1 - ddn * ddn);

    if (cos2t < 0){    // Total internal reflection
      ray->direction = new_dir;
      ray->origin = hit_pos;
      return;
    }

    float3 tdir = normalize((ray->direction*nnt - normal * ((into?1:-1) * (ddn * nnt+sqrt(cos2t)))));
    float a=nt-nc;
    float b=nt+nc;
    float R0=a*a/(b*b);
    float c = 1-(into?-ddn:dot(tdir, normal));
    float Re=R0+(1-R0)*c*c*c*c*c;
    float Tr=1-Re;
    float P=.25+.5*Re;
    float RP=Re/P;
    float TP=Tr/(1-P);

    if (random(seed)<P){
      *accumulated_reflectance = (*accumulated_reflectance)*RP;
      ray->direction = new_dir;
      ray->origin = hit_pos;
    } else {
      *accumulated_reflectance = (*accumulated_reflectance)*TP;
      ray->direction = tdir;
      ray->origin = hit_pos;
    }
}