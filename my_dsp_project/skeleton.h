#ifndef SKELETON_H_
#define SKELETON_H

/* DEFINES */
#define AIC_BUFFER_LEN 1000
#define RINGBUFFER_LEN	6000

#define DECODING_BUFF_LEN	AIC_BUFFER_LEN/2
#define ENCODING_BUFF_LEN DECODING_BUFF_LEN

#define LINK_PREAM_START (short) 32767
#define LINK_PREAM_STOP (short) -32768


#define ORDER	6

//#define SEND_DEBUG_BUFFER
//#define DEBUG_BUF_ENABLE

/* BUFFER ARRAYS */

/* Ping-Pong buffers. Place them in the compiler section .datenpuffer */
/* How do you place the compiler section in the memory?     */
#pragma DATA_SECTION(AIC_Buffer_in_ping, ".datenpuffer");
short AIC_Buffer_in_ping[AIC_BUFFER_LEN];
#pragma DATA_SECTION(AIC_Buffer_in_pong, ".datenpuffer");
short AIC_Buffer_in_pong[AIC_BUFFER_LEN];
#pragma DATA_SECTION(AIC_Buffer_out_ping, ".datenpuffer");
short AIC_Buffer_out_ping[AIC_BUFFER_LEN];
#pragma DATA_SECTION(AIC_Buffer_out_pong, ".datenpuffer");
short AIC_Buffer_out_pong[AIC_BUFFER_LEN];


#pragma DATA_SECTION(Ringbuffer_Audio_out, ".datenpuffer");
short Ringbuffer_Audio_out[RINGBUFFER_LEN];


#pragma DATA_SECTION(Decoding_Buffer, ".datenpuffer");
short Decoding_Buffer[DECODING_BUFF_LEN];

#pragma DATA_SECTION(Encoding_Buffer, ".datenpuffer");
short Encoding_Buffer[ENCODING_BUFF_LEN];

/*Send Debug Buffer */
#ifdef SEND_DEBUG_BUFFER
#pragma DATA_SECTION(Debug_Buff_ping, ".datenpuffer");
short Debug_Buff_ping[AIC_BUFFER_LEN];
#pragma DATA_SECTION(Debug_Buff_pong, ".datenpuffer");
short Debug_Buff_pong[AIC_BUFFER_LEN];
#endif

/* Debug Buffer */
#ifdef DEBUG_BUF_ENABLE
#define DEBUG_BUF_LEN 10000
#pragma DATA_SECTION(Debug_Buff, ".datenpuffer");
short Debug_Buff[DEBUG_BUF_LEN];
#endif
Uint16 debug_buff_i =0;




/* function prototipes */
extern void encode_audio_data(short * bufferdes);
extern void write_decoding_buffer(short * buffersrc);
extern void read_buffer_audio_out(short * bufferdes);
extern void write_buffer_audio_in(short * bufferscr);
extern void framing_link_data(short * bufferdes);

extern void EDMA_interrupt_service(void);
extern void config_AIC23_EDMA(void);
void config_interrupts(void);
// Datenverarbeitung


extern void decode_buffer(void);
extern void tsk_led_toggle(void);

/* Global Handler */

/* McBSP1 for AIC23 DATA */
MCBSP_Handle hMcbsp_AIC23 = 0;

EDMA_Handle hEdmaRcv; /* Empfang handle auf EDMA REVT1-Channel */
EDMA_Handle hEdmaRcvRelPing; /* Empfang handle auf einen reload-Parametersatz */
EDMA_Handle hEdmaRcvRelPong;
/* braucht man noch mehr? ja -> pong */
EDMA_Handle hEdmaXmt; /* handle auf EDMA XEVT1-Channel */
EDMA_Handle hEdmaXmtRelPing;
EDMA_Handle hEdmaXmtRelPong;

/* Transfer-Complete-Codes for EDMA-Jobs */
int tccRcvPing;
int tccRcvPong;
int tccXmtPing;
int tccXmtPong;

/*
 * CONFIGURATION STRUCTS
 */

//Configuration for McBSP1 (data-interface)
MCBSP_Config datainterface_config = {
/* McBSP Control Register */
MCBSP_FMKS(SPCR, FREE, NO) |	//  Freilauf
		MCBSP_FMKS(SPCR, SOFT, NO) |// YES:  Soft mode is enabled. During emulation halt, serial port clock stops after completion of current transmission.
		MCBSP_FMKS(SPCR, FRST, YES) |	// Framesync ist ein
		MCBSP_FMKS(SPCR, GRST, YES) |// Reset aus, damit läuft der Samplerate- Generator
		MCBSP_FMKS(SPCR, XINTM, XRDY) |// XINT is driven by XRDY (end-of-word) and end-of-frame in A-bis mode.
		MCBSP_FMKS(SPCR, XSYNCERR, NO) |// empfängerseitig keine Überwachung der Synchronisation
		MCBSP_FMKS(SPCR, XRST, YES) |	// Sender läuft (kein Reset- Status)
		MCBSP_FMKS(SPCR, DLB, OFF) |	// Loopback (Kurschluss) nicht aktiv
		MCBSP_FMKS(SPCR, RJUST, RZF) |	// rechtsbündige Ausrichtung der Daten im Puffer
		MCBSP_FMKS(SPCR, CLKSTP, DISABLE) |// Clock startet ohne Verzögerung auf fallenden Flanke (siehe auch PCR-Register)
		MCBSP_FMKS(SPCR, DXENA, OFF) |	// DX- Enabler wird nicht verwendet
		MCBSP_FMKS(SPCR, RINTM, RRDY) |// Sender Interrupt wird durch "RRDY-Bit" ausgelöst
		MCBSP_FMKS(SPCR, RSYNCERR, NO) |// senderseitig keine Überwachung der Synchronisation
		MCBSP_FMKS(SPCR, RRST, YES),	// Empfänger läuft (kein Reset- Status)
		/* Empfangs-Control Register */
MCBSP_FMKS(RCR, RPHASE, SINGLE) |	// Nur eine Phase pro Frame
		MCBSP_FMKS(RCR, RFRLEN2, DEFAULT) |	// Länge in Phase 2, unrelevant
		MCBSP_FMKS(RCR, RWDLEN2, DEFAULT) |// Wortlänge in Phase 2, unrelevant
		MCBSP_FMKS(RCR, RCOMPAND, MSB) |// kein Compandierung der Daten (MSB first)
		MCBSP_FMKS(RCR, RFIG, NO) |// Rahmensynchronisationspulse (nach dem ersten Puls)) startet die Übertragung neu
		MCBSP_FMKS(RCR, RDATDLY, 0BIT) |// keine Verzögerung (delay) der Daten
		MCBSP_FMKS(RCR, RFRLEN1, OF(1)) |	// Länge der Phase 1 --> 1 Wort
		MCBSP_FMKS(RCR, RWDLEN1, 16BIT) |	//
		MCBSP_FMKS(RCR, RWDREVRS, DISABLE),	// 32-bit Reversal nicht genutzt
		/* Sende-Control Register */
MCBSP_FMKS(XCR, XPHASE, SINGLE) |	//
		MCBSP_FMKS(XCR, XFRLEN2, DEFAULT) |	// Länge in Phase 2, unrelevant
		MCBSP_FMKS(XCR, XWDLEN2, DEFAULT) |// Wortlänge in Phase 2, unrelevant
		MCBSP_FMKS(XCR, XCOMPAND, MSB) |// kein Compandierung der Daten (MSB first)
		MCBSP_FMKS(XCR, XFIG, NO) |// Rahmensynchronisationspulse (nach dem ersten Puls)) startet die Übertragung neu
		MCBSP_FMKS(XCR, XDATDLY, 0BIT) |// keine Verzögerung (delay) der Daten
		MCBSP_FMKS(XCR, XFRLEN1, OF(1)) |	// Länge der Phase 1 --> 1 Wort
		MCBSP_FMKS(XCR, XWDLEN1, 16BIT) |	// Wortlänge in Phase 1 --> 16 bit
		MCBSP_FMKS(XCR, XWDREVRS, DISABLE),	// 32-bit Reversal nicht genutzt
		/* Sample Rate Generator Register */
MCBSP_FMKS(SRGR, GSYNC, DEFAULT) |	// Einstellungen nicht relevant da
		MCBSP_FMKS(SRGR, CLKSP, DEFAULT) |	// der McBSP1 als Slave läuft
		MCBSP_FMKS(SRGR, CLKSM, DEFAULT) |	// und den Takt von aussen vorgegeben bekommt.
		MCBSP_FMKS(SRGR, FSGM, DEFAULT) |// Egal, da FSXM auf Extern Pin steht. See *spru580g.pdf Page 16
		MCBSP_FMKS(SRGR, FPER, DEFAULT) |	// --
		MCBSP_FMKS(SRGR, FWID, DEFAULT) |	// --
		MCBSP_FMKS(SRGR, CLKGDV, DEFAULT),		// --
		/* Mehrkanal */
MCBSP_MCR_DEFAULT,				// Mehrkanal wird nicht verwendet
		MCBSP_RCER_DEFAULT,				// dito
		MCBSP_XCER_DEFAULT,				// dito
		/* Pinout Control Register */
		MCBSP_FMKS(PCR, XIOEN, SP) |// Pin wird für serielle Schnittstelle verwendet (alternativ GPIO)
				MCBSP_FMKS(PCR, RIOEN, SP) |// Pin wird für serielle Schnittstelle verwendet (alternativ GPIO)
				MCBSP_FMKS(PCR, FSXM, EXTERNAL) |// Framesync- Signal für Sender kommt von extern (Slave)
				MCBSP_FMKS(PCR, FSRM, EXTERNAL) |// Framesync- Signal für Empfänger kommt von extern (Slave)
				MCBSP_FMKS(PCR, CLKXM, INPUT) |// Takt für Sender kommt von extern (Slave)
				MCBSP_FMKS(PCR, CLKRM, INPUT) |// Takt für Empfänger kommt von extern (Slave)
				MCBSP_FMKS(PCR, CLKSSTAT, DEFAULT) |// unrelevant da PINS keine GPIOs
				MCBSP_FMKS(PCR, DXSTAT, DEFAULT) |	// unrelevant da PINS keine GPIOs
				MCBSP_FMKS(PCR, FSXP, ACTIVEHIGH) |// Framesync senderseitig ist "activehigh"
				MCBSP_FMKS(PCR, FSRP, ACTIVEHIGH) |// Framesync empfängerseitig ist "activehigh"
				MCBSP_FMKS(PCR, CLKXP, FALLING) |// Datum wird bei fallender Flanke gesendet
				MCBSP_FMKS(PCR, CLKRP, RISING)// Datum wird bei steigender Flanke übernommen
};

/* template for a EDMA configuration */
/* Empfangen */
EDMA_Config configEDMARcv = { EDMA_FMKS(OPT, PRI, LOW) | // auf beide Queues verteilen
		EDMA_FMKS(OPT, ESIZE, 16BIT) |  // Element size
		EDMA_FMKS(OPT, 2DS, NO) |  // kein 2D-Transfer
		EDMA_FMKS(OPT, SUM, NONE) |  // Quell-update mode -> FEST (McBSP)!!!
		EDMA_FMKS(OPT, 2DD, NO) |  // 2kein 2D-Transfer
		EDMA_FMKS(OPT, DUM, INC) | // Ziel-update mode -> inkrementieren (ping/pong in buffer)
		EDMA_FMKS(OPT, TCINT, YES) |  // EDMA interrupt erzeugen?
		EDMA_FMKS(OPT, TCC, OF(0)) |  // Transfer complete code (TCC)
		EDMA_FMKS(OPT, LINK, YES) |  // Link Parameter nutzen?
		EDMA_FMKS(OPT, FS, NO),               // Frame Sync nutzen?

EDMA_FMKS(SRC, SRC, OF(0)),           // Quell-Adresse

EDMA_FMK(CNT, FRMCNT, NULL) |  // Anzahl Frames
		EDMA_FMK(CNT, ELECNT, AIC_BUFFER_LEN),   // Anzahl Elemente

(Uint32) AIC_Buffer_in_ping,     // Ziel-Adresse ( initialsierung auf ping )

		EDMA_FMKS(IDX, FRMIDX, DEFAULT) |  // Frame index Wert
				EDMA_FMKS(IDX, ELEIDX, DEFAULT),      // Element index Wert

		EDMA_FMK(RLD, ELERLD, NULL) |  // Reload Element
				EDMA_FMK(RLD, LINK, NULL)            // Reload Link
};

/* Senden */
EDMA_Config configEDMAXmt = { EDMA_FMKS(OPT, PRI, LOW) | // auf beide Queues verteilen
		EDMA_FMKS(OPT, ESIZE, 16BIT) |  // Element size
		EDMA_FMKS(OPT, 2DS, NO) |  // kein 2D-Transfer
		EDMA_FMKS(OPT, SUM, INC) | // Quell-update mode -> inkrementieren (ping/pong out buffer)
		EDMA_FMKS(OPT, 2DD, NO) |  // 2kein 2D-Transfer
		EDMA_FMKS(OPT, DUM, NONE) |  // Ziel-update mode -> FEST (McBSP)!!!
		EDMA_FMKS(OPT, TCINT, YES) |  // EDMA interrupt erzeugen?
		EDMA_FMKS(OPT, TCC, OF(0)) |  // Transfer complete code (TCC)
		EDMA_FMKS(OPT, LINK, YES) |  // Link Parameter nutzen?
		EDMA_FMKS(OPT, FS, NO),               // Frame Sync nutzen?

(Uint32) AIC_Buffer_out_ping,	// Quell-Adresse (initialisierung auf ping)

		EDMA_FMK(CNT, FRMCNT, NULL) |  // Anzahl Frames
				EDMA_FMK(CNT, ELECNT, AIC_BUFFER_LEN),   // Anzahl Elemente

		EDMA_FMKS(DST, DST, OF(0)),       	  // Ziel-Adresse

		EDMA_FMKS(IDX, FRMIDX, DEFAULT) |  // Frame index Wert
				EDMA_FMKS(IDX, ELEIDX, DEFAULT),      // Element index Wert

		EDMA_FMK(RLD, ELERLD, NULL) |  // Reload Element
				EDMA_FMK(RLD, LINK, NULL)            // Reload Link
};

#endif /*SKELETON_H*/
