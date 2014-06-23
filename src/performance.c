#include <performance.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX_KERNEL_NAME_LENGTH 100
#define MAX_KERNEL_EXECUTION_COUNT 100000
#define MAX_KERNEL_BUILD_OPT 1000
typedef struct kernel_storage_node
{
  char kernel_name[MAX_KERNEL_NAME_LENGTH];
  float kernel_times[MAX_KERNEL_EXECUTION_COUNT];
  char build_option[MAX_KERNEL_BUILD_OPT];
  int current_count;
  float kernel_sum_time;
  struct kernel_storage_node *next;
} kernel_storage_node;

typedef struct context_storage_node
{
  uintptr_t context_id;
  kernel_storage_node *kernels_storage;
  char max_time_kernel_name[MAX_KERNEL_NAME_LENGTH];
  float kernel_max_time;
  int kernel_count;
  struct context_storage_node *next;
} context_storage_node;

typedef struct storage
{
  context_storage_node * context_storage;
} storage;



static storage record;
static int atexit_registered = 0;


static context_storage_node * prev_context_pointer = NULL;
static kernel_storage_node * prev_kernel_pointer = NULL;

static context_storage_node * find_context(cl_context context)
{
  if(NULL != prev_context_pointer )
  {
    if(prev_context_pointer->context_id == (uintptr_t)context)
      return prev_context_pointer;
  }

  if(NULL == record.context_storage)
  {
    record.context_storage = (context_storage_node *) malloc(sizeof(context_storage_node));
    record.context_storage->context_id = (uintptr_t)context;
    record.context_storage->kernels_storage = NULL;
    record.context_storage->kernel_max_time = 0.0f;
    record.context_storage->next = NULL;
    record.context_storage->kernel_count = 0;
    return record.context_storage;
  }

  context_storage_node *pre = record.context_storage;
  context_storage_node *cur = record.context_storage;
  while(NULL !=cur && (uintptr_t)context != cur->context_id )
  {
    pre = cur;
    cur = cur->next;
  }
  if(NULL != cur)
    return cur;

  pre->next = (context_storage_node *)malloc(sizeof(context_storage_node));
  pre = pre->next;
  pre->context_id = (uintptr_t)context;
  pre->kernels_storage = NULL;
  pre->kernel_max_time = 0.0f;
  pre->next = NULL;
  pre->kernel_count = 0;
  return pre;
}

static kernel_storage_node * find_kernel(context_storage_node *p_context, const char *kernel_name, const char *build_opt)
{
  if(NULL != prev_kernel_pointer && NULL != prev_context_pointer &&
     p_context == prev_context_pointer &&
     !strncmp(kernel_name, prev_kernel_pointer->kernel_name, MAX_KERNEL_NAME_LENGTH) &&
     !strncmp(build_opt, prev_kernel_pointer->build_option, MAX_KERNEL_BUILD_OPT))
    return prev_kernel_pointer;

  if(NULL == p_context)
    return NULL;

  if(NULL == p_context->kernels_storage)
  {
    p_context->kernels_storage = (kernel_storage_node *)malloc(sizeof(kernel_storage_node));
    p_context->kernel_count++;
    strncpy(p_context->kernels_storage->kernel_name,kernel_name, MAX_KERNEL_NAME_LENGTH);
    p_context->kernels_storage->kernel_name[MAX_KERNEL_NAME_LENGTH - 1] = '\0';
    strncpy(p_context->kernels_storage->build_option, build_opt, MAX_KERNEL_BUILD_OPT);
    p_context->kernels_storage->build_option[MAX_KERNEL_BUILD_OPT - 1] = '\0';
    p_context->kernels_storage->current_count = 0;
    p_context->kernels_storage->kernel_sum_time = 0.0f;
    p_context->kernels_storage->next = NULL;
    return p_context->kernels_storage;
  }

  kernel_storage_node *pre = p_context->kernels_storage;
  kernel_storage_node *cur = p_context->kernels_storage;
  while(NULL != cur &&
        (strncmp(cur->kernel_name, kernel_name, MAX_KERNEL_NAME_LENGTH) ||
         strncmp(cur->build_option, build_opt, MAX_KERNEL_BUILD_OPT)))
  {
    pre = cur;
    cur = cur->next;
  }
  if(NULL != cur)
    return cur;

  p_context->kernel_count++;
  pre->next = (kernel_storage_node *)malloc(sizeof(kernel_storage_node));
  pre = pre->next;
  pre->current_count = 0;
  pre->kernel_sum_time = 0.0f;
  pre->next = NULL;
  strncpy(pre->kernel_name, kernel_name, MAX_KERNEL_NAME_LENGTH);
  pre->kernel_name[MAX_KERNEL_NAME_LENGTH - 1] = '\0';
  strncpy(pre->build_option, build_opt, MAX_KERNEL_BUILD_OPT);
  pre->build_option[MAX_KERNEL_NAME_LENGTH - 1] = '\0';
  return pre;
}

static void free_storage()
{
  context_storage_node *p_context = record.context_storage;
  while(NULL != p_context)
  {
    context_storage_node *p_tmp_context = p_context->next;
    kernel_storage_node *p_kernel = p_context->kernels_storage;
    while(NULL != p_kernel)
    {
      kernel_storage_node *p_tmp_kernel = p_kernel->next;
      free(p_kernel);
      p_kernel = p_tmp_kernel;
    }
    free(p_context);
    p_context = p_tmp_context;
  }
}

typedef struct time_element
{
  char kernel_name[MAX_KERNEL_NAME_LENGTH];
  float kernel_sum_time;
  int kernel_execute_count;
  double dev;
  float kernel_times[MAX_KERNEL_EXECUTION_COUNT];
  uint32_t time_index;
} time_element;

static int cmp(const void *a, const void *b)
{
  if(((time_element *)a)->kernel_sum_time < ((time_element *)b)->kernel_sum_time)
    return 1;
  else if(((time_element *)a)->kernel_sum_time > ((time_element *)b)->kernel_sum_time)
    return -1;
  else
    return 0;
}

static void print_time_info()
{
  context_storage_node *p_context = record.context_storage;
  if(NULL == p_context)
  {
    printf("Nothing to output !\n");
    return;
  }

  int tmp_context_id = 0;
  while(NULL != p_context)
  {
    printf("[------------ CONTEXT %4d ------------]\n", tmp_context_id++);
    printf("  ->>>> KERNELS TIME SUMMARY <<<<-\n");

    kernel_storage_node *p_kernel = p_context->kernels_storage;
    kernel_storage_node *p_tmp_kernel = p_kernel;
    time_element *te = (time_element *)malloc(sizeof(time_element)*p_context->kernel_count);
    memset(te, 0, sizeof(time_element)*p_context->kernel_count);
    int i = -1, j = 0, k = 0;
    while(NULL != p_tmp_kernel)
    {
      for(k=0; k<=i; k++)
      {
        if(!strncmp(te[k].kernel_name, p_tmp_kernel->kernel_name, MAX_KERNEL_NAME_LENGTH))
          break;
      }
      if(k == i+1)
      {
        i++;
        k = i;
      }
      te[k].kernel_execute_count += p_tmp_kernel->current_count;
      strncpy(te[k].kernel_name, p_tmp_kernel->kernel_name, MAX_KERNEL_NAME_LENGTH);
      te[k].kernel_name[MAX_KERNEL_NAME_LENGTH - 1] = '\0';
      te[k].kernel_sum_time += p_tmp_kernel->kernel_sum_time;
      for(j=0; j != p_tmp_kernel->current_count; ++j)
        te[k].kernel_times[te[k].time_index++] = p_tmp_kernel->kernel_times[j];
      p_tmp_kernel = p_tmp_kernel->next;
    }

    for(k=0; k<=i; k++)
    {
      float average = te[k].kernel_sum_time / te[k].kernel_execute_count;
      double sumsquare = 0.0;
      for(j=0; j<te[k].time_index; ++j)
        sumsquare += pow((te[k].kernel_times[j] - average), 2.0);
      te[k].dev = sqrt(sumsquare / te[k].kernel_execute_count);
    }

    float sum_time = 0.0f;
    qsort((void *)te, p_context->kernel_count, sizeof(time_element), cmp);
    for(j=0; j<=i; ++j)
      sum_time += te[j].kernel_sum_time;

    for(j=0; j<=i; ++j)
    {
      printf("    [Kernel Name: %-30s Time(ms): (%4.1f%%) %9.2f  Count: %-7d  Ave(ms): %7.2f  Dev: %.1lf%%]\n",
             te[j].kernel_name,
             te[j].kernel_sum_time / sum_time * 100,
             te[j].kernel_sum_time,
             te[j].kernel_execute_count,
             te[j].kernel_sum_time / te[j].kernel_execute_count,
             te[j].dev / te[j].kernel_sum_time * te[j].kernel_execute_count * 100);
    }
    free(te);
    printf("    Total : %.2f\n", sum_time);
    if(2 != b_output_kernel_perf)
    {
      printf("[------------  CONTEXT ENDS------------]\n\n");
      p_context = p_context->next;
      continue;
    }
    p_tmp_kernel = p_kernel;
    printf("\n  ->>>> KERNELS TIME DETAIL <<<<-\n");
    while(NULL != p_kernel)
    {
      printf("    [Kernel Name : %30s   Time(ms): %.2f]\n", p_kernel->kernel_name, p_kernel->kernel_sum_time);
      if(*p_kernel->build_option != '\0')
      {
        int count = 0;
        printf("      ->Build Options : ");
        while(p_kernel->build_option[count] != '\0' )
        {
          printf("%c", p_kernel->build_option[count++]);
          if(count % 100 == 0)
            printf("\n                         ");
        }
        printf("\n");
      }
      for(i=0; i!=p_kernel->current_count; ++i)
        printf("      Execution Round%5d : %.2f (ms)\n", i+1, p_kernel->kernel_times[i]);
      p_kernel = p_kernel->next;
    }
    printf("[------------  CONTEXT ENDS------------]\n\n");
    p_context = p_context->next;
  }
  free_storage();
}


static void insert(cl_context context, const char *kernel_name, const char *build_opt, float time)
{
  if(!atexit_registered)
  {
    atexit_registered = 1;
    atexit(print_time_info);
  }
  context_storage_node *p_context = find_context(context);
  kernel_storage_node *p_kernel = find_kernel(p_context, kernel_name, build_opt);
  prev_context_pointer = p_context;
  prev_kernel_pointer = p_kernel;
  p_kernel->kernel_times[p_kernel->current_count++] = time;
  p_kernel->kernel_sum_time += time;
  if(p_kernel->kernel_sum_time > p_context->kernel_max_time)
  {
    p_context->kernel_max_time = p_kernel->kernel_sum_time;
    strncpy(p_context->max_time_kernel_name, kernel_name, MAX_KERNEL_NAME_LENGTH);
    p_context->max_time_kernel_name[MAX_KERNEL_NAME_LENGTH - 1] = '\0';
  }
}


static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int b_output_kernel_perf = 0;
static struct timeval start, end;

void initialize_env_var()
{
  char *env = getenv("OCL_OUTPUT_KERNEL_PERF");
  if(NULL == env || !strncmp(env,"0", 1))
    b_output_kernel_perf = 0;
  else if(!strncmp(env,"1", 1))
    b_output_kernel_perf = 1;
  else
    b_output_kernel_perf = 2;
}

void time_start(cl_context context, const char * kernel_name, cl_command_queue cq)
{
  pthread_mutex_lock(&mutex);
  gettimeofday(&start, NULL);
}

void time_end(cl_context context, const char * kernel_name, const char * build_opt, cl_command_queue cq)
{
  clFinish(cq);
  gettimeofday(&end, NULL);
  float t = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000.0f;
  insert(context, kernel_name, build_opt, t);
  pthread_mutex_unlock(&mutex);
}
