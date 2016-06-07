/*
 * config_BSPLink.h
 *
 *  Created on: 30.05.2016
 *      Author: Simon
 */

#ifndef CONFIG_BSPLINK_H_
#define CONFIG_BSPLINK_H_

/* Function Prototypes */
void config_BSPLink_EDMA_XMT(void);
void config_BSPLink_EDMA_RCV(void);

void config_BSPLink();

void BSPLink_EDMA_Send_Ping();
void BSPLink_EDMA_Send_Pong();

#define LINK_BUFFER_LEN 5000  // !!! Usually adapted with BUFFER_LEN in Skeleton.c

#define EDMABSPLINK_CH_REVT				EDMA_CHA_REVT0
#define EDMABSPLINK_CH_XEVT				EDMA_CHA_XEVT0

/* McBSP-Handler for bi-direction DSP-Board Link */
#define MCBSP_LINK_DEV					MCBSP_DEV0

/* Transfer-Complete-Codes for EDMA-Jobs */
extern int tccBSPLinkRcvPing;
extern int tccBSPLinkRcvPong;
extern int tccBSPLinkXmtPing;
extern int tccBSPLinkXmtPong;


//#pragma DATA_SECTION(BSPLinkBuffer_in_ping, ".datenpuffer");
extern short BSPLinkBuffer_in_ping[LINK_BUFFER_LEN];
//#pragma DATA_SECTION(BSPLinkBuffer_in_pong, ".datenpuffer");
extern short BSPLinkBuffer_in_pong[LINK_BUFFER_LEN];
//#pragma DATA_SECTION(BSPLinkBuffer_out_ping, ".datenpuffer");
extern short BSPLinkBuffer_out_ping[LINK_BUFFER_LEN];
//#pragma DATA_SECTION(BSPLinkBuffer_out_pong, ".datenpuffer");
extern short BSPLinkBuffer_out_pong[LINK_BUFFER_LEN];
//#pragma DATA_SECTION(BSPLinkBuffer_out_pong, ".datenpuffer");
extern short BSPLinkBuffer_Send[LINK_BUFFER_LEN];

/* Global Handles */
extern MCBSP_Handle hMcbsp_Link;

#endif /* CONFIG_BSPLINK_H_ */
