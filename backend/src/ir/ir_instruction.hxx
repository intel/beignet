// ======================================================================== //
// Copyright (C) 2012 Benjamin Segovia                                      //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

DECL_INSN(MOV, UnaryInstruction)
DECL_INSN(COS, UnaryInstruction)
DECL_INSN(SIN, UnaryInstruction)
DECL_INSN(TAN, UnaryInstruction)
DECL_INSN(LOG, UnaryInstruction)
DECL_INSN(SQR, UnaryInstruction)
DECL_INSN(RSQ, UnaryInstruction)
DECL_INSN(EXP2, UnaryInstruction)
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
DECL_INSN(CVT, ConvertInstruction)
DECL_INSN(BRA, BranchInstruction)
DECL_INSN(TEX, TextureInstruction)
DECL_INSN(LOADI, LoadImmInstruction)
DECL_INSN(LOAD, LoadInstruction)
DECL_INSN(STORE, StoreInstruction)
DECL_INSN(FENCE, FenceInstruction)
DECL_INSN(LABEL, LabelInstruction)

