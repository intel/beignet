kernel void compiler_vector_inc(global char *dst, global char *src) {
    size_t i = get_global_id(0);
    char2 dst2 = vload2(i, dst);
    if (src[i] == 0)
      dst2++;
    else if(src[i] == 1)
      ++dst2;
    else if(src[i] == 2)
      dst2--;
    else
      --dst2;
    vstore2(dst2, i, dst);
}
