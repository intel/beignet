typedef struct {
  int  offset;
  int  threshold0;
  int  threshold1;
}hop;

__kernel void compiler_argument_structure_select(__global int *dst, hop h)
{
  int i = get_global_id (0);
  int threshold=0;
  if (i == 0)  {
    threshold = h.threshold0;
  } else {
    threshold = h.threshold1;
  }
  dst[i] = threshold;
}

