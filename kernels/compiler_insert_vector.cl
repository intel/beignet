__kernel void
compiler_insert_vector(__global int4 *out )
{
    int tid = get_global_id(0);
    int4 output = (int4)(0, 0, 0, 1); //black
    if (tid > 16)
    {
        output = (int4)(tid, tid, 1, 1);
    }
    out[tid] = output;
}
