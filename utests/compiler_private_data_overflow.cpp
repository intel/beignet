#include "utest_helper.hpp"

void compiler_private_data_overflow(void)
{
	OCL_CREATE_KERNEL( "compiler_private_data_overflow" );
	OCL_CREATE_BUFFER( buf[0], 0, sizeof(cl_int4), NULL );
	OCL_SET_ARG( 0, sizeof(cl_mem), &buf[0] );
	globals[0] = 64;
	locals[0] = 32;
	OCL_NDRANGE(1);
	OCL_MAP_BUFFER(0);
	OCL_ASSERT( ((uint32_t *)buf_data[0])[0] == 0 );
	OCL_UNMAP_BUFFER(0);
}
MAKE_UTEST_FROM_FUNCTION( compiler_private_data_overflow );
