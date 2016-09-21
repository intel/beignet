/*
 * Copyright Â© 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "cl_command_queue.h"
#include "CL/cl.h"
#include <stdio.h>

cl_int
clFlush(cl_command_queue command_queue)
{
  if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
    return CL_INVALID_COMMAND_QUEUE;
  }

  return cl_command_queue_wait_flush(command_queue);
}

cl_int
clFinish(cl_command_queue command_queue)
{
  if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
    return CL_INVALID_COMMAND_QUEUE;
  }

  return cl_command_queue_wait_finish(command_queue);
}


cl_int
clReleaseCommandQueue(cl_command_queue command_queue)
{
  if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
    return CL_INVALID_COMMAND_QUEUE;
  }

  cl_command_queue_wait_flush(command_queue);

  cl_command_queue_delete(command_queue);
  return CL_SUCCESS;
}

