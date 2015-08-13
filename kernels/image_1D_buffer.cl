__kernel void image_1D_buffer(image1d_buffer_t image1, image1d_buffer_t image2)
{
   int x = get_global_id(0);

   uint4 color = read_imageui(image1, x);
   write_imageui(image2, x, color);
}
