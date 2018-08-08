; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; Check that 64-bit division is bypassed correctly.
; RUN: llc < %s -mattr=+idivq-to-divl -mtriple=x86_64-unknown-linux-gnu | FileCheck %s

; Additional tests for 64-bit divide bypass

define i64 @Test_get_quotient(i64 %a, i64 %b) nounwind {
; CHECK-LABEL: Test_get_quotient:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    orq %rsi, %rax
; CHECK-NEXT:    shrq $32, %rax
; CHECK-NEXT:    je .LBB0_1
; CHECK-NEXT:  # %bb.2:
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    cqto
; CHECK-NEXT:    idivq %rsi
; CHECK-NEXT:    retq
; CHECK-NEXT:  .LBB0_1:
; CHECK-NEXT:    xorl %edx, %edx
; CHECK-NEXT:    movl %edi, %eax
; CHECK-NEXT:    divl %esi
; CHECK-NEXT:    # kill: def %eax killed %eax def %rax
; CHECK-NEXT:    retq
  %result = sdiv i64 %a, %b
  ret i64 %result
}

define i64 @Test_get_remainder(i64 %a, i64 %b) nounwind {
; CHECK-LABEL: Test_get_remainder:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    orq %rsi, %rax
; CHECK-NEXT:    shrq $32, %rax
; CHECK-NEXT:    je .LBB1_1
; CHECK-NEXT:  # %bb.2:
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    cqto
; CHECK-NEXT:    idivq %rsi
; CHECK-NEXT:    movq %rdx, %rax
; CHECK-NEXT:    retq
; CHECK-NEXT:  .LBB1_1:
; CHECK-NEXT:    xorl %edx, %edx
; CHECK-NEXT:    movl %edi, %eax
; CHECK-NEXT:    divl %esi
; CHECK-NEXT:    # kill: def %edx killed %edx def %rdx
; CHECK-NEXT:    movq %rdx, %rax
; CHECK-NEXT:    retq
  %result = srem i64 %a, %b
  ret i64 %result
}

define i64 @Test_get_quotient_and_remainder(i64 %a, i64 %b) nounwind {
; CHECK-LABEL: Test_get_quotient_and_remainder:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    orq %rsi, %rax
; CHECK-NEXT:    shrq $32, %rax
; CHECK-NEXT:    je .LBB2_1
; CHECK-NEXT:  # %bb.2:
; CHECK-NEXT:    movq %rdi, %rax
; CHECK-NEXT:    cqto
; CHECK-NEXT:    idivq %rsi
; CHECK-NEXT:    addq %rdx, %rax
; CHECK-NEXT:    retq
; CHECK-NEXT:  .LBB2_1:
; CHECK-NEXT:    xorl %edx, %edx
; CHECK-NEXT:    movl %edi, %eax
; CHECK-NEXT:    divl %esi
; CHECK-NEXT:    # kill: def %edx killed %edx def %rdx
; CHECK-NEXT:    # kill: def %eax killed %eax def %rax
; CHECK-NEXT:    addq %rdx, %rax
; CHECK-NEXT:    retq
  %resultdiv = sdiv i64 %a, %b
  %resultrem = srem i64 %a, %b
  %result = add i64 %resultdiv, %resultrem
  ret i64 %result
}
