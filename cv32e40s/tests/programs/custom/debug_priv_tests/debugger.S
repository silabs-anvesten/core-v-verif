/*
**
** Copyright 2022 OpenHW Group
**
** Licensed under the Solderpad Hardware Licence, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
** https://solderpad.org/licenses/
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
*******************************************************************************
**
** Assembly file which holds functions for switching to U- and D-mode. Also contains checks to help the main test 'debug_privilege_tests.c' 
**
*******************************************************************************
*/

#include "corev_uvmt.h"

.section .debugger, "ax" 
//.section .text
.global _debugger_start 
.global glb_debug_status 
.global glb_expect_debug_entry 
.global glb_debug_status 
.global glb_expect_ebreaku_exc
.global glb_setmprv_test
.set test_ret_val, CV_VP_STATUS_FLAGS_BASE

//.section .debugger, "ax" 
_debugger_start:
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

	la t0, glb_setmprv_test
	lw t0, 0(t0)
	bnez t0, MPRV_test

	j end_debugger

end_debugger:
	// set the  glb_debug_status bit
	la t2, glb_debug_status
	lw t1, 0(t2)
	addi t1, t1, 1
	sw t1, 0(t2)

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


MPRV_test: // Debugger MPRV test
	// Set U-mode and then MPRV=1 in mstatus
	li	t0, 0x1800 
	csrrc   x0, mstatus, t0 // set U-mode 
	li t0, 1
	slli t0, t0, 17
	csrrs x0, mstatus, t0 // set MPRV=1

	j end_debugger




/*
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
*/