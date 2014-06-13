__kernel void image_1D_buffer(image1d_buffer_t image1, image1d_t image2, sampler_t sampler, __global int *results)
{
   int x = get_global_id(0);
   int offset = x;

   int4 col = read_imagei(image1, x);
   int4 test = (col != read_imagei(image2, sampler, x));

   if (test.x || test.y || test.z || test.w)
      results[offset] = 0;
   else
      results[offset] = 1;
}
