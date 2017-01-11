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
 */

#include "cl_utils.h"
#include <string.h>
#include <assert.h>

LOCAL void
list_node_insert_before(struct list_node *node, struct list_node *the_new)
{
  list_node *before_node = node->p;
  the_new->p = before_node;
  the_new->n = node;
  node->p = the_new;
  before_node->n = the_new;
}

LOCAL void
list_node_insert_after(struct list_node *node, struct list_node *the_new)
{
  list_node *after_node = node->n;
  the_new->n = after_node;
  the_new->p = node;
  node->n = the_new;
  after_node->p = the_new;
}

LOCAL void
list_move(struct list_head *the_old, struct list_head *the_new)
{
  assert(list_empty(the_new));
  if (list_empty(the_old)) {
    return;
  }

  memcpy(&the_new->head_node, &the_old->head_node, sizeof(list_node));
  the_new->head_node.n->p = &the_new->head_node;
  the_new->head_node.p->n = &the_new->head_node;
  list_init(the_old);
}

LOCAL void
list_merge(struct list_head *head, struct list_head *to_merge)
{
  if (list_empty(to_merge))
    return;

  list_node *merge_last_node = to_merge->head_node.p;
  list_node *merge_first_node = to_merge->head_node.n;

  merge_last_node->n = &head->head_node;
  merge_first_node->p = head->head_node.p;
  head->head_node.p->n = merge_first_node;
  head->head_node.p = merge_last_node;
  list_init(to_merge);
}

LOCAL cl_int
cl_get_info_helper(const void *src, size_t src_size, void *dst, size_t dst_size, size_t *ret_size)
{
  if (dst && dst_size < src_size)
    return CL_INVALID_VALUE;

  if (dst && dst_size) {
    memcpy(dst, src, src_size);
  }

  if (ret_size)
    *ret_size = src_size;
  return CL_SUCCESS;
}
