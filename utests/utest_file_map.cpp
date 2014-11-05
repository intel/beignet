/* 
 * Copyright © 2012 Intel Corporation
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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "utest_file_map.hpp"
#include "CL/cl.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int
cl_file_map_init(cl_file_map_t *fm)
{
  assert(fm);
  memset(fm,0,sizeof(*fm));
  return CL_SUCCESS;
}

void
cl_file_map_destroy(cl_file_map_t *fm)
{
  if (fm->mapped) {
    munmap(fm->start, fm->size);
    fm->start = fm->stop = 0;
    fm->size = 0;
    fm->mapped = CL_FALSE;
  }
  if(fm->fd) {
    close(fm->fd);
    fm->fd = 0;
  }
  free(fm->name);
  memset(fm,0,sizeof(*fm));
}

void
cl_file_map_delete(cl_file_map_t *fm)
{
  if (fm == NULL)
    return;
  cl_file_map_destroy(fm);
  free(fm);
}

cl_file_map_t*
cl_file_map_new(void)
{
  cl_file_map_t *fm = NULL;

  if ((fm = (cl_file_map_t *) calloc(1, sizeof(cl_file_map_t))) == NULL)
    goto error;
  if (cl_file_map_init(fm) != CL_SUCCESS)
    goto error;

exit:
  return fm;
error:
  cl_file_map_delete(fm);
  fm = NULL;
  goto exit;
}

int
cl_file_map_open(cl_file_map_t *fm, const char *name)
{
  int err = CL_FILE_MAP_SUCCESS;

  /* Open the file */
  fm->fd = open(name, O_RDONLY);
  if(fm->fd < 0) {
    err = CL_FILE_MAP_FILE_NOT_FOUND;
    goto error;
  }
  if ((fm->name = (char*) calloc(strlen(name) + 1, sizeof(char))) == NULL)
    goto error;
  sprintf(fm->name, "%s", name);

  /* Map it */
  fm->size = lseek(fm->fd, 0, SEEK_END);
  lseek(fm->fd, 0, SEEK_SET);
  fm->start = mmap(0, fm->size, PROT_READ, MAP_SHARED, fm->fd, 0);
  if(fm->start == NULL) {
    err = CL_FILE_MAP_FAILED_TO_MMAP;
    goto error;
  }

  fm->stop = ((char *) fm->start) + fm->size;
  fm->mapped = CL_TRUE;

exit:
  return err;
error:
  cl_file_map_destroy(fm);
  goto exit;
}

