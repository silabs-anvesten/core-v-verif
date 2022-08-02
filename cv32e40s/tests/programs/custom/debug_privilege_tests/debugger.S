#include "corev_uvmt.h"

.section .debugger, "ax" 
.global _debugger_start 
.global glb_debug_status 
.global glb_expect_debug_entry 
.global glb_debug_status 

.set test_ret_val, CV_VP_STATUS_FLAGS_BASE

_debugger_start:
        // Debugger Stack
	addi sp,sp,-52
	sw ra, 0(sp)
	sw s0, 4(sp)
	sw s1, 8(sp)
	sw s2, 12(sp)
	sw s3, 16(sp)
	sw s4, 20(sp)
	sw s5, 24(sp)
	sw s6, 28(sp)
	sw s7, 32(sp)
	sw s8, 36(sp)
	sw s9, 40(sp)
	sw s10, 44(sp)
	sw s11, 48(sp)

        j end_debugger

end_debugger:
	lw ra, 0(sp)
	lw s0, 4(sp)
	lw s1, 8(sp)
	lw s2, 12(sp)
	lw s3, 16(sp)
	lw s4, 20(sp)
	lw s5, 24(sp)
	lw s6, 28(sp)
	lw s7, 32(sp)
	lw s8, 36(sp)
	lw s9, 40(sp)
	lw s10, 44(sp)
	lw s11, 48(sp)
	addi sp,sp,52        
        dret


ebreaku_test:
        csrr t0, dcsr
        add t1, t1, x0
        lui t0, 1
        or t0, t0, t1 // add the ebreaku-field to the DCSR
        csrw dcsr, t0
        csrr t0, dcsr
        la t2, glb_expect_debug_entry
        sw t0, 0(t2)

        la t2, glb_debug_status
	lw t1, 0(t2)
        addi t1, t1, 1
        sw t1, 0(t2)