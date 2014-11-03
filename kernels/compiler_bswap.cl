kernel void compiler_bswap_short(global short* src, global short* dst){
   dst[get_global_id(0)]= __builtin_bswap16(src[get_global_id(0)]);
}

kernel void compiler_bswap_int(global int* src, global int* dst){
   dst[get_global_id(0)]= __builtin_bswap32(src[get_global_id(0)]);
}
