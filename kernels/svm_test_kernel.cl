__kernel 
void test1(__global char* svm,
   uint svmBase,
   uint context)
{
   int i = get_global_id(0);
   svm -= svmBase;
   __global int *ptr = (__global int *)&svm[context];
   ptr[i]=i;
}

