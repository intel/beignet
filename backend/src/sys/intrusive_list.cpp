/*
 * Copyright (c) 2007 Maciej Sinilo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "intrusive_list.hpp"

namespace gbe
{
  intrusive_list_base::intrusive_list_base() : m_root() {}

  intrusive_list_base::size_type intrusive_list_base::size() const {
    size_type numNodes(0);
    const intrusive_list_node* iter = &m_root;
    do {
      iter = iter->next;
      ++numNodes;
    } while (iter != &m_root);
    return numNodes - 1;
  }

  void append(intrusive_list_node *node, intrusive_list_node *prev) {
    GBE_ASSERT(!node->in_list());
    node->next = prev->next;
    node->next->prev = node;
    prev->next = node;
    node->prev = prev;
  }

  void prepend(intrusive_list_node *node, intrusive_list_node *next) {
    GBE_ASSERT(!node->in_list());
    node->prev = next->prev;
    node->prev->next = node;
    next->prev = node;
    node->next = next;
  }

  void link(intrusive_list_node* node, intrusive_list_node* nextNode) {
    prepend(node, nextNode);
  }

  void unlink(intrusive_list_node* node) {
    GBE_ASSERT(node->in_list());
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = node->prev = node;
  }
} /* namespace gbe */

