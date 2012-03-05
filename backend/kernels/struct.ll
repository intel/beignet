; ModuleID = 'struct.o'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { i32, [2 x i32] }

@g = constant [4 x i32] [i32 0, i32 1, i32 2, i32 3], align 16
@struct_cl.hop = internal global %struct.my_struct zeroinitializer, align 4
@struct_cl.array = internal global [256 x %struct.my_struct] zeroinitializer, align 16

define void @struct_cl(i64 %s.coerce0, i32 %s.coerce1, i32 %x, i32* %mem, i32 %y) nounwind uwtable {
entry:
  %s = alloca %struct.my_struct, align 8
  %x.addr = alloca i32, align 4
  %mem.addr = alloca i32*, align 8
  %y.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = bitcast %struct.my_struct* %s to { i64, i32 }*
  %1 = getelementptr { i64, i32 }* %0, i32 0, i32 0
  store i64 %s.coerce0, i64* %1
  %2 = getelementptr { i64, i32 }* %0, i32 0, i32 1
  store i32 %s.coerce1, i32* %2
  store i32 %x, i32* %x.addr, align 4
  store i32* %mem, i32** %mem.addr, align 8
  store i32 %y, i32* %y.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %3 = load i32* %i, align 4
  %cmp = icmp slt i32 %3, 256
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %4 = load i32* %i, align 4
  %5 = load i32* %i, align 4
  %idxprom = sext i32 %5 to i64
  %arrayidx = getelementptr inbounds [256 x %struct.my_struct]* @struct_cl.array, i32 0, i64 %idxprom
  %a = getelementptr inbounds %struct.my_struct* %arrayidx, i32 0, i32 0
  store i32 %4, i32* %a, align 4
  %6 = load i32* %i, align 4
  %7 = load i32* %i, align 4
  %idxprom1 = sext i32 %7 to i64
  %arrayidx2 = getelementptr inbounds [256 x %struct.my_struct]* @struct_cl.array, i32 0, i64 %idxprom1
  %b = getelementptr inbounds %struct.my_struct* %arrayidx2, i32 0, i32 1
  %arrayidx3 = getelementptr inbounds [2 x i32]* %b, i32 0, i64 0
  store i32 %6, i32* %arrayidx3, align 4
  %8 = load i32* %i, align 4
  %add = add nsw i32 %8, 1
  %9 = load i32* %i, align 4
  %idxprom4 = sext i32 %9 to i64
  %arrayidx5 = getelementptr inbounds [256 x %struct.my_struct]* @struct_cl.array, i32 0, i64 %idxprom4
  %b6 = getelementptr inbounds %struct.my_struct* %arrayidx5, i32 0, i32 1
  %arrayidx7 = getelementptr inbounds [2 x i32]* %b6, i32 0, i64 0
  store i32 %add, i32* %arrayidx7, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %10 = load i32* %i, align 4
  %inc = add nsw i32 %10, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %11 = load i32* %y.addr, align 4
  %cmp8 = icmp eq i32 %11, 0
  br i1 %cmp8, label %if.then, label %if.else

if.then:                                          ; preds = %for.end
  %12 = load i32* %y.addr, align 4
  %idxprom9 = sext i32 %12 to i64
  %arrayidx10 = getelementptr inbounds [256 x %struct.my_struct]* @struct_cl.array, i32 0, i64 %idxprom9
  %13 = bitcast %struct.my_struct* %arrayidx10 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* bitcast (%struct.my_struct* @struct_cl.hop to i8*), i8* %13, i64 12, i32 4, i1 false)
  br label %if.end

if.else:                                          ; preds = %for.end
  %14 = load i32* %y.addr, align 4
  %add11 = add nsw i32 %14, 1
  %idxprom12 = sext i32 %add11 to i64
  %arrayidx13 = getelementptr inbounds [256 x %struct.my_struct]* @struct_cl.array, i32 0, i64 %idxprom12
  %15 = bitcast %struct.my_struct* %arrayidx13 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* bitcast (%struct.my_struct* @struct_cl.hop to i8*), i8* %15, i64 12, i32 4, i1 false)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* bitcast ([256 x %struct.my_struct]* @struct_cl.array to i8*), i8* bitcast (%struct.my_struct* @struct_cl.hop to i8*), i64 12, i32 4, i1 false)
  %a14 = getelementptr inbounds %struct.my_struct* %s, i32 0, i32 0
  %16 = load i32* %a14, align 4
  %17 = load i32* %x.addr, align 4
  %idxprom15 = sext i32 %17 to i64
  %arrayidx16 = getelementptr inbounds [256 x %struct.my_struct]* @struct_cl.array, i32 0, i64 %idxprom15
  %a17 = getelementptr inbounds %struct.my_struct* %arrayidx16, i32 0, i32 0
  %18 = load i32* %a17, align 4
  %add18 = add nsw i32 %16, %18
  %19 = load i32* %x.addr, align 4
  %add19 = add nsw i32 %19, 1
  %idxprom20 = sext i32 %add19 to i64
  %arrayidx21 = getelementptr inbounds [256 x %struct.my_struct]* @struct_cl.array, i32 0, i64 %idxprom20
  %b22 = getelementptr inbounds %struct.my_struct* %arrayidx21, i32 0, i32 1
  %arrayidx23 = getelementptr inbounds [2 x i32]* %b22, i32 0, i64 0
  %20 = load i32* %arrayidx23, align 4
  %add24 = add nsw i32 %add18, %20
  %21 = load i32* %x.addr, align 4
  %idxprom25 = sext i32 %21 to i64
  %arrayidx26 = getelementptr inbounds [4 x i32]* @g, i32 0, i64 %idxprom25
  %22 = load i32* %arrayidx26, align 4
  %add27 = add nsw i32 %add24, %22
  %23 = load i32* getelementptr inbounds ([4 x i32]* @g, i32 0, i64 3), align 4
  %add28 = add nsw i32 %add27, %23
  %24 = load i32** %mem.addr, align 8
  %arrayidx29 = getelementptr inbounds i32* %24, i64 0
  store i32 %add28, i32* %arrayidx29
  ret void
}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture, i64, i32, i1) nounwind

!opencl.kernels = !{!0}

!0 = metadata !{void (i64, i32, i32, i32*, i32)* @struct_cl}
