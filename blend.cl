
inline void linear(__global float *out_r, __global float *out_g, __global float *out_b, 
                   __global float *prev_r, __global float *prev_g,__global float *prev_b,
                   __global float *cur_r, __global float *cur_g,  __global float *cur_b,
                   int width, int height,
                   float blend
                   );

inline void median(__global float *out_r, __global float *out_g, __global float *out_b, 
                   __global float *prev_r, __global float *prev_g,__global float *prev_b,
                   __global float *cur_r, __global float *cur_g,  __global float *cur_b,
                   int width, int height,
                   float blend
                   );

__kernel void blend (__global float *out_r,
                     __global float *out_g,
                     __global float *out_b,
                     __global float *prev_r,
                     __global float *prev_g,
                     __global float *prev_b,
                     __global float *cur_r,
                     __global float *cur_g,
                     __global float *cur_b,
                     int width,
                     int height,
                     float blend
                    )
{
   //linear(out_r, out_g, out_b, prev_r, prev_g, prev_b, cur_r, cur_g, cur_b, width, height, blend);
   median(out_r, out_g, out_b, prev_r, prev_g, prev_b, cur_r, cur_g, cur_b, width, height, blend);


	/*int array_index = get_global_id(0);
	int x = (array_index)%width;
	int y = (array_index)/width; */

/*
	out_r[array_index] = prev_r[array_index] * (1-blend) + current_r[array_index] * (blend);
	out_g[array_index] = prev_g[array_index] * (1-blend) + current_g[array_index] * (blend);
	out_b[array_index] = prev_b[array_index] * (1-blend) + current_b[array_index] * (blend); */
};
inline void median(__global float *out_r,
                     __global float *out_g,
                     __global float *out_b,
                     __global float *prev_r,
                     __global float *prev_g,
                     __global float *prev_b,
                     __global float *cur_r,
                     __global float *cur_g,
                     __global float *cur_b,
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
       rA[count] = cur_r[((y+j-off)*width)+i+x-off];
       gA[count] = cur_g[((y+j-off)*width)+i+x-off];
       bA[count] = cur_b[((y+j-off)*width)+i+x-off];
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
   out_r[index] = rA[4];
   out_g[index] = gA[4];
   out_b[index] = bA[4];

}

inline void linear(__global float *out_r,
                     __global float *out_g,
                     __global float *out_b,
                     __global float *prev_r,
                     __global float *prev_g,
                     __global float *prev_b,
                     __global float *cur_r,
                     __global float *cur_g,
                     __global float *cur_b,
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

   if (x > 10 && x < width - 10){
   if (y > 10 && y < height - 10){
      for (i = 0; i < win; i++){
        for (j = 0; j < win; j++){
           rVal += cur_r[((y+j-off)*width)+i+x-off];
           gVal += cur_g[((y+j-off)*width)+i+x-off];
           bVal += cur_b[((y+j-off)*width)+i+x-off];
           count++;
        }
      }
      rVal = rVal/9;
      gVal = gVal/9;
      bVal = bVal/9;
      out_r[index] = rVal;
      out_g[index] = gVal;
      out_b[index] = bVal;
    }
   }
   else{
    out_r[index] = cur_r[index];
    out_g[index] = cur_g[index];
    out_b[index] = cur_b[index];
   }

}
