/***********************************************************
 *  skeleton.c
 *  Example for ping-pong processing
 *  Caution: It is intended, that this file ist not runnable.
 *  The file contains mistakes and omissions, which shall be
 *  corrected and completed by the students.
 *
 *   F. Quint, HsKA
 *
 *
 ************************************************************/

#include <my_dsp_projectcfg.h>
#include <std.h>
#include <csl.h>
#include <csl_mcbsp.h>
#include <csl_irq.h>
#include <csl_edma.h>
#include <swi.h>
#include <sem.h>
#include <dsk6713_led.h>
#include <dsk6713.h>
#include "config_AIC23.h"
#include "config_BSPLink.h"
#include "Sounds.h"
#include "skeleton.h"

Uint32 ringbuff_in_read_i = RINGBUFFER_LEN / 2;
Uint32 ringbuff_in_write_i = 0;

Uint32 soundBuffer_i = 0;

Uint16 time_cnt = 0;

int configComplete = 0;
Uint8 t_reg = 0;

main() {
	Uint16 i = 0;

	DSK6713_init();
	CSL_init();

	for (ringbuff_in_write_i = 0; ringbuff_in_write_i < RINGBUFFER_LEN;
			ringbuff_in_write_i++) {
		*(Ringbuffer_in + ringbuff_in_write_i) = (short) 0;
	}
	ringbuff_in_write_i = 0;

	for (i = 0; i < AIC_BUFFER_LEN; i++) {
		Debug_Buff_ping[i] = (short) i;
		Debug_Buff_pong[AIC_BUFFER_LEN - i - 1] = (short) i + 1;
	}

	ringbuff_in_write_i = 0;

	/* Configure McBSP0 and AIC23 */
	Config_DSK6713_AIC23();

	/* Configure McBSP1*/
	hMcbsp_AIC23 = MCBSP_open(MCBSP_DEV1, MCBSP_OPEN_RESET);
	MCBSP_config(hMcbsp_AIC23, &datainterface_config);

	/* configure EDMA */
	config_AIC23_EDMA();

	DSK6713_LED_on(0);
	//DSK6713_LED_on(1);
	//DSK6713_LED_on(2);
	//DSK6713_LED_on(3);

	/* finally the interrupts */
	config_interrupts();

//	MCBSP_start(hMcbsp_AIC23, MCBSP_XMIT_START | MCBSP_RCV_START, 0xffffffff); // Start Audio IN & OUT transmision
//	MCBSP_write(hMcbsp_AIC23, 0x0); /* one shot */

	configComplete = 1;

} /* finished*/

void config_AIC23_EDMA(void) {
	/* Konfiguration der EDMA zum Lesen*/
	hEdmaRcv = EDMA_open(EDMA_CHA_REVT1, EDMA_OPEN_RESET); // EDMA Kanal für das Event REVT1
	hEdmaRcvRelPing = EDMA_allocTable(-1); // einen Reload-Parametersatz für Ping
	hEdmaRcvRelPong = EDMA_allocTable(-1); // einen Reload-Parametersatz für Pong

	configEDMARcv.src = MCBSP_getRcvAddr(hMcbsp_AIC23); //  Quell-Adresse zum Lesen

	tccRcvPing = EDMA_intAlloc(-1); // nächsten freien Transfer-Complete-Code Ping
	tccRcvPong = EDMA_intAlloc(-1); // nächsten freien Transfer-Complete-Code Pong
	configEDMARcv.opt |= EDMA_FMK(OPT, TCC, tccRcvPing); // dann der Grundkonfiguration des EDMA Empfangskanals zuweisen

	/* ersten Transfer und Reload-Ping mit ConfigPing konfigurieren */
	EDMA_config(hEdmaRcv, &configEDMARcv);
	EDMA_config(hEdmaRcvRelPing, &configEDMARcv);

	/* braucht man auch noch andere EDMA-Konfigurationen fuer das Lesen? ja -> pong */
	configEDMARcv.opt &= 0xFFF0FFFF;
	configEDMARcv.opt |= EDMA_FMK(OPT, TCC, tccRcvPong);
	configEDMARcv.dst = (Uint32) AIC_Buffer_in_pong;
	EDMA_config(hEdmaRcvRelPong, &configEDMARcv);

	/* Transfers verlinken ping -> pong -> ping */
	EDMA_link(hEdmaRcv, hEdmaRcvRelPong); /* noch mehr verlinken? */
	EDMA_link(hEdmaRcvRelPong, hEdmaRcvRelPing);
	EDMA_link(hEdmaRcvRelPing, hEdmaRcvRelPong);

	/* muss man mittels EDMA auch schreiben? */
	/* Konfiguration der EDMA zum Schreiben */
	hEdmaXmt = EDMA_open(EDMA_CHA_XEVT1, EDMA_OPEN_RESET); // EDMA Kanal für das Event REVT1
	hEdmaXmtRelPing = EDMA_allocTable(-1); // einen Reload-Parametersatz für Ping
	hEdmaXmtRelPong = EDMA_allocTable(-1); // einen Reload-Parametersatz für Pong

	configEDMAXmt.dst = MCBSP_getXmtAddr(hMcbsp_AIC23);	//  Ziel-Adresse zum Schreiben

	tccXmtPing = EDMA_intAlloc(-1); // nächsten freien Transfer-Complete-Code Ping
	tccXmtPong = EDMA_intAlloc(-1); // nächsten freien Transfer-Complete-Code Pong
	configEDMAXmt.opt |= EDMA_FMK(OPT, TCC, tccXmtPing); // dann der Grundkonfiguration des EDMA Sendekanal zuweisen

	/* ersten Transfer und Reload-Ping mit ConfigPing konfigurieren */
	EDMA_config(hEdmaXmt, &configEDMAXmt);
	EDMA_config(hEdmaXmtRelPing, &configEDMAXmt);

	/* braucht man auch noch andere EDMA-Konfigurationen fuer das Schreiben? ja -> pong */
	configEDMAXmt.opt &= 0xFFF0FFFF;
	configEDMAXmt.opt |= EDMA_FMK(OPT, TCC, tccXmtPong);
	configEDMAXmt.src = (Uint32) AIC_Buffer_out_pong;
	EDMA_config(hEdmaXmtRelPong, &configEDMAXmt);

	/* Transfers verlinken ping -> pong -> ping */
	EDMA_link(hEdmaXmt, hEdmaXmtRelPong); /* noch mehr verlinken? */
	EDMA_link(hEdmaXmtRelPong, hEdmaXmtRelPing);
	EDMA_link(hEdmaXmtRelPing, hEdmaXmtRelPong);

	/* EDMA TCC-Interrupts freigeben */
	EDMA_intClear(tccRcvPing);
	EDMA_intEnable(tccRcvPing);
	EDMA_intClear(tccRcvPong);
	EDMA_intEnable(tccRcvPong);
	/* sind das alle? nein -> pong und alle für Sendeseite */
	EDMA_intClear(tccXmtPing);
	EDMA_intEnable(tccXmtPing);
	EDMA_intClear(tccXmtPong);
	EDMA_intEnable(tccXmtPong);

	/* EDMA starten, wen alles? */
	EDMA_enableChannel(hEdmaRcv);
	EDMA_enableChannel(hEdmaXmt);
}

void config_interrupts(void) {
	IRQ_map(IRQ_EVT_EDMAINT, 8);		// CHECK same settings in BIOS!!!
	IRQ_clear(IRQ_EVT_EDMAINT);
	IRQ_enable(IRQ_EVT_EDMAINT);

	SWI_enable();

	IRQ_globalEnable();
}

void EDMA_ISR(void) {
	DSK6713_LED_on(2);
	static volatile int rcvPingDone = 0;	//static
	static volatile int rcvPongDone = 0;
	static volatile int xmtPingDone = 0;
	static volatile int xmtPongDone = 0;

	// BSP Data Link Interface
	static volatile int xmtBSPLinkPingDone = 0;
	static volatile int xmtBSPLinkPongDone = 0;

	static volatile int rcvBSPLinkPingDone = 0;
	static volatile int rcvBSPLinkPongDone = 0;

	if (EDMA_intTest(tccRcvPing)) {
		EDMA_intClear(tccRcvPing); // clear is mandatory
		rcvPingDone = 1;
	} else if (EDMA_intTest(tccRcvPong)) {
		EDMA_intClear(tccRcvPong);
		rcvPongDone = 1;
	}

	if (EDMA_intTest(tccXmtPing)) {
		EDMA_intClear(tccXmtPing);
		xmtPingDone = 1;
	} else if (EDMA_intTest(tccXmtPong)) {
		EDMA_intClear(tccXmtPong);
		xmtPongDone = 1;
	}

	/*
	 * *********** BSP Data Link Interface **************
	 */
	// Transmit
	if (EDMA_intTest(tccBSPLinkXmtPing)) {
		EDMA_intClear(tccBSPLinkXmtPing);
		xmtBSPLinkPingDone = 1;
	} else if (EDMA_intTest(tccBSPLinkXmtPong)) {
		EDMA_intClear(tccBSPLinkXmtPong);
		xmtBSPLinkPongDone = 1;
	}

	// BSPLink Anhalten, bis neuer Buffer verarbeitet
	if (xmtBSPLinkPingDone == 1 || xmtBSPLinkPongDone == 1) {
		BSPLink_EDMA_Stop();
	}

	// Receive
	if (EDMA_intTest(tccBSPLinkRcvPing)) {
		EDMA_intClear(tccBSPLinkRcvPing);
		rcvBSPLinkPingDone = 1;
		DSK6713_LED_on(3);
	} else if (EDMA_intTest(tccBSPLinkRcvPong)) {
		EDMA_intClear(tccBSPLinkRcvPong);
		rcvBSPLinkPongDone = 1;
		DSK6713_LED_on(3);
	}

	/*
	 * *********** DECODER **********************
	 *
	 *  BSPLink_in_ Ping/Pong -> Ringbuffer_in -> ADC Out ping/pong
	 */
	if (rcvBSPLinkPingDone) {
		rcvBSPLinkPingDone = 0;
		SWI_post(&SWI_BSPLink_In_Ping);
	} else if (rcvBSPLinkPongDone) {
		rcvBSPLinkPongDone = 0;
		SWI_post(&SWI_BSPLink_In_Pong);
	}

// Ringbuffer lesen -> ADC schreiben
	if (xmtPingDone) {
		DSK6713_LED_on(0);
		xmtPingDone = 0;
		SWI_post(&SWI_ADC_Out_Ping);
	} else if (xmtPongDone) {
		DSK6713_LED_on(0);
		xmtPongDone = 0;
		SWI_post(&SWI_ADC_Out_Pong);
	}
	/*
	 * *********** ENCODER **********************
	 *
	 *  ADC_in_ping/pong -> BSPLink_out ping/pong
	 */
	if (xmtBSPLinkPingDone && rcvPingDone) {
		DSK6713_LED_on(1);
		xmtBSPLinkPingDone = 0;
		rcvPingDone = 0;
		BSPLink_EDMA_Start_Pong();
		SWI_post(&SWI_ADC_In_Ping);
	} else if (xmtBSPLinkPongDone && rcvPongDone) {
		DSK6713_LED_on(1);
		xmtBSPLinkPongDone = 0;
		rcvPongDone = 0;
		BSPLink_EDMA_Start_Ping();
		SWI_post(&SWI_ADC_In_Pong);
	}

	DSK6713_LED_off(2);
}

/************************ SWI Section ****************************************/

// BSP Input RingBuffer lesen
void ADC_Out_Ping(void) {
	read_ring_buffer_in(AIC_Buffer_out_ping);
	DSK6713_LED_off(0);
}

void ADC_Out_Pong(void) {
	read_ring_buffer_in(AIC_Buffer_out_pong);
	DSK6713_LED_off(0);
}

// BSP Output schreiben
void ADC_In_Ping(void) {
	Uint16 Link_Buff_i = 0;
	Uint16 ADC_Buff_i = 0;

	for (Link_Buff_i = 0; Link_Buff_i < 20; Link_Buff_i++) {
		BSPLinkBuffer_out_ping[Link_Buff_i] = LINK_PREAM_START;
	}

	for (ADC_Buff_i = 0; ADC_Buff_i < AIC_BUFFER_LEN; ADC_Buff_i++) {
#ifdef SEND_DEBUG_BUFFER
		BSPLinkBuffer_out_ping[Link_Buff_i] = Debug_Buff_ping[ADC_Buff_i];
#else
		BSPLinkBuffer_out_ping[Link_Buff_i] = AIC_Buffer_in_ping[ADC_Buff_i];
#endif
		Link_Buff_i++;
	}

	for (; Link_Buff_i < LINK_BUFFER_LEN; Link_Buff_i++) {
		BSPLinkBuffer_out_ping[Link_Buff_i] = LINK_PREAM_STOP;
	}
	DSK6713_LED_off(1);
}

void ADC_In_Pong(void) {
	Uint16 Link_Buff_i = 0;
	Uint16 ADC_Buff_i = 0;

	for (Link_Buff_i = 0; Link_Buff_i < 20; Link_Buff_i++) {
		BSPLinkBuffer_out_pong[Link_Buff_i] = LINK_PREAM_START;
	}

	for (ADC_Buff_i = 0; ADC_Buff_i < AIC_BUFFER_LEN; ADC_Buff_i++) {
#ifdef SEND_DEBUG_BUFFER
		BSPLinkBuffer_out_pong[Link_Buff_i] = Debug_Buff_pong[ADC_Buff_i];
#else
		BSPLinkBuffer_out_pong[Link_Buff_i] = AIC_Buffer_in_pong[ADC_Buff_i];
#endif
		Link_Buff_i++;
	}

	for (; Link_Buff_i < LINK_BUFFER_LEN; Link_Buff_i++) {
		BSPLinkBuffer_out_pong[Link_Buff_i] = LINK_PREAM_STOP;
	}
	DSK6713_LED_off(1);
}

// BSP Input RingBuffer schreiben
void BSPLink_In_Ping(void) {
	write_ring_buffer_in(BSPLinkBuffer_in_ping);
	DSK6713_LED_off(3);
}

void BSPLink_In_Pong(void) {
	write_ring_buffer_in(BSPLinkBuffer_in_pong);
	DSK6713_LED_off(3);
}

/****************************************************************************/

void write_ring_buffer_in(short * buffersrc) {
	// BSP_in ping/pong Buffer in Ringbuffer schreiben

	Uint32 i_read;
	for (i_read = 0; i_read < LINK_BUFFER_LEN; i_read++) {

		*(Ringbuffer_in + ringbuff_in_write_i) = *(buffersrc + i_read);
		inc_ringbuff_i(&ringbuff_in_write_i);
	}
}

void read_ring_buffer_in(short * bufferdes) {
// Ringbuffer nach Daten durchsuchen

	Uint32 i_read;
	Uint32 i_write = 0;
	Uint8 dataDetected = 0;

	for (i_read = 0; i_read < RINGBUFFER_LEN; ringbuff_in_read_i++, i_read++) {
		// Ringbuffer einmal durchlaufen

		if (ringbuff_in_read_i >= RINGBUFFER_LEN)
			ringbuff_in_read_i = 0;

		if (Ringbuffer_in[ringbuff_in_read_i] == LINK_PREAM_STOP) {
			if (dataDetected)
				dataDetected = 3;
			continue;
		} else if (Ringbuffer_in[ringbuff_in_read_i] == LINK_PREAM_START) {
			dataDetected = 1;
			continue;

		} else if (Ringbuffer_in[ringbuff_in_read_i] != LINK_PREAM_START
				&& dataDetected == 1) {
			// Erstes Datum nach Start-Preämble
			dataDetected = 2;

		}

		if (dataDetected == 2) {
			bufferdes[i_write] = Ringbuffer_in[ringbuff_in_read_i];
			i_write++;
		} else if (dataDetected == 3) {
			bufferdes[i_write] = 0;
			i_write++;
		}

		if (i_write >= AIC_BUFFER_LEN)
			break;
	}
}

inline void inc_ringbuff_i(Uint32 * index) {
	if (*index < RINGBUFFER_LEN - 1)
		*index = *index + 1;
	else
		*index = 0;
}

/* Periodic Function */
void SWI_LEDToggle(void) {
	SEM_postBinary(&SEM_LEDToggle);
}

void tsk_led_toggle(void) {

	/* 1 sec Tick */
	while (1) {
		SEM_pendBinary(&SEM_LEDToggle, SYS_FOREVER);

		if (configComplete == 1) {

			MCBSP_close(hMcbsp_AIC23_Config);

			/* Set McBSP0 MUX to EXTERN */
			t_reg = DSK6713_rget(DSK6713_MISC);
			t_reg |= MCBSP1SEL;				// Set MCBSP0 to 1 (extern)
			DSK6713_rset(DSK6713_MISC, t_reg);

			/* configure BSPLink-Interface */
			config_BSPLink();

			/* Start AIC McBSP */
			MCBSP_start(hMcbsp_AIC23, MCBSP_XMIT_START | MCBSP_RCV_START,0xffffffff); // Start Audio IN & OUT transmision
			MCBSP_write(hMcbsp_AIC23, 0x0); /* one shot */

			configComplete = 2;

		} else if (configComplete == 2) {
		}

	}
}
