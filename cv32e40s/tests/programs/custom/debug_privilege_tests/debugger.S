#include "corev_uvmt.h"

.section .debugger, "ax"
.global _debugger_start
.global glb_debug_status
.global glb_expect_debug_entry
.global _debugger_fail
.global _debugger_end
.global glb_debug_status

.set test_ret_val, CV_VP_STATUS_FLAGS_BASE

_debugger_start:
        // Debugger Stack
        csrr t0, dcsr
        add t1, t1, x0
        lui t0, 1
        or t0, t0, t1 // add the ebreaku 
        csrw dcsr, t0
        csrr t0, dcsr
        la t2, glb_expect_debug_entry
        sw t0, 0(t2)

        la t2, glb_debug_status
	lw t1, 0(t2)
        addi t1, t1, 1
        sw t1, 0(t2)

        dret

