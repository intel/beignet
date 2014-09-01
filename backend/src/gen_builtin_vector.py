#!/usr/bin/env python
#
# Copyright (C) 2012 Intel Corporation
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library. If not, see <http://www.gnu.org/licenses/>.
#
# Author: Zhigang Gong <zhigang.gong@linux.intel.com>
#/

# This file is to generate inline code to lower down those builtin
# vector functions to scalar functions.
import re
import sys
import os

if len(sys.argv) != 3:
    print "Invalid argument {0}".format(sys.argv)
    print "use {0} spec_file_name output_file_name".format(sys.argv[0])
    raise

all_vector = 1,2,3,4,8,16

# generate generic type sets
def gen_vector_type(type_set, vector_set = all_vector):
    ret = []
    for t in type_set:
        for i in vector_set:
            ret.append((t, i))
    return ret

def set_vector_memspace(vector_type_set, memspace):
    ret = []
    if memspace == '':
        return vector_type_set
    for t in vector_type_set:
        ret.append((t[0], t[1], memspace))
    return ret

# if we have 3 elements in the type tuple, we are a pointer with a memory space type
# at the third element.
def isPointer(t):
    return len(t) == 3

all_itype = "char","short","int","long"
all_utype = "uchar","ushort","uint","ulong"
all_int_type = all_itype + all_utype

all_float_type = "float","double"
all_type = all_int_type + all_float_type

# all vector/scalar types
for t in all_type:
    exec "{0}n = [\"{0}n\", gen_vector_type([\"{0}\"])]".format(t)
    exec "s{0} = [\"{0}\", gen_vector_type([\"{0}\"], [1])]".format(t)

# Predefined type sets according to the Open CL spec.
math_gentype = ["math_gentype", gen_vector_type(all_float_type)]
math_gentypef = ["math_gentypef", gen_vector_type(["float"])]
math_gentyped = ["math_gentyped", gen_vector_type(["double"])]

half_native_math_gentype = ["half_native_math_gentype", gen_vector_type(["float"])]

integer_gentype = ["integer_gentype", gen_vector_type(all_int_type)]
integer_ugentype = ["integer_ugentype", gen_vector_type(all_utype)]
integer_sgentype = ["integer_sgentype", gen_vector_type(all_int_type, [1])]

fast_integer_gentype = ["fast_integer_gentype", gen_vector_type(["uint", "int"])]

common_gentype = ["common_gentype", gen_vector_type(all_float_type)]
common_gentypef = ["common_gentypef", gen_vector_type(["float"])]
common_gentyped = ["common_gentyped", gen_vector_type(["double"])]

relational_gentype = ["relational_gentype", gen_vector_type(all_type)]
relational_igentype = ["relational_igentype", gen_vector_type(all_itype)]
relational_ugentype = ["relational_ugentype", gen_vector_type(all_utype)]

misc_gentypem = ["misc_gentypem", gen_vector_type(all_type, [2, 4, 8, 16])]
misc_gentypen = ["misc_gentypen", gen_vector_type(all_type, [2, 4, 8, 16])]
misc_ugentypem = ["misc_ugentypem", gen_vector_type(all_utype, [2, 4, 8, 16])]
misc_ugentypen = ["misc_ugentypen", gen_vector_type(all_utype, [2, 4, 8, 16])]

all_predefined_type = math_gentype, math_gentypef, math_gentyped,                \
                      half_native_math_gentype, integer_gentype,integer_sgentype,\
                      integer_ugentype, charn, ucharn, shortn, ushortn, intn,    \
                      uintn, longn, ulongn, floatn, doublen,                     \
                      fast_integer_gentype, common_gentype, common_gentypef,     \
                      common_gentyped, relational_gentype, relational_igentype,  \
                      relational_ugentype, schar, suchar, sshort, sint, suint,   \
                      slong, sulong, sfloat, sdouble, misc_gentypem,              \
                      misc_ugentypem, misc_gentypen, misc_ugentypen

# type dictionary contains all the predefined type sets.
type_dict = {}

for t in all_predefined_type:
    type_dict.update({t[0]:t[1]})

def _prefix(prefix, dtype):
    if dtype.count("gentype") != 0:
        return prefix + '_' + dtype
    return dtype

memspaces = ["__local ", "__private ", "__global "]

def stripMemSpace(t):
    if t[0:2] == '__':
        for memspace in memspaces :
            if t[0:len(memspace)] == memspace:
                return memspace, t[len(memspace):]
    return '', t

def check_type(types):
    for t in types:
        memspace, t = stripMemSpace(t)
        if not t in type_dict:
            print t
            raise "found invalid type."

def match_unsigned(dtype):
    if dtype[0] == 'float':
        return ["uint", dtype[1]]
    if dtype[0] == 'double':
        return ["ulong", dtype[1]]
    if dtype[0][0] == 'u':
        return dtype
    return ['u' + dtype[0], dtype[1]]

def match_signed(dtype):
    if dtype[0] == 'float':
        return ["int", dtype[1]]
    if dtype[0] == 'double':
        return ["long", dtype[1]]
    if dtype[0][0] != 'u':
        return dtype
    return [dtype[0][1:], dtype[1]]

def match_scalar(dtype):
    return [dtype[0], 1]

# The dstType is the expected type, srcType is
# the reference type. Sometimes, the dstType and
# srcType are different. We need to fix this issue
# and return correct dst type.
def fixup_type(dstType, srcType, n):
    if dstType == srcType:
       return dstType[n]

    if dstType != srcType:
        # scalar dst type
        if len(dstType) == 1:
            return dstType[0]
        # dst is not scalar bug src is scalar
        if len(srcType) == 1:
            return dstType[n]
        if dstType == integer_sgentype[1] and srcType == integer_gentype[1]:
            return match_scalar(srcType[n])

        if dstType == integer_gentype[1] and  \
           (srcType == integer_sgentype[1] or \
            srcType == integer_ugentype[1]):
            return dstType[n]

        if dstType == integer_ugentype[1] and srcType == integer_gentype[1]:
            return match_unsigned(srcType[n])

        if dstType == relational_igentype[1] and srcType == relational_gentype[1]:
            return match_signed(srcType[n])
        if dstType == relational_ugentype[1] and srcType == relational_gentype[1]:
            return match_unsigned(srcType[n])

        if dstType == relational_gentype[1] and    \
           (srcType == relational_igentype[1] or   \
            srcType == relational_ugentype[1]):
            return dstType[n]

        if (len(dstType) == len(srcType)):
            return dstType[n]

    print dstType, srcType
    raise "type mispatch"

class builtinProto():
    valueTypeStr = ""
    functionName = ""
    paramTypeStrs = []
    paramCount = 0
    outputStr = []
    prefix = ""

    def init(self, sectionHeader, sectionPrefix):
        self.valueTypeStr = ""
        self.functionName = ""
        self.paramTypeStrs = []
        self.paramCount = 0
        if sectionHeader != "":
            self.outputStr = [sectionHeader]
        else:
            self.outputStr = []
        if sectionPrefix != "":
            self.prefix = sectionPrefix
        self.indent = 0

    def append(self, line, nextInit = ""):
        self.outputStr.append(line);
        return nextInit;

    def indentSpace(self):
        ret = ""
        for i in range(self.indent):
            ret += ' '

        return ret

    def init_from_line(self, t):
        self.append('//{0}'.format(t))
        line = filter(None, re.split(',| |\(', t.rstrip(')\n')))
        self.paramCount = 0
        stripped = 0
        memSpace = ''
        for i, text in enumerate(line):
            idx = i - stripped
            if idx == 0:
                self.valueTypeStr = _prefix(self.prefix, line[i])
                continue

            if idx == 1:
                self.functionName = line[i];
                continue

            if idx % 2 == 0:
                if line[i][0] == '(':
                    tmpType = line[i][1:]
                else:
                    tmpType = line[i]
                if tmpType == '__local' or   \
                   tmpType == '__private' or \
                   tmpType == '__global':
                   memSpace = tmpType + ' '
                   stripped += 1
                   continue
                self.paramTypeStrs.append(memSpace + _prefix(self.prefix, tmpType))
                memSpace = ''
                self.paramCount += 1

    def gen_proto_str_1(self, vtypeSeq, ptypeSeqs, i):
        for n in range(0, self.paramCount):
            ptype = fixup_type(ptypeSeqs[n], vtypeSeq, i);
            vtype = fixup_type(vtypeSeq, ptypeSeqs[n], i);
            # XXX FIXME now skip all double vector, as we don't
            # defined those scalar version's prototype.
            if ptype[0].find('double') != -1 or \
               vtype[0].find('double') != -1:
                return

            if (n == 0):
                formatStr = 'INLINE_OVERLOADABLE {0}{1} {2} ('.format(vtype[0], vtype[1], self.functionName)
            else:
                formatStr += ', '

            if vtype[1] == 1:
                return

            if isPointer(ptype):
                formatStr += ptype[2]
                pointerStr = '*'
            else:
                pointerStr = ''

            if ptype[1] != 1:
                formatStr += '{0}{1} {2}param{3}'.format(ptype[0], ptype[1], pointerStr, n)
            else:
                formatStr += '{0} {1}param{2}'.format(ptype[0], pointerStr, n)

        formatStr += ')'
        formatStr = self.append(formatStr, '{{return ({0}{1})('.format(vtype[0], vtype[1]))
        self.indent = len(formatStr)
        for j in range(0, vtype[1]):
            if (j != 0):
                formatStr += ','
                if (j + 1) % 2 == 0:
                    formatStr += ' '
                if j % 2 == 0:
                    formatStr = self.append(formatStr, self.indentSpace())

            if self.prefix == 'relational' and self.functionName != 'bitselect' and self.functionName != 'select':
                formatStr += '-'
            formatStr += '{0}('.format(self.functionName)
            for n in range(0, self.paramCount):
                if n != 0:
                    formatStr += ', '

                ptype = fixup_type(ptypeSeqs[n], vtypeSeq, i)
                vtype = fixup_type(vtypeSeq, ptypeSeqs[n], i)
                if vtype[1] != ptype[1]:
                    if ptype[1] != 1:
                        raise "parameter is not a scalar but has different width with result value."
                    if isPointer(ptype):
                        formatStr += '&'
                    formatStr += 'param{0}'.format(n)
                    continue

                if (isPointer(ptype)):
                    formatStr += '({0} {1} *)param{2} + {3:2d}'.format(ptype[2], ptype[0], n, j)
                else:
                    if (self.functionName == 'select' and n == 2):
                        formatStr += '({0})(param{1}.s{2:X} & (({0})1 << (sizeof({0})*8 - 1)))'.format(ptype[0], n, j)
                    else:
                        formatStr += 'param{0}.s{1:X}'.format(n, j)

            formatStr += ')'

        formatStr += '); }\n'
        self.append(formatStr)

        return formatStr

    def output(self):
        for line in self.outputStr:
            print line

    def output(self, outFile):
        for line in self.outputStr:
            outFile.write('{0}\n'.format(line))

    def gen_proto_str(self):
        check_type([self.valueTypeStr] + self.paramTypeStrs)
        vtypeSeq = type_dict[self.valueTypeStr]
        ptypeSeqs = []
        count = len(vtypeSeq);
        for t in self.paramTypeStrs:
            memspace,t = stripMemSpace(t)
            ptypeSeqs.append(set_vector_memspace(type_dict[t], memspace))
            count = max(count, len(type_dict[t]))

        for i in range(count):
            formatStr = self.gen_proto_str_1(vtypeSeq, ptypeSeqs, i)

        self.append("")

def safeUnlink(filename):
    try:
        os.remove(filename)
    except OSError:
        pass

# save the prototypes into ocl_vector.h
specFile = open(sys.argv[1], 'r')
headerFileName = sys.argv[2]
tempHeaderFileName = sys.argv[2] + '.tmp'
safeUnlink(headerFileName)
tempHeader = open(tempHeaderFileName, 'w')

tempHeader.write("//This file is autogenerated by {0}.\n".format(sys.argv[0]))
tempHeader.write("//Don't modify it manually.\n")

functionProto = builtinProto()
for line in specFile:
    if line.isspace():
        continue
    if line[0] == '#':
        if line[1] == '#':
            sectionHeader = "//{0} builtin functions".format(line[2:].rstrip())
            sectionPrefix=(line[2:].split())[0]
        continue
    functionProto.init(sectionHeader, sectionPrefix)
    sectionHeader = ""
    setionPrefix = ""
    functionProto.init_from_line(line)
    functionProto.gen_proto_str()
    functionProto.output(tempHeader)

tempHeader.close()
os.rename(tempHeaderFileName, headerFileName)
