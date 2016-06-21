/*
 * config_BSPLink.h
 *
 *  Created on: 30.05.2016
 */

#ifndef CONFIG_BSPLINK_H_
#define CONFIG_BSPLINK_H_

/* Function Prototypes */
void config_BSPLink_EDMA(void);
void config_BSPLink(Uint32 encoder, Uint32 decoder);

void BSPLink_EDMA_Start_Ping();
void BSPLink_EDMA_Start_Pong();
void BSPLink_EDMA_Stop();

#define EDMABSPLINK_CH_REVT				EDMA_CHA_REVT0
#define EDMABSPLINK_CH_XEVT				EDMA_CHA_XEVT0

/* McBSP-Handler for bi-direction DSP-Board Link */
#define MCBSP_LINK_DEV					MCBSP_DEV0

/* Transfer-Complete-Codes for EDMA-Jobs */
extern int tccBSPLinkRcvPing;
extern int tccBSPLinkRcvPong;
extern int tccBSPLinkXmtPing;
extern int tccBSPLinkXmtPong;

#define LINK_BUFFER_LEN 550

extern short BSPLinkBuffer_in_ping[LINK_BUFFER_LEN];
extern short BSPLinkBuffer_in_pong[LINK_BUFFER_LEN];
extern short BSPLinkBuffer_out_ping[LINK_BUFFER_LEN];
extern short BSPLinkBuffer_out_pong[LINK_BUFFER_LEN];

#endif /* CONFIG_BSPLINK_H_ */
