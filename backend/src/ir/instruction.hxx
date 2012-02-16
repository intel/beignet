/*
 * Copyright 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file instruction.hxx
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
DECL_INSN(MOV, UnaryInstruction)
DECL_INSN(COS, UnaryInstruction)
DECL_INSN(SIN, UnaryInstruction)
DECL_INSN(TAN, UnaryInstruction)
DECL_INSN(LOG, UnaryInstruction)
DECL_INSN(SQR, UnaryInstruction)
DECL_INSN(RSQ, UnaryInstruction)
DECL_INSN(POW, BinaryInstruction)
DECL_INSN(MUL, BinaryInstruction)
DECL_INSN(ADD, BinaryInstruction)
DECL_INSN(SUB, BinaryInstruction)
DECL_INSN(DIV, BinaryInstruction)
DECL_INSN(REM, BinaryInstruction)
DECL_INSN(SHL, BinaryInstruction)
DECL_INSN(SHR, BinaryInstruction)
DECL_INSN(ASR, BinaryInstruction)
DECL_INSN(BSF, BinaryInstruction)
DECL_INSN(BSB, BinaryInstruction)
DECL_INSN(OR, BinaryInstruction)
DECL_INSN(XOR, BinaryInstruction)
DECL_INSN(AND, BinaryInstruction)
DECL_INSN(MAD, TernaryInstruction)
DECL_INSN(CMP, CompareInstruction)
DECL_INSN(CVT, ConvertInstruction)
DECL_INSN(BRA, BranchInstruction)
DECL_INSN(TEX, TextureInstruction)
DECL_INSN(LOADI, LoadImmInstruction)
DECL_INSN(LOAD, LoadInstruction)
DECL_INSN(STORE, StoreInstruction)
DECL_INSN(FENCE, FenceInstruction)
DECL_INSN(LABEL, LabelInstruction)

