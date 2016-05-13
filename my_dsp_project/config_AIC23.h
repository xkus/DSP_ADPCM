/*********************************/
/* config_AIC23.h                */
/* Interface zu config_AIC23.c   */
/*   F. Quint, 2008              */
/*********************************/

void Config_DSK6713_AIC23(void);
void set_aic23_register(MCBSP_Handle hMcbsp,unsigned short regnum, unsigned short regval);