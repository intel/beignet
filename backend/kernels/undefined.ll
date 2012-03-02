; ModuleID = 'undefined.o'
target datalayout = "e-p:32:32-i64:64:64-f64:64:64-n1:8:16:32:64"
target triple = "ptx32--"

define ptx_kernel void @undefined(i32* %dst) nounwind noinline {
entry:
  %dst.addr = alloca i32*, align 4
  %x = alloca i32, align 4
  store i32* %dst, i32** %dst.addr, align 4
  %0 = load i32* %x, align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %1 = load i32** %dst.addr, align 4
  %arrayidx = getelementptr inbounds i32* %1, i32 0
  store i32 0, i32* %arrayidx
  br label %if.end

if.else:                                          ; preds = %entry
  %2 = load i32** %dst.addr, align 4
  %arrayidx1 = getelementptr inbounds i32* %2, i32 0
  store i32 1, i32* %arrayidx1
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (i32*)* @undefined}
