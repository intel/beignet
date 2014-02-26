__kernel void compiler_getelementptr_bitcast(global float *src, global float *dst)
{
  int i = get_global_id(0);

  __local  float ldata[256];
  ldata[get_local_id(0)] = src[i];

  //if use get_local_id(0) to index ldata, the issue is not reproduced
  //so, just set the work group as 1 in the application
  __local uchar *  pldata = (__local uchar *)&ldata[0];
  uchar data;
  for(int k = 0; k < 3; k++){
    data = *pldata;
    pldata++;
  }

  dst[i] = data;
}
