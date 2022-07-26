#include <stdio.h>
#include <stdlib.h>
#include "corev_uvmt.h"


//TODO:
// Need to add the follwing:
// add struct, and values for start and stop
// write the required debugger software in assembly
// try and execute, most of the tests are short once the framework is implemented


// Global debug values
volatile int glb_debug_status = 0; // Written by debug code only, read by main code
volatile int glb_ebreak_status = 0; // Written by ebreak code only, read by main code
volatile int glb_expect_irq_entry = 0;
volatile int glb_expect_ebreak_handler  = 0;
volatile int glb_expect_debug_entry     = 0;


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



int main(void){
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
  //glb_expect_debug_entry = 1;
  DEBUG_REQ_CONTROL_REG = debug_req_control.bits; // this will initiate debug mode 
  while(glb_debug_status != glb_hart_status){
      printf("Waiting for Debugger\n");
  }
  printf("This is glb_expect_debug_entry: %08X\n", glb_expect_debug_entry);

}