/*
 * Copyright © 2012 - 2014 Intel Corporation
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
#include "ocl_pipe.h"
#include "ocl_atom.h"
#include "ocl_workitem.h"

#define PIPE_SUCCESS 0
#define PIPE_EMPTY -2
#define PIPE_FULL -3
#define PIPE_HEADER_SZ 128
#define PIPE_INDEX_OUTRANGE -4
#define PIPE_RESERVE_FAIL -5
#define RID_MAGIC 0xDE
#define RIDT ushort
#define DEAD_PTR 0xFFFFFFFF

PURE CONST __global void* __gen_ocl_get_pipe(pipe int p);
PURE CONST ulong __gen_ocl_get_rid(reserve_id_t rid);
PURE CONST reserve_id_t __gen_ocl_make_rid(ulong rid);

int __read_pipe_2(pipe int p, __generic void* dst)
{
  __global int* pheader = (__global int*)__gen_ocl_get_pipe(p);
  int data_size = atomic_sub(pheader + 6, 1);
  if(data_size < 0){
    atomic_add(pheader + 6, 1);
    return PIPE_EMPTY; //Check if element exist
  }
  __global char* psrc = (__global char*)pheader + PIPE_HEADER_SZ;
  int pack_num = pheader[0];
  int pack_size = pheader[1];
  int pipe_size = pack_num * pack_size;
  int read_ptr = atomic_add(pheader + 3, 1);
  if(read_ptr == pack_num - 1)
    atomic_sub(pheader + 3, pack_num);
  read_ptr = read_ptr % pack_num;
  for(int i = 0; i < pack_size ; i++)
    ((char*)dst)[i] = psrc[i + read_ptr*pack_size];
  return 0;
}

int __read_pipe_4(pipe int p, reserve_id_t id, uint index, void* dst)
{
  __global int* pheader = (__global int*)__gen_ocl_get_pipe(p);
  __global char* psrc = (__global char*)pheader + PIPE_HEADER_SZ;
  ulong uid = __gen_ocl_get_rid(id);
  RIDT* pid = (RIDT*)&uid;
  RIDT start_pt = pid[0];
  RIDT reserve_size = pid[1];
  if(index > reserve_size) return PIPE_INDEX_OUTRANGE;
  int pack_num = pheader[0];
  int pack_size = pheader[1];
  int read_ptr = (start_pt + index) % pack_num;
  int offset = read_ptr * pack_size;
  for(int i = 0; i < pack_size ; i++)
    ((char*)dst)[i] = psrc[i + offset];
  return 0;
}


int __write_pipe_2(pipe int p, __generic void* src)
{
  __global int* pheader = (__global int*)__gen_ocl_get_pipe(p);
  int pack_num = pheader[0];
  int data_size = atomic_add(pheader + 6, 1);
  if(data_size >= pack_num){
    atomic_sub(pheader + 6, 1);
    return PIPE_FULL; //Check if pipe full
  }
  __global char* psrc = (__global char*)pheader + PIPE_HEADER_SZ;
  int pack_size = pheader[1];
  int pipe_size = pack_num * pack_size;
  int write_ptr = atomic_add(pheader + 2, 1);
  if(write_ptr == pack_num - 1)
    atomic_sub(pheader + 2, pack_num);
  write_ptr = write_ptr % pack_num;
  for(int i = 0; i < pack_size ; i++)
    psrc[i + write_ptr * pack_size] = ((char*)src)[i];
  return 0;
}

int __write_pipe_4(pipe int p, reserve_id_t id, uint index, void* src)
{
  __global int* pheader = __gen_ocl_get_pipe(p);
  __global char* psrc = (__global char*)pheader + PIPE_HEADER_SZ;
  ulong uid = __gen_ocl_get_rid(id);
  RIDT* pid = (RIDT*)&uid;
  RIDT start_pt = pid[0];
  RIDT reserve_size = pid[1];
  if(index > reserve_size) return PIPE_INDEX_OUTRANGE;
  int pack_num = pheader[0];
  int pack_size = pheader[1];
  int write_ptr = (start_pt + index) % pack_num;
  int offset = write_ptr * pack_size;
  for(int i = 0; i < pack_size ; i++)
    psrc[i + offset] = ((char*)src)[i];
  return pack_size;
}

reserve_id_t __reserve_read_pipe(pipe int p, uint num)
{
  __global int* pheader = (__global int*)__gen_ocl_get_pipe(p);
  int data_size = atomic_sub(pheader + 6, num);
  if(data_size < num){
    atomic_add(pheader + 6, num);
    return __gen_ocl_make_rid(0l);
  }
  int pack_num = pheader[0];
  int pack_size = pheader[1];
  int pipe_size = pack_num * pack_size;
  int read_ptr = atomic_add(pheader + 3, num);
  if(read_ptr == pack_num - num)
    atomic_sub(pheader + 3, pack_num);
  ulong uid = 0l;
  RIDT* pid = (RIDT*)&uid;
  pid[0] = read_ptr % pack_num;
  pid[1] = num;
  pid[2] = RID_MAGIC ;
  return __gen_ocl_make_rid(uid);
}

void __commit_read_pipe(pipe int p, reserve_id_t rid) {}

reserve_id_t __work_group_reserve_read_pipe(pipe int p, uint num)
{
  uint rid_ptr = DEAD_PTR;
  int ret0 = 0;
  if(get_local_linear_id()==0){
    __global int* pheader = (__global int*)__gen_ocl_get_pipe(p);
    int data_size = atomic_sub(pheader + 6, num);
    if(data_size < num){
      atomic_add(pheader + 6, num);
      int ret0 = 1;
    }
    int pack_num = pheader[0];
    int pack_size = pheader[1];
    int pipe_size = pack_num * pack_size;
    int read_ptr = atomic_add(pheader + 3, num);
    if(read_ptr == pack_num - num && !ret0)
      atomic_sub(pheader + 3, pack_num);
    if(!ret0)
      rid_ptr = read_ptr % pack_num;
  }
  ulong uid = 0l;
  RIDT* pid = (RIDT*)&uid;
  rid_ptr = work_group_broadcast(rid_ptr,0,0,0);
  pid[0] = rid_ptr;
  pid[1] = num;
  pid[2] = RID_MAGIC ;
  if(rid_ptr == DEAD_PTR)
    uid = 0l;
  return __gen_ocl_make_rid(uid);
}

void __work_group_commit_read_pipe(pipe int p, reserve_id_t rid) {}

reserve_id_t __sub_group_reserve_read_pipe(pipe int p, uint num)
{
  __global int* pheader = (__global int*)__gen_ocl_get_pipe(p);
  int data_size = atomic_sub(pheader + 6, num);
  if(data_size < num){
    atomic_add(pheader + 6, num);
    return __gen_ocl_make_rid(0l);
  }
  int pack_num = pheader[0];
  int pack_size = pheader[1];
  int pipe_size = pack_num * pack_size;
  int read_ptr = atomic_add(pheader + 3, num);
  if(read_ptr == pack_num - num)
    atomic_sub(pheader + 3, pack_num);
  ulong uid = 0l;
  RIDT* pid = (RIDT*)&uid;
  pid[0] = read_ptr % pack_num;
  pid[1] = num;
  pid[2] = RID_MAGIC ;
  return __gen_ocl_make_rid(uid);
}

void __sub_group_commit_read_pipe(pipe int p, reserve_id_t rid) {}

reserve_id_t __reserve_write_pipe(pipe int p, uint num)
{
  __global int* pheader = (__global int*)__gen_ocl_get_pipe(p);
  int pack_num = pheader[0];
  int data_size = atomic_add(pheader + 6, num);
  if(data_size > pack_num - num){
    atomic_sub(pheader + 6, num);
    return __gen_ocl_make_rid(0l);
  }
  int pack_size = pheader[1];
  int pipe_size = pack_num * pack_size;
  int write_ptr = atomic_add(pheader + 2, num);
  if(write_ptr == pack_num - num)
    atomic_sub(pheader + 2, pack_num);
  ulong uid = 0l;
  RIDT* pid = (RIDT*)&uid;
  pid[0] = write_ptr % pack_num;
  pid[1] = num;
  pid[2] = RID_MAGIC ;
  return __gen_ocl_make_rid(uid);
}
void __commit_write_pipe(pipe int p, reserve_id_t rid) {}

reserve_id_t __work_group_reserve_write_pipe(pipe int p, uint num)
{
  uint rid_ptr = DEAD_PTR;
  int ret0 = 0;
  if(get_local_linear_id()==0){
    __global int* pheader = (__global int*)__gen_ocl_get_pipe(p);
    int pack_num = pheader[0];
    int data_size = atomic_add(pheader + 6, num);
    if(data_size > pack_num - num){
      atomic_sub(pheader + 6, num);
      ret0 = 1;
    }
    int pack_size = pheader[1];
    int pipe_size = pack_num * pack_size;
    int write_ptr = atomic_add(pheader + 2, num);
    if(write_ptr == pack_num - num && !ret0)
      atomic_sub(pheader + 2, pack_num);
    if(!ret0)
      rid_ptr = write_ptr % pack_num;
  }
  ulong uid = 0l;
  RIDT* pid = (RIDT*)&uid;
  rid_ptr = work_group_broadcast(rid_ptr,0,0,0);
  pid[0] = rid_ptr;
  pid[1] = num;
  pid[2] = RID_MAGIC ;
  if(rid_ptr == DEAD_PTR)
    uid = 0l;
  return __gen_ocl_make_rid(uid);
}
void __work_group_commit_write_pipe(pipe int p, reserve_id_t rid) {}


reserve_id_t __sub_group_reserve_write_pipe(pipe int p, uint num)
{
  __global int* pheader = (__global int*)__gen_ocl_get_pipe(p);
  int pack_num = pheader[0];
  int data_size = atomic_add(pheader + 6, num);
  if(data_size > pack_num - num){
    atomic_sub(pheader + 6, num);
    return __gen_ocl_make_rid(0l);
  }
  int pack_size = pheader[1];
  int pipe_size = pack_num * pack_size;
  int write_ptr = atomic_add(pheader + 2, num);
  if(write_ptr == pack_num - num)
    atomic_sub(pheader + 2, pack_num);
  ulong uid = 0l;
  RIDT* pid = (RIDT*)&uid;
  pid[0] = write_ptr % pack_num;
  pid[1] = num;
  pid[2] = RID_MAGIC ;
  return __gen_ocl_make_rid(uid);
}

void __sub_group_commit_write_pipe(pipe int p, reserve_id_t rid) {}

bool is_valid_reserve_id(reserve_id_t rid)
{
  ulong uid = __gen_ocl_get_rid(rid);
  RIDT* pid = (RIDT*)&uid;
  if(pid[1] == 0) return false;
  if(pid[2] != RID_MAGIC) return false;
  return true;
}

/* Query Function */
uint __get_pipe_max_packets(pipe int p)
{
  __global int* pheader = __gen_ocl_get_pipe(p);
  return pheader[0];
}

uint __get_pipe_num_packets(pipe int p)
{
  __global int* pheader = __gen_ocl_get_pipe(p);
  return pheader[6];
}
