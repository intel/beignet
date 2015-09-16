target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

declare i8 @llvm.ctlz.i8(i8, i1)
declare i16 @llvm.ctlz.i16(i16, i1)
declare i32 @llvm.ctlz.i32(i32, i1)
declare i64 @llvm.ctlz.i64(i64, i1)

define i8 @clz_s8(i8 %x) nounwind readnone alwaysinline {
  %call = call i8 @llvm.ctlz.i8(i8 %x, i1 0)
  ret i8 %call
}

define i8 @clz_u8(i8 %x) nounwind readnone alwaysinline {
  %call = call i8 @llvm.ctlz.i8(i8 %x, i1 0)
  ret i8 %call
}

define i16 @clz_s16(i16 %x) nounwind readnone alwaysinline {
  %call = call i16 @llvm.ctlz.i16(i16 %x, i1 0)
  ret i16 %call
}

define i16 @clz_u16(i16 %x) nounwind readnone alwaysinline {
  %call = call i16 @llvm.ctlz.i16(i16 %x, i1 0)
  ret i16 %call
}

define i32 @clz_s32(i32 %x) nounwind readnone alwaysinline {
  %call = call i32 @llvm.ctlz.i32(i32 %x, i1 0)
  ret i32 %call
}

define i32 @clz_u32(i32 %x) nounwind readnone alwaysinline {
  %call = call i32 @llvm.ctlz.i32(i32 %x, i1 0)
  ret i32 %call
}

define i64 @clz_s64(i64 %x) nounwind readnone alwaysinline {
  %1 = bitcast i64 %x to <2 x i32>
  %2 = extractelement <2 x i32> %1, i32 0
  %3 = extractelement <2 x i32> %1, i32 1
  %call1 = call i32 @llvm.ctlz.i32(i32 %2, i1 0)
  %call2 = call i32 @llvm.ctlz.i32(i32 %3, i1 0)
  %cmp = icmp ult i32 %call2, 32
  %4 = add i32 %call1, 32
  %5 = select i1 %cmp, i32 %call2, i32 %4
  %6 = insertelement <2 x i32> undef, i32 %5, i32 0
  %call = bitcast <2 x i32> %6 to i64
  ret i64 %call
}

define i64 @clz_u64(i64 %x) nounwind readnone alwaysinline {
  %1 = bitcast i64 %x to <2 x i32>
  %2 = extractelement <2 x i32> %1, i32 0
  %3 = extractelement <2 x i32> %1, i32 1
  %call1 = call i32 @llvm.ctlz.i32(i32 %2, i1 0)
  %call2 = call i32 @llvm.ctlz.i32(i32 %3, i1 0)
  %cmp = icmp ult i32 %call2, 32
  %4 = add i32 %call1, 32
  %5 = select i1 %cmp, i32 %call2, i32 %4
  %6 = insertelement <2 x i32> undef, i32 %5, i32 0
  %call = bitcast <2 x i32> %6 to i64
  ret i64 %call
}
