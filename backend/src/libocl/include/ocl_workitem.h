#ifndef __OCL_WORKITEM_H__
#define __OCL_WORKITEM_H__

#include "ocl_types.h"

uint get_work_dim(void);
uint get_global_size(uint dimindx);
uint get_global_id(uint dimindx);
uint get_local_size(uint dimindx);
uint get_local_id(uint dimindx);
uint get_num_groups(uint dimindx);
uint get_group_id(uint dimindx);
uint get_global_offset(uint dimindx);

#endif  /* __OCL_WORKITEM_H__ */
