#include <stdio.h>
#include <stdlib.h>
#include "corev_uvmt.h"


//TODO:
// Need to add the follwing:
// add struct, and values for start and stop
// write the required debugger software in assembly
// try and execute, most of the tests are short once the framework is implemented


// Global debug values
volatile int glb_debug_status           = 0; // Written by debug code only, read by main code
volatile int glb_expect_debug_entry     = 0;
volatile int glb_expect_ebreaku_exc     = 0; // Written by main code, 
volatile int glb_setmprv_test           = 0; // Written by main code, 
volatile int wmcause, wmstatus;



extern volatile void setup_pmp(), set_u_mode();
static void assert_or_die(uint32_t actual, uint32_t expect, char *msg) {
  if (actual != expect) {
    printf(msg);
    printf("expected = 0x%lx (%ld), got = 0x%lx (%ld)\n", expect, (int32_t)expect, actual, (int32_t)actual);
    exit(EXIT_FAILURE);
  }
}

/* Checks the mepc for compressed instructions and increments appropriately */
void increment_mepc(void){
  volatile unsigned int insn, mepc;

    __asm__ volatile("csrrs %0, mepc, x0" : "=r"(mepc)); // read the mepc
    __asm__ volatile("lw %0, 0(%1)" : "=r"(insn) : "r"(mepc)); // read the contents of the mepc pc.
    if ((insn & 0x3) == 0x3) { // check for compressed instruction before increment.
      mepc += 4;
    } else {
      mepc += 2;
    }
    __asm__ volatile("csrrw x0, mepc, %0" :: "r"(mepc)); // write to the mepc 
}


// Rewritten interrupt handler
__attribute__ ((interrupt ("machine")))
void u_sw_irq_handler(void) {

    __asm__ volatile("csrrs %0, mcause, x0" : "=r"(wmcause));
    __asm__ volatile("csrrs %0, mstatus, x0" : "=r"(wmstatus));
    int tmstatus = 0x1800;
    __asm__ volatile("csrrw x0, mstatus, %0" :: "r"(tmstatus)); // set machine mode 
    increment_mepc();
}


typedef union {
  struct {
    unsigned int start_delay      : 15; // 14: 0
    unsigned int rand_start_delay : 1;  //    15
    unsigned int pulse_width      : 13; // 28:16
    unsigned int rand_pulse_width : 1;  //    29
    unsigned int pulse_mode       : 1;  //    30    0 = level, 1 = pulse
    unsigned int value            : 1;  //    31
  } fields;
  unsigned int bits;
}  debug_req_control_t;

#define DEBUG_REQ_CONTROL_REG *((volatile uint32_t *) (CV_VP_DEBUG_CONTROL_BASE))

void ebreakzero(void){

  set_u_mode();
  asm volatile("ebreak");
  assert_or_die(wmcause, 0x3, "Error: Illegal 'ebreak' did not trigger breakpoint exception!");

}

int main(void){
  setup_pmp();

  //ebreakzero();


  debug_req_control_t debug_req_control;
  debug_req_control = (debug_req_control_t) {
    .fields.value            = 1,
    .fields.pulse_mode       = 1, //PULSE Mode
    .fields.rand_pulse_width = 0,
    .fields.pulse_width      = 5,// FIXME: BUG: one clock pulse cause core to lock up
    .fields.rand_start_delay = 0,
    .fields.start_delay      = 200
  };
  volatile int glb_hart_status = 1;


  // test 2: Check MPRV == 0 when entering U-mode from D-mode.
  DEBUG_REQ_CONTROL_REG = debug_req_control.bits; // this will initiate debug mode 
  while(glb_debug_status != glb_hart_status){
      printf("Waiting for Debugger\n");
  }

  printf("successfully exited debug mode.");
/* 
  asm volatile("ecall");
  int mprvfield = (wmstatus & (0x1 << 17));
  printf("this is the mstatus: %08X\n", wmstatus);
  assert_or_die(mprvfield, 0x0, "Error: MPRV did not change to 0 after Debug --> User change! "); // check that MPRV = 0 after debug exit.
  */

  return EXIT_SUCCESS;
}