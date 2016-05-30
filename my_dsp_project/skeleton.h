#ifndef SKELETON_H_
#define SKELETON_H

/* function prototipes */
extern void process_ping_SWI(void);
extern void process_pong_SWI(void);
extern void EDMA_interrupt_service(void);
extern void config_EDMA(void);
void config_interrupts(void);
// Datenverarbeitung
void process_buffer(short * buffersrc, short * bufferdes);
//extern void SWI_LEDToggle(void);
extern void tsk_led_toggle(void);

/* Global Handler */

/* McBSP1 for AIC23 DATA */
MCBSP_Handle hMcbsp=0;

EDMA_Handle hEdmaRcv;  			/* Empfang handle auf EDMA REVT1-Channel */
EDMA_Handle hEdmaRcvRelPing; 	/* Empfang handle auf einen reload-Parametersatz */
EDMA_Handle hEdmaRcvRelPong;
								/* braucht man noch mehr? ja -> pong */
EDMA_Handle hEdmaXmt;			/* handle auf EDMA XEVT1-Channel */
EDMA_Handle hEdmaXmtRelPing;
EDMA_Handle hEdmaXmtRelPong;

	
/* Transfer-Complete-Codes for EDMA-Jobs */
int tccRcvPing;
int tccRcvPong;
int tccXmtPing;
int tccXmtPong;
#endif /*SKELETON_H*/
