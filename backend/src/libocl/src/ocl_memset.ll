;The memset's source code.
; INLINE_OVERLOADABLE void __gen_memset_align(uchar* dst, uchar val, size_t size) {
;   size_t index = 0;
;   uint v = (val << 24) | (val << 16) | (val << 8) | val;
;   while((index + 4) >= size) {
;     *((uint *)(dst + index)) = v;
;     index += 4;
;   }
;   while(index < size) {
;     dst[index] = val;
;     index++;
;  }
; }

define void @__gen_memset_p_align(i8* %dst, i8 zeroext %val, i32 %size) nounwind alwaysinline {
entry:
  %conv = zext i8 %val to i32
  %shl = shl nuw i32 %conv, 24
  %shl2 = shl nuw nsw i32 %conv, 16
  %or = or i32 %shl, %shl2
  %shl4 = shl nuw nsw i32 %conv, 8
  %or5 = or i32 %or, %shl4
  %or7 = or i32 %or5, %conv
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %index.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %add = add i32 %index.0, 4
  %cmp = icmp ugt i32 %add, %size
  br i1 %cmp, label %while.cond10, label %while.body

while.body:                                       ; preds = %while.cond
  %add.ptr = getelementptr inbounds i8* %dst, i32 %index.0
  %0 = bitcast i8* %add.ptr to i32*
  store i32 %or7, i32* %0, align 4
  br label %while.cond

while.cond10:                                     ; preds = %while.cond, %while.body13
  %index.1 = phi i32 [ %index.0, %while.cond ], [ %inc, %while.body13 ]
  %cmp11 = icmp ult i32 %index.1, %size
  br i1 %cmp11, label %while.body13, label %while.end14

while.body13:                                     ; preds = %while.cond10
  %arrayidx = getelementptr inbounds i8* %dst, i32 %index.1
  store i8 %val, i8* %arrayidx, align 1
  %inc = add i32 %index.1, 1
  br label %while.cond10

while.end14:                                      ; preds = %while.cond10
  ret void
}

define void @__gen_memset_g_align(i8 addrspace(1)* %dst, i8 zeroext %val, i32 %size) nounwind alwaysinline {
entry:
  %conv = zext i8 %val to i32
  %shl = shl nuw i32 %conv, 24
  %shl2 = shl nuw nsw i32 %conv, 16
  %or = or i32 %shl, %shl2
  %shl4 = shl nuw nsw i32 %conv, 8
  %or5 = or i32 %or, %shl4
  %or7 = or i32 %or5, %conv
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %index.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %add = add i32 %index.0, 4
  %cmp = icmp ugt i32 %add, %size
  br i1 %cmp, label %while.cond10, label %while.body

while.body:                                       ; preds = %while.cond
  %add.ptr = getelementptr inbounds i8 addrspace(1)* %dst, i32 %index.0
  %0 = bitcast i8 addrspace(1)* %add.ptr to i32 addrspace(1)*
  store i32 %or7, i32 addrspace(1)* %0, align 4
  br label %while.cond

while.cond10:                                     ; preds = %while.cond, %while.body13
  %index.1 = phi i32 [ %index.0, %while.cond ], [ %inc, %while.body13 ]
  %cmp11 = icmp ult i32 %index.1, %size
  br i1 %cmp11, label %while.body13, label %while.end14

while.body13:                                     ; preds = %while.cond10
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %dst, i32 %index.1
  store i8 %val, i8 addrspace(1)* %arrayidx, align 1
  %inc = add i32 %index.1, 1
  br label %while.cond10

while.end14:                                      ; preds = %while.cond10
  ret void
}

define void @__gen_memset_l_align(i8 addrspace(3)* %dst, i8 zeroext %val, i32 %size) nounwind alwaysinline {
entry:
  %conv = zext i8 %val to i32
  %shl = shl nuw i32 %conv, 24
  %shl2 = shl nuw nsw i32 %conv, 16
  %or = or i32 %shl, %shl2
  %shl4 = shl nuw nsw i32 %conv, 8
  %or5 = or i32 %or, %shl4
  %or7 = or i32 %or5, %conv
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %index.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %add = add i32 %index.0, 4
  %cmp = icmp ugt i32 %add, %size
  br i1 %cmp, label %while.cond10, label %while.body

while.body:                                       ; preds = %while.cond
  %add.ptr = getelementptr inbounds i8 addrspace(3)* %dst, i32 %index.0
  %0 = bitcast i8 addrspace(3)* %add.ptr to i32 addrspace(3)*
  store i32 %or7, i32 addrspace(3)* %0, align 4
  br label %while.cond

while.cond10:                                     ; preds = %while.cond, %while.body13
  %index.1 = phi i32 [ %index.0, %while.cond ], [ %inc, %while.body13 ]
  %cmp11 = icmp ult i32 %index.1, %size
  br i1 %cmp11, label %while.body13, label %while.end14

while.body13:                                     ; preds = %while.cond10
  %arrayidx = getelementptr inbounds i8 addrspace(3)* %dst, i32 %index.1
  store i8 %val, i8 addrspace(3)* %arrayidx, align 1
  %inc = add i32 %index.1, 1
  br label %while.cond10

while.end14:                                      ; preds = %while.cond10
  ret void
}

;The memset's source code.
; INLINE_OVERLOADABLE void __gen_memset(uchar* dst, uchar val, size_t size) {
;   size_t index = 0;
;   while(index < size) {
;     dst[index] = val;
;     index++;
;  }
; }

define void @__gen_memset_p(i8 addrspace(0)* %dst, i8 zeroext %val, i32 %size) nounwind alwaysinline {
entry:
  %cmp3 = icmp eq i32 %size, 0
  br i1 %cmp3, label %while.end, label %while.body

while.body:                                       ; preds = %entry, %while.body
  %index.04 = phi i32 [ %inc, %while.body ], [ 0, %entry ]
  %0 = ptrtoint i8 addrspace(0)* %dst to i32
  %1 = add i32 %0, %index.04
  %2 = inttoptr i32 %1 to i8 addrspace(0)*
  store i8 %val, i8 addrspace(0)* %2, align 1
  %inc = add i32 %index.04, 1
  %cmp = icmp ult i32 %inc, %size
  br i1 %cmp, label %while.body, label %while.end

while.end:                                        ; preds = %while.body, %entry
  ret void
}

define void @__gen_memset_g(i8 addrspace(1)* %dst, i8 zeroext %val, i32 %size) nounwind alwaysinline {
entry:
  %cmp3 = icmp eq i32 %size, 0
  br i1 %cmp3, label %while.end, label %while.body

while.body:                                       ; preds = %entry, %while.body
  %index.04 = phi i32 [ %inc, %while.body ], [ 0, %entry ]
  %0 = ptrtoint i8 addrspace(1)* %dst to i32
  %1 = add i32 %0, %index.04
  %2 = inttoptr i32 %1 to i8 addrspace(1)*
  store i8 %val, i8 addrspace(1)* %2, align 1
  %inc = add i32 %index.04, 1
  %cmp = icmp ult i32 %inc, %size
  br i1 %cmp, label %while.body, label %while.end

while.end:                                        ; preds = %while.body, %entry
  ret void
}

define void @__gen_memset_l(i8 addrspace(3)* %dst, i8 zeroext %val, i32 %size) nounwind alwaysinline {
entry:
  %cmp3 = icmp eq i32 %size, 0
  br i1 %cmp3, label %while.end, label %while.body

while.body:                                       ; preds = %entry, %while.body
  %index.04 = phi i32 [ %inc, %while.body ], [ 0, %entry ]
  %0 = ptrtoint i8 addrspace(3)* %dst to i32
  %1 = add i32 %0, %index.04
  %2 = inttoptr i32 %1 to i8 addrspace(3)*
  store i8 %val, i8 addrspace(3)* %2, align 1
  %inc = add i32 %index.04, 1
  %cmp = icmp ult i32 %inc, %size
  br i1 %cmp, label %while.body, label %while.end

while.end:                                        ; preds = %while.body, %entry
  ret void
}
