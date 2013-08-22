__kernel
void compiler_bool_cross_basic_block(__global int *src,
				     __global int *dst,
				     int scale){
  int id = (int)get_global_id(0);

  bool isRedRow = false;
  bool isRed;
  int val = src[id];
  for (unsigned int i=0; i<scale; i++, isRedRow = !isRedRow) {
    if (isRedRow) {
      isRed= false;
      for (unsigned int j=0; j < scale; j++, isRed=!isRed) {
        if (isRed) {
	  val++;
        }
      }
    }
  }
  dst[id] = val;
}
