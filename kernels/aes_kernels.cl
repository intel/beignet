/**
 *
 */

unsigned char
galoisMultiplication(unsigned char a, unsigned char b)
{
    unsigned char p = 0; 
    for(unsigned int i=0; i < 8; ++i)
    {
        if((b&1) == 1)
        {
            p^=a;
        }
        unsigned char hiBitSet = (a & 0x80);
        a <<= 1;
        if(hiBitSet == 0x80)
        {
            a ^= 0x1b;
        }
        b >>= 1;
    }
    return p;
}

inline uchar4
sbox(__global uchar * SBox, uchar4 block)
{
    return (uchar4)(SBox[block.x], SBox[block.y], SBox[block.z], SBox[block.w]);
}

uchar4
mixColumns(__local uchar4 * block, __private uchar4 * galiosCoeff, unsigned int j)
{
    unsigned int bw = 4;

    uchar x, y, z, w;

    x = galoisMultiplication(block[0].x, galiosCoeff[(bw-j)%bw].x);
    y = galoisMultiplication(block[0].y, galiosCoeff[(bw-j)%bw].x);
    z = galoisMultiplication(block[0].z, galiosCoeff[(bw-j)%bw].x);
    w = galoisMultiplication(block[0].w, galiosCoeff[(bw-j)%bw].x);
   
    for(unsigned int k=1; k< 4; ++k)
    {
        x ^= galoisMultiplication(block[k].x, galiosCoeff[(k+bw-j)%bw].x);
        y ^= galoisMultiplication(block[k].y, galiosCoeff[(k+bw-j)%bw].x);
        z ^= galoisMultiplication(block[k].z, galiosCoeff[(k+bw-j)%bw].x);
        w ^= galoisMultiplication(block[k].w, galiosCoeff[(k+bw-j)%bw].x);
    }
    
    return (uchar4)(x, y, z, w);
}

uchar4
shiftRows(uchar4 row, unsigned int j)
{
    uchar4 r = row;
    for(uint i=0; i < j; ++i)  
    {
        r = r.yzwx;
    }
    return r;
}

__kernel 
void AESEncrypt(__global  uchar4  * output  ,
                __global  uchar4  * input   ,
                __global  uchar4  * roundKey,
                __global  uchar   * SBox    ,
                __local   uchar4  * block0  ,
                __local   uchar4  * block1  ,
                const     uint      width   , 
                const     uint     rounds   )
                                
{
    unsigned int blockIdx = get_group_id(0);
    unsigned int blockIdy = get_group_id(1);
 
    unsigned int localIdx = get_local_id(0);
    unsigned int localIdy = get_local_id(1);
    
    unsigned int globalIndex = (((blockIdy * width/4) + blockIdx) * 4 )+ (localIdy);
    unsigned int localIndex  = localIdy;

    __private uchar4 galiosCoeff[4];
     galiosCoeff[0] = (uchar4)(2, 0, 0, 0);
     galiosCoeff[1] = (uchar4)(3, 0, 0, 0);
     galiosCoeff[2] = (uchar4)(1, 0, 0, 0);
     galiosCoeff[3] = (uchar4)(1, 0, 0, 0);

    block0[localIndex]  = input[globalIndex];
    
    block0[localIndex] ^= roundKey[localIndex];

    for(unsigned int r=1; r < rounds; ++r)
    {
        block0[localIndex] = sbox(SBox, block0[localIndex]);

        block0[localIndex] = shiftRows(block0[localIndex], localIndex); 
       
        barrier(CLK_LOCAL_MEM_FENCE);
        block1[localIndex]  = mixColumns(block0, galiosCoeff, localIndex); 
        
        barrier(CLK_LOCAL_MEM_FENCE);
        block0[localIndex] = block1[localIndex]^roundKey[r*4 + localIndex];
    }  
    block0[localIndex] = sbox(SBox, block0[localIndex]);
  
    block0[localIndex] = shiftRows(block0[localIndex], localIndex); 

    output[globalIndex] =  block0[localIndex]^roundKey[(rounds)*4 + localIndex];
}

uchar4
shiftRowsInv(uchar4 row, unsigned int j)
{
    uchar4 r = row;
    for(uint i=0; i < j; ++i)  
    {
        r = r.wxyz;
    }
    return r;
}

__kernel 
void AESDecrypt(__global  uchar4  * output    ,
                __global  uchar4  * input     ,
                __global  uchar4  * roundKey  ,
                __global  uchar   * SBox      ,
                __local   uchar4  * block0    ,
                __local   uchar4  * block1    ,
                const     uint      width     , 
                const     uint      rounds    )
                                
{
    unsigned int blockIdx = get_group_id(0);
    unsigned int blockIdy = get_group_id(1);
 
    unsigned int localIdx = get_local_id(0);
    unsigned int localIdy = get_local_id(1);
    
    unsigned int globalIndex = (((blockIdy * width/4) + blockIdx) * 4 )+ (localIdy);
    unsigned int localIndex  = localIdy;

    __private uchar4 galiosCoeff[4];
     galiosCoeff[0] = (uchar4)(14, 0, 0, 0);
     galiosCoeff[1] = (uchar4)(11, 0, 0, 0);
     galiosCoeff[2] = (uchar4)(13, 0, 0, 0);
     galiosCoeff[3] = (uchar4)(9, 0, 0, 0);

    block0[localIndex]  = input[globalIndex];
    
    block0[localIndex] ^= roundKey[4*rounds + localIndex];

    for(unsigned int r=rounds -1 ; r > 0; --r)
    {
        block0[localIndex] = shiftRowsInv(block0[localIndex], localIndex); 
    
        block0[localIndex] = sbox(SBox, block0[localIndex]);
        
        barrier(CLK_LOCAL_MEM_FENCE);
        block1[localIndex] = block0[localIndex]^roundKey[r*4 + localIndex];

        barrier(CLK_LOCAL_MEM_FENCE);
        block0[localIndex]  = mixColumns(block1, galiosCoeff, localIndex); 
    }  

    block0[localIndex] = shiftRowsInv(block0[localIndex], localIndex); 

    block0[localIndex] = sbox(SBox, block0[localIndex]);

    output[globalIndex] =  block0[localIndex]^roundKey[localIndex];
}
