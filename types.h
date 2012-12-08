#ifndef __TYPES_H__
#define __TYPES_H__

typedef struct
{
  float3 origin;
  float3 direction;
} Ray;

typedef struct
{
  float radius;
  float3 position;
  float3 emission;
  float3 color;
  int material;
} Sphere;

#endif