#!/bin/bash
lspci -nn | grep "VGA.*Gen.*\[8086:" -i  | grep "\[8086:.*\]" -o | awk -F : '{print $2}' | awk -F ] '{print $1}'
