#!/bin/bash
genpciid=(0152 0162 0156 0166 015a 016a 0f31 0402 0412 0422 040a 041a 042a 0406 0416 0426 0c02 0c12 0c22 0c0a 0c1a 0c2a 0c06 0c16 0c26 0a02 0a12 0a22 0a0a 0a1a 0a2a 0a06 0a16 0a26 0d02 0d12 0d22 0d0a 0d1a 0d2a 0d06 0d16 0d26)
pciid=($(lspci -nn | grep "\[8086:.*\]" -o | awk -F : '{print $2}' | awk -F ] '{print $1}'))
n=${#pciid[*]}
i=0
m=${#genpciid[*]}
j=0
while [ $i -lt $n ]
do
    id1=${pciid[$i]}
    let j=0

    while [ $j -lt $m ]
    do
	id2=${genpciid[$j]}

	if [ ${id1} == ${id2} ]
	then
	    echo ${id1}
	    exit 0
	fi
	let j=j+1
    done

    let i=i+1
done
