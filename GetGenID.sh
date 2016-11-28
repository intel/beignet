#!/bin/bash
#IVB
genpciid=(0152 0162 0156 0166 015a 016a)
#BYT
genpciid+=(0f31)
#HSW
genpciid+=(0402 0412 0422 040a 041a 042a 0406 0416 0426 040b 041b 042b 040e 041e 042e)
genpciid+=(0c02 0c12 0c22 0c0a 0c1a 0c2a 0c06 0c16 0c26 0c0b 0c1b 0c2b 0c0e 0c1e 0c2e)
genpciid+=(0a02 0a12 0a22 0a0a 0a1a 0a2a 0a06 0a16 0a26 0a0b 0a1b 0a2b 0a0e 0a1e 0a2e)
genpciid+=(0d02 0d12 0d22 0d0a 0d1a 0d2a 0d06 0d16 0d26 0d0b 0d1b 0d2b 0d0e 0d1e 0d2e)
#BRW
genpciid+=(1602 1606 160a 160d 160e 1612 1616 161a 161d 161e 1622 1626 162a 162d 162e)
#BSW
genpciid+=(22b0 22b1 22b2 22b3)
#Only enable OpenCL 2.0 after SKL.
#SKL
genpciid_20=(1906 1916 1926 190e 191e 1902 1912 1932 190b 191b 192b 193b 190a 191a 192a 193a)
#BXT
genpciid_20+=(5a84 5a85 1a84 1a85)
#KBL
genpciid_20+=(5906 5916 5926 5913 5921 5923 5927 5902 5912 5917)
genpciid_20+=(590b 591b 593b 5908 590e 591e 5915 590a 591a 591d)
pciid=($(lspci -nn | grep "\[8086:.*\]" -o | awk -F : '{print $2}' | awk -F ] '{print $1}'))
n=${#pciid[*]}
i=0
m=${#genpciid[*]}
t=${#genpciid_20[*]}
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

  let j=0
  while [ $j -lt $t ]
  do
    id2=${genpciid_20[$j]}

    if [ ${id1} == ${id2} ]
    then
      echo ${id1}
      exit 1
    fi
    let j=j+1
  done

  let i=i+1
done
exit -1
