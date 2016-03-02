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
#ifndef __OCL_PIPE_H__
#define __OCL_PIPE_H__

#include "ocl_types.h"
#include "ocl_work_group.h"
#include "ocl_simd.h"

/* The pipe read function. */
int __read_pipe_2(pipe int p, __generic void* dst);
int __read_pipe_4(pipe int p, reserve_id_t id, uint index, void* dst);
reserve_id_t __reserve_read_pipe(pipe int p, uint num);
void __commit_read_pipe(pipe int p, reserve_id_t rid);
reserve_id_t __work_group_reserve_read_pipe(pipe int p, uint num);
void __work_group_commit_read_pipe(pipe int p, reserve_id_t rid);
reserve_id_t __sub_group_reserve_read_pipe(pipe int p, uint num);
void __sub_group_commit_read_pipe(pipe int p, reserve_id_t rid);

/* The pipe write function. */
int __write_pipe_2(pipe int p, __generic void* src);
int __write_pipe_4(pipe int p, reserve_id_t id, uint index, void* src);
reserve_id_t __reserve_write_pipe(pipe int p, uint num);
void __commit_write_pipe(pipe int p, reserve_id_t rid);
reserve_id_t __work_group_reserve_write_pipe(pipe int p, uint num);
void __work_group_commit_write_pipe(pipe int p, reserve_id_t rid);
reserve_id_t __sub_group_reserve_write_pipe(pipe int p, uint num);
void __sub_group_commit_write_pipe(pipe int p, reserve_id_t rid);

/* The reserve_id_t function. */
bool is_valid_reserve_id(reserve_id_t rid);

/* The pipe query function. */
uint __get_pipe_num_packets(pipe int p);
uint __get_pipe_max_packets(pipe int p);
#endif
