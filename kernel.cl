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

//random number generator (currently just returns .5)
float random()
{
  return .5; 
}

//pseudo random number generator (returns a 2d vector living inside unit square)
inline float2 tent_distribution()
{

  //collect first random value
  float r = 2 * random();

  //create return variable
  float2 ret;

  //maps your uniform distribution to a tent distribution
  if (r < 1) {
    ret.x = sqrt(r)-1;
  } else {
    ret.x = 1-sqrt(2-r);
  }
  
  //collects second random value
  r = 2 * random();

  //maps your uniform distribution to a tent distribution
  if (r < 1) {
    ret.y = sqrt(r)-1;
  } else {
    ret.y = 1-sqrt(2-r);
  }
  return ret;
}

//this needs to be filled out (put complicated math in here)
float3 radiance(Ray * ray, int depth)
{
  float3 ret = {.8,.1,.3};

  return ret;
}

//inputs are the five kernel arguments (the first 3 are output buffer pointers)
__kernel void path_trace(__global float *out_r, __global float *out_g, __global float *out_b, int width, int height)
{
  
  //defines camera position
  float3 origin = {50.0f, 52.0f, 295.6f};
  
  //defines camera direction
  float3 direction = {0.0f, -.042612f, -1.0f};
  
  //defines camera vector
  Ray cam = {origin, normalize(direction)};
	
  //returns the position of pixel in the work group
  int x = get_global_id(0);
  int y = get_global_id(1);

  //projects rays through the lens
  float3 cx = width * .5135f / height;
  float3 cy = normalize( cross( cx, cam.direction ) ) * .5135f;
  
  //number of samples being run for the pixel
  int samps = 1;
  
  float3 final_radiance = 0;

  //pixel is a position in the pixel array 
  float2 pixel = {x,y};

  //iterates through subpixels in y direction (2 of these)
  for (int sy = 0; sy < 2; ++sy) {

    //not sure what i does (comment this out later)
    int i=(height - y - 1) * width + x;

    //iterates through subpixels in x direction (2 of these)
    for (int sx = 0; sx < 2; ++sx) {

      //initializes sub_radiance (which is radiance of a subpixel)
      float3 sub_radiance = 0;

      //iterates samps times
      for (int s=0; s<samps; s++){

	//what is r? I don't know what this does (comment it out later)
        float4 r = 0;

	//delta is a random vector (lives inside the unit square)
        float2 delta = tent_distribution();

	//what is d? it is used, but I'm not quite sure what it does
        float3 d = cx * ( ( ( sx + .5f + delta.x) / 2 + pixel.x) 
              / width) - .5f +
                   cy * ( ( ( sy + .5f + delta.y) / 2 + pixel.y) 
              / height) - .5f + cam.direction;

	//ray is a vector starting at the camera position + d*140
	//pointing in the direction of d
        Ray ray = { cam.origin + d * 140, normalize( d ) };

	//calculate the radiance value for the subpixel for this sample
	//then average them across samples
        sub_radiance += radiance( &ray, 0 ) * ( 1.0f / samps);
      }

	//average the radiance values across subpixels
	//to find the radiance value of a given pixel
        final_radiance +=sub_radiance*.25f;// clamp(sub_radiance, 0.f, 1.0f)*.25f;
    }
  }

  //places result in the correct place within the output buffer
  out_r[x+width*y] = final_radiance.x;
  out_g[x+width*y] = final_radiance.y;
  out_b[x+width*y] = final_radiance.z;
}
