/***********************************************************
*  skeleton.c
*  Example for ping-pong processing
*  Caution: It is intended, that this file ist not runnable. 
*  The file contains mistakes and omissions, which shall be
*  corrected and completed by the students.
*
*   F. Quint, HsKA
*
*   Von mir gegittet!
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


//#define BUFFER_LEN 1000
#define RINGBUFFER_LEN	12001
/* Ping-Pong buffers. Place them in the compiler section .datenpuffer */
/* How do you place the compiler section in the memory?     */
#pragma DATA_SECTION(Buffer_in_ping, ".datenpuffer");
short Buffer_in_ping[BUFFER_LEN];
#pragma DATA_SECTION(Buffer_in_pong, ".datenpuffer");
short Buffer_in_pong[BUFFER_LEN];
#pragma DATA_SECTION(Buffer_out_ping, ".datenpuffer");
short Buffer_out_ping[BUFFER_LEN];
#pragma DATA_SECTION(Buffer_out_pong, ".datenpuffer");
short Buffer_out_pong[BUFFER_LEN];


#pragma DATA_SECTION(Temp_Buffer_0, ".datenpuffer");
short Temp_Buffer_0[BUFFER_LEN];
#pragma DATA_SECTION(Temp_Buffer_1, ".datenpuffer");
short Temp_Buffer_1[BUFFER_LEN];



#pragma DATA_SECTION(Ringbuffer_out, ".datenpuffer");
short Ringbuffer_out[RINGBUFFER_LEN];
#pragma DATA_SECTION(Ringbuffer_in, ".datenpuffer");
short Ringbuffer_in[RINGBUFFER_LEN];

Uint32 ringbuff_in_read_i = RINGBUFFER_LEN/2;
Uint32 ringbuff_in_write_i = 0;

Uint32 ringbuff_out_read_i = RINGBUFFER_LEN/2;
Uint32 ringbuff_out_write_i = 0;

Uint32 soundBuffer_i = 0;

Uint16 time_cnt = 0;
//Configuration for McBSP1 (data-interface)
MCBSP_Config datainterface_config = {
		/* McBSP Control Register */
        MCBSP_FMKS(SPCR, FREE, NO)              |	//  Freilauf
        MCBSP_FMKS(SPCR, SOFT, NO)		        |	// YES:  Soft mode is enabled. During emulation halt, serial port clock stops after completion of current transmission.
        MCBSP_FMKS(SPCR, FRST, YES)             |	// Framesync ist ein
        MCBSP_FMKS(SPCR, GRST, YES)             |	// Reset aus, damit läuft der Samplerate- Generator
        MCBSP_FMKS(SPCR, XINTM, XRDY)           |	// XINT is driven by XRDY (end-of-word) and end-of-frame in A-bis mode.
        MCBSP_FMKS(SPCR, XSYNCERR, NO)          |	// empfängerseitig keine Überwachung der Synchronisation
        MCBSP_FMKS(SPCR, XRST, YES)             |	// Sender läuft (kein Reset- Status)	
        MCBSP_FMKS(SPCR, DLB, OFF)              |	// Loopback (Kurschluss) nicht aktiv
        MCBSP_FMKS(SPCR, RJUST, RZF)            |	// rechtsbündige Ausrichtung der Daten im Puffer
        MCBSP_FMKS(SPCR, CLKSTP, DISABLE)       |	// Clock startet ohne Verzögerung auf fallenden Flanke (siehe auch PCR-Register)
        MCBSP_FMKS(SPCR, DXENA, OFF)            |	// DX- Enabler wird nicht verwendet
        MCBSP_FMKS(SPCR, RINTM, RRDY)           |	// Sender Interrupt wird durch "RRDY-Bit" ausgelöst
        MCBSP_FMKS(SPCR, RSYNCERR, NO)          |	// senderseitig keine Überwachung der Synchronisation
        MCBSP_FMKS(SPCR, RRST, YES),			// Empfänger läuft (kein Reset- Status)
		/* Empfangs-Control Register */
        MCBSP_FMKS(RCR, RPHASE, SINGLE)         |	// Nur eine Phase pro Frame
        MCBSP_FMKS(RCR, RFRLEN2, DEFAULT)       |	// Länge in Phase 2, unrelevant
        MCBSP_FMKS(RCR, RWDLEN2, DEFAULT)       |	// Wortlänge in Phase 2, unrelevant
        MCBSP_FMKS(RCR, RCOMPAND, MSB)          |	// kein Compandierung der Daten (MSB first)
        MCBSP_FMKS(RCR, RFIG, NO)               |	// Rahmensynchronisationspulse (nach dem ersten Puls)) startet die Übertragung neu
        MCBSP_FMKS(RCR, RDATDLY, 0BIT)          |	// keine Verzögerung (delay) der Daten
        MCBSP_FMKS(RCR, RFRLEN1, OF(1))         |	// Länge der Phase 1 --> 1 Wort
        MCBSP_FMKS(RCR, RWDLEN1, 16BIT)         |	//
        MCBSP_FMKS(RCR, RWDREVRS, DISABLE),		// 32-bit Reversal nicht genutzt
		/* Sende-Control Register */
        MCBSP_FMKS(XCR, XPHASE, SINGLE)         |	//
        MCBSP_FMKS(XCR, XFRLEN2, DEFAULT)       |	// Länge in Phase 2, unrelevant
        MCBSP_FMKS(XCR, XWDLEN2, DEFAULT)       |	// Wortlänge in Phase 2, unrelevant
        MCBSP_FMKS(XCR, XCOMPAND, MSB)          |	// kein Compandierung der Daten (MSB first)
        MCBSP_FMKS(XCR, XFIG, NO)               |	// Rahmensynchronisationspulse (nach dem ersten Puls)) startet die Übertragung neu
        MCBSP_FMKS(XCR, XDATDLY, 0BIT)          |	// keine Verzögerung (delay) der Daten
        MCBSP_FMKS(XCR, XFRLEN1, OF(1))         |	// Länge der Phase 1 --> 1 Wort
        MCBSP_FMKS(XCR, XWDLEN1, 16BIT)         |	// Wortlänge in Phase 1 --> 16 bit
        MCBSP_FMKS(XCR, XWDREVRS, DISABLE),		// 32-bit Reversal nicht genutzt
		/* Sample Rate Generator Register */
        MCBSP_FMKS(SRGR, GSYNC, DEFAULT)        |	// Einstellungen nicht relevant da
        MCBSP_FMKS(SRGR, CLKSP, DEFAULT)        |	// der McBSP1 als Slave läuft
        MCBSP_FMKS(SRGR, CLKSM, DEFAULT)        |	// und den Takt von aussen vorgegeben bekommt.
        MCBSP_FMKS(SRGR, FSGM, DEFAULT)         |	// Egal, da FSXM auf Extern Pin steht. See *spru580g.pdf Page 16
        MCBSP_FMKS(SRGR, FPER, DEFAULT)         |	// --
        MCBSP_FMKS(SRGR, FWID, DEFAULT)         |	// --
        MCBSP_FMKS(SRGR, CLKGDV, DEFAULT),		// --
		/* Mehrkanal */
        MCBSP_MCR_DEFAULT,				// Mehrkanal wird nicht verwendet
        MCBSP_RCER_DEFAULT,				// dito
        MCBSP_XCER_DEFAULT,				// dito
		/* Pinout Control Register */
        MCBSP_FMKS(PCR, XIOEN, SP)              |	// Pin wird für serielle Schnittstelle verwendet (alternativ GPIO)
        MCBSP_FMKS(PCR, RIOEN, SP)              |	// Pin wird für serielle Schnittstelle verwendet (alternativ GPIO)
        MCBSP_FMKS(PCR, FSXM, EXTERNAL)         |	// Framesync- Signal für Sender kommt von extern (Slave)
        MCBSP_FMKS(PCR, FSRM, EXTERNAL)         |	// Framesync- Signal für Empfänger kommt von extern (Slave)
        MCBSP_FMKS(PCR, CLKXM, INPUT)           |	// Takt für Sender kommt von extern (Slave)
        MCBSP_FMKS(PCR, CLKRM, INPUT)           |	// Takt für Empfänger kommt von extern (Slave)
        MCBSP_FMKS(PCR, CLKSSTAT, DEFAULT)      |	// unrelevant da PINS keine GPIOs
        MCBSP_FMKS(PCR, DXSTAT, DEFAULT)        |	// unrelevant da PINS keine GPIOs
        MCBSP_FMKS(PCR, FSXP, ACTIVEHIGH)       |	// Framesync senderseitig ist "activehigh"
        MCBSP_FMKS(PCR, FSRP, ACTIVEHIGH)       |	// Framesync empfängerseitig ist "activehigh"
        MCBSP_FMKS(PCR, CLKXP, FALLING)         |	// Datum wird bei fallender Flanke gesendet
        MCBSP_FMKS(PCR, CLKRP, RISING)			// Datum wird bei steigender Flanke übernommen
};

/* template for a EDMA configuration */
/* Empfangen */
EDMA_Config configEDMARcv = {
    EDMA_FMKS(OPT, PRI, LOW)           |  // auf beide Queues verteilen
    EDMA_FMKS(OPT, ESIZE, 16BIT)       |  // Element size
    EDMA_FMKS(OPT, 2DS, NO)            |  // kein 2D-Transfer
    EDMA_FMKS(OPT, SUM, NONE)          |  // Quell-update mode -> FEST (McBSP)!!!
    EDMA_FMKS(OPT, 2DD, NO)            |  // 2kein 2D-Transfer
    EDMA_FMKS(OPT, DUM, INC)           |  // Ziel-update mode -> inkrementieren (ping/pong in buffer)
    EDMA_FMKS(OPT, TCINT,YES)          |  // EDMA interrupt erzeugen?
    EDMA_FMKS(OPT, TCC, OF(0))         |  // Transfer complete code (TCC)
    EDMA_FMKS(OPT, LINK, YES)          |  // Link Parameter nutzen?
    EDMA_FMKS(OPT, FS, NO),               // Frame Sync nutzen?

    EDMA_FMKS(SRC, SRC, OF(0)),           // Quell-Adresse

    EDMA_FMK(CNT, FRMCNT, NULL)        |  // Anzahl Frames
    EDMA_FMK(CNT, ELECNT, BUFFER_LEN),   // Anzahl Elemente

    (Uint32)Buffer_in_ping,       		  // Ziel-Adresse ( initialsierung auf ping )

    EDMA_FMKS(IDX, FRMIDX, DEFAULT)    |  // Frame index Wert
    EDMA_FMKS(IDX, ELEIDX, DEFAULT),      // Element index Wert

    EDMA_FMK (RLD, ELERLD, NULL)       |  // Reload Element
    EDMA_FMK (RLD, LINK, NULL)            // Reload Link
};

/* Senden */
EDMA_Config configEDMAXmt = {
    EDMA_FMKS(OPT, PRI, LOW)           |  // auf beide Queues verteilen
    EDMA_FMKS(OPT, ESIZE, 16BIT)       |  // Element size
    EDMA_FMKS(OPT, 2DS, NO)            |  // kein 2D-Transfer
    EDMA_FMKS(OPT, SUM, INC)           |  // Quell-update mode -> inkrementieren (ping/pong out buffer)
    EDMA_FMKS(OPT, 2DD, NO)            |  // 2kein 2D-Transfer
    EDMA_FMKS(OPT, DUM, NONE)          |  // Ziel-update mode -> FEST (McBSP)!!!
    EDMA_FMKS(OPT, TCINT,YES)          |  // EDMA interrupt erzeugen?
    EDMA_FMKS(OPT, TCC, OF(0))         |  // Transfer complete code (TCC)
    EDMA_FMKS(OPT, LINK, YES)          |  // Link Parameter nutzen?
    EDMA_FMKS(OPT, FS, NO),               // Frame Sync nutzen?

	(Uint32)Buffer_out_ping,			// Quell-Adresse (initialisierung auf ping)

    EDMA_FMK(CNT, FRMCNT, NULL)       |  // Anzahl Frames
    EDMA_FMK(CNT, ELECNT, BUFFER_LEN),   // Anzahl Elemente

    EDMA_FMKS(DST, DST, OF(0)),       	  // Ziel-Adresse

    EDMA_FMKS(IDX, FRMIDX, DEFAULT)    |  // Frame index Wert
    EDMA_FMKS(IDX, ELEIDX, DEFAULT),      // Element index Wert

    EDMA_FMK (RLD, ELERLD, NULL)       |  // Reload Element
    EDMA_FMK (RLD, LINK, NULL)            // Reload Link
};


								
int configComplete = 0;
Uint8 t_reg = 0;
Uint8 mul = 0;
short dat = -6000;
main()
{
	DSK6713_init();
	CSL_init();


	Uint32 i = 0;

	for(i = 0; i < BUFFER_LEN; i++)
						{
							*(Temp_Buffer_0+i) = 1000;
						}

	for(i = 0; i < BUFFER_LEN; i++)
						{
							*(Temp_Buffer_1+i) = 2000;
						}

/*
 * 	ringbuff_in_write_i =0;
	Uint32 i = 0;
		for(ringbuff_in_write_i = 0; ringbuff_in_write_i < RINGBUFFER_LEN; ringbuff_in_write_i++)
					{
						*(Ringbuffer_in+ringbuff_in_write_i) = (short) 888;
					}

		for(i = 0; i < BUFFER_LEN; i++)
					{
						*(Buffer_in_ping+i) = (short) 999;
						*(Buffer_in_pong+i) = (short) 777;
					}

*/
//	ringbuff_out_write_i =0;
//	for(soundBuffer_i = 0; ringbuff_out_write_i < RINGBUFFER_LEN -1 ; ringbuff_out_write_i++ )
//	{
//		if(ringbuff_out_write_i < 30000)
//		{
//
//
//					*(Ringbuffer_out+ringbuff_out_write_i) = (short) *(MySinus+soundBuffer_i)*((float) ringbuff_out_write_i/10000);
//					ringbuff_out_write_i++;
//					*(Ringbuffer_out+ringbuff_out_write_i) = (short) *(MySinus+soundBuffer_i)*((float) ringbuff_out_write_i/10000);;
//
//					if(soundBuffer_i >= SOUND_BUFF_LEN-1)
//							soundBuffer_i = 0;
//						else
//							soundBuffer_i++;
//		}
//		else
//			*(Ringbuffer_out+ringbuff_out_write_i) = 0;
//
//	}
//
//	ringbuff_out_write_i =0;
//	ringbuff_out_write_i =0;
//		for(soundBuffer_i = 0; ringbuff_out_write_i < RINGBUFFER_LEN; ringbuff_out_write_i++)
//					{
//						*(Ringbuffer_out+ringbuff_out_write_i) = dat ++;
//					}


	/* Configure McBSP0 and AIC23 */
	Config_DSK6713_AIC23();

	/* Configure McBSP1*/
	hMcbsp = MCBSP_open(MCBSP_DEV1, MCBSP_OPEN_RESET);
    MCBSP_config(hMcbsp, &datainterface_config);
    
	/* configure EDMA */
    config_EDMA();

	DSK6713_LED_on(0);
	DSK6713_LED_on(1);
	DSK6713_LED_on(2);
	DSK6713_LED_on(3);
    /* finally the interrupts */
    config_interrupts();

    MCBSP_start(hMcbsp, MCBSP_XMIT_START | MCBSP_RCV_START, 0xffffffff);		// Start Audio IN & OUT transmision
    MCBSP_write(hMcbsp, 0x0); 	/* one shot */

    configComplete = 1;
    //t_reg = DSK6713_rget(DSK6713_MISC);
    //t_reg |= MCBSP1SEL;				// Set MCBSP1SEL to 1 (extern)
    //DSK6713_rset(DSK6713_MISC,t_reg);

} /* finished*/


void config_EDMA(void)
{
	/* Konfiguration der EDMA zum Lesen*/
	hEdmaRcv = EDMA_open(EDMA_CHA_REVT1, EDMA_OPEN_RESET);  // EDMA Kanal für das Event REVT1
	hEdmaRcvRelPing = EDMA_allocTable(-1);               // einen Reload-Parametersatz für Ping
	hEdmaRcvRelPong = EDMA_allocTable(-1);               // einen Reload-Parametersatz für Pong

	configEDMARcv.src = MCBSP_getRcvAddr(hMcbsp);          //  Quell-Adresse zum Lesen

	tccRcvPing = EDMA_intAlloc(-1);                        // nächsten freien Transfer-Complete-Code Ping
	tccRcvPong = EDMA_intAlloc(-1); 						// nächsten freien Transfer-Complete-Code Pong
	configEDMARcv.opt |= EDMA_FMK(OPT,TCC,tccRcvPing);     // dann der Grundkonfiguration des EDMA Empfangskanals zuweisen

	/* ersten Transfer und Reload-Ping mit ConfigPing konfigurieren */
	EDMA_config(hEdmaRcv, &configEDMARcv);
	EDMA_config(hEdmaRcvRelPing, &configEDMARcv);

	/* braucht man auch noch andere EDMA-Konfigurationen fuer das Lesen? ja -> pong */
	configEDMARcv.opt &= 0xFFF0FFFF;
	configEDMARcv.opt |= EDMA_FMK(OPT,TCC,tccRcvPong);
	configEDMARcv.dst = (Uint32)Buffer_in_pong ;
	EDMA_config(hEdmaRcvRelPong, &configEDMARcv);


	/* Transfers verlinken ping -> pong -> ping */
	EDMA_link(hEdmaRcv, hEdmaRcvRelPong);  /* noch mehr verlinken? */
	EDMA_link(hEdmaRcvRelPong, hEdmaRcvRelPing);
	EDMA_link(hEdmaRcvRelPing, hEdmaRcvRelPong);


	/* muss man mittels EDMA auch schreiben? */
	/* Konfiguration der EDMA zum Schreiben */
	hEdmaXmt = EDMA_open(EDMA_CHA_XEVT1, EDMA_OPEN_RESET);  // EDMA Kanal für das Event REVT1
	hEdmaXmtRelPing = EDMA_allocTable(-1);               // einen Reload-Parametersatz für Ping
	hEdmaXmtRelPong = EDMA_allocTable(-1);               // einen Reload-Parametersatz für Pong

	configEDMAXmt.dst = MCBSP_getXmtAddr(hMcbsp);	//  Ziel-Adresse zum Schreiben

	tccXmtPing = EDMA_intAlloc(-1);                        // nächsten freien Transfer-Complete-Code Ping
	tccXmtPong = EDMA_intAlloc(-1); 						// nächsten freien Transfer-Complete-Code Pong
	configEDMAXmt.opt |= EDMA_FMK(OPT,TCC,tccXmtPing);     // dann der Grundkonfiguration des EDMA Sendekanal zuweisen

	/* ersten Transfer und Reload-Ping mit ConfigPing konfigurieren */
	EDMA_config(hEdmaXmt, &configEDMAXmt);
	EDMA_config(hEdmaXmtRelPing, &configEDMAXmt);

	/* braucht man auch noch andere EDMA-Konfigurationen fuer das Schreiben? ja -> pong */
	configEDMAXmt.opt &= 0xFFF0FFFF;
	configEDMAXmt.opt |= EDMA_FMK(OPT,TCC,tccXmtPong);
	configEDMAXmt.src = (Uint32)Buffer_out_pong ;
	EDMA_config(hEdmaXmtRelPong, &configEDMAXmt);

	/* Transfers verlinken ping -> pong -> ping */
	EDMA_link(hEdmaXmt, hEdmaXmtRelPong);  /* noch mehr verlinken? */
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


void config_interrupts(void)
{
	IRQ_map(IRQ_EVT_EDMAINT, 8);		// CHECK same settings in BIOS!!!
	IRQ_clear(IRQ_EVT_EDMAINT);
	IRQ_enable(IRQ_EVT_EDMAINT);

	SWI_enable();

	IRQ_globalEnable();
}


void EDMA_ISR(void)
{
	static volatile int rcvPingDone=0;	//static
	static volatile int rcvPongDone=0;
	static volatile int xmtPingDone=0;
	static volatile int xmtPongDone=0;

	// BSP Data Link Interface
	static volatile int xmtBSPLinkPingDone=0;
	static volatile int xmtBSPLinkPongDone=0;

	static volatile int rcvBSPLinkPingDone=0;
	static volatile int rcvBSPLinkPongDone=0;

	if(EDMA_intTest(tccRcvPing)) {
		EDMA_intClear(tccRcvPing); // clear is mandatory
		rcvPingDone=1;
	}
	else if(EDMA_intTest(tccRcvPong)) {
			EDMA_intClear(tccRcvPong);
			rcvPongDone=1;
	}
	
	if(EDMA_intTest(tccXmtPing)) {
			EDMA_intClear(tccXmtPing);
			xmtPingDone=1;
	}
	else if(EDMA_intTest(tccXmtPong)) {
			EDMA_intClear(tccXmtPong);
			xmtPongDone=1;
	}

	// BSP Data Link Interface
	// Transmit
	if(EDMA_intTest(tccBSPLinkXmtPing)) {
		EDMA_intClear(tccBSPLinkXmtPing);
		xmtBSPLinkPingDone = 1;
	}
	else if(EDMA_intTest(tccBSPLinkXmtPong)) {
		EDMA_intClear(tccBSPLinkXmtPong);
		xmtBSPLinkPongDone = 1;
	}

	// Receive
	if(EDMA_intTest(tccBSPLinkRcvPing)) {
		EDMA_intClear(tccBSPLinkRcvPing);
		rcvBSPLinkPingDone=1;
	}
	else if(EDMA_intTest(tccBSPLinkRcvPong)) {
		EDMA_intClear(tccBSPLinkRcvPong);
		rcvBSPLinkPongDone=1;
	}

	// Buffer Verarbeitung
if(xmtBSPLinkPingDone == 1 || xmtBSPLinkPongDone == 1)
{
	BSPLink_EDMA_Stop();
}


// Decoder
//BSPLink IN -> ADC Out

// BSPLink In lesen -> Ringbuffer Schreiben
	if( rcvBSPLinkPingDone) {
		rcvBSPLinkPingDone=0;
		SWI_post(&SWI_BSPLink_In_Ping);
	}
	else if(rcvBSPLinkPongDone) {
		rcvBSPLinkPongDone=0;
		SWI_post(&SWI_BSPLink_In_Pong);
	}

// Ringbuffer lesen -> ADC schreiben

	if( xmtPingDone) {
		xmtPingDone=0;
		SWI_post(&SWI_ADC_Out_Ping);
	}
	else if(xmtPongDone) {
		xmtPongDone=0;
		SWI_post(&SWI_ADC_Out_Pong);
	}



	// Encoder
	// Ringbuffer lesen -> BSP Link schreiben
	if(xmtBSPLinkPingDone ==1) {
		xmtBSPLinkPingDone = 2;

		DSK6713_LED_off(3);
		//BSPLink_EDMA_Start_Pong();
		SWI_post(&SWI_BSPLink_Out_Ping);
	}
	else if(xmtBSPLinkPongDone == 1) {
		xmtBSPLinkPongDone=2;

		//BSPLink_EDMA_Start_Ping();
		SWI_post(&SWI_BSPLink_Out_Pong);
	}

	if(xmtBSPLinkPingDone && rcvPingDone)
	{
		xmtBSPLinkPingDone=0;
		BSPLink_EDMA_Start_Pong();
	}
	else if(xmtBSPLinkPongDone && rcvPongDone)
	{
		xmtBSPLinkPongDone=0;

		DSK6713_LED_on(3);
		BSPLink_EDMA_Start_Ping();
	}

	// ADC lesen -> Ringbuffer schreiben
	if(rcvPingDone) {
		rcvPingDone=0;

		SWI_post(&SWI_ADC_In_Pong);
	}
	else if(rcvPongDone) {
		rcvPongDone=0;

		SWI_post(&SWI_ADC_In_Ping);
	}
}

/************************ SWI Section ****************************************/

// BSP Input RingBuffer schreiben
void BSPLink_In_Ping(void)
{
	write_ring_buffer_in(BSPLinkBuffer_in_ping);
}

void BSPLink_In_Pong(void)
{
	write_ring_buffer_in(BSPLinkBuffer_in_pong);
}

// BSP Input RingBuffer lesen
void ADC_Out_Ping(void)
{
	read_ring_buffer_in(Buffer_out_pong);
}

void ADC_Out_Pong(void)
{
	read_ring_buffer_in(Buffer_out_ping);
}

// BSP Output RingBuffer schreiben
void ADC_In_Ping(void)
{
	DSK6713_LED_on(1);
	write_ring_buffer_out(Buffer_in_pong);
	DSK6713_LED_off(1);
}

void ADC_In_Pong(void)
{
	DSK6713_LED_on(2);
	write_ring_buffer_out(Buffer_in_ping);
	DSK6713_LED_off(2);
}

// BSP Output RingBuffer lesen
void BSPLink_Out_Ping(void)
{
	read_ring_buffer_out(BSPLinkBuffer_out_ping);
}

void BSPLink_Out_Pong(void)
{
	read_ring_buffer_out(BSPLinkBuffer_out_pong);
}
/****************************************************************************/

void write_ring_buffer_in(short * buffersrc)
{
	// Buffer ablaufen, Daten verarbeiten
	Uint32 i;
		for(i=0; i<BUFFER_LEN; i++)
		{
			/*
			 *
			if(*(buffersrc+i) == MAGIC_NR)
			{
				// Element vor Magic-Nr löschen
				if(ringbuff_in_write_i)
					ringbuff_in_write_i--;
				else
					ringbuff_in_write_i = RINGBUFFER_LEN-1;

			}else
			*/
			*(Ringbuffer_in+ringbuff_in_write_i) = *(buffersrc+i);

			if(ringbuff_in_write_i < RINGBUFFER_LEN-1)
				ringbuff_in_write_i++;
				else
				ringbuff_in_write_i = 0;

			if(ringbuff_in_read_i == ringbuff_in_write_i)
			DSK6713_LED_off(0);
		}
}

void read_ring_buffer_in(short * bufferdes)
{
	// Buffer ablaufen, Daten verarbeiten
	int i;
		for(i=0; i<BUFFER_LEN; i++)
		{
			*(bufferdes+i) = *(Ringbuffer_in+ringbuff_in_read_i);

			if(ringbuff_in_read_i < RINGBUFFER_LEN-1)
				ringbuff_in_read_i++;
				else
				ringbuff_in_read_i = 0;

			if(ringbuff_in_read_i == ringbuff_in_write_i)
			DSK6713_LED_off(0);
		}
}

void write_ring_buffer_out(short * buffersrc)
{
	// Buffer ablaufen, Daten verarbeiten
	int i;
		for(i=0; i<BUFFER_LEN; i++)
		{
			//*(Ringbuffer_out+ringbuff_out_write_i) = *(buffersrc+i);

			Ringbuffer_out[ringbuff_out_write_i] = buffersrc[i];

			if(ringbuff_out_write_i < RINGBUFFER_LEN-1)
				ringbuff_out_write_i++;
				else
					ringbuff_out_write_i = 0;

			if(ringbuff_out_read_i == ringbuff_out_write_i)
			DSK6713_LED_off(0);
		}


//		if(mul >= 5)
//			mul = 0;
//		else
//			mul ++;
}

void read_ring_buffer_out(short * bufferdes)
{
	// Buffer ablaufen, Daten verarbeiten
	/*
	 * Buffer Index steht nach der Verarbeitung auf dem nächsten,
	 * noch nicht verarbeiteten Wert
	 */
	int i;
		for(i=0; i<BUFFER_LEN; i++)
		{
			//*(bufferdes+i) = *(Ringbuffer_out+ringbuff_out_read_i);

			bufferdes[i] = Ringbuffer_out[ringbuff_out_read_i];

			if(ringbuff_out_read_i < RINGBUFFER_LEN-1)
				ringbuff_out_read_i++;
				else
				ringbuff_out_read_i = 0;

			if(ringbuff_out_read_i == ringbuff_out_write_i)
				DSK6713_LED_off(0);

		}
}




/* Periodic Function */
void SWI_LEDToggle(void)
{
	SEM_postBinary(&SEM_LEDToggle);	
}

void tsk_led_toggle(void)
{

	/* initializatoin of the task */
	/* nothing to do */
	
	/* process */
	while(1) {
		SEM_pendBinary(&SEM_LEDToggle, SYS_FOREVER);

		if(configComplete)
				configComplete ++;

		if(configComplete >= 2)
		{
			MCBSP_close(hMcbsp_AIC23_Config);

			/* Set McBSP0 MUX to EXTERN */

			t_reg = DSK6713_rget(DSK6713_MISC);
			t_reg |= MCBSP1SEL;				// Set MCBSP0 to 1 (extern)
			DSK6713_rset(DSK6713_MISC,t_reg);

			/* configure BSPLink-Interface */
			config_BSPLink();
		    //config_interrupts();
			configComplete = 0;

		DSK6713_LED_on(0);
			//DSK6713_LED_on(1);
		}

//		if(time_cnt > 2)
//		{
//			time_cnt = 0;
//			DSK6713_LED_on(0);
//		}
//		else
//		{
//			time_cnt ++;
//			DSK6713_LED_off(0);
//		}


	}
}
