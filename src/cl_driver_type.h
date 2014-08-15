/**************************************************************************
 * cl_driver:
 * Hide behind some call backs the buffer allocation / deallocation ... This
 * will allow us to make the use of a software performance simulator easier and
 * to minimize the code specific for the HW and for the simulator
 **************************************************************************/
#ifndef __CL_DRIVER_TYPE_H__
#define __CL_DRIVER_TYPE_H__

/* Encapsulates command buffer / data buffer / kernels */
typedef struct _cl_buffer *cl_buffer;

/* Encapsulates buffer manager */
typedef struct _cl_buffer_mgr *cl_buffer_mgr;

/* Encapsulates the driver backend functionalities */
typedef struct _cl_driver *cl_driver;

/* Encapsulates the gpgpu stream of commands */
typedef struct _cl_gpgpu *cl_gpgpu;

/* Encapsulates the event  of a command stream */
typedef struct _cl_gpgpu_event *cl_gpgpu_event;

typedef struct _cl_context_prop *cl_context_prop;

#endif
