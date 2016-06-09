/*********************************/
/* config_AIC23.h                */
/* Interface zu config_AIC23.c   */
/*   F. Quint, 2008              */
/*********************************/
#ifndef CONFIG_AIC23_H_
#define CONFIG_AIC23_H_

void Config_DSK6713_AIC23(void);
void close_AIC23_ConfMcBSP();
static void set_aic23_register(MCBSP_Handle hMcbsp,unsigned short regnum, unsigned short regval);

/* Global Vars defined in config_AIC23.c */
extern MCBSP_Handle hMcbsp_AIC23_Config;

#endif
