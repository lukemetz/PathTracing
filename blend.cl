#define INDEX(x, y, width) (((y)*(width)+(x))*3)
inline void linear(__global float *out,
                   __global float *prev,
                   __global float *cur,
                   int width, int height,
                   float blend
                   );

inline void median(__global float *out,
                   __global float *prev,
                   __global float *cur,
                   int width, int height,
                   float blend
                   );

__kernel void blend (__global float *out,
                     __global float *prev,
                     __global float *cur,
                     int width,
                     int height,
                     float blend
                    )
{
   int array_index = get_global_id(0);
   //linear(out, prev, cur, width, height, blend);
   //median(out, prev, cur, width, height, blend);

	out[array_index*3] = prev[array_index*3] * (1-blend) + cur[array_index*3] * (blend);
	out[array_index*3+1] = prev[array_index*3+1] * (1-blend) + cur[array_index*3+1] * (blend);
	out[array_index*3+2] = prev[array_index*3+2] * (1-blend) + cur[array_index*3+2] * (blend);
};

inline void median(__global float *out,
                  __global float *prev,
                  __global float *cur,
                  int width,
                  int height,
                  float blend
                  ) {
   int index = get_global_id(0);
   int x = (index)%width;
   int y = (index)/width;
   int win = 3;
   int off = 1;
   float rA[9];
   float gA[9];
   float bA[9];
   int i;
   int j;
   int n = 9;
   int count = 0;
   float temp;
   float rVal = 0;
   float gVal = 0;
   float bVal = 0;
   for (i = 0; i < win; i++){
    for (j = 0; j < win; j++){
       rA[count] = cur[INDEX(i+x-off, y+j-off, width)];
       gA[count] = cur[INDEX(i+x-off, y+j-off, width)+1];
       bA[count] = cur[INDEX(i+x-off, y+j-off, width)+2];
       count = count+1;
    }
   }

   for (i = (n - 1); i > 0; i--)
   {
     for (j = 1; j <= i; j++)
     {
       if (rA[j-1] > rA[j])
       {
         temp = rA[j-1];
         rA[j-1] = rA[j];
         rA[j] = temp;
       }
       if (gA[j-1] > gA[j])
       {
         temp = gA[j-1];
         gA[j-1] = gA[j];
         gA[j] = temp;
       }
       if (bA[j-1] > bA[j])
       {
         temp = bA[j-1];
         bA[j-1] = bA[j];
         bA[j] = temp;
       }
     }
   }

   out[index*3] = rA[4];
   out[index*3+1] = gA[4];
   out[index*3+2] = bA[4];

}

inline void linear(__global float *out,
                   __global float *prev,
                   __global float *cur,
                   int width,
                   int height,
                   float blend
                   ) {
    int index = get_global_id(0);
    int x = (index)%width;
    int y = (index)/width;
    int win = 3;
    int off = 1;
    float rA[9];
    float gA[9];
    float bA[9];
    int i;
    int j;
    int n = 9;
    int count = 0;
    int temp;
    float rVal = 0;
    float gVal = 0;
    float bVal = 0;


  for (i = 0; i < win; i++){
    for (j = 0; j < win; j++){
       rVal += cur[INDEX(i+x-off, y+j-off, width)];
       gVal += cur[INDEX(i+x-off, y+j-off, width)+1];
       bVal += cur[INDEX(i+x-off, y+j-off, width)+2];
       count++;
    }
  }
  rVal = rVal/9;
  gVal = gVal/9;
  bVal = bVal/9;
  out[index*3] = rVal;
  out[index*3+1] = gVal;
  out[index*3+2] = bVal;
}
