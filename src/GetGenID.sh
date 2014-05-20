#!/bin/bash
lspci -nn | grep "Gen .* Graphics" -i  | grep "\[8086:.*\]" -o | awk -F : '{print $2}' | awk -F ] '{print $1}'
