/**
  ******************************************************************************
  * @file    main.c 
  * @author  Орловский А.С.
  * @version V1.0
  * @date    28.07.21
  * @brief   Прошивка УФОЧ
  ******************************************************************************
  */ 

#include "stm8s.h"

void MCU_Init (void)
{
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // f = 16 MHz
  // SPI, сначала шлем старший байт
	SPI_Init (SPI_FIRSTBIT_MSB, SPI_BAUDRATEPRESCALER_2, SPI_MODE_MASTER, SPI_CLOCKPOLARITY_LOW, 
	          SPI_CLOCKPHASE_1EDGE, SPI_DATADIRECTION_1LINE_TX, SPI_NSS_HARD, (uint8_t)0x07);
	SPI_Cmd(ENABLE);
  // LD
	GPIO_Init (GPIOC, GPIO_PIN_4, GPIO_MODE_IN_PU_NO_IT); // Input pull-up, no external interrupt     
  // NSS (SYNC) для AD5141 (инверсная логика)
	GPIO_Init (GPIOC, GPIO_PIN_2, GPIO_MODE_OUT_PP_HIGH_FAST); // Output push-pull, high level, 10MHz
  // LE для ADF4351
	GPIO_Init (GPIOC, GPIO_PIN_1, GPIO_MODE_OUT_PP_LOW_FAST); // Output push-pull, high level, 10MHz
}

void write2adf4351(uint8_t firstByte, uint8_t secondByte, uint8_t thirdByte, uint8_t fourthByte) // запись в регистры ADF4351 via SPI
{
  GPIO_WriteHigh (GPIOC, GPIO_PIN_1); // LE
  SPI_SendData (firstByte);
  while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) { // ждем пока буфер передачи станет пуст
  }
  SPI_SendData (secondByte);
  while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) {
  }
  SPI_SendData (thirdByte);
  while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) {
  }
  SPI_SendData (fourthByte);
  while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) {
  }
  while (SPI_GetFlagStatus(SPI_FLAG_BSY) == SET) { // ждем пока устройство занято
  }
  GPIO_WriteLow (GPIOC, GPIO_PIN_1);
}

void write2ad5141(uint8_t firstByte, uint8_t secondByte) // запись в регистр AD5141 via SPI
{
  GPIO_WriteLow (GPIOC, GPIO_PIN_2); // SYNC
  SPI_SendData (firstByte);
  while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) { // ждем пока буфер передачи станет пуст
  }
  SPI_SendData (secondByte);
  while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET) {
  }
  while (SPI_GetFlagStatus(SPI_FLAG_BSY) == SET) { // ждем пока устройство занято
  }
  GPIO_WriteHigh (GPIOC, GPIO_PIN_2);
}

void main(void)
{
  MCU_Init();
  write2ad5141(0x10, 0x8B); // write 0<D=139=0x8B<255 to RDAC(~4,5V)
  write2ad5141(0x70, 0x01); // copy RDAC register to EEPROM
  do {
    write2adf4351(0x00, 0x58, 0x00, 0x05); //R5
    write2adf4351(0x00, 0xEA, 0x00, 0x24); //R4
    write2adf4351(0x00, 0x60, 0x84, 0xB3); //R3
    write2adf4351(0x1A, 0x00, 0x4F, 0xC2); //R2
    write2adf4351(0x08, 0x00, 0x80, 0x11); //R1
    write2adf4351(0x00, 0x50, 0x00, 0x00); //R0
  } while (GPIO_ReadInputPin (GPIOC, GPIO_PIN_4) != 1); //пытаемся записать значение в SPI, пока lock detect не станет равен 1
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif