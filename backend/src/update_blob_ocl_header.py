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
import sys
import os

if len(sys.argv) != 3:
    print "Invalid argument {0}".format(sys.argv)
    print "use {0} tmpl_file_name output_file_name".format(sys.argv[0])
    raise

def safeUnlink(filename):
    try:
        os.remove(filename)
    except OSError:
        pass

header_segments = [ "vector", "as", "convert", "common_defines"]
blobFileName = sys.argv[2]
blobTempName = sys.argv[2] + '.tmp'
safeUnlink(blobFileName)
tmplFile = open(sys.argv[1], 'r')
blob = open(blobTempName, 'w')
path = os.path.dirname(sys.argv[1])
if path == '':
    path = '.'

matched_header = ""
for tline in tmplFile:
    if matched_header == "":
        blob.write(tline)
        for header in header_segments:
            if tline.strip() == '// ##BEGIN_{0}##'.format(header.upper()) :
                hFile = open(path + '/ocl_' + header + '.h', 'r')
                lineNr = 0
                for hline in hFile:
                    if lineNr >= 2:  #ignore the 2 lines of comment at the top of file.
                        blob.write(hline)
                    lineNr += 1
                hFile.close()
                matched_header = header
    else:
        if tline.strip() == '// ##END_{0}##'.format(matched_header.upper()) :
            blob.write(tline)
            matched_header = "";

tmplFile.close()
blob.close()
os.rename(blobTempName, blobFileName)
