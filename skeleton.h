#ifndef SKELETON_H_
#define SKELETON_H

extern void process_ping_SWI(void);
extern void process_pong_SWI(void);
extern void EDMA_interrupt_service(void);
extern void config_EDMA(void);
extern void config_interrutps(void);
extern void SWI_LEDToggle(void);
extern void tsk_led_toggle(void);

	
#endif /*SKELETON_H*/
