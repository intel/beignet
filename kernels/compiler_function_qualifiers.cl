/* test OpenCL 1.1 Function Qualifiers (section 6.7) */
kernel void compiler_function_qualifiers()
__attribute__((vec_type_hint(float)))
__attribute__((work_group_size_hint(4,1,1)))
__attribute__((reqd_work_group_size(4,1,1)));

kernel void compiler_function_qualifiers()
{
}
