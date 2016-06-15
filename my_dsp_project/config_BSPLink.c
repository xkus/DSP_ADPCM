/*
 * config_BSPLink.c
 *
 *  Created on: 30.05.2016
 *      Author: Simon
 */

#include <csl.h>
#include <csl_mcbsp.h>
#include <csl_irq.h>
#include <csl_edma.h>
#include <dsk6713.h>
#include <dsk6713_led.h>
#include "config_BSPLink.h"

/* Ping-Pong buffers. Place them in the compiler section .datenpuffer */
/* How do you place the compiler section in the memory?     */
#pragma DATA_SECTION(BSPLinkBuffer_in_ping, ".datenpuffer");
short BSPLinkBuffer_in_ping[LINK_BUFFER_LEN];
#pragma DATA_SECTION(BSPLinkBuffer_in_pong, ".datenpuffer");
short BSPLinkBuffer_in_pong[LINK_BUFFER_LEN];
#pragma DATA_SECTION(BSPLinkBuffer_out_ping, ".datenpuffer");
short BSPLinkBuffer_out_ping[LINK_BUFFER_LEN];
#pragma DATA_SECTION(BSPLinkBuffer_out_pong, ".datenpuffer");
short BSPLinkBuffer_out_pong[LINK_BUFFER_LEN];

MCBSP_Handle hMcbsp_Link = 0;

/* EDMA-Handles */
EDMA_Handle hEdmaBSPLinkRcv; /* Empfang handle auf EDMA REVT1-Channel */
EDMA_Handle hEdmaBSPLinkRcvRelPing; /* Empfang handle auf einen reload-Parametersatz */
EDMA_Handle hEdmaBSPLinkRcvRelPong;
/* braucht man noch mehr? ja -> pong */
EDMA_Handle hEdmaBSPLinkXmt; /* handle auf EDMA XEVT1-Channel */
EDMA_Handle hEdmaBSPLinkXmtRelPing;
EDMA_Handle hEdmaBSPLinkXmtRelPong;
/* Transfer-Complete-Codes for EDMA-Jobs */

int tccBSPLinkRcvPing;
int tccBSPLinkRcvPong;
int tccBSPLinkXmtPing;
int tccBSPLinkXmtPong;

//Configuration for McBSP (data-interface)
MCBSP_Config BSPLink_interface_config = {
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
				MCBSP_FMKS(RCR, RFRLEN2, DEFAULT) |// Länge in Phase 2, unrelevant
				MCBSP_FMKS(RCR, RWDLEN2, DEFAULT) |// Wortlänge in Phase 2, unrelevant
				MCBSP_FMKS(RCR, RCOMPAND, MSB) |// kein Compandierung der Daten (MSB first)
				MCBSP_FMKS(RCR, RFIG, NO) |// Rahmensynchronisationspulse (nach dem ersten Puls)) startet die Übertragung neu
				MCBSP_FMKS(RCR, RDATDLY, 0BIT) |// keine Verzögerung (delay) der Daten
				MCBSP_FMKS(RCR, RFRLEN1, OF(1)) |// Länge der Phase 1 --> 1 Wort
				MCBSP_FMKS(RCR, RWDLEN1, 16BIT) |	//
				MCBSP_FMKS(RCR, RWDREVRS, DISABLE),	// 32-bit Reversal nicht genutzt
		/* Sende-Control Register */
		MCBSP_FMKS(XCR, XPHASE, SINGLE) |	// Sende nur eine Phase pro Frame
				MCBSP_FMKS(XCR, XFRLEN2, DEFAULT) |// Länge in Phase 2, unrelevant
				MCBSP_FMKS(XCR, XWDLEN2, DEFAULT) |// Wortlänge in Phase 2, unrelevant
				MCBSP_FMKS(XCR, XCOMPAND, MSB) |// kein Compandierung der Daten (MSB first)
				MCBSP_FMKS(XCR, XFIG, NO) |// Rahmensynchronisationspulse (nach dem ersten Puls)) startet die Übertragung neu
				MCBSP_FMKS(XCR, XDATDLY, 0BIT) |// keine Verzögerung (delay) der Daten
				MCBSP_FMKS(XCR, XFRLEN1, OF(1)) |// Länge der Phase 1 --> 1 Wort
				MCBSP_FMKS(XCR, XWDLEN1, 16BIT) |// Wortlänge in Phase 1 --> 16 bit
				MCBSP_FMKS(XCR, XWDREVRS, DISABLE),	// 32-bit Reversal nicht genutzt

		/* Sample Rate Generator Register */
		MCBSP_FMKS(SRGR, GSYNC, FREE) |	// Samplerate-Clock läuft frei
				MCBSP_FMKS(SRGR, CLKSP, DEFAULT) |	// ""		Unrelevant
				MCBSP_FMKS(SRGR, CLKSM, INTERNAL) |// Samplerate-Clock wird vom CPU-Clock abgeleitet
				MCBSP_FMKS(SRGR, FSGM, DXR2XSR) |// Framesync- Signal bei jedem DXR zu XSR Kopiervorgang (setzt FPER und FWID ausser Kraft)
				MCBSP_FMKS(SRGR, FPER, OF(15)) |	//
				MCBSP_FMKS(SRGR, FWID, OF(0)) |	//
				MCBSP_FMKS(SRGR, CLKGDV, OF(70)),// CPU-Clock Teiler -> 225 MHz / 2*100= 1,125MHz
		/* Mehrkanal */
		MCBSP_MCR_DEFAULT,				// Mehrkanal wird nicht verwendet
		MCBSP_RCER_DEFAULT,				// dito
		MCBSP_XCER_DEFAULT,				// dito
		/* Pinout Control Register */
		MCBSP_FMKS(PCR, XIOEN, SP) |// Pin wird für serielle Schnittstelle verwendet (alternativ GPIO)
				MCBSP_FMKS(PCR, RIOEN, SP) |// Pin wird für serielle Schnittstelle verwendet (alternativ GPIO)
				MCBSP_FMKS(PCR, FSXM, INTERNAL) |// Framesync- Signal für Sender generieren (MASTER)
				MCBSP_FMKS(PCR, FSRM, EXTERNAL) |// Framesync- Signal für Empfänger kommt von extern (Slave)
				MCBSP_FMKS(PCR, CLKXM, OUTPUT) |// Takt für Sender generieren (MASTER)
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
EDMA_Config configEDMABSPLinkRcv = {
EDMA_FMKS(OPT, PRI, LOW) |  // auf beide Queues verteilen
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
				EDMA_FMK(CNT, ELECNT, LINK_BUFFER_LEN),   // Anzahl Elemente

		(Uint32) BSPLinkBuffer_in_ping, // Ziel-Adresse ( initialsierung auf ping )

		EDMA_FMKS(IDX, FRMIDX, DEFAULT) |  // Frame index Wert
				EDMA_FMKS(IDX, ELEIDX, DEFAULT),      // Element index Wert

		EDMA_FMK (RLD, ELERLD, NULL) |  // Reload Element
				EDMA_FMK(RLD, LINK, NULL)            // Reload Link
};

/* Senden Ping*/
EDMA_Config configEDMABSPLinkXmt = {
EDMA_FMKS(OPT, PRI, LOW) |  // auf beide Queues verteilen
		EDMA_FMKS(OPT, ESIZE, 16BIT) |  // Element size
		EDMA_FMKS(OPT, 2DS, NO) |  // kein 2D-Transfer
		EDMA_FMKS(OPT, SUM, INC) | // Quell-update mode -> inkrementieren (ping/pong out buffer)
		EDMA_FMKS(OPT, 2DD, NO) |  // 2kein 2D-Transfer
		EDMA_FMKS(OPT, DUM, NONE) |  // Ziel-update mode -> FEST (McBSP)!!!
		EDMA_FMKS(OPT, TCINT, YES) |  // EDMA interrupt erzeugen?
		EDMA_FMKS(OPT, TCC, OF(0)) |  // Transfer complete code (TCC)
		EDMA_FMKS(OPT, LINK, YES) |  // Link Parameter nutzen?
		EDMA_FMKS(OPT, FS, NO),               // Frame Sync nutzen?

		(Uint32) BSPLinkBuffer_out_ping,// Quell-Adresse (initialisierung auf ping)

		EDMA_FMK(CNT, FRMCNT, NULL) |  // Anzahl Frames
				EDMA_FMK(CNT, ELECNT, LINK_BUFFER_LEN),   // Anzahl Elemente

		EDMA_FMKS(DST, DST, OF(0)),       	  // Ziel-Adresse

		EDMA_FMKS(IDX, FRMIDX, DEFAULT) |  // Frame index Wert
				EDMA_FMKS(IDX, ELEIDX, DEFAULT),      // Element index Wert

		EDMA_FMK (RLD, ELERLD, NULL) |  // Reload Element
				EDMA_FMK(RLD, LINK, NULL)            // Reload Link
};


void config_BSPLink() {
	/* Configure Link McBSP*/

	hMcbsp_Link = MCBSP_open(MCBSP_LINK_DEV, MCBSP_OPEN_RESET);
	if (hMcbsp_Link == INV) {
		DSK6713_LED_on(2);
	}
	MCBSP_config(hMcbsp_Link, &BSPLink_interface_config);

	/* configure EDMA */
	config_BSPLink_EDMA();

	MCBSP_start(hMcbsp_Link, MCBSP_XMIT_START | MCBSP_RCV_START | MCBSP_SRGR_START | MCBSP_SRGR_FRAMESYNC, 220);// Start Data Link IN & OUT transmision
	MCBSP_write(hMcbsp_Link, 0x0); 	/* one shot */
}

void config_BSPLink_EDMA(void) {
	/* Konfiguration der EDMA zum Lesen*/
	hEdmaBSPLinkRcv = EDMA_open(EDMABSPLINK_CH_REVT, EDMA_OPEN_RESET); // EDMA Kanal für das Event McBSP
	hEdmaBSPLinkRcvRelPing = EDMA_allocTable(-1); // einen Reload-Parametersatz für Ping
	hEdmaBSPLinkRcvRelPong = EDMA_allocTable(-1); // einen Reload-Parametersatz für Pong

	configEDMABSPLinkRcv.src = MCBSP_getRcvAddr(hMcbsp_Link); //  Quell-Adresse zum Lesen

	tccBSPLinkRcvPing = EDMA_intAlloc(-1); // nächsten freien Transfer-Complete-Code Ping
	tccBSPLinkRcvPong = EDMA_intAlloc(-1); // nächsten freien Transfer-Complete-Code Pong
	configEDMABSPLinkRcv.opt |= EDMA_FMK(OPT, TCC, tccBSPLinkRcvPing); // dann der Grundkonfiguration des EDMA Empfangskanals zuweisen

	/* ersten Transfer und Reload-Ping mit ConfigPing konfigurieren */
	EDMA_config(hEdmaBSPLinkRcv, &configEDMABSPLinkRcv);
	EDMA_config(hEdmaBSPLinkRcvRelPing, &configEDMABSPLinkRcv);

	/* braucht man auch noch andere EDMA-Konfigurationen fuer das Lesen? ja -> pong */
	configEDMABSPLinkRcv.opt &= 0xFFF0FFFF;
	configEDMABSPLinkRcv.opt |= EDMA_FMK(OPT, TCC, tccBSPLinkRcvPong);
	configEDMABSPLinkRcv.dst = (Uint32) BSPLinkBuffer_in_pong;
	EDMA_config(hEdmaBSPLinkRcvRelPong, &configEDMABSPLinkRcv);

	/* Transfers verlinken ping -> pong -> ping */
	EDMA_link(hEdmaBSPLinkRcv, hEdmaBSPLinkRcvRelPong); /* noch mehr verlinken? */
	EDMA_link(hEdmaBSPLinkRcvRelPong, hEdmaBSPLinkRcvRelPing);
	EDMA_link(hEdmaBSPLinkRcvRelPing, hEdmaBSPLinkRcvRelPong);

	/* muss man mittels EDMA auch schreiben? */
	/* Konfiguration der EDMA zum Schreiben */
	hEdmaBSPLinkXmt = EDMA_open(EDMABSPLINK_CH_XEVT, EDMA_OPEN_RESET); // EDMA Kanal für das Event REVT1
	hEdmaBSPLinkXmtRelPing = EDMA_allocTable(-1);               // einen Reload-Parametersatz für Ping
	hEdmaBSPLinkXmtRelPong = EDMA_allocTable(-1);               // einen Reload-Parametersatz für Pong

	configEDMABSPLinkXmt.dst = MCBSP_getXmtAddr(hMcbsp_Link);//  Ziel-Adresse zum Schreiben

	tccBSPLinkXmtPing = EDMA_intAlloc(-1); // nächsten freien Transfer-Complete-Code Ping
	tccBSPLinkXmtPong = EDMA_intAlloc(-1); // nächsten freien Transfer-Complete-Code Pong
	configEDMABSPLinkXmt.opt |= EDMA_FMK(OPT, TCC, tccBSPLinkXmtPing); // dann der Grundkonfiguration des EDMA Sendekanal zuweisen

	/* ersten Transfer und Reload-Ping mit ConfigPing konfigurieren */
	EDMA_config(hEdmaBSPLinkXmt, &configEDMABSPLinkXmt);
	EDMA_config(hEdmaBSPLinkXmtRelPing, &configEDMABSPLinkXmt);

	/* braucht man auch noch andere EDMA-Konfigurationen fuer das Schreiben? ja -> pong */
	configEDMABSPLinkXmt.opt &= 0xFFF0FFFF;
	configEDMABSPLinkXmt.opt |= EDMA_FMK(OPT,TCC,tccBSPLinkXmtPong);
	configEDMABSPLinkXmt.src = (Uint32)BSPLinkBuffer_out_pong ;
	EDMA_config(hEdmaBSPLinkXmtRelPong, &configEDMABSPLinkXmt);

	/* Transfers verlinken ping -> pong -> ping */
	EDMA_link(hEdmaBSPLinkXmt, hEdmaBSPLinkXmtRelPong);  /* noch mehr verlinken? */
	EDMA_link(hEdmaBSPLinkXmtRelPong, hEdmaBSPLinkXmtRelPing);
	EDMA_link(hEdmaBSPLinkXmtRelPing, hEdmaBSPLinkXmtRelPong);

	/* EDMA TCC-Interrupts freigeben */
	EDMA_intClear(tccBSPLinkRcvPing);
	EDMA_intEnable(tccBSPLinkRcvPing);
	EDMA_intClear(tccBSPLinkRcvPong);
	EDMA_intEnable(tccBSPLinkRcvPong);
	/* sind das alle? nein -> pong und alle für Sendeseite */
	EDMA_intClear(tccBSPLinkXmtPing);
	EDMA_intEnable(tccBSPLinkXmtPing);
	EDMA_intClear(tccBSPLinkXmtPong);
	EDMA_intEnable(tccBSPLinkXmtPong);

	/* EDMA starten, wen alles? */
	EDMA_enableChannel(hEdmaBSPLinkRcv);
	EDMA_enableChannel(hEdmaBSPLinkXmt);
}
