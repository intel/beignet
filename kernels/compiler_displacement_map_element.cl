kernel void compiler_displacement_map_element(const global uint *in, const global uint *offset, int w, int h, global uint *out) {
    const int cx = get_global_id(0);
    const int cy = get_global_id(1);
    uint c = offset[cy * w + cx];
    int x_pos = cx + c;
    int y_pos = cy + c;
    if(0 <= x_pos && x_pos < w && 0 <= y_pos && y_pos < h)
        out[cy * w + cx] = in[y_pos * w + x_pos];
    else
        out[cy * w + cx] = 0;
}
