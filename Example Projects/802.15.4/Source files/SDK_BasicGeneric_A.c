/**
* @file    SDK_BasicGeneric_A.c
* @author  STMicroelectronics
* @version 1.3.0
* @date    July, 2019
* @brief   Example of transmission of S2-LP Basic packets.
* @details
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOURCE CODE IS PROTECTED BY A LICENSE.
* FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
* IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
*
* <h2><center>&copy; COPYRIGHT 2019 STMicroelectronics</center></h2>
*/


/* Includes ------------------------------------------------------------------*/
#include "SDK_EVAL_Config.h"
#include "S2LP_Config.h"
#include "SDK_Configuration_Common.h"
#include "st_main.h"

#define USE_VCOM

/**
* @addtogroup SDK_Examples SDK Examples
* @{
*/


/**
* @addtogroup SDK_Basic_Generic        SDK Basic Generic
* @{
*/

/**
* @addtogroup SDK_Basic_Generic_A                                      SDK Basic Generic A
* @brief Device A configured as a transmitter.
* @details This code explains how to configure and manage
* in the simpler way a transmitter of basic packets.
*
* The user can change the Basic packet configuration parameters editing the defines at the beginning of the file.
* @{
*/


/**
* @defgroup Basic_Generic_A_Private_Variables                                  Basic Generic A Private Variables
* @{
*/

/**
* @brief Radio structure fitting
*/
SRadioInit xRadioInit = {
  BASE_FREQUENCY,
  MODULATION_SELECT,
  DATARATE,
  FREQ_DEVIATION,
  BANDWIDTH
};


/**
* @brief Packet Basic structure fitting
*/
PktBasicInit xBasicInit={
  PREAMBLE_LENGTH,
  SYNC_LENGTH,
  SYNC_WORD,
  VARIABLE_LENGTH,
  EXTENDED_LENGTH_FIELD,
  CRC_MODE,
  EN_ADDRESS,
  EN_FEC,
  EN_WHITENING
};


/**
* @brief GPIO structure fitting
*/
SGpioInit xGpioIRQ={
  S2LP_GPIO_3,
  S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP,
  S2LP_GPIO_DIG_OUT_IRQ
};


/**
* @brief Declare the Tx done flag
*/
volatile FlagStatus xTxDoneFlag = RESET;


/**
* @brief IRQ status struct declaration
*/
S2LPIrqs xIrqStatus;


/**
* @brief Tx buffer declaration: data to transmit
*/
uint8_t vectcTxBuff[20]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

/**
* @brief Preemption priority IRQ
*/
#define IRQ_PREEMPTION_PRIORITY         0x03

/**
*@}
*/


/**
* @defgroup Basic_Generic_A_Private_Functions                                                  Basic Generic A Private Functions
* @{
*/

/**
* @brief  This function handles External interrupt request (associated with S2LP GPIO 3).
* @param  None
* @retval None
*/

static uint32_t M2S_GPIO_PIN_IRQ;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin==M2S_GPIO_PIN_IRQ)
  {
    /* Get the IRQ status */
    S2LPGpioIrqGetStatus(&xIrqStatus);

    /* Check the SPIRIT TX_DATA_SENT IRQ flag */
    if(xIrqStatus.IRQ_TX_DATA_SENT)
    {
      /* set the tx_done_flag to manage the event in the main() */
      xTxDoneFlag = SET;

      /* toggle LED2 */
	SdkEvalLedToggle(LED2);
    }
  }
}

/* 
1. FEC Enable
2. Data Whitening Enable
3. preamble-->0x55
4. Synchronization words-->0x6F4E0000
5. 802.15.4g format
6. Secondary Synchronization words-->0xAABB0000
7. CRC Mode-->3
8. FCS type-->1
9. RF settings 
   -- Frequency base      :868Mhz
   -- Data Rate           :38.4ksps
   -- Frequency deviation :20Khz
   -- Channel filter      :100Khz

Note: the total packet length is equal to MHR+MAC Payload + CRC
 */
void SpiritBaseConfiguration1(void)
{
  uint8_t tmp[5];

  tmp[0] = 0xA3; /* reg. GPIO3_CONF (0x03) */
  S2LPSpiWriteRegisters(0x03, 1, tmp);
  tmp[0] = 0x62; /* reg. SYNT3 (0x05) */
  tmp[1] = 0x2B; /* reg. SYNT2 (0x06) */
  tmp[2] = 0x84; /* reg. SYNT1 (0x07) */
  S2LPSpiWriteRegisters(0x05, 3, tmp);
  tmp[0] = 0x2F; /* reg. IF_OFFSET_ANA (0x09) */
  tmp[1] = 0xC2; /* reg. IF_OFFSET_DIG (0x0A) */
  S2LPSpiWriteRegisters(0x09, 2, tmp);
  tmp[0] = 0x92; /* reg. MOD4 (0x0E) */
  tmp[1] = 0xA7; /* reg. MOD3 (0x0F) */
  tmp[2] = 0x27; /* reg. MOD2 (0x10) */
  S2LPSpiWriteRegisters(0x0E, 3, tmp);
  tmp[0] = 0xA3; /* reg. MOD0 (0x12) */
  tmp[1] = 0x13; /* reg. CHFLT (0x13) */
  S2LPSpiWriteRegisters(0x12, 2, tmp);
  tmp[0] = 0x55; /* reg. ANT_SELECT_CONF (0x1F) */
  S2LPSpiWriteRegisters(0x1F, 1, tmp);
  tmp[0] = 0x40; /* reg. PCKTCTRL6 (0x2B) */
  tmp[1] = 0x04; /* reg. PCKTCTRL5 (0x2C) */
  S2LPSpiWriteRegisters(0x2B, 2, tmp);
  tmp[0] = 0x40; /* reg. PCKTCTRL3 (0x2E) */
  tmp[1] = 0x21; /* reg. PCKTCTRL2 (0x2F) */
  tmp[2] = 0x73; /* reg. PCKTCTRL1 (0x30) */
  S2LPSpiWriteRegisters(0x2E, 3, tmp);
  tmp[0] = 0x00; /* reg. SYNC3 (0x33) */
  tmp[1] = 0x00; /* reg. SYNC2 (0x34) */
  tmp[2] = 0x4E; /* reg. SYNC1 (0x35) */
  tmp[3] = 0x6F; /* reg. SYNC0 (0x36) */
  S2LPSpiWriteRegisters(0x33, 4, tmp);
  tmp[0] = 0x01; /* reg. PROTOCOL1 (0x3A) */
  S2LPSpiWriteRegisters(0x3A, 1, tmp);
  tmp[0] = 0x40; /* reg. FIFO_CONFIG3 (0x3C) */
  tmp[1] = 0x40; /* reg. FIFO_CONFIG2 (0x3D) */
  tmp[2] = 0x40; /* reg. FIFO_CONFIG1 (0x3E) */
  tmp[3] = 0x40; /* reg. FIFO_CONFIG0 (0x3F) */
  tmp[4] = 0x41; /* reg. PCKT_FLT_OPTIONS (0x40) */
  S2LPSpiWriteRegisters(0x3C, 5, tmp);
  tmp[0] = 0xDD; /* reg. PCKT_FLT_GOALS3 (0x42) */
  tmp[1] = 0xCC; /* reg. PCKT_FLT_GOALS2 (0x43) */
  tmp[2] = 0xBB; /* reg. PCKT_FLT_GOALS1 (0x44) */
  tmp[3] = 0xAA; /* reg. PCKT_FLT_GOALS0 (0x45) */
  S2LPSpiWriteRegisters(0x42, 4, tmp);
  tmp[0] = 0x1D; /* reg. PA_POWER8 (0x5A) */
  S2LPSpiWriteRegisters(0x5A, 1, tmp);
  tmp[0] = 0x07; /* reg. PA_POWER0 (0x62) */
  tmp[1] = 0x01; /* reg. PA_CONFIG1 (0x63) */
  S2LPSpiWriteRegisters(0x62, 2, tmp);
  tmp[0] = 0x9B; /* reg. PM_CONF3 (0x76) */
  tmp[1] = 0xF4; /* reg. PM_CONF2 (0x77) */
  S2LPSpiWriteRegisters(0x76, 2, tmp);
}

/* 
1. FEC Disable
2. Data Whitening Enable
3. preamble-->0x55
4. Synchronization words-->0x6F4E0000
5. 802.15.4g format
6. Secondary Synchronization words-->disable
7. CRC Mode-->3
8. FCS type-->1
9. RF settings 
   -- Frequency base      :868Mhz
   -- Data Rate           :38.4ksps
   -- Frequency deviation :20Khz
   -- Channel filter      :100Khz

Note: the total packet length is equal to MHR+MAC Payload + CRC
 */
void SpiritBaseConfiguration2(void)
{
  uint8_t tmp[5];

  tmp[0] = 0xA3; /* reg. GPIO3_CONF (0x03) */
  S2LPSpiWriteRegisters(0x03, 1, tmp);
  tmp[0] = 0x62; /* reg. SYNT3 (0x05) */
  tmp[1] = 0x2B; /* reg. SYNT2 (0x06) */
  tmp[2] = 0x84; /* reg. SYNT1 (0x07) */
  S2LPSpiWriteRegisters(0x05, 3, tmp);
  tmp[0] = 0x2F; /* reg. IF_OFFSET_ANA (0x09) */
  tmp[1] = 0xC2; /* reg. IF_OFFSET_DIG (0x0A) */
  S2LPSpiWriteRegisters(0x09, 2, tmp);
  tmp[0] = 0x92; /* reg. MOD4 (0x0E) */
  tmp[1] = 0xA7; /* reg. MOD3 (0x0F) */
  tmp[2] = 0x27; /* reg. MOD2 (0x10) */
  S2LPSpiWriteRegisters(0x0E, 3, tmp);
  tmp[0] = 0xA3; /* reg. MOD0 (0x12) */
  tmp[1] = 0x13; /* reg. CHFLT (0x13) */
  S2LPSpiWriteRegisters(0x12, 2, tmp);
  tmp[0] = 0x55; /* reg. ANT_SELECT_CONF (0x1F) */
  S2LPSpiWriteRegisters(0x1F, 1, tmp);
  tmp[0] = 0x40; /* reg. PCKTCTRL6 (0x2B) */
  tmp[1] = 0x04; /* reg. PCKTCTRL5 (0x2C) */
  S2LPSpiWriteRegisters(0x2B, 2, tmp);
  tmp[0] = 0x40; /* reg. PCKTCTRL3 (0x2E) */
  tmp[1] = 0x20; /* reg. PCKTCTRL2 (0x2F) */
  tmp[2] = 0x70; /* reg. PCKTCTRL1 (0x30) */
  S2LPSpiWriteRegisters(0x2E, 3, tmp);
  tmp[0] = 0x00; /* reg. SYNC3 (0x33) */
  tmp[1] = 0x00; /* reg. SYNC2 (0x34) */
  tmp[2] = 0x4E; /* reg. SYNC1 (0x35) */
  tmp[3] = 0x6F; /* reg. SYNC0 (0x36) */
  S2LPSpiWriteRegisters(0x33, 4, tmp);
  tmp[0] = 0x01; /* reg. PROTOCOL1 (0x3A) */
  S2LPSpiWriteRegisters(0x3A, 1, tmp);
  tmp[0] = 0x40; /* reg. FIFO_CONFIG3 (0x3C) */
  tmp[1] = 0x40; /* reg. FIFO_CONFIG2 (0x3D) */
  tmp[2] = 0x40; /* reg. FIFO_CONFIG1 (0x3E) */
  tmp[3] = 0x40; /* reg. FIFO_CONFIG0 (0x3F) */
  tmp[4] = 0x41; /* reg. PCKT_FLT_OPTIONS (0x40) */
  S2LPSpiWriteRegisters(0x3C, 5, tmp);
  tmp[0] = 0x1D; /* reg. PA_POWER8 (0x5A) */
  S2LPSpiWriteRegisters(0x5A, 1, tmp);
  tmp[0] = 0x07; /* reg. PA_POWER0 (0x62) */
  tmp[1] = 0x01; /* reg. PA_CONFIG1 (0x63) */
  S2LPSpiWriteRegisters(0x62, 2, tmp);
  tmp[0] = 0x9B; /* reg. PM_CONF3 (0x76) */
  tmp[1] = 0xF4; /* reg. PM_CONF2 (0x77) */
  S2LPSpiWriteRegisters(0x76, 2, tmp);
}
/* 
1. FEC Disable
2. Data Whitening Disable
3. preamble-->0x55
4. Synchronization words-->0x6F4E0000
5. 802.15.4g format
6. Secondary Synchronization words-->Enable
7. CRC Mode-->3
8. FCS type-->1
9. RF settings 
   -- Frequency base      :868Mhz
   -- Data Rate           :38.4ksps
   -- Frequency deviation :20Khz
   -- Channel filter      :100Khz

Note: the total packet length is equal to MHR+MAC Payload + CRC
 */
void SpiritBaseConfiguration3(void)
{
  uint8_t tmp[6];

  tmp[0] = 0x42; /* reg. GPIO0_CONF (0x00) */
  tmp[1] = 0x4A; /* reg. GPIO1_CONF (0x01) */
  S2LPSpiWriteRegisters(0x00, 2, tmp);
  tmp[0] = 0xA3; /* reg. GPIO3_CONF (0x03) */
  S2LPSpiWriteRegisters(0x03, 1, tmp);
  tmp[0] = 0x62; /* reg. SYNT3 (0x05) */
  tmp[1] = 0x2B; /* reg. SYNT2 (0x06) */
  tmp[2] = 0x86; /* reg. SYNT1 (0x07) */
  tmp[3] = 0x0E; /* reg. SYNT0 (0x08) */
  tmp[4] = 0x2F; /* reg. IF_OFFSET_ANA (0x09) */
  tmp[5] = 0xC2; /* reg. IF_OFFSET_DIG (0x0A) */
  S2LPSpiWriteRegisters(0x05, 6, tmp);
  tmp[0] = 0x92; /* reg. MOD4 (0x0E) */
  tmp[1] = 0xA8; /* reg. MOD3 (0x0F) */
  tmp[2] = 0x27; /* reg. MOD2 (0x10) */
  S2LPSpiWriteRegisters(0x0E, 3, tmp);
  tmp[0] = 0xA3; /* reg. MOD0 (0x12) */
  tmp[1] = 0x13; /* reg. CHFLT (0x13) */
  S2LPSpiWriteRegisters(0x12, 2, tmp);
  tmp[0] = 0x38; /* reg. RSSI_TH (0x18) */
  S2LPSpiWriteRegisters(0x18, 1, tmp);
  tmp[0] = 0x55; /* reg. ANT_SELECT_CONF (0x1F) */
  S2LPSpiWriteRegisters(0x1F, 1, tmp);
  tmp[0] = 0x40; /* reg. PCKTCTRL6 (0x2B) */
  tmp[1] = 0x04; /* reg. PCKTCTRL5 (0x2C) */
  S2LPSpiWriteRegisters(0x2B, 2, tmp);
  tmp[0] = 0x40; /* reg. PCKTCTRL3 (0x2E) */
  tmp[1] = 0x21; /* reg. PCKTCTRL2 (0x2F) */
  tmp[2] = 0x62; /* reg. PCKTCTRL1 (0x30) */
  S2LPSpiWriteRegisters(0x2E, 3, tmp);
  tmp[0] = 0x00; /* reg. SYNC3 (0x33) */
  tmp[1] = 0x00; /* reg. SYNC2 (0x34) */
  tmp[2] = 0x4E; /* reg. SYNC1 (0x35) */
  tmp[3] = 0x6F; /* reg. SYNC0 (0x36) */
  S2LPSpiWriteRegisters(0x33, 4, tmp);
  tmp[0] = 0x44; /* reg. PROTOCOL2 (0x39) */
  tmp[1] = 0x01; /* reg. PROTOCOL1 (0x3A) */
  S2LPSpiWriteRegisters(0x39, 2, tmp);
  tmp[0] = 0x40; /* reg. FIFO_CONFIG3 (0x3C) */
  tmp[1] = 0x40; /* reg. FIFO_CONFIG2 (0x3D) */
  tmp[2] = 0x40; /* reg. FIFO_CONFIG1 (0x3E) */
  tmp[3] = 0x40; /* reg. FIFO_CONFIG0 (0x3F) */
  tmp[4] = 0x41; /* reg. PCKT_FLT_OPTIONS (0x40) */
  S2LPSpiWriteRegisters(0x3C, 5, tmp);
  tmp[0] = 0xBB; /* reg. PCKT_FLT_GOALS1 (0x44) */
  tmp[1] = 0xAA; /* reg. PCKT_FLT_GOALS0 (0x45) */
  tmp[2] = 0xCE; /* reg. TIMERS5 (0x46) */
  tmp[3] = 0x09; /* reg. TIMERS4 (0x47) */
  S2LPSpiWriteRegisters(0x44, 4, tmp);
  tmp[0] = 0x1D; /* reg. PA_POWER8 (0x5A) */
  S2LPSpiWriteRegisters(0x5A, 1, tmp);
  tmp[0] = 0x07; /* reg. PA_POWER0 (0x62) */
  tmp[1] = 0x01; /* reg. PA_CONFIG1 (0x63) */
  S2LPSpiWriteRegisters(0x62, 2, tmp);
  tmp[0] = 0x8F; /* reg. PM_CONF3 (0x76) */
  tmp[1] = 0xF9; /* reg. PM_CONF2 (0x77) */
  S2LPSpiWriteRegisters(0x76, 2, tmp);
}
/**
* @brief  System main function.
* @param  None
* @retval None
*/
int main (void)
{
  uint8_t temp;
  ST_Init();

  do
  {
    for (volatile uint8_t i = 0; i != 0xFF; i++)
      ;
    S2LPRefreshStatus(); // add a timer expiration callback
  } while (g_xStatus.MC_STATE != MC_STATE_READY);
  xRadioInit.lFrequencyBase = xRadioInit.lFrequencyBase;

  SpiritBaseConfiguration();
  /* CW Mode Initialization and Settings*/
  temp = 0x79;
  S2LPSpiWriteRegisters(0x10, 1, &temp);
  temp = 0x3C;
  S2LPSpiWriteRegisters(0x30, 1, &temp);

  do
  {
    for (volatile uint8_t i = 0; i != 0xFF; i++)
      ;
    S2LPRefreshStatus(); // add a timer expiration callback
  } while (g_xStatus.MC_STATE != MC_STATE_READY);
  /* send the TX command */
  S2LPCmdStrobeTx();
  do
  {
    for (volatile uint8_t i = 0; i != 0xFF; i++)
      ;
    S2LPRefreshStatus(); // add a timer expiration callback
  } while (g_xStatus.MC_STATE != MC_STATE_TX);
  while (1)
    ;
  /* uC IRQ config */
  S2LP_Middleware_GpioInit(M2S_GPIO_3,M2S_MODE_EXTI_IN);
  M2S_GPIO_PIN_IRQ = S2LP_Middleware_GpioGetPin(M2S_GPIO_3);

  /* S2LP IRQ config */
  S2LPGpioInit(&xGpioIRQ);

  /* uC IRQ enable */
  S2LP_Middleware_GpioInterruptCmd(M2S_GPIO_3,IRQ_PREEMPTION_PRIORITY,0,ENABLE);

  /* S2LP Radio config */
  S2LPRadioInit(&xRadioInit);

  /* S2LP Radio set power */
  S2LPRadioSetMaxPALevel(S_DISABLE);

  if(!S2LPManagementGetRangeExtender())
  {
    /* If we haven't an external PA, use the library function */
    S2LPRadioSetPALeveldBm(7,POWER_DBM);
  }
  else
  {
    /* in case we are using a board with external PA, the S2LPRadioSetPALeveldBm will be not functioning because
    the output power is affected by the amplification of this external component.
    Set the raw register. */
    uint8_t paLevelValue = 0x25; /* For example, this value will give about 19dBm on a STEVAL FKI-915V1 */
    S2LPSpiWriteRegisters(PA_POWER8_ADDR, 1, &paLevelValue);
  }
  S2LPRadioSetPALevelMaxIndex(7);

  /* S2LP Packet config */
  S2LPPktBasicInit(&xBasicInit);

  /* S2LP IRQs enable */
  S2LPGpioIrqDeInit(NULL);
  S2LPGpioIrqConfig(TX_DATA_SENT , S_ENABLE);

  /* payload length config */
  S2LPPktBasicSetPayloadLength(20);

  /* IRQ registers blanking */
  S2LPGpioIrqClearStatus();

  /* infinite loop */
  while (1){
#ifdef USE_VCOM
    printf("A data to transmit: [");

    for(uint8_t i=0 ; i<20 ; i++)
      printf("%d ", vectcTxBuff[i]);
    printf("]\n\r");
#endif

    /* fit the TX FIFO */
    S2LPCmdStrobeFlushTxFifo();
    S2LPSpiWriteFifo(20, vectcTxBuff);

    /* send the TX command */
    S2LPCmdStrobeTx();

    /* wait for TX done */
    while(!xTxDoneFlag);
    xTxDoneFlag = RESET;

    /* pause between two transmissions */
    SdkDelayMs(500);
  }
}

#ifdef  USE_FULL_ASSERT
/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param file: pointer to the source file name
* @param line: assert_param error line source number
* @retval : None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number */
  printf("Wrong parameters value: file %s on line %d\r\n", file, line);

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/**
*@}
*/

/**
*@}
*/

/**
*@}
*/

/**
*@}
*/


/******************* (C) COPYRIGHT 2019 STMicroelectronics *****END OF FILE****/
