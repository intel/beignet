kernel void compiler_private_data_overflow( __global int4 *output )
{
	int4 data[65];
	for( int i=0; i<65; ++i )
	{
		data[i] = (int4)i;
	}
	if( get_global_id(0) == 1 )
		*output = data[0];
}
