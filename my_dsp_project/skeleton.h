#ifndef SKELETON_H_
#define SKELETON_H

/* function prototipes */
extern void process_ping_SWI(void);
extern void process_pong_SWI(void);
extern void EDMA_interrupt_service(void);
extern void config_EDMA(void);
void config_interrupts(void);
//extern void SWI_LEDToggle(void);
extern void tsk_led_toggle(void);

/* Global Handler */

/* McBSP1 for AIC23 DATA */
MCBSP_Handle hMcbsp=0;

/* McBSP0 for AIC23 Configuration & ADPCM RX */
MCBSP_Handle hMcbsp0=0;

	
#endif /*SKELETON_H*/
