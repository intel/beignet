#!/usr/bin/python
from __future__ import print_function
import os,sys,re,string

FLT_MAX_POSI='0x1.fffffep127f'
FLT_MIN_NEGA='-0x1.fffffep127f'
FLT_MIN_POSI='ldexpf(1.0, -126)'
FLT_MAX_NEGA='ldexpf(-1.0, -126)'

paraTypeList={'float':'%e','int':'%d','double':'%lf','uint':'%d','string':'%s'}

Single_Precision_ULPs={'acos' : '4' ,
                      'acospi' : '5' ,
                      'asin' : '4' ,
                      'asinpi' : '5' ,
                      'atan' : '5' ,
                      'atan2' : '6' ,
                      'atanpi' : '5' ,
                      'atan2pi' : '6' ,
                      'acosh' : '4' ,
                      'asinh' : '4' ,
                      'atanh' : '5' ,
                      'cbrt' : '2' ,
                      'ceil' : '-1' ,
                      'copysign' : '0' ,
                      'cos' : '4' ,
                      'cosh' : '4' ,
                      'cospi' : '4' ,
                      'erfc' : '16' ,
                      'erf' : '16' ,
                      'exp' : '3' ,
                      'exp2' : '3' ,
                      'exp10' : '3' ,
                      'expm1' : '3' ,
                      'fabs' : '0' ,
                      'fdim' : '-1' ,
                      'floor' : '-1' ,
                      'fma' : '-1' ,
                      'fmax' : '0' ,
                      'fmin' : '0' ,
                      'fmod' : '0' ,
                      'fract' : '-1' ,
                      'frexp' : '0' ,
                      'hypot' : '4' ,
                      'ilogb' : '0' ,
                      'ldexp' : '-1' ,
                      'log' : '3' ,
                      'log2' : '3' ,
                      'log10' : '3' ,
                      'log1p' : '2' ,
                      'logb' : '0' ,
                      'maxmag' : '0' ,
                      'minmag' : '0' ,
                      'modf' : '0' ,
                      'nan' : '0' ,
                      'nextafter' : '0' ,
                      'pow' : '16' ,
                      'pown' : '16' ,
                      'powr' : '16' ,
                      'remainder' : '0' ,
                      'remquo' : '0' ,
                      'rint' : '-1' ,
                      'rootn' : '16' ,
                      'round' : '-1' ,
                      'rsqrt' : '2' ,
                      'sin' : '4' ,
                      'sincos' : '4' ,
                      'sinh' : '4' ,
                      'sinpi' : '4' ,
                      'sqrt' : '3' ,
                      'tan' : '5' ,
                      'tanh' : '5' ,
                      'tanpi' : '6' ,
                      'tgamma' : '16' ,
                      'trunc' : '-1' ,
                      'half_cos' : '8192' ,
                      'half_divide' : '8192' ,
                      'half_exp' : '8192' ,
                      'half_exp2' : '8192' ,
                      'half_exp10' : '8192' ,
                      'half_log2' : '8192' ,
                      'half_log10' : '8192' ,
                      'half_powr' : '8192' ,
                      'half_recip' : '8192' ,
                      'half_rsqrt' : '8192' ,
                      'half_sin' : '8192' ,
                      'half_sqrt' : '8192' ,
                      'half_tan' : '8192' }

def Min_ulp(function):
    if function in Single_Precision_ULPs.keys():
         ulpValues = Single_Precision_ULPs[function]
    else:
         ulpValues = 1
    return ulpValues


def ulpUnit(ulpSize):
  return re.findall(r"([a-zA-Z_]+)",ulpSize)[0]

def ulpNum(ulpSize):
  return re.findall(r"([0-9]+)",ulpSize)[0]

def udebug(ulpSize,returnType,function):
  #ulpUnit=re.findall(r"([a-zA-Z_]+)",ulpSize)[0]
  #ulpNum=re.findall(r"([0-9]+)",ulpSize)[0]
  text='''
    static const char* INFORNAN;
    static %s ULPSIZE, ULPSIZE_FACTOR;

    float ULPSIZE_NO_FAST_MATH = %s;
    ULPSIZE_FACTOR = select_ulpsize(ULPSIZE_FAST_MATH,ULPSIZE_NO_FAST_MATH);
    bool fast_math = ULPSIZE_FACTOR == ULPSIZE_FAST_MATH;

    if (std::isinf(cpu_data[index])){
      INFORNAN="INF";
    }
    else if (std::isnan(cpu_data[index])){
      INFORNAN="NAN";
    }
    else{
       ULPSIZE=ULPSIZE_FACTOR * cl_%s((cpu_data[index] == 0) ? 1 : cpu_data[index])
               * ((ULPSIZE_FACTOR == 1) ? %s : ( (%s == 0) ? 1 : %s));
    }

#if udebug 
    if (std::isinf(cpu_data[index])){
      if (std::isinf(gpu_data[index]))
        printf("%s expect:%s\\n", log, INFORNAN);
      else
        printf_c("%s expect:%s\\n", log, INFORNAN);
      }
    else if (std::isnan(cpu_data[index])){
      if (std::isnan(gpu_data[index]))
        printf("%s expect:%s\\n", log, INFORNAN);
      else
        printf_c("%s expect:%s\\n", log, INFORNAN);
      }
    else if ((ULPSIZE >= 0 && diff <= ULPSIZE) || (ULPSIZE < 0 && diff == 0)){
      printf("%s expect:%s\\n", log, ULPSIZE);
    }
    else
      printf_c("%s expect:%s\\n", log, ULPSIZE);
#else
    if (std::isinf(cpu_data[index])){
      sprintf(log, "%s expect:%s\\n", log, INFORNAN);
      OCL_ASSERTM(std::isinf(gpu_data[index]) || fast_math,log);
    }
    else if (std::isnan(cpu_data[index])){
      sprintf(log, "%s expect:%s\\n", log, INFORNAN);
      OCL_ASSERTM(std::isnan(gpu_data[index]) || fast_math,log);
    }
    else{
      sprintf(log, "%s expect:%s\\n", log, ULPSIZE);
      if (ULPSIZE < 0)
            OCL_ASSERTM(gpu_data[index] == cpu_data[index], log);
      else
            OCL_ASSERTM(fabs(gpu_data[index]-cpu_data[index]) <= ULPSIZE, log);
    }
#endif
  }
}\n'''%(returnType,Min_ulp(function),\
        ulpUnit(ulpSize),ulpNum(ulpSize),\
        ulpNum(ulpSize), ulpNum(ulpSize),\
        paraTypeList['string'],paraTypeList['string'],\
        paraTypeList['string'],paraTypeList['string'],\
        paraTypeList['string'],paraTypeList['string'],\
        paraTypeList['string'],paraTypeList['string'],\
        paraTypeList['string'],paraTypeList['%s'%(returnType)],\
        paraTypeList['string'],paraTypeList['%s'%(returnType)],\
        paraTypeList['string'],paraTypeList['string'],\
        paraTypeList['string'],paraTypeList['string'],\
        paraTypeList['string'],paraTypeList['%s'%(returnType)])

  return text

def gene2ValuesLoop(values1,values2,inputValues):
  values2=values2+inputValues*len(inputValues)

  for i in inputValues:
    for j in range(0,len(inputValues)):
      values1 += [i]

  return values1,values2

def gene3ValuesLoop(values1,values2,values3,inputValues):
  for i in inputValues:
    for j in range(0,len(inputValues)):
      for k in range(0,len(inputValues)):
        values1 += [i]

  for i in inputValues:
    for j in inputValues:
      for k in range(0,len(inputValues)):
        values2 += [j]

  values3=inputValues*(len(inputValues)**2)
  return values1,values2,values3

class func:
  """ This class will define all needed instance attribute in fundation a c programing file. """

  def __init__(self,name,cpuFuncName,inputType,outputType,values,ulp, cpu_func=''):
    self.funcName = name
    self.cpuFuncName = cpuFuncName
    self.fileName = 'builtin_'+name
    self.inputtype = inputType
    self.outputtype = outputType
    self.values = values
    self.ulp = ulp
    self.cpufunc=cpu_func
    self.cpplines = []
    
#####cpp file required information:
    self.Head='''/*
This file is generated by utest_generator.py.
Usually you need NOT modify this file manually.
But when any bug occured, you can change the value of udebug from 0 to 1,
which can print more values and information to assist debuging the issue.
*/

#include "utest_helper.hpp"
#include <stdio.h>
#include <cmath>
#include <algorithm>
#include <string.h>

#define udebug 0
#define FLT_MAX 0x1.fffffep127f
#define FLT_MIN 0x1.0p-126f
#define INT_ULP 0

#define printf_c(...) \\
{\\
  printf("\\033[1m\\033[40;31m");\\
  printf( __VA_ARGS__ );\\
  printf("\\033[0m");\\
}
'''
    #########Execute class itself
    self.geneProcess()

#####Computer vector && argument type:
  def argtype(self,paraN,index):
    return re.findall(r"[a-zA-Z_]+",self.inputtype[paraN][index])[0]

  def argvector(self,paraN,index):
    vector=re.findall(r"[0-9]+",self.inputtype[paraN][index])
    if vector:
      vector=int(vector[0])
    else:
      vector=1
    return vector

  def returnVector(self,index):
    returnVector=re.findall(r"[0-9]+",self.outputtype[index])
    if returnVector:
      returnVector=returnVector[0]
    else:
      returnVector=1
    return returnVector

  def retType(self,index):
    return re.findall("[a-zA-Z_]+",self.outputtype[index])[0]

  def inputNumFormat(self,paraN,index):
    return paraTypeList['%s'%(self.argtype(paraN,index))]

  def outputNumFormat(self,index):
    return paraTypeList['%s'%(self.retType(index))]

#####Cpu values analyse
  def GenInputValues(self,index):
    #namesuffix=self.inputtype[0][index]
    vlen = self.argvector(self.inputtype.__len__()-1,index)
    for i in range(0,self.values.__len__()):
      vals = []
      for j in range(0, vlen):
        if (len(vals) >= 128):	#avoid too many data
          vals = vals[0:128]
          break
        vals += self.values[i]
      self.cpplines += [ "%s input_data%d[] = {%s};" %(self.argtype(i,index),i+1,str(vals).strip('[]').replace('\'','')) ]
    self.cpplines += [ "const int count_input = sizeof(input_data1) / sizeof(input_data1[0]);" ]
    self.cpplines += [ "int vector = %s;\n"%(vlen) ]

#####Cpu Function
  def GenCpuCompilerMath(self,index):
    #namesuffix=self.inputtype[0][index]
    defline='static void cpu_compiler_math(%s *dst, '%(self.retType(index))
    cpufunargs='('
    funcline = ['{']
    vectorargs=[]

    if (self.returnVector(index) == 1 and self.argvector(0,index) != 1):
      for i in range(0,self.values.__len__()):
        defline += 'const %s *src%d'%(self.argtype(i,index),i+1)
        defline += ( i == self.values.__len__()-1 ) and ')' or ','
        vectorargs.append('(')
      for i in range(0,self.values.__len__()):
        for j in range(0,self.vector):
          vectorargs += "x%d%d"%(i+1,j+1)
          vectorargs += ( j == self.vector-1 ) and ');' or ','
          funcline += ["  const %s x%d%d = *(src%d+%d);"%(self.argtype(i,index),i+1,j+1,i+1,j)]

      return 0

    for i in range(0,self.values.__len__()):
      defline += 'const %s *src%d'%(self.argtype(i,index),i+1)
      defline += ( i == self.values.__len__()-1 ) and ')' or ','
      cpufunargs += "x%d"%(i+1)
      cpufunargs += ( i == self.values.__len__()-1 ) and ');' or ','
      funcline += ["  const %s x%d = *src%d;"%(self.argtype(i,index),i+1,i+1)]

    funcline += [ "  dst[0] = %s%s"%(self.cpuFuncName, cpufunargs) ]
    funcline += [ '}'] 

    funcline = [defline] + funcline

    self.cpplines += funcline
#    self.writeCPP( '\n'.join(funcline), 'a', namesuffix)

  def writeCPP(self,content,authority,namesuffix):
    file_object = open("generated/%s_%s.cpp"%(self.fileName,namesuffix),authority)
    file_object.writelines(content)
    file_object.close()

  def writeCL(self,content,authority,namesuffix):
    file_object = open(os.getcwd()+"/../kernels/%s_%s.cl"%(self.fileName,namesuffix),authority)
    file_object.writelines(content)
    file_object.close()

  def nameForCmake(self,content,namesuffix):
    print("generated/%s_%s.cpp"%(self.fileName,namesuffix),end=" ")

  def utestFunc(self,index):
    funcLines=[]
    namesuffix=self.inputtype[0][index]
    funcline=[]
    funchead='''
static void %s_%s(void)
{
  int index;
  %s gpu_data[count_input] = {0}, cpu_data[count_input] = {0}, diff=0.0;
  char log[1024] = {0};

  OCL_CREATE_KERNEL(\"%s_%s\");
  OCL_CREATE_BUFFER(buf[0], CL_MEM_READ_WRITE, count_input * sizeof(%s), NULL); 

  globals[0] = count_input / vector;
  locals[0] = 1;
 '''%(self.fileName,namesuffix,\
     self.retType(index),\
     self.fileName, namesuffix,\
     self.retType(index))

    funcline += [funchead]
    for i in range(1,self.values.__len__()+1): 
      funcline += ["  OCL_CREATE_BUFFER(buf[%d], CL_MEM_READ_WRITE, count_input * sizeof(%s), NULL);"%(i,self.argtype(i-1,index))]
      funcline += ["  clEnqueueWriteBuffer( queue, buf[%d], CL_TRUE, 0, count_input * sizeof(%s), input_data%d, 0, NULL, NULL);"%(i,self.argtype(i-1,index),i)]

    funcline += ["  OCL_CREATE_BUFFER(buf[%d], CL_MEM_READ_WRITE, sizeof(int), NULL);"%(self.inputtype.__len__()+1)]
    funcline += ["  clEnqueueWriteBuffer( queue, buf[%d], CL_TRUE, 0, sizeof(int), &vector, 0, NULL, NULL);"%(self.inputtype.__len__()+1)]

	#0=output 1=input1 2=input2 ... len+2=output
    for i in range(0,self.values.__len__()+2): 
      funcline += ["  OCL_SET_ARG(%d, sizeof(cl_mem), &buf[%d]);"%(i,i)]

    funcrun='''
  // Run the kernel:
  //int errRead = clEnqueueReadBuffer( queue, buf[0], CL_TRUE, 0, sizeof(%s) * count_input, gpu_data, 0, NULL, NULL);
  OCL_NDRANGE( 1 );
  OCL_MAP_BUFFER(0);
'''%(self.argtype(0,index))
    funcline += [ funcrun ]

    text = ''' memcpy(gpu_data, buf_data[0], sizeof(gpu_data)); '''
    funcline += [ text ]

    funcsprintfa='    sprintf(log, \"'
    funcsprintfb=''
    if (self.returnVector(index) == 1 and self.argvector(0,index) != 1):
      funccompare='''
  for (index = 0; index < count_input/vector; index++)
  {
    cpu_compiler_math( cpu_data + index, '''
    else:
      funccompare='''
  for (index = 0; index < count_input; index++)
  {
    cpu_compiler_math( cpu_data + index,'''

    for i in range(0,self.values.__len__()):
      funccompare += " input_data%d + index"%(i+1)
      funccompare += (self.values.__len__() - 1 == i) and ');' or ','

      funcsprintfa += "input_data%d:"%(i+1)
      funcsprintfa += "%s "%(self.inputNumFormat(i,index))
      funcsprintfb += " input_data%d[index],"%(i+1)

    funcline += [ funccompare ]

    funcsprintfa += " -> gpu:%s  cpu:%s diff:%s\","%(self.outputNumFormat(index),self.outputNumFormat(index),self.outputNumFormat(index))#,self.outputNumFormat(index))
    funcsprintfb += " gpu_data[index], cpu_data[index], diff);"#%(ulpUnit(self.ulp),ulpNum(self.ulp))

    #funcdiff = "    diff = fabs((gpu_data[index]-cpu_data[index])"
    #funcdiff += (self.retType(index) == "int") and ');' or '/(cpu_data[index]>1?cpu_data[index]:1));'
    valuejudge = "    if (std::fpclassify(gpu_data[index]) == FP_SUBNORMAL){ gpu_data[index] = 0; }\n"
    valuejudge += "    if (std::fpclassify(cpu_data[index]) == FP_SUBNORMAL){ cpu_data[index] = 0; }\n"
    funcdiff = "    diff = fabs((gpu_data[index]-cpu_data[index]));"
    funcline += [ valuejudge ]
    funcline += [ funcdiff ]
    funcline += [ funcsprintfa + funcsprintfb ]

    self.cpplines += funcline

    self.cpplines += [ udebug(self.ulp,self.retType(index),self.funcName) ]
    self.cpplines += [ "MAKE_UTEST_FROM_FUNCTION(%s_%s)"%(self.fileName,namesuffix) ]

  def genCL(self,index):
    namesuffix=self.inputtype[0][index]
    clLine = []
    clhead = '__kernel void %s_%s(__global %s *dst, '%(self.fileName,namesuffix,self.retType(index))
    clvalueDef=''
    clcomputer=''
    tmp=''

    for i in range(0,self.values.__len__()):
      clhead += ' __global %s *src%d,'%(self.argtype(i,index),i+1)
      clvalueDef +=   '  %s x%d = (%s) ('%(self.inputtype[i][index],i+1,self.inputtype[i][index])
      tmp = 'src%d[i * (*vector) + '%(i+1)
      for j in range(0,self.argvector(i,index)):
        clvalueDef += tmp + ((self.argvector(i-1,index) == j+1 ) and '%d]);\n'%(j) or '%d],'%(j))
      clcomputer += (self.values.__len__() == i+1) and 'x%d);'%(i+1) or 'x%d,'%(i+1)
      
    clhead += ' __global int *vector) {\n'
    clhead += '  int i = get_global_id(0);'
    clLine += [ clhead ]
    clLine += [ clvalueDef ]
    clLine += [ '  %s ret;'%(self.outputtype[index]) ]
    clLine += [ '  ret = %s('%(self.funcName) + clcomputer ] 

    if (int(self.returnVector(index)) == 1):
      clLine += [ '  dst[i] = ret;' ]
    else:
      for i in range(0,int(self.returnVector(index))):
        clLine += [ '  dst[i * (*vector) + %d] = ret[%d];'%(i,i) ]
    clLine += [ '};' ]

    self.writeCL('\n'.join(clLine),'w',namesuffix)
  
  def geneProcess(self):
    for i in range(0,self.inputtype[0].__len__()):
##########Write Cpp file          
      namesuffix=self.inputtype[0][i]
      self.cpplines = []
      #The head:
      self.cpplines += [self.Head]

      self.cpplines += ["namespace {\n"]

      #Parameters:
      self.GenInputValues(i)

      #cpu function generator:
      self.cpplines += [self.cpufunc]

      #Cpu function:
      self.GenCpuCompilerMath(i)

      #utest function
      self.utestFunc(i)

      self.cpplines += ["}\n"]

      #kernel cl
      self.genCL(i)

      #CMakelists.txt
      self.nameForCmake(self.fileName,namesuffix)

      self.writeCPP( '\n'.join(self.cpplines) ,'w',namesuffix)
#########End

#def main():
#
#if __name__ == "__main__":
#  main()
