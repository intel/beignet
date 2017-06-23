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

#include "cl_gen.h"
#include <stdarg.h>

#define GEN_PRINTF_LOG_MAGIC 0xAABBCCDD

typedef struct _cl_gen_printf_log {
  uint32_t magic;         // 0xAABBCCDD as magic for ASSERT.
  uint32_t size;          // Size of this printf log, include header.
  uint32_t statement_num; // which printf within one kernel.
  char *content;
} _cl_gen_printf_log;
typedef _cl_gen_printf_log *cl_gen_printf_log;

/* Things about printf info. */
enum {
  GEN_PRINTF_LM_NONE,
  GEN_PRINTF_LM_HH,
  GEN_PRINTF_LM_H,
  GEN_PRINTF_LM_L,
  GEN_PRINTF_LM_HL,
};

enum {
  GEN_PRINTF_CONVERSION_INVALID,
  GEN_PRINTF_CONVERSION_D,
  GEN_PRINTF_CONVERSION_I,
  GEN_PRINTF_CONVERSION_O,
  GEN_PRINTF_CONVERSION_U,
  GEN_PRINTF_CONVERSION_X,
  GEN_PRINTF_CONVERSION_x,
  GEN_PRINTF_CONVERSION_F,
  GEN_PRINTF_CONVERSION_f,
  GEN_PRINTF_CONVERSION_E,
  GEN_PRINTF_CONVERSION_e,
  GEN_PRINTF_CONVERSION_G,
  GEN_PRINTF_CONVERSION_g,
  GEN_PRINTF_CONVERSION_A,
  GEN_PRINTF_CONVERSION_a,
  GEN_PRINTF_CONVERSION_C,
  GEN_PRINTF_CONVERSION_S,
  GEN_PRINTF_CONVERSION_P
};

typedef struct _gen_printf_state {
  struct _gen_printf_state *next;
  cl_int left_justified;
  cl_int sign_symbol; //0 for nothing, 1 for sign, 2 for space.
  cl_int alter_form;
  cl_int zero_padding;
  cl_int vector_n;
  cl_int min_width;
  cl_int precision;
  cl_int length_modifier;
  cl_int conversion_specifier;
  char *str;
} _gen_printf_state;
typedef _gen_printf_state *gen_printf_state;

static char *
generate_printf_fmt(gen_printf_state state)
{
  char num_str[16];
  char *str = CL_CALLOC(1, 256);
  int len = 0;

  str[len] = '%';
  len++;

  if (state->left_justified) {
    str[len] = '-';
    len++;
  }

  if (state->sign_symbol == 1) {
    str[len] = '+';
    len++;
  } else if (state->sign_symbol == 2) {
    str[len] = ' ';
    len++;
  }

  if (state->alter_form) {
    str[len] = '#';
    len++;
  }

  if (state->zero_padding) {
    str[len] = '0';
    len++;
  }

  if (state->min_width >= 0) {
    snprintf(num_str, 16, "%d", state->min_width);
    memcpy(&(str[len]), num_str, strlen(num_str));
    len += strlen(num_str);
  }

  if (state->precision >= 0) {
    str[len] = '.';
    len++;
    memcpy(&(str[len]), num_str, strlen(num_str));
    len += strlen(num_str);
  }

  switch (state->length_modifier) {
  case GEN_PRINTF_LM_HH:
    str[len] = 'h';
    len++;
    str[len] = 'h';
    len++;
    break;
  case GEN_PRINTF_LM_H:
    str[len] = 'h';
    len++;
    break;
  case GEN_PRINTF_LM_L:
    str[len] = 'l';
    len++;
    break;
  case GEN_PRINTF_LM_HL:
    break;
  default:
    assert(state->length_modifier == GEN_PRINTF_LM_NONE);
  }

  switch (state->conversion_specifier) {
  case GEN_PRINTF_CONVERSION_D:
  case GEN_PRINTF_CONVERSION_I:
    str[len] = 'd';
    break;

  case GEN_PRINTF_CONVERSION_O:
    str[len] = 'o';
    break;
  case GEN_PRINTF_CONVERSION_U:
    str[len] = 'u';
    break;
  case GEN_PRINTF_CONVERSION_X:
    str[len] = 'X';
    break;
  case GEN_PRINTF_CONVERSION_x:
    str[len] = 'x';
    break;
  case GEN_PRINTF_CONVERSION_C:
    str[len] = 'c';
    break;
  case GEN_PRINTF_CONVERSION_F:
    str[len] = 'F';
    break;
  case GEN_PRINTF_CONVERSION_f:
    str[len] = 'f';
    break;
  case GEN_PRINTF_CONVERSION_E:
    str[len] = 'E';
    break;
  case GEN_PRINTF_CONVERSION_e:
    str[len] = 'e';
    break;
  case GEN_PRINTF_CONVERSION_G:
    str[len] = 'G';
    break;
  case GEN_PRINTF_CONVERSION_g:
    str[len] = 'g';
    break;
  case GEN_PRINTF_CONVERSION_A:
    str[len] = 'A';
    break;
  case GEN_PRINTF_CONVERSION_a:
    str[len] = 'a';
    break;
  case GEN_PRINTF_CONVERSION_P:
    str[len] = 'p';
    break;
  default:
    assert(0);
    break;
  }

  return str;
}

static cl_int
parse_printf_state(char *begin, char *end, char **rend, gen_printf_state state)
{
  const char *fmt;
  state->left_justified = 0;
  state->sign_symbol = 0; //0 for nothing, 1 for sign, 2 for space.
  state->alter_form = 0;
  state->zero_padding = 0;
  state->vector_n = 0;
  state->min_width = -1;
  state->precision = -1;
  state->length_modifier = GEN_PRINTF_LM_NONE;
  state->conversion_specifier = GEN_PRINTF_CONVERSION_INVALID;

  fmt = begin;

  if (*fmt != '%')
    return -1;

#define FMT_PLUS_PLUS                                   \
  do {                                                  \
    if (fmt + 1 <= end)                                 \
      fmt++;                                            \
    else {                                              \
      printf("Error, line: %d, fmt > end\n", __LINE__); \
      return -1;                                        \
    }                                                   \
  } while (0)

  FMT_PLUS_PLUS;

  // parse the flags.
  while (*fmt == '-' || *fmt == '+' || *fmt == ' ' || *fmt == '#' || *fmt == '0')
    switch (*fmt) {
    case '-':
      /* The result of the conversion is left-justified within the field. */
      state->left_justified = 1;
      FMT_PLUS_PLUS;
      break;
    case '+':
      /* The result of a signed conversion always begins with a plus or minus sign. */
      state->sign_symbol = 1;
      FMT_PLUS_PLUS;
      break;
    case ' ':
      /* If the first character of a signed conversion is not a sign, or if a signed
         conversion results in no characters, a space is prefixed to the result.
         If the space and + flags both appear,the space flag is ignored. */
      if (state->sign_symbol == 0)
        state->sign_symbol = 2;
      FMT_PLUS_PLUS;
      break;
    case '#':
      /*The result is converted to an alternative form. */
      state->alter_form = 1;
      FMT_PLUS_PLUS;
      break;
    case '0':
      if (!state->left_justified)
        state->zero_padding = 1;
      FMT_PLUS_PLUS;
      break;
    default:
      break;
    }

  // The minimum field width
  while ((*fmt >= '0') && (*fmt <= '9')) {
    if (state->min_width < 0)
      state->min_width = 0;
    state->min_width = state->min_width * 10 + (*fmt - '0');
    FMT_PLUS_PLUS;
  }

  // The precision
  if (*fmt == '.') {
    FMT_PLUS_PLUS;
    state->precision = 0;
    while (*fmt >= '0' && *fmt <= '9') {
      state->precision = state->precision * 10 + (*fmt - '0');
      FMT_PLUS_PLUS;
    }
  }

  // handle the vector specifier.
  if (*fmt == 'v') {
    FMT_PLUS_PLUS;
    switch (*fmt) {
    case '2':
    case '3':
    case '4':
    case '8':
      state->vector_n = *fmt - '0';
      FMT_PLUS_PLUS;
      break;
    case '1':
      FMT_PLUS_PLUS;
      if (*fmt == '6') {
        state->vector_n = 16;
        FMT_PLUS_PLUS;
      } else
        return -1;
      break;
    default:
      //Wrong vector, error.
      return -1;
    }
  }

  // length modifiers
  if (*fmt == 'h') {
    FMT_PLUS_PLUS;
    if (*fmt == 'h') { //hh
      state->length_modifier = GEN_PRINTF_LM_HH;
      FMT_PLUS_PLUS;
    } else if (*fmt == 'l') { //hl
      state->length_modifier = GEN_PRINTF_LM_HL;
      FMT_PLUS_PLUS;
    } else { //h
      state->length_modifier = GEN_PRINTF_LM_H;
    }
  } else if (*fmt == 'l') {
    state->length_modifier = GEN_PRINTF_LM_L;
    FMT_PLUS_PLUS;
  }

#define CONVERSION_SPEC_AND_RET(XXX, xxx)                      \
  case XXX:                                                    \
    state->conversion_specifier = GEN_PRINTF_CONVERSION_##xxx; \
    FMT_PLUS_PLUS;                                             \
    *rend = (char *)fmt;                                       \
    return XXX;                                                \
    break;

  // conversion specifiers
  switch (*fmt) {
    CONVERSION_SPEC_AND_RET('d', D)
    CONVERSION_SPEC_AND_RET('i', I)
    CONVERSION_SPEC_AND_RET('o', O)
    CONVERSION_SPEC_AND_RET('u', U)
    CONVERSION_SPEC_AND_RET('x', x)
    CONVERSION_SPEC_AND_RET('X', X)
    CONVERSION_SPEC_AND_RET('f', f)
    CONVERSION_SPEC_AND_RET('F', F)
    CONVERSION_SPEC_AND_RET('e', e)
    CONVERSION_SPEC_AND_RET('E', E)
    CONVERSION_SPEC_AND_RET('g', g)
    CONVERSION_SPEC_AND_RET('G', G)
    CONVERSION_SPEC_AND_RET('a', a)
    CONVERSION_SPEC_AND_RET('A', A)
    CONVERSION_SPEC_AND_RET('c', C)
    CONVERSION_SPEC_AND_RET('s', S)
    CONVERSION_SPEC_AND_RET('p', P)

  // %% has been handled

  default:
    return -1;
  }
}

static void
free_printf_state(gen_printf_state state)
{
  gen_printf_state s;

  while (state) {
    s = state->next;

    if (state->str)
      CL_FREE(state->str);

    CL_FREE(state);
    state = s;
  }
}

static gen_printf_state
parser_printf_fmt(char *format)
{
  char *begin;
  char *end;
  char *p;
  char ret_char;
  char *rend;
  gen_printf_state curr, prev, first;

  p = format;
  begin = format;
  end = format + strlen(format);
  first = NULL;
  prev = NULL;

  /* Now parse it. */
  while (*begin) {
    p = begin;

  again:
    while (p < end && *p != '%') {
      p++;
    }
    if (p < end && p + 1 == end) { // String with % at end.
      printf("string end with %%\n");
      goto error;
    }
    if (p + 1 < end && *(p + 1) == '%') { // %%
      p += 2;
      goto again;
    }

    if (p != begin) {
      curr = CL_CALLOC(1, sizeof(_gen_printf_state));
      curr->conversion_specifier = GEN_PRINTF_CONVERSION_S;
      curr->str = CL_MALLOC(p - begin + 1);
      memcpy(curr->str, begin, p - begin);

      curr->str[p - begin] = 0;
      if (first == NULL) {
        first = curr;
      }
      if (prev) {
        prev->next = curr;
      }
      prev = curr;
    }

    if (p == end) // finish
      break;

    /* Now parse the % start conversion_specifier. */
    curr = CL_CALLOC(1, sizeof(_gen_printf_state));
    ret_char = parse_printf_state(p, end, &rend, curr);
    if (ret_char < 0) {
      goto error;
    }

    if (curr->vector_n > 0) {
      curr->str = generate_printf_fmt(curr); // Standard printf can not recognize %v4XXX
    } else {
      curr->str = CL_MALLOC(rend - p + 1);
      memcpy(curr->str, p, rend - p);
      curr->str[rend - p] = 0;
    }

    if (first == NULL) {
      first = curr;
    }
    if (prev) {
      prev->next = curr;
    }
    prev = curr;

    if (rend == end)
      break;

    begin = rend;
  }

#if 0
  {
    cl_int j = 0;
    gen_printf_state s = first;
    while (s) {
      fprintf(stderr, "---- %d ---- state : \n", j);
      fprintf(stderr, "             conversion_specifier : %d\n", s->conversion_specifier);
      fprintf(stderr, "             vector_n : %d\n", s->vector_n);
      fprintf(stderr, "             left_justified : %d\n", s->left_justified);
      fprintf(stderr, "             sign_symbol: %d\n", s->sign_symbol);
      fprintf(stderr, "             alter_form : %d\n", s->alter_form);
      fprintf(stderr, "             zero_padding : %d\n", s->zero_padding);
      fprintf(stderr, "             min_width : %d\n", s->min_width);
      fprintf(stderr, "             precision : %d\n", s->precision);
      fprintf(stderr, "             length_modifier : %d\n", s->length_modifier);
      fprintf(stderr, "             string :  %s      strlen is %ld\n", s->str, strlen(s->str));
      j++;
      s = s->next;
    }
  }
#endif

  return first;

error:
  printf("error format string.\n");
  free_printf_state(first);
  return NULL;
}

static void
output_one_printf(gen_printf_state all_state, cl_gen_printf_log log)
{
#define PRINT_SOMETHING(target_ty)                      \
  do {                                                  \
    printf(s->str, *(target_ty *)(data + data_offset)); \
    data_offset += sizeof(target_ty);                   \
  } while (0)

  gen_printf_state s = all_state;
  cl_int vec_num, vec_i;
  char *data = (char *)(log) + 3 * sizeof(uint32_t);
  size_t data_offset = 0;

  while (s) {
    if (s->conversion_specifier == GEN_PRINTF_CONVERSION_S) {
      printf("%s", s->str);
      s = s->next;
      continue;
    }

    vec_num = s->vector_n > 0 ? s->vector_n : 1;
    for (vec_i = 0; vec_i < vec_num; vec_i++) {
      if (vec_i)
        printf(",");

      switch (s->conversion_specifier) {
      case GEN_PRINTF_CONVERSION_D:
      case GEN_PRINTF_CONVERSION_I:
        if (s->length_modifier == GEN_PRINTF_LM_L)
          PRINT_SOMETHING(uint64_t);
        else
          PRINT_SOMETHING(int);
        break;

      case GEN_PRINTF_CONVERSION_O:
        if (s->length_modifier == GEN_PRINTF_LM_L)
          PRINT_SOMETHING(uint64_t);
        else
          PRINT_SOMETHING(int);
        break;
      case GEN_PRINTF_CONVERSION_U:
        if (s->length_modifier == GEN_PRINTF_LM_L)
          PRINT_SOMETHING(uint64_t);
        else
          PRINT_SOMETHING(int);
        break;
      case GEN_PRINTF_CONVERSION_X:
        if (s->length_modifier == GEN_PRINTF_LM_L)
          PRINT_SOMETHING(uint64_t);
        else
          PRINT_SOMETHING(int);
        break;
      case GEN_PRINTF_CONVERSION_x:
        if (s->length_modifier == GEN_PRINTF_LM_L)
          PRINT_SOMETHING(uint64_t);
        else
          PRINT_SOMETHING(int);
        break;

      case GEN_PRINTF_CONVERSION_C:
        PRINT_SOMETHING(char);
        break;

      case GEN_PRINTF_CONVERSION_F:
        PRINT_SOMETHING(float);
        break;
      case GEN_PRINTF_CONVERSION_f:
        PRINT_SOMETHING(float);
        break;
      case GEN_PRINTF_CONVERSION_E:
        PRINT_SOMETHING(float);
        break;
      case GEN_PRINTF_CONVERSION_e:
        PRINT_SOMETHING(float);
        break;
      case GEN_PRINTF_CONVERSION_G:
        PRINT_SOMETHING(float);
        break;
      case GEN_PRINTF_CONVERSION_g:
        PRINT_SOMETHING(float);
        break;
      case GEN_PRINTF_CONVERSION_A:
        PRINT_SOMETHING(float);
        break;
      case GEN_PRINTF_CONVERSION_a:
        PRINT_SOMETHING(float);
        break;
      case GEN_PRINTF_CONVERSION_P:
        PRINT_SOMETHING(int);
        break;

      default:
        assert(0);
        return;
      }
    }

    s = s->next;
  }
}

LOCAL void
cl_gen_output_printf(void *buf_addr, uint32_t buf_size, cl_uint *ids,
                     char **fmts, uint32_t printf_num)
{
  uint32_t parsed;
  uint32_t total_sz = ((uint32_t *)buf_addr)[0];
  char *p = (char *)buf_addr + sizeof(uint32_t);
  uint32_t i;
  gen_printf_state all_states;

  if (total_sz > buf_size)
    total_sz = buf_size;

  for (parsed = 4; parsed < total_sz;) {
    cl_gen_printf_log log = (cl_gen_printf_log)(p);
    if (log->magic != GEN_PRINTF_LOG_MAGIC) {
      CL_LOG_ERROR("Printf log output has wrong magic");
      return;
    }

    for (i = 0; i < printf_num; i++) {
      if (ids[i] == log->statement_num)
        break;
    }
    if (i == printf_num) {
      CL_LOG_ERROR("Printf log output, can not find the printf statement for %d",
                   log->statement_num);
      return;
    }

    all_states = parser_printf_fmt(fmts[i]);
    if (all_states == NULL) {
      CL_LOG_ERROR("Printf statement %d with wrong format %s",
                   log->statement_num, fmts[i]);
      continue;
    }

    output_one_printf(all_states, log);
    free_printf_state(all_states);

    parsed += log->size;
    p += log->size;
  }
}
