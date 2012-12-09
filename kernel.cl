//kernel.cl
//Luke Metz,Alec Radford, Orion Taylor
//This defines the various functions that are run within the graphics card
//Franklin W. Olin College of Engineering
//11/12/2012


//defines the Ray structure. Rays have an origin and a direction.
//They are really just vectors with an associated position
#include "types.h"

//hard code in the scene for now
__constant Sphere spheres[] = {//Scene: radius, position, emission, color, material
  {1e3,   { 1e3+1,40.8,81.6}, {0.1,0,0},    {.75,.25,.25}, 0},//Left
  {1e3,   {-1e3+99,40.8,81.6},{0,0.1,0},    {.25,.25,.75}, 0},//Rght
  {1e3,   {50,40.8, 1e3},     {0,0,0},    {.75,.75,.75}, 0},//Back
  {1e3,   {50,40.8,-1e3+170}, {0,0,0},    {0,0,0}, 0},//Frnt
  {1e3,   {50, 1e3, 81.6},    {0,0,0},    {.75,.75,.75}, 0},//Botm
  {1,   {50,-1e3+81.6,81.6},{0,0,0},    {.75,.75,.75}, 0},//Top
  {16.5,  {27,16.5,47},       {0,0,0},    {0.9f, 0.9f, 0.9f}, 1},
  {16.5,  {73,16.5,78},       {0,0,0},    {0.9f, 0.9f, 0.9f}, 2},
  {600,   {50,681.6-.27,81.6},{1.5,1.5,1.5}, {0,0,0}, 0} //Lite
};


//generates a random value and increments the seed
__constant static unsigned int mult = 2654435761U;
inline float random(unsigned int *seed)
{
  *seed = (*seed)*mult;
  return (*seed)/((float)0xFFFFFFFF);
}

//pseudo random number generator (returns a 2d vector living inside unit square)
inline float2 tent_distribution(unsigned int *seed)
{
  //collect first random value
  float r = 2 * random(seed);

  //create return variable
  float2 ret;

  //maps your uniform distribution to a tent distribution
  if (r < 1) {
    ret.x = sqrt(r)-1;
    } else {
    ret.x = 1-sqrt(2-r);
  }

  //collects second random value
  r = 2 * random(seed);

  //maps your uniform distribution to a tent distribution
  if (r < 1) {
    ret.y = sqrt(r)-1;
    } else {
    ret.y = 1-sqrt(2-r);
  }
  return ret;
}



inline float sphere_intersect_ray(__constant Sphere *sphere, Ray *ray)
{
  float3 op = sphere->position - ray->origin; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
  float eps = 1e-2;
  float b = dot(ray->direction, op);

  float det = b * b - dot(op, op) + sphere->radius * sphere->radius;
  //double det = (double)b * b - (double)dot(op, op) + (double) sphere->radius * sphere->radius; 
  //Think about doing a check to check the sanity of the det. If its like way to big, then return inf and ignore.
  //Check with like det (dist squared) squared. Check det squared with reference 
  //Det with old distance cubed
  //NOTE Have to add one before squaring
  if (det > 0) {
    det = sqrt(det);
    float t = b-det;
    if (t > eps) {
      return t;
    }
    t = b+det;
    if (t > eps) {
      return t;
    }
  }
  return 0;
}

inline bool intersect(Ray *ray, float *t, int *id, int oldID)
{
  int n=9;//sizeof(spheres);
  float inf = 1e20;
  *t = inf;
  for(int i=0; n > i; ++i) {
   // if (i != oldID || spheres[i].material == 2) { //CHECK HERE if object CAN self intersect, transparent obj for example.
      float d = sphere_intersect_ray(&(spheres[i]), ray);
      if ( d != 0 && d < (*t)) {
        (*t) = d;
        (*id) = i;
      }
    //}
  }
  return (*t) < inf;
}


#include "materials.cl"

float3 radiance(unsigned int * seed, Ray * ray, int depth)
{
  float t;
  int id;
  float3 accumulated_color = {0,0,0};
  float3 accumulated_reflectance = {1,1,1};

  int oldID = -1;
  while(1) {
    if (!intersect(ray, &t, &id, oldID)) {
      return accumulated_color;
    }
    oldID = id;

    __constant Sphere * obj = &(spheres[id]);
    float3 hit_pos = ray->origin + ray->direction*t;
    float3 normal = normalize(hit_pos-obj->position);
    float3 f = obj->color;

    float p;//max refl
    if (f.x > f.y && f.x > f.z) {
      p = f.x;
    } else if (f.y > f.z) {
      p = f.y;
    } else {
      p = f.z;
    }

    accumulated_color += accumulated_reflectance*obj->emission;
    if (++depth > 5) {
      if (random(seed) < p) {
        f = f*(1/p);
      } else {
        return accumulated_color;
      }
    }
    accumulated_reflectance = accumulated_reflectance*f;

    if (obj->material == 0) {
      diffuse(ray, normal, hit_pos, seed);
    } else if (obj->material == 1) {
      reflect(ray, normal, hit_pos, seed);
    } else if (obj->material == 2) {
      refract(ray, normal, hit_pos, &accumulated_reflectance, seed);
    }
  }

  return accumulated_color;
}

//inputs are the five kernel arguments (the first 3 are output buffer pointers)
__kernel void path_trace(__global int *seeds,
                        int seeds_size,
                         int seed_offset,
                        __global float *out,
                        int width,
                        int height,
                        int offset,
			                  __global float *origin_in,
			                  __global float *direction_in
                        )
{

  //defines camera position
  //float3 origin = {50.0f, 52.0f, 295.6f};
  float3 origin = {origin_in[0], origin_in[1], origin_in[2]};
  //defines camera direction
  //float3 direction = {0.0f, -.042612f, -1.0f};
  float3 direction = {direction_in[0], direction_in[1], direction_in[2]};

  //defines camera vector
  Ray cam = {origin, normalize(direction)};

  //returns the position of pixel in the work group
  int array_index = get_global_id(0);
  int x = (array_index+offset)%width;
  int y = (array_index+offset)/width;
  //int array_index = (height-y-1)*width+x;//x+width*y;
  unsigned int seed = seeds[(array_index+seed_offset)%seeds_size];

  //projects rays through the lens
  float3 cx = {width * .5135f / height, 0, 0};
 // float3 crss = {cx.x*cam.direction.z-cx.z*cam.direction.y,cx.z*cam.direction.x-cx.x*cam.direction.z,cx.x*cam.direction.y-cx.y*cam.direction.x};
  float3 cy = normalize(cross(cx, cam.direction)) * .5135f;
  //number of samples being run for the pixel
  //int samps = 50;
  int samps = 10;
  float3 final_radiance = 0;

  //pixel is a position in the pixel array
  float2 pixel = {x,y};

  //iterates through subpixels in y direction (2 of these)
  for (int sy = 0; sy < 2; ++sy) {

    //iterates through subpixels in x direction (2 of these)
    for (int sx = 0; sx < 2; ++sx) {

      //initializes radiance of this subpixel
      float3 sub_radiance = 0;

      //iterates samps times
      for (int s=0; s<samps; s++){
        //delta is a random vector (lives inside the unit square)
        float2 delta = tent_distribution(&seed);

        float3 ray_direction = cx * ( ( (sx + .5f + delta.x)/2 + pixel.x)/width - .5f)
                             + cy * ( ( (sy + .5f + delta.y)/2 + pixel.y)/height - .5f)
                             + cam.direction;

        //ray is a vector starting at the camera position + d*140
        //pointing in the direction of d
        Ray ray = { cam.origin + ray_direction * 140, normalize( ray_direction )};
        //calculate the radiance value for the subpixel for this sample
        //then average them across samples
        sub_radiance += radiance(&seed, &ray, 0 ) * ( 1.0f / samps);
      }

      //average the radiance values across subpixels
      //to find the radiance value of a given pixel
        final_radiance += sub_radiance*clamp(sub_radiance, 0.f, 1.0f)*.25f;
      }
    }

    //places result in the correct place within the output buffer
  out[array_index*3] = final_radiance.x;
  out[array_index*3+1] = final_radiance.y;
  out[array_index*3+2] = final_radiance.z;
}
