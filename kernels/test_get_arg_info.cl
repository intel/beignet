typedef struct _test_arg_struct {
    int a;
    int b;
}test_arg_struct;

kernel void test_get_arg_info(read_only global float const volatile *src, read_write local int read_only *dst, test_arg_struct extra) {

}
