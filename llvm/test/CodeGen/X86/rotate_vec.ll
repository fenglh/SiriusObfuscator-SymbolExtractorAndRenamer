; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown -mcpu=bdver4 | FileCheck %s

define <4 x i32> @rot_v4i32_splat(<4 x i32> %x) {
; CHECK-LABEL: rot_v4i32_splat:
; CHECK:       # BB#0:
; CHECK-NEXT:    vprotd $31, %xmm0, %xmm0
; CHECK-NEXT:    retq
  %1 = lshr <4 x i32> %x, <i32 1, i32 1, i32 1, i32 1>
  %2 = shl <4 x i32> %x, <i32 31, i32 31, i32 31, i32 31>
  %3 = or <4 x i32> %1, %2
  ret <4 x i32> %3
}

define <4 x i32> @rot_v4i32_non_splat(<4 x i32> %x) {
; CHECK-LABEL: rot_v4i32_non_splat:
; CHECK:       # BB#0:
; CHECK-NEXT:    vprotd {{.*}}(%rip), %xmm0, %xmm0
; CHECK-NEXT:    retq
  %1 = lshr <4 x i32> %x, <i32 1, i32 2, i32 3, i32 4>
  %2 = shl <4 x i32> %x, <i32 31, i32 30, i32 29, i32 28>
  %3 = or <4 x i32> %1, %2
  ret <4 x i32> %3
}

define <4 x i32> @rot_v4i32_splat_2masks(<4 x i32> %x) {
; CHECK-LABEL: rot_v4i32_splat_2masks:
; CHECK:       # BB#0:
; CHECK-NEXT:    vprotd $31, %xmm0, %xmm0
; CHECK-NEXT:    vpand {{.*}}(%rip), %xmm0, %xmm0
; CHECK-NEXT:    retq
  %1 = lshr <4 x i32> %x, <i32 1, i32 1, i32 1, i32 1>
  %2 = and <4 x i32> %1, <i32 4294901760, i32 4294901760, i32 4294901760, i32 4294901760>

  %3 = shl <4 x i32> %x, <i32 31, i32 31, i32 31, i32 31>
  %4 = and <4 x i32> %3, <i32 0, i32 4294901760, i32 0, i32 4294901760>
  %5 = or <4 x i32> %2, %4
  ret <4 x i32> %5
}

define <4 x i32> @rot_v4i32_non_splat_2masks(<4 x i32> %x) {
; CHECK-LABEL: rot_v4i32_non_splat_2masks:
; CHECK:       # BB#0:
; CHECK-NEXT:    vprotd {{.*}}(%rip), %xmm0, %xmm0
; CHECK-NEXT:    vpand {{.*}}(%rip), %xmm0, %xmm0
; CHECK-NEXT:    retq
  %1 = lshr <4 x i32> %x, <i32 1, i32 2, i32 3, i32 4>
  %2 = and <4 x i32> %1, <i32 4294901760, i32 4294901760, i32 4294901760, i32 4294901760>

  %3 = shl <4 x i32> %x, <i32 31, i32 30, i32 29, i32 28>
  %4 = and <4 x i32> %3, <i32 0, i32 4294901760, i32 0, i32 4294901760>
  %5 = or <4 x i32> %2, %4
  ret <4 x i32> %5
}