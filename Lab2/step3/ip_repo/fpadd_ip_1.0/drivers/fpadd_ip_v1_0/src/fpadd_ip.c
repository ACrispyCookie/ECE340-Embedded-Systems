

/***************************** Include Files *******************************/
#include "fpadd_ip.h"

/************************** Function Definitions ***************************/

void FPADD_IP_Trigger(void * baseaddr_p) {
  u32 baseaddr;
  baseaddr = (u32) baseaddr_p;
  
  FPADD_IP_mWriteReg (baseaddr, FPADD_IP_S00_AXI_SLV_REG0_OFFSET, 0x1);
  FPADD_IP_mWriteReg (baseaddr, FPADD_IP_S00_AXI_SLV_REG0_OFFSET, 0x0);
}

void FPADD_IP_SetA(void * baseaddr_p, u32 data) {  
  u32 baseaddr;
  baseaddr = (u32) baseaddr_p;
  
  FPADD_IP_mWriteReg (baseaddr, FPADD_IP_S00_AXI_SLV_REG1_OFFSET, data);
}

void FPADD_IP_SetB(void * baseaddr_p, u32 data) {  
  u32 baseaddr;
  baseaddr = (u32) baseaddr_p;
  
  FPADD_IP_mWriteReg (baseaddr, FPADD_IP_S00_AXI_SLV_REG2_OFFSET, data);
}
