/**************************************************/
/* Konfiguration des AIC23 auf DSK6713            */
/*                                                */
/* F. Quint, 2008                                 */
/**************************************************/

#include <csl.h>
#include <csl_mcbsp.h>
#include "config_AIC23.h"


MCBSP_Handle hMcbsp_AIC23_Config;
/***************************************************/
/* Konfiguration des AIC23 aus Online-Help des CCS */
/* Data Converter/Devices/TLV320AIC23              */
/***************************************************/
static unsigned short myAIC23_registers[10] = { \
	    0x0017, /* Set-Up Reg 0       Left line input channel volume control */  \
	            /* LRS     0          simultaneous left/right volume: disabled  */\
	            /* LIM     0          left line input mute: disabled */          \
	            /* XX      00         reserved */                                \
	            /* LIV     10111      left line input volume: 0 dB */            \
	                                                                             \
	    0x0017, /* Set-Up Reg 1       Right line input channel volume control */ \
	            /* RLS     0          simultaneous right/left volume: disabled  */\
	            /* RIM     0          right line input mute: disabled */         \
	            /* XX      00         reserved */                                \
	            /* RIV     10111      right line input volume: 0 dB */           \
	                                                                             \
		0x01f9, /* Set-Up Reg 2       Left channel headphone volume control */   \
	            /* LRS     0          simultaneous left/right volume: disabled */ \
	            /* LZC     1          left channel zero-cross detect: enabled */ \
	            /* LHV     1111001    left headphone volume: 0 dB */             \
	                                                                             \
	    0x01f9, /* Set-Up Reg 3       Right channel headphone volume control */  \
	            /* RLS     0          simultaneous right/left volume:disnabled */ \
	            /* RZC     1          right channel zero-cross detect: enabled */\
	            /* RHV     1111001 (f9)   right headphone volume: 0 dB */            \
	                                                                             \
	    0x0011, /* Set-Up Reg 4       Analog audio path control */               \
	            /* X       0          reserved */                                \
	            /* STA     00         sidetone attenuation: -6 dB */             \
	            /* STE     0          sidetone: disabled */                      \
	            /* DAC     1          DAC: selected */                           \
	            /* BYP     0          bypass: off */                             \
	            /* INSEL   0          input select for ADC: line */              \
	            /* MICM    0          microphone mute: disabled */               \
	            /* MICB    1          microphone boost: enabled */               \
	                                                                             \
	    0x0000, /* Set-Up Reg 5       Digital audio path control */              \
	            /* XXXXX   00000      reserved */                                \
	            /* DACM    0          DAC soft mute: disabled */                 \
	            /* DEEMP   00         deemphasis control: disabled */            \
	            /* ADCHP   0          ADC high-pass filter: disabled */          \
	                                                                             \
	    0x0000, /* Set-Up Reg 6       Power down control */                      \
	            /* X       0          reserved */                                \
	            /* OFF     0          device power: on (i.e. not off) */         \
	            /* CLK     0          clock: on */                               \
	            /* OSC     0          oscillator: on */                          \
	            /* OUT     0          outputs: on */                             \
	            /* DAC     0          DAC: on */                                 \
	            /* ADC     0          ADC: on */                                 \
	            /* MIC     0          microphone: on */                          \
	            /* LINE    0          line input: on */                          \
	                                                                             \
	    0x0043, /* Set-Up Reg 7       Digital audio interface format */          \
	            /* XX      00         reserved */                                \
	            /* MS      1          master/slave mode: master */               \
	            /* LRSWAP  0          DAC left/right swap: disabled */           \
	            /* LRP     0          DAC lrp: MSB on 1st BCLK */                \
	            /* IWL     00         input bit length: 16 bit */                \
	            /* FOR     11         USB-Mode */                 \
	                                                                           \
	    0x0001, /* Set-Up Reg 8       Sample rate control */                     \
	            /* X       0          reserved */                                \
	            /* CLKOUT  0          clock output divider: 2 (MCLK/2) */        \
	            /* CLKIN   0          clock input divider: 2 (MCLK/2) */         \
	            /* SR,BOSR 00000      sampling rate: ADC 48 kHz, DAC 48 kHz in USB mode*/  \
	            /* USB/N   1          clock mode select (USB/normal): USB */     \
	                                                                             \
	    0x0001  /* Set-Up Reg 9       Digital interface activation */            \
	            /* XX..X   00000000   reserved */                                \
	            /* ACT     1          not reseted */                                  \
	};
//Konfig Struktur McBSP0
static MCBSP_Config mcbspforAIC23Cfg = {
        /* McBSP Control Register */
		MCBSP_FMKS(SPCR, FREE, NO)              |	// Deaktivierter Freilauf
        MCBSP_FMKS(SPCR, SOFT, NO)              |	// Clock stoppt beim debuggen (sofort)
        MCBSP_FMKS(SPCR, FRST, YES)             |	// Frame-Sync Signal erzeugen da MASTER
        MCBSP_FMKS(SPCR, GRST, YES)             |	// Reset aus, damit läuft der Samplerate- Generator
        MCBSP_FMKS(SPCR, XINTM, XRDY)           |	// Sender Interrupt wird durch "XRDY-Bit" ausgelöst
        MCBSP_FMKS(SPCR, XSYNCERR, NO)          |	// keine Überwachung der Synchronisation
        MCBSP_FMKS(SPCR, XRST, NO)              |	// Reset des SPI, löscht Error-Register
        MCBSP_FMKS(SPCR, DLB, OFF)              |	// Loopback  nicht aktiv
        MCBSP_FMKS(SPCR, RJUST, RZF)            |	// Right justify and zero fill
        MCBSP_FMKS(SPCR, CLKSTP, NODELAY)       |	// Clock startet ohne Verzögerung auf fallenden Flanke (siehe auch PCR-Register)
        MCBSP_FMKS(SPCR, DXENA, OFF)            |	// DX- Enabler wird nicht verwendet
        MCBSP_FMKS(SPCR, RINTM, RRDY)           |	// Sender Interrupt wird durch "RRDY-Bit" ausgelöst
        MCBSP_FMKS(SPCR, RSYNCERR, NO)          |	//  keine Überwachung der Synchronisation
        MCBSP_FMKS(SPCR, RRST, YES),			// Empfänger läuft (kein Reset- Status)
		/* Empfangs-Control Register */
        MCBSP_FMKS(RCR, RPHASE, DEFAULT)        |	// Nur eine Phase pro Frame
        MCBSP_FMKS(RCR, RFRLEN2, DEFAULT)       |	// Länge in Phase 2, unrelevant
        MCBSP_FMKS(RCR, RWDLEN2, DEFAULT)       |	// Wortlänge in Phase 2, unrelevant
        MCBSP_FMKS(RCR, RCOMPAND, DEFAULT)      |	// kein Compandierung der Daten 
        MCBSP_FMKS(RCR, RFIG, DEFAULT)          |	// Rahmensynchronisationspulse werden nicht ignoriert
        MCBSP_FMKS(RCR, RDATDLY, DEFAULT)       |	// keine Verzögerung (delay) der Daten
        MCBSP_FMKS(RCR, RFRLEN1, DEFAULT)       |	// Länge der Phase 1, unrelevant
        MCBSP_FMKS(RCR, RWDLEN1, DEFAULT)       |	// Wortlänge in Phase 1, unrelevant
        MCBSP_FMKS(RCR, RWDREVRS, DEFAULT),		// 32-bit Reversal nicht genutzt
		/* Sende-Control Register */
        MCBSP_FMKS(XCR, XPHASE, SINGLE)         |	// Nur eine Phase pro Frame
        MCBSP_FMKS(XCR, XFRLEN2, OF(0))         |	// Länge in Phase 2, unrelevant
        MCBSP_FMKS(XCR, XWDLEN2, 8BIT)          |	// Wortlänge in Phase 2, unrelevant
        MCBSP_FMKS(XCR, XCOMPAND, MSB)          |	// kein Compandierung der Daten 
        MCBSP_FMKS(XCR, XFIG, NO)               |	// Rahmensynchronisationspulse werden nicht ignoriert
        MCBSP_FMKS(XCR, XDATDLY, 1BIT)          |	// 1 bit Verzögerung (delay) der Daten
        MCBSP_FMKS(XCR, XFRLEN1, OF(0))         |	// Länge der Phase 1 --> 1 Wort
        MCBSP_FMKS(XCR, XWDLEN1, 16BIT)         |	// Wortlänge in Phase 1 --> 16 bit
        MCBSP_FMKS(XCR, XWDREVRS, DISABLE),		// 32-bit Reversal nicht genutzt
		/* Sample Rate Generator Register */
        MCBSP_FMKS(SRGR, GSYNC, FREE)           |	// Samplerate-Clock läuft frei
        MCBSP_FMKS(SRGR, CLKSP, RISING)         |	// nicht relevant da interner Clock verwendet wird
        MCBSP_FMKS(SRGR, CLKSM, INTERNAL)       |	// Samplerate-Clock wird vom CPU-Clock abgeleitet
        MCBSP_FMKS(SRGR, FSGM, DXR2XSR)         |	// Framesync- Signal bei jedem DXR zu XSR Kopiervorgang (setzt FPER und FWID ausser Kraft)
        MCBSP_FMKS(SRGR, FPER, OF(0))           |	// s.o
        MCBSP_FMKS(SRGR, FWID, OF(19))          |	// s.o
        MCBSP_FMKS(SRGR, CLKGDV, OF(99)),		// Teilerwert für die CPU-Clock
		/* Mehrkanal */
        MCBSP_MCR_DEFAULT,				// Mehrkanal wird nicht verwendet		
        MCBSP_RCER_DEFAULT,				// dito
        MCBSP_XCER_DEFAULT,				// dito
		/* Pinout Control Register */
        MCBSP_FMKS(PCR, XIOEN, SP)              |	// Pin wird für serielle Schnittstelle verwendet (alternativ GPIO)
        MCBSP_FMKS(PCR, RIOEN, SP)              |	// Pin wird für serielle Schnittstelle verwendet (alternativ GPIO)
        MCBSP_FMKS(PCR, FSXM, INTERNAL)         |	// Signal wird vom SRG abgeleitet
        MCBSP_FMKS(PCR, FSRM, EXTERNAL)         |	// Empfänger ist unrelevant
        MCBSP_FMKS(PCR, CLKXM, OUTPUT)          |	// McBSP0 gibt Takt vor (Master)
        MCBSP_FMKS(PCR, CLKRM, INPUT)           |	// unrelevant da nur gesendet wird
        MCBSP_FMKS(PCR, CLKSSTAT, DEFAULT)      |	// unrelevant da PINS keine GPIOs
        MCBSP_FMKS(PCR, DXSTAT, DEFAULT)        |	// unrelevant da PINS keine GPIOs
        MCBSP_FMKS(PCR, FSXP, ACTIVELOW)        |	// Framesync ist "activelow"
        MCBSP_FMKS(PCR, FSRP, DEFAULT)          |	// unrelevant da nur gesendet wird
        MCBSP_FMKS(PCR, CLKXP, FALLING)         |	// Daten werden bei fallender Flanke gesendet 
        MCBSP_FMKS(PCR, CLKRP, DEFAULT)			// unrelevant da nur gesendet wird
};


/************************************************************/
/* Konfguration des AIC23 entsprechend der obigen PArameter */
/* zum einfachen Aufruf werden diese über die globale       */
/* static-Variablen übergeben                               */
/* es wird erwartet, dass CSL_init schon vorher aufgerufen  */
/* wurde                                                    */
/************************************************************/

void Config_DSK6713_AIC23(void)
{
	/* Handle für Steuerkanal */
	unsigned short i;

	hMcbsp_AIC23_Config = MCBSP_open(MCBSP_DEV0, MCBSP_OPEN_RESET);
    MCBSP_config(hMcbsp_AIC23_Config, &mcbspforAIC23Cfg);
    MCBSP_start(hMcbsp_AIC23_Config,  MCBSP_XMIT_START | MCBSP_SRGR_START | MCBSP_SRGR_FRAMESYNC , 220);
  
	/* jetzt alle Register konfigurieren */
	/* ein Reset am Anfang ist immer gut, Register 15 --> 0 */
	/* die 0 haben wir uns gespart, nach AIC23_registers aufzunehmen */
    set_aic23_register(hMcbsp_AIC23_Config,15,0);

    /* power-down Register zuerst */
    set_aic23_register(hMcbsp_AIC23_Config,6,myAIC23_registers[6]);

    /* jetzt die anderen */
    for (i = 0; i < 6; i++)
      set_aic23_register(hMcbsp_AIC23_Config,i,myAIC23_registers[i]);
    for (i = 7; i < 10; i++)
      set_aic23_register(hMcbsp_AIC23_Config,i,myAIC23_registers[i]);

	 /* fertig, aber MCBSP nicht schließen, sonst kein Takt an Codec!*/
}


static void set_aic23_register(MCBSP_Handle hMcbsp,unsigned short regnum, unsigned short regval)
{
	/* Programmierung erfolgt so, dass in B[15:9] die Registernummer steht und
		in B[8:0] die zu schreibenden Daten */

    /* zur Sicherheit maskieren auf 9 Bit */
    regval &= 0x1ff;
    
    /* warten */
    while (!MCBSP_xrdy(hMcbsp));
    
    /* schreiben */
   /*The control word is divided into two parts. The first part is the address block, the second part is the data block:
		B[15:9] Control Address Bits
		B[8:0] Control Data Bits
    */
    MCBSP_write(hMcbsp, (regnum << 9) | regval);
}
