#include <stdio.h>
#include <stdlib.h>
#include "corev_uvmt.h"


//TODO:
// Need to add the follwing:
// add struct, and values for start and stop
// write the required debugger software in assembly
// try and execute, most of the tests are short once the framework is implemented


// Global debug values
volatile uint8_t glb_debug_status           = 0; // Written by debug code only, read by main code
volatile uint8_t glb_expect_debug_entry     = 0;
volatile uint8_t glb_expect_ebreaku_exc     = 0; // Written by main code, 
volatile uint8_t glb_setmprv_test           = 0; // Written by main code, 
extern volatile uint32_t whatisdcsr         = 4; // written by the assembly code.
volatile uint32_t wmcause, wmstatus;
// standard value for the mstatus register
#define MSTATUS_STD_VAL 0x1800
// mstatus.MPRV bit
#define MPRV_BIT 17

extern volatile void setup_pmp(), set_u_mode();


static void assert_or_die(uint32_t actual, uint32_t expect, char *msg) {
  if (actual != expect) {
    printf(msg);
    printf("expected = 0x%lx (%ld), got = 0x%lx (%ld)\n", expect, (int32_t)expect, actual, (int32_t)actual);
    exit(EXIT_FAILURE);
  }
}

/* 
Retuns specific bit-field from [bit_indx_low : bit_indx_high] in register x
*/
unsigned int get_field(unsigned int x, int bit_indx_low, int bit_indx_high){
    int field = ( 1 << ( (bit_indx_high - bit_indx_low) + 1) )  - 1;
    x = (x & (field << bit_indx_low) ) >> bit_indx_low;
    return x;
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
    __asm__ volatile("csrrw x0, mstatus, %0" :: "r"(MSTATUS_STD_VAL)); // set machine mode 
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

void ebreakzero(void){ // TODO: change name 

  set_u_mode();
  asm volatile("ebreak");
  assert_or_die(wmcause, 0x3, "Error: Illegal 'ebreak' did not trigger breakpoint exception!");

}


/* 
Transition out of D-mode (dret) into U-mode, while mstatus.mprv=1, ensure that when execution continues outside D-mode that mstatus.mprv=0
*/
void MPRV_exit_test(void){
  asm volatile("ecall");
  printf("dcsr from before dret: %08X\n", whatisdcsr);
  printf("mstatus after dret: %08X\n", wmstatus);
  int mprvfield = get_field(wmstatus, MPRV_BIT, MPRV_BIT);
  assert_or_die(mprvfield, 0x0, "Error: MPRV did not change to 0 after Debug --> User mode change! "); // check that MPRV = 0 after debug exit.

}

int main(void){
  setup_pmp();


  // test 1: Check that U-mode can't change into debug mode while dcsr.priv == 0
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
  glb_setmprv_test = 1; // flag the MRPRV-test for the debug module.
  DEBUG_REQ_CONTROL_REG = debug_req_control.bits; // this will initiate debug mode 
  while(glb_debug_status != glb_hart_status){
      printf("Waiting for Debugger\n");
  }
  MPRV_exit_test(); // check the MPRV-bit after transition from Debug to User mode with the MPRV previously == 1.
}