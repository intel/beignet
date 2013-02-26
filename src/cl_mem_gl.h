#ifndef __CL_MEM_GL_H__
#define __CL_MEM_GL_H__
#include "cl_mem.h"

cl_mem cl_mem_new_gl_buffer(cl_context ctx,
                            cl_mem_flags flags,
                            GLuint buf_obj,
                            cl_int *errcode_ret);

cl_mem cl_mem_new_gl_texture(cl_context ctx,
                             cl_mem_flags flags,
                             GLenum texture_target,
                             GLint miplevel,
                             GLuint texture,
                             cl_int *errcode_ret);

#endif
