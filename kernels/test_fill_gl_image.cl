__kernel void
test_fill_gl_image(image2d_t img, int color)
{
	int2 coord;
        float4 color_v4;
        coord.x = get_global_id(0);
        coord.y = get_global_id(1);
        color_v4 = (float4){((color >> 24) & 0xFF), (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF};
        color_v4 = color_v4 / 255.0f;
        write_imagef(img, coord, color_v4);
}
