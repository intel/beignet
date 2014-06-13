typedef struct _test_arg_struct {
    int a;
    int b;
}test_arg_struct;

kernel void test_get_arg_info(global float const volatile *src, local int *dst, test_arg_struct extra) {

}
