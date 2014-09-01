;The memcpy's source code.
; INLINE_OVERLOADABLE void __gen_memcpy(uchar* dst, uchar* src, size_t size) {
;   size_t index = 0;
;   while((index + 4) >= size) {
;     *((uint *)(dst + index)) = *((uint *)(src + index));
;     index += 4;
;   }
;   while(index < size) {
;     dst[index] = src[index];
;     index++;
;   }
; }

define void @__gen_memcpy_gg(i8 addrspace(1)* %dst, i8 addrspace(1)* %src, i32 %size) nounwind alwaysinline {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %index.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %add = add i32 %index.0, 4
  %cmp = icmp ult i32 %add, %size
  br i1 %cmp, label %while.cond3, label %while.body

while.body:                                       ; preds = %while.cond
  %add.ptr = getelementptr inbounds i8 addrspace(1)* %src, i32 %index.0
  %0 = bitcast i8 addrspace(1)* %add.ptr to i32 addrspace(1)*
  %1 = load i32 addrspace(1)* %0, align 4
  %add.ptr1 = getelementptr inbounds i8 addrspace(1)* %dst, i32 %index.0
  %2 = bitcast i8 addrspace(1)* %add.ptr1 to i32 addrspace(1)*
  store i32 %1, i32 addrspace(1)* %2, align 4
  br label %while.cond

while.cond3:                                      ; preds = %while.cond, %while.body5
  %index.1 = phi i32 [ %index.0, %while.cond ], [ %inc, %while.body5 ]
  %cmp4 = icmp ult i32 %index.1, %size
  br i1 %cmp4, label %while.body5, label %while.end7

while.body5:                                      ; preds = %while.cond3
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %src, i32 %index.1
  %3 = load i8 addrspace(1)* %arrayidx, align 1
  %arrayidx6 = getelementptr inbounds i8 addrspace(1)* %dst, i32 %index.1
  store i8 %3, i8 addrspace(1)* %arrayidx6, align 1
  %inc = add i32 %index.1, 1
  br label %while.cond3

while.end7:                                       ; preds = %while.cond3
  ret void
}

define void @__gen_memcpy_gp(i8 addrspace(1)* %dst, i8 addrspace(0)* %src, i32 %size) nounwind alwaysinline {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %index.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %add = add i32 %index.0, 4
  %cmp = icmp ult i32 %add, %size
  br i1 %cmp, label %while.cond3, label %while.body

while.body:                                       ; preds = %while.cond
  %add.ptr = getelementptr inbounds i8 addrspace(0)* %src, i32 %index.0
  %0 = bitcast i8 addrspace(0)* %add.ptr to i32 addrspace(0)*
  %1 = load i32 addrspace(0)* %0, align 4
  %add.ptr1 = getelementptr inbounds i8 addrspace(1)* %dst, i32 %index.0
  %2 = bitcast i8 addrspace(1)* %add.ptr1 to i32 addrspace(1)*
  store i32 %1, i32 addrspace(1)* %2, align 4
  br label %while.cond

while.cond3:                                      ; preds = %while.cond, %while.body5
  %index.1 = phi i32 [ %index.0, %while.cond ], [ %inc, %while.body5 ]
  %cmp4 = icmp ult i32 %index.1, %size
  br i1 %cmp4, label %while.body5, label %while.end7

while.body5:                                      ; preds = %while.cond3
  %arrayidx = getelementptr inbounds i8 addrspace(0)* %src, i32 %index.1
  %3 = load i8 addrspace(0)* %arrayidx, align 1
  %arrayidx6 = getelementptr inbounds i8 addrspace(1)* %dst, i32 %index.1
  store i8 %3, i8 addrspace(1)* %arrayidx6, align 1
  %inc = add i32 %index.1, 1
  br label %while.cond3

while.end7:                                       ; preds = %while.cond3
  ret void
}

define void @__gen_memcpy_gl(i8 addrspace(1)* %dst, i8 addrspace(3)* %src, i32 %size) nounwind alwaysinline {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %index.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %add = add i32 %index.0, 4
  %cmp = icmp ult i32 %add, %size
  br i1 %cmp, label %while.cond3, label %while.body

while.body:                                       ; preds = %while.cond
  %add.ptr = getelementptr inbounds i8 addrspace(3)* %src, i32 %index.0
  %0 = bitcast i8 addrspace(3)* %add.ptr to i32 addrspace(3)*
  %1 = load i32 addrspace(3)* %0, align 4
  %add.ptr1 = getelementptr inbounds i8 addrspace(1)* %dst, i32 %index.0
  %2 = bitcast i8 addrspace(1)* %add.ptr1 to i32 addrspace(1)*
  store i32 %1, i32 addrspace(1)* %2, align 4
  br label %while.cond

while.cond3:                                      ; preds = %while.cond, %while.body5
  %index.1 = phi i32 [ %index.0, %while.cond ], [ %inc, %while.body5 ]
  %cmp4 = icmp ult i32 %index.1, %size
  br i1 %cmp4, label %while.body5, label %while.end7

while.body5:                                      ; preds = %while.cond3
  %arrayidx = getelementptr inbounds i8 addrspace(3)* %src, i32 %index.1
  %3 = load i8 addrspace(3)* %arrayidx, align 1
  %arrayidx6 = getelementptr inbounds i8 addrspace(1)* %dst, i32 %index.1
  store i8 %3, i8 addrspace(1)* %arrayidx6, align 1
  %inc = add i32 %index.1, 1
  br label %while.cond3

while.end7:                                       ; preds = %while.cond3
  ret void
}

define void @__gen_memcpy_pg(i8 addrspace(0)* %dst, i8 addrspace(1)* %src, i32 %size) nounwind alwaysinline {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %index.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %add = add i32 %index.0, 4
  %cmp = icmp ult i32 %add, %size
  br i1 %cmp, label %while.cond3, label %while.body

while.body:                                       ; preds = %while.cond
  %add.ptr = getelementptr inbounds i8 addrspace(1)* %src, i32 %index.0
  %0 = bitcast i8 addrspace(1)* %add.ptr to i32 addrspace(1)*
  %1 = load i32 addrspace(1)* %0, align 4
  %add.ptr1 = getelementptr inbounds i8 addrspace(0)* %dst, i32 %index.0
  %2 = bitcast i8 addrspace(0)* %add.ptr1 to i32 addrspace(0)*
  store i32 %1, i32 addrspace(0)* %2, align 4
  br label %while.cond

while.cond3:                                      ; preds = %while.cond, %while.body5
  %index.1 = phi i32 [ %index.0, %while.cond ], [ %inc, %while.body5 ]
  %cmp4 = icmp ult i32 %index.1, %size
  br i1 %cmp4, label %while.body5, label %while.end7

while.body5:                                      ; preds = %while.cond3
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %src, i32 %index.1
  %3 = load i8 addrspace(1)* %arrayidx, align 1
  %arrayidx6 = getelementptr inbounds i8 addrspace(0)* %dst, i32 %index.1
  store i8 %3, i8 addrspace(0)* %arrayidx6, align 1
  %inc = add i32 %index.1, 1
  br label %while.cond3

while.end7:                                       ; preds = %while.cond3
  ret void
}

define void @__gen_memcpy_pp(i8 addrspace(0)* %dst, i8 addrspace(0)* %src, i32 %size) nounwind alwaysinline {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %index.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %add = add i32 %index.0, 4
  %cmp = icmp ult i32 %add, %size
  br i1 %cmp, label %while.cond3, label %while.body

while.body:                                       ; preds = %while.cond
  %add.ptr = getelementptr inbounds i8 addrspace(0)* %src, i32 %index.0
  %0 = bitcast i8 addrspace(0)* %add.ptr to i32 addrspace(0)*
  %1 = load i32 addrspace(0)* %0, align 4
  %add.ptr1 = getelementptr inbounds i8 addrspace(0)* %dst, i32 %index.0
  %2 = bitcast i8 addrspace(0)* %add.ptr1 to i32 addrspace(0)*
  store i32 %1, i32 addrspace(0)* %2, align 4
  br label %while.cond

while.cond3:                                      ; preds = %while.cond, %while.body5
  %index.1 = phi i32 [ %index.0, %while.cond ], [ %inc, %while.body5 ]
  %cmp4 = icmp ult i32 %index.1, %size
  br i1 %cmp4, label %while.body5, label %while.end7

while.body5:                                      ; preds = %while.cond3
  %arrayidx = getelementptr inbounds i8 addrspace(0)* %src, i32 %index.1
  %3 = load i8 addrspace(0)* %arrayidx, align 1
  %arrayidx6 = getelementptr inbounds i8 addrspace(0)* %dst, i32 %index.1
  store i8 %3, i8 addrspace(0)* %arrayidx6, align 1
  %inc = add i32 %index.1, 1
  br label %while.cond3

while.end7:                                       ; preds = %while.cond3
  ret void
}

define void @__gen_memcpy_pl(i8 addrspace(0)* %dst, i8 addrspace(3)* %src, i32 %size) nounwind alwaysinline {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %index.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %add = add i32 %index.0, 4
  %cmp = icmp ult i32 %add, %size
  br i1 %cmp, label %while.cond3, label %while.body

while.body:                                       ; preds = %while.cond
  %add.ptr = getelementptr inbounds i8 addrspace(3)* %src, i32 %index.0
  %0 = bitcast i8 addrspace(3)* %add.ptr to i32 addrspace(3)*
  %1 = load i32 addrspace(3)* %0, align 4
  %add.ptr1 = getelementptr inbounds i8 addrspace(0)* %dst, i32 %index.0
  %2 = bitcast i8 addrspace(0)* %add.ptr1 to i32 addrspace(0)*
  store i32 %1, i32 addrspace(0)* %2, align 4
  br label %while.cond

while.cond3:                                      ; preds = %while.cond, %while.body5
  %index.1 = phi i32 [ %index.0, %while.cond ], [ %inc, %while.body5 ]
  %cmp4 = icmp ult i32 %index.1, %size
  br i1 %cmp4, label %while.body5, label %while.end7

while.body5:                                      ; preds = %while.cond3
  %arrayidx = getelementptr inbounds i8 addrspace(3)* %src, i32 %index.1
  %3 = load i8 addrspace(3)* %arrayidx, align 1
  %arrayidx6 = getelementptr inbounds i8 addrspace(0)* %dst, i32 %index.1
  store i8 %3, i8 addrspace(0)* %arrayidx6, align 1
  %inc = add i32 %index.1, 1
  br label %while.cond3

while.end7:                                       ; preds = %while.cond3
  ret void
}

define void @__gen_memcpy_lg(i8 addrspace(3)* %dst, i8 addrspace(1)* %src, i32 %size) nounwind alwaysinline {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %index.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %add = add i32 %index.0, 4
  %cmp = icmp ult i32 %add, %size
  br i1 %cmp, label %while.cond3, label %while.body

while.body:                                       ; preds = %while.cond
  %add.ptr = getelementptr inbounds i8 addrspace(1)* %src, i32 %index.0
  %0 = bitcast i8 addrspace(1)* %add.ptr to i32 addrspace(1)*
  %1 = load i32 addrspace(1)* %0, align 4
  %add.ptr1 = getelementptr inbounds i8 addrspace(3)* %dst, i32 %index.0
  %2 = bitcast i8 addrspace(3)* %add.ptr1 to i32 addrspace(3)*
  store i32 %1, i32 addrspace(3)* %2, align 4
  br label %while.cond

while.cond3:                                      ; preds = %while.cond, %while.body5
  %index.1 = phi i32 [ %index.0, %while.cond ], [ %inc, %while.body5 ]
  %cmp4 = icmp ult i32 %index.1, %size
  br i1 %cmp4, label %while.body5, label %while.end7

while.body5:                                      ; preds = %while.cond3
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %src, i32 %index.1
  %3 = load i8 addrspace(1)* %arrayidx, align 1
  %arrayidx6 = getelementptr inbounds i8 addrspace(3)* %dst, i32 %index.1
  store i8 %3, i8 addrspace(3)* %arrayidx6, align 1
  %inc = add i32 %index.1, 1
  br label %while.cond3

while.end7:                                       ; preds = %while.cond3
  ret void
}

define void @__gen_memcpy_lp(i8 addrspace(3)* %dst, i8 addrspace(0)* %src, i32 %size) nounwind alwaysinline {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %index.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %add = add i32 %index.0, 4
  %cmp = icmp ult i32 %add, %size
  br i1 %cmp, label %while.cond3, label %while.body

while.body:                                       ; preds = %while.cond
  %add.ptr = getelementptr inbounds i8 addrspace(0)* %src, i32 %index.0
  %0 = bitcast i8 addrspace(0)* %add.ptr to i32 addrspace(0)*
  %1 = load i32 addrspace(0)* %0, align 4
  %add.ptr1 = getelementptr inbounds i8 addrspace(3)* %dst, i32 %index.0
  %2 = bitcast i8 addrspace(3)* %add.ptr1 to i32 addrspace(3)*
  store i32 %1, i32 addrspace(3)* %2, align 4
  br label %while.cond

while.cond3:                                      ; preds = %while.cond, %while.body5
  %index.1 = phi i32 [ %index.0, %while.cond ], [ %inc, %while.body5 ]
  %cmp4 = icmp ult i32 %index.1, %size
  br i1 %cmp4, label %while.body5, label %while.end7

while.body5:                                      ; preds = %while.cond3
  %arrayidx = getelementptr inbounds i8 addrspace(0)* %src, i32 %index.1
  %3 = load i8 addrspace(0)* %arrayidx, align 1
  %arrayidx6 = getelementptr inbounds i8 addrspace(3)* %dst, i32 %index.1
  store i8 %3, i8 addrspace(3)* %arrayidx6, align 1
  %inc = add i32 %index.1, 1
  br label %while.cond3

while.end7:                                       ; preds = %while.cond3
  ret void
}

define void @__gen_memcpy_ll(i8 addrspace(3)* %dst, i8 addrspace(3)* %src, i32 %size) nounwind alwaysinline {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %index.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %add = add i32 %index.0, 4
  %cmp = icmp ult i32 %add, %size
  br i1 %cmp, label %while.cond3, label %while.body

while.body:                                       ; preds = %while.cond
  %add.ptr = getelementptr inbounds i8 addrspace(3)* %src, i32 %index.0
  %0 = bitcast i8 addrspace(3)* %add.ptr to i32 addrspace(3)*
  %1 = load i32 addrspace(3)* %0, align 4
  %add.ptr1 = getelementptr inbounds i8 addrspace(3)* %dst, i32 %index.0
  %2 = bitcast i8 addrspace(3)* %add.ptr1 to i32 addrspace(3)*
  store i32 %1, i32 addrspace(3)* %2, align 4
  br label %while.cond

while.cond3:                                      ; preds = %while.cond, %while.body5
  %index.1 = phi i32 [ %index.0, %while.cond ], [ %inc, %while.body5 ]
  %cmp4 = icmp ult i32 %index.1, %size
  br i1 %cmp4, label %while.body5, label %while.end7

while.body5:                                      ; preds = %while.cond3
  %arrayidx = getelementptr inbounds i8 addrspace(3)* %src, i32 %index.1
  %3 = load i8 addrspace(3)* %arrayidx, align 1
  %arrayidx6 = getelementptr inbounds i8 addrspace(3)* %dst, i32 %index.1
  store i8 %3, i8 addrspace(3)* %arrayidx6, align 1
  %inc = add i32 %index.1, 1
  br label %while.cond3

while.end7:                                       ; preds = %while.cond3
  ret void
}
