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
#include <dsk6713_dip.h>
#include <dsk6713.h>
#include "config_AIC23.h"
#include "config_BSPLink.h"
#include "Sounds.h"
#include "skeleton.h"
#include <math.h>

/*******************************
 * 			Typedefs
 *******************************/
union gamma {
	float f;
	Uint32 i;
};

/*******************************
 * 			VARIABLES
 *******************************/

Uint16 time_cnt = 0;

int configComplete = 0;	// Flag for switching McBSP0 to extern
Uint8 t_reg = 0;		// Used for bit manipulation in MCBSP config register

// Enable Encoder/Decoder with Switch 0 & 1
// Only set on startup
Uint32 ENCODER = 0;
Uint32 DECODER = 0;

/*********** ENCODER VARS ***********/
union gamma y_enc[ORDER];
short e_enc[ORDER + 1][ENCODING_BUFF_LEN] = { 0 };
short b_enc[ORDER + 1][ENCODING_BUFF_LEN] = { 0 };
float den_f;
float num_f;

/* Buffer for encoded data
 * is written by encoder & read by BSPLink transmiter 	*/
Uint32 Encoding_Buffer_i = 0;
Uint8 encoding_buff_valid = 0;

/*********** DECODER VARS ***********/
union gamma y_dec[ORDER];
short e_dec[ORDER + 1][DECODING_BUFF_LEN] = { 0 };
short b_dec[ORDER + 1][DECODING_BUFF_LEN] = { 0 };

/* Buffer for received, encoded data
 * is read by decoder 	*/
Uint32 Decoding_Buffer_i = 0;
Uint8 decoding_buff_valid = 0;

Uint8 dataDetected = 0;		// State of Frame-Detection in BSPLink received data

/* Audio ringbuffer for AIC output. Mono-Signal! */
/* is written by decoder, read by AIC transfer */
Uint32 ringbuff_audio_out_read_i = RINGBUFFER_LEN / 2;
Uint32 ringbuff_audio_out_write_i = 0;

main() {
	Uint16 i = 0;
	Uint16 j = 0;

	DSK6713_init();
	CSL_init();

	ENCODER = !DSK6713_DIP_get(0);
	DECODER = !DSK6713_DIP_get(1);

	DSK6713_LED_off(0);
	DSK6713_LED_off(1);
	DSK6713_LED_off(2);
	DSK6713_LED_off(3);

#ifdef SEND_DEBUG_BUFFER
	for (i = 0; i < AIC_BUFFER_LEN/2; i++, j += 2) {
//		Debug_Buff_ping[i] = LINK_PREAM_START;
//		Debug_Buff_pong[AIC_BUFFER_LEN-i-1] = LINK_PREAM_START;
		Debug_Buff_ping[j] = MySound[i];
		Debug_Buff_ping[j + 1] = MySound[i];
		Debug_Buff_pong[j] = MySound[AIC_BUFFER_LEN/2 -i -1];
		Debug_Buff_pong[j + 1] = MySound[AIC_BUFFER_LEN/2 - i-1];
	}
#endif

	for (i = 0; i < RINGBUFFER_LEN; i++) {
		Ringbuffer_Audio_out[i] = 0;
	}

	/* Configure McBSP0 and AIC23 */
	Config_DSK6713_AIC23();

	/* Configure McBSP1*/
	hMcbsp_AIC23 = MCBSP_open(MCBSP_DEV1, MCBSP_OPEN_RESET);
	MCBSP_config(hMcbsp_AIC23, &datainterface_config);

	/* configure EDMA */
	config_AIC23_EDMA();

	DSK6713_LED_on(0);

	/* finally the interrupts */
	config_interrupts();

	MCBSP_start(hMcbsp_AIC23, MCBSP_XMIT_START | MCBSP_RCV_START, 0xffffffff); // Start Audio IN & OUT transmision
	MCBSP_write(hMcbsp_AIC23, 0x0); /* one shot */

	configComplete = 1;

} /* finished*/

void config_AIC23_EDMA(void) {
	/* RECEIVE */
	/* Konfiguration der EDMA zum Lesen*/
	if (ENCODER) {
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

		/* EDMA TCC-Interrupts freigeben */
		EDMA_intClear(tccRcvPing);
		EDMA_intEnable(tccRcvPing);
		EDMA_intClear(tccRcvPong);
		EDMA_intEnable(tccRcvPong);
	}

	if (DECODER) {
		/* TRANSMIT */
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

		/* sind das alle? nein -> pong und alle für Sendeseite */
		EDMA_intClear(tccXmtPing);
		EDMA_intEnable(tccXmtPing);
		EDMA_intClear(tccXmtPong);
		EDMA_intEnable(tccXmtPong);
	}

	/* EDMA starten, wen alles? */
	if (ENCODER)
		EDMA_enableChannel(hEdmaRcv);
	if (DECODER)
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
	SWI_disable();
	static volatile int rcvPingDone = 0;
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

	// Receive
	if (EDMA_intTest(tccBSPLinkRcvPing)) {
		EDMA_intClear(tccBSPLinkRcvPing);
		rcvBSPLinkPingDone = 1;

	} else if (EDMA_intTest(tccBSPLinkRcvPong)) {
		EDMA_intClear(tccBSPLinkRcvPong);
		rcvBSPLinkPongDone = 1;
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
		xmtPingDone = 0;
		SWI_post(&SWI_ADC_Out_Ping);
	} else if (xmtPongDone) {
		xmtPongDone = 0;
		SWI_post(&SWI_ADC_Out_Pong);
	}
	/*
	 * *********** ENCODER **********************
	 *
	 *  ADC_in_ping/pong -> BSPLink_out ping/pong
	 */
	if (xmtBSPLinkPingDone) {
		xmtBSPLinkPingDone = 0;
		SWI_post(&SWI_BSPLink_Out_Ping);
	} else if (xmtBSPLinkPongDone) {
		xmtBSPLinkPongDone = 0;
		SWI_post(&SWI_BSPLink_Out_Pong);
	}

	// ADC lesen -> Ringbuffer schreiben
	if (rcvPingDone) {
		rcvPingDone = 0;
		SWI_post(&SWI_ADC_In_Ping);
	} else if (rcvPongDone) {
		rcvPongDone = 0;
		SWI_post(&SWI_ADC_In_Pong);
	}
	DSK6713_LED_off(2);
	SWI_enable();
}

/************************ SWI Section ****************************************/

// BSP Input RingBuffer lesen
void ADC_Out_Ping(void) {
	DSK6713_LED_on(0);
	read_buffer_audio_out(AIC_Buffer_out_ping);
}

void ADC_Out_Pong(void) {
	DSK6713_LED_off(0);
	read_buffer_audio_out(AIC_Buffer_out_pong);

}

void ADC_In_Ping(void) {
	//DSK6713_LED_on(0);
#ifdef SEND_DEBUG_BUFFER
	encode_audio_data(Debug_Buff_ping);
#else
	encode_audio_data(AIC_Buffer_in_ping);
#endif
}

void ADC_In_Pong(void) {
	//DSK6713_LED_off(0);
#ifdef SEND_DEBUG_BUFFER
	encode_audio_data(Debug_Buff_pong);
#else
	encode_audio_data(AIC_Buffer_in_pong);
#endif
}

// BSP Input RingBuffer schreiben
void BSPLink_In_Ping(void) {
	DSK6713_LED_on(1);
	write_decoding_buffer(BSPLinkBuffer_in_ping);
	DSK6713_LED_off(1);
}

void BSPLink_In_Pong(void) {
	DSK6713_LED_on(1);
	write_decoding_buffer(BSPLinkBuffer_in_pong);
	DSK6713_LED_off(1);
}

// BSP Input RingBuffer schreiben
void BSPLink_Out_Ping(void) {

	Uint16 i_write;
	if (encoding_buff_valid) {

		framing_link_data(BSPLinkBuffer_out_ping);
		encoding_buff_valid = 0;

	} else {
		// Encoding Buffer hat neue Werte
		for (i_write = 0; i_write < LINK_BUFFER_LEN; i_write++) {
			// Wenn encoder nicht bereit, STOP senden
			BSPLinkBuffer_out_ping[i_write] = LINK_PREAM_STOP;
		}

	}

}

void BSPLink_Out_Pong(void) {

	Uint16 i_write;
	if (encoding_buff_valid) {

		framing_link_data(BSPLinkBuffer_out_pong);
		encoding_buff_valid = 0;

	} else {
		// Encoding Buffer hat neue Werte
		for (i_write = 0; i_write < LINK_BUFFER_LEN; i_write++) {
			// Wenn encoder nicht bereit, STOP senden
			BSPLinkBuffer_out_pong[i_write] = LINK_PREAM_STOP;
		}

	}

}

void decode_buffer(void) {
	DSK6713_LED_on(3);

	//DECODER
	int i = 0;
	int k = 0;
	int n = 0;

	//gamma berechnen / ausmaskieren
	for (k = 0; k < (ORDER * 4); k = k + 4) {

		//build a float Y from 4x 8 Bit
		y_dec[i].i = 0;
		y_dec[i].i |= ((Decoding_Buffer[ORDER + k] & 0xFF00) >> 8);
		y_dec[i].i |= ((Decoding_Buffer[ORDER + k + 1] & 0xFF00));
		y_dec[i].i |= ((Decoding_Buffer[ORDER + k + 2] & 0xFF00) << 8);
		y_dec[i].i |= ((Decoding_Buffer[ORDER + k + 3] & 0xFF00) << 16);
		i++;
	}

	for (k = 0; k < DECODING_BUFF_LEN; k++) {
		// initialize with 0
		for (n = 0; n < ORDER; n++) {
			e_enc[n][k] = 0;
			b_enc[n][k] = 0;
		}
	}

	e_dec[0][0] = Decoding_Buffer[0];
	b_dec[0][0] = e_dec[0][0];

	// algorithm
	for (k = 1; k < DECODING_BUFF_LEN; k++) {
		if (k < ORDER) {
			i = k;
			//it must be a 16 BIT Value
			e_dec[k][k] = Decoding_Buffer[k];
		} else {
			i = ORDER;
			//e is a 8 BIT Value
			e_dec[ORDER][k] = (short) ((signed char) (Decoding_Buffer[k]
					& 0x00FF) * (float) 39.37007874);
		}

		for (n = i; n > 0; n--) {
			e_dec[n - 1][k] = e_dec[n][k]
					+ y_dec[n - 1].f * b_dec[n - 1][k - 1];
			b_dec[n][k] = b_dec[n - 1][k - 1]
					- y_dec[n - 1].f * e_dec[n - 1][k];
		}

		b_dec[0][k] = e_dec[0][k];
	}

	// write in output
	for (k = 0; k < DECODING_BUFF_LEN; k++) {
		Ringbuffer_Audio_out[ringbuff_audio_out_write_i] = e_dec[0][k];
		ringbuff_audio_out_write_i++;

		if (ringbuff_audio_out_write_i >= RINGBUFFER_LEN)
			ringbuff_audio_out_write_i = 0;
	}

	DSK6713_LED_off(3);
}

void encode_audio_data(short * buffersrc) {
	// Hier Encoding Machen
	//DSK6713_LED_on(3);
	signed long num = 0;
	unsigned long den = 0;

	int i = 0;
	int n = 0;

	Uint32 i_read;

	for (i_read = 0; i_read < AIC_BUFFER_LEN - 1; i++, i_read = i_read + 2) {

		// Audiodaten als MONO Signal in Buffer schreiben
		e_enc[0][i] =
				(short) (buffersrc[i_read] / 2 + buffersrc[i_read + 1] / 2);
		b_enc[0][i] = e_enc[0][i];

	}

	Encoding_Buffer[0] = e_enc[0][0];

	for (n = 1; n <= ORDER; n++) {
		num = 0;
		den = 0;
		for (i = 0; i < n + 1; i++) {
			e_enc[n][i] = 0;
			b_enc[n][i] = 0;
		}

		// denominator
		for (i = 1; i < ENCODING_BUFF_LEN; i++) {
			den += (e_enc[n - 1][i] * e_enc[n - 1][i])
					+ (b_enc[n - 1][i - 1] * b_enc[n - 1][i - 1]);
		}

		//debug_denum[n] = den;

		for (i = n; i < ENCODING_BUFF_LEN; i++) {
			// numerator
			num += (signed long) e_enc[n - 1][i] * b_enc[n - 1][i - 1];
		}
		// reflection factor
		den_f = (float) den;
		num_f = (float) num;
		y_enc[n - 1].f = num_f / den_f;
		y_enc[n - 1].f *= 2;

		for (i = n; i < ENCODING_BUFF_LEN; i++) {
			e_enc[n][i] = (short) (e_enc[n - 1][i]
					- y_enc[n - 1].f * (float) b_enc[n - 1][i - 1]);
			b_enc[n][i] = (short) (b_enc[n - 1][i - 1]
					- y_enc[n - 1].f * (float) e_enc[n - 1][i]);
		}

		Encoding_Buffer[n] = e_enc[n][n];
	}

	// write error values in buffer
	for (i = ORDER; i < ENCODING_BUFF_LEN; i++) {
		Encoding_Buffer[i] = (short) _nround((float) 0.0254 * e_enc[ORDER][i]);
		Encoding_Buffer[i] &= 0x00FF;
	}

	// write y in buffer
	i = 0;
	for (n = 0; n < (ORDER * 4); n += 4) {

		Encoding_Buffer[ORDER + n] |= (y_enc[i].i & (0xFF)) << 8;
		Encoding_Buffer[ORDER + n + 1] |= (y_enc[i].i & (0xFF00));
		Encoding_Buffer[ORDER + n + 2] |= (y_enc[i].i & (0xFF0000)) >> 8;
		Encoding_Buffer[ORDER + n + 3] |= (y_enc[i].i & (0xFF000000)) >> 16;
		i++;
	}

	encoding_buff_valid = 1;
	//DSK6713_LED_off(3);
}

/****************************************************************************/

void framing_link_data(short * bufferdes) {
// Kapselt Encoding_Buffer mit START und STOP und schreibt in BSP Output ping od. pong

	Uint16 Link_Buff_i = 0;
	Uint16 Enc_Buff_i = 0;

	for (Link_Buff_i = 0; Link_Buff_i < 20; Link_Buff_i++) {
		bufferdes[Link_Buff_i] = LINK_PREAM_START;
	}

	for (Enc_Buff_i = 0; Enc_Buff_i < ENCODING_BUFF_LEN; Enc_Buff_i++) {
		bufferdes[Link_Buff_i] = Encoding_Buffer[Enc_Buff_i];
		Link_Buff_i++;
	}

	for (; Link_Buff_i < LINK_BUFFER_LEN; Link_Buff_i++) {
		BSPLinkBuffer_out_ping[Link_Buff_i] = LINK_PREAM_STOP;
	}
}

void write_decoding_buffer(short * buffersrc) {
// Daten extrahieren, Decoder füllen

	Uint32 i_read;
#ifdef DEBUG_BUF_ENABLE
	for (i_read = 0; i_read < LINK_BUFFER_LEN; i_read++) {
// Ringbuffer einmal durchlaufen

		Debug_Buff[debug_buff_i] = buffersrc[i_read];
		debug_buff_i++;
		if (debug_buff_i >= 30000)
		debug_buff_i = 0;
	}
#endif

	for (i_read = 0; i_read < LINK_BUFFER_LEN; i_read++) {

		if (buffersrc[i_read] == LINK_PREAM_STOP) {
			if (dataDetected == DATA_DETECTED) {
				dataDetected = 0;
				SWI_post(&SWI_Decode_Buffer);
			}
		} else if (buffersrc[i_read] == LINK_PREAM_START) {
			Decoding_Buffer_i = 0;
			//if (dataDetected < DATA_DETECTED - 1)
				dataDetected++;
		} else {
			if (dataDetected > 5)
				dataDetected = DATA_DETECTED;

			Decoding_Buffer[Decoding_Buffer_i] = buffersrc[i_read];
			Decoding_Buffer_i++;
			if (Decoding_Buffer_i >= DECODING_BUFF_LEN) {
				Decoding_Buffer_i = 0;
			}
		}
	}
}

void read_buffer_audio_out(short * bufferdes) {
// Audio_OUT Ringbuffer nach AIC schreiben

	Uint32 i_write;

	for (i_write = 0; i_write < AIC_BUFFER_LEN - 1; i_write = i_write + 2) {

		bufferdes[i_write] = Ringbuffer_Audio_out[ringbuff_audio_out_read_i];
		bufferdes[i_write + 1] =
				Ringbuffer_Audio_out[ringbuff_audio_out_read_i];

		ringbuff_audio_out_read_i++;

		if (ringbuff_audio_out_read_i >= RINGBUFFER_LEN)
			ringbuff_audio_out_read_i = 0;
	}
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
			config_BSPLink(ENCODER, DECODER);

			configComplete = 2;

		} else if (configComplete == 2) {
		}

	}
}
