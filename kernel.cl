//kernel.cl
//Luke Metz,Alec Radford, Orion Taylor
//This defines the various functions that are run within the graphics card
//Franklin W. Olin College of Engineering
//11/12/2012


//defines the Ray structure. Rays have an origin and a direction.
//They are really just vectors with an associated position
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
} Sphere;

//hard code in the scene for now
__constant Sphere spheres[] = {//Scene: radius, position, emission, color, material
  {1e5,   { 1e5+1,40.8,81.6}, {0,0,0},    {.75,.25,.25}},//Left
  {1e5,   {-1e5+99,40.8,81.6},{0,0,0},    {.25,.25,.75}},//Rght
  {1e5,   {50,40.8, 1e5},     {0,0,0},    {.75,.75,.75}},//Back
  {1e5,   {50,40.8,-1e5+170}, {0,0,0},    {0,0,0}},//Frnt
  {1e5,   {50, 1e5, 81.6},    {0,0,0},    {.75,.75,.75}},//Botm
  {1e5,   {50,-1e5+81.6,81.6},{0,0,0},    {.75,.75,.75}},//Top
  {16.5,  {27,16.5,47},       {0,0,0},    {0.9f, 0.9f, 0.9f}},
  {16.5,  {73,16.5,78},       {0,0,0},    {0.9f, 0.9f, 0.9f}},
  {600,   {50,681.6-.27,81.6},{12,12,12}, {0,0,0}} //Lite
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
  float eps = 1e-4;
  float b = dot(ray->direction, op);

  float det = b * b - dot(op, op) + sphere->radius * sphere->radius;
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

inline bool intersect(Ray *ray, float *t, int *id)
{
  int n=9;//sizeof(spheres);
  float inf = 1e20;
  *t = inf;
  for(int i=0; n > i; ++i) {
    float d = sphere_intersect_ray(&(spheres[i]), ray);
    if ( d != 0 && d < (*t)) {
      (*t) = d;
      (*id) = i;
    }
  }
  return (*t) < inf;
}

float3 radiance(unsigned int * seed, Ray * ray, int depth)
{
  float t;
  int id;
  float3 accumulated_color = {0,0,0};
  float3 accumulated_reflectance = {1,1,1};
  while(1) {
    if (!intersect(ray, &t, &id)) {
      return accumulated_color; 
    }
    __constant Sphere * obj = &(spheres[id]);
    float3 hit_pos = ray->origin + ray->direction*t;
    float3 normal = normalize(hit_pos-obj->position);
    float3 normal_l;
    if (dot(normal, ray->direction) < 0) {
      normal_l = normal;
    } else {
      normal_l = -1*normal;
    }
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
    if (++depth > 3) {
      if (random(seed) < p) {
        f = f*(1/p);
      } else {
        return accumulated_color;
      }
    }
    accumulated_reflectance = accumulated_reflectance*f;

    //Just diffusion for now
    {
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
      continue;
    }
  }

  return accumulated_color;
}

//inputs are the five kernel arguments (the first 3 are output buffer pointers)
__kernel void path_trace(__global int *seeds,
                        __global float *out_r,
                        __global float *out_g,
                        __global float *out_b,
                        int width,
                        int height,
                        int offset
                        )
{

  //defines camera position
  float3 origin = {50.0f, 52.0f, 295.6f};
  
  //defines camera direction
  float3 direction = {0.0f, -.042612f, -1.0f};
  
  //defines camera vector
  Ray cam = {origin, normalize(direction)};

  //returns the position of pixel in the work group
  int array_index = get_global_id(0);
  int x = (array_index+offset)%width;
  int y = (array_index+offset)/width;
  //int array_index = (height-y-1)*width+x;//x+width*y;
  unsigned int seed = seeds[array_index];

  //projects rays through the lens
  float3 cx = {width * .5135f / height, 0, 0};
 // float3 crss = {cx.x*cam.direction.z-cx.z*cam.direction.y,cx.z*cam.direction.x-cx.x*cam.direction.z,cx.x*cam.direction.y-cx.y*cam.direction.x};
  float3 cy = normalize(cross(cx, cam.direction)) * .5135f;
  //number of samples being run for the pixel
  int samps = 50;
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
  out_r[array_index] = final_radiance.x;
  out_g[array_index] = final_radiance.y;
  out_b[array_index] = final_radiance.z;
}