#!/usr/bin/python

import re
import sys
import os.path
from io import open

if len(sys.argv) != 3:
    raise TypeError("requires version_string and output_directory")
version_string = sys.argv[1]
output_directory = sys.argv[2]
source_directory = os.path.dirname(sys.argv[0])
source_file = open(os.path.join(source_directory,"src/cl_device_data.h"),"r",encoding = 'utf-8')
device_ids = []
supported = False # first few devices in the file aren't supported
for line in source_file:
    device_id = re.match(r"#define\s+PCI_CHIP_([A-Za-z0-9_]+)\s+0x([0-9A-Fa-f]+)",line)
    if device_id is None:
        continue
    if "IVYBRIDGE" in device_id.group(1):
        supported = True # start of supported devices
    if supported:
        device_ids.append(device_id.group(2).upper())
source_file.close()
modalias_list_string = "\n".join("<modalias>pci:v00008086d0000{}*</modalias>".format(device_id) for device_id in sorted(device_ids))
metadata_file_in = open(os.path.join(source_directory,"com.intel.beignet.metainfo.xml.in"),"r",encoding = 'utf-8')
metadata_string = metadata_file_in.read()
metadata_file_in.close()
metadata_string = metadata_string.replace("@modalias_list@",modalias_list_string).replace("@version@",version_string)
metadata_file_out = open(os.path.join(output_directory,"com.intel.beignet.metainfo.xml"),"w",encoding = 'utf-8')
metadata_file_out.write(metadata_string)
metadata_file_out.close()
