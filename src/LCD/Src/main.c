#include <stdlib.h>
#include "main.h"

#define 		LCD_DELAY									600
#define 		LCD_CLEAR_CMD							0x01
#define 		LCD_PIN_OFFSET  					12
#define 		LCD_PORT									GPIOE

void LcdWriteByte(uint8_t data);
void LcdWriteHalfByte(uint8_t data);
void LcdWriteCmd(uint8_t data);
void LcdWriteData(uint8_t data);
void LcdClearDisplay(void);
void LcdSetCursor(uint8_t line, uint8_t pos);
void LcdWriteStr(char* str);
int rand (void);
int log10 (int);
char * ToArray(int number);
void DisplayInit(void);
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
  HAL_Init();
  MX_GPIO_Init();
  DisplayInit();
  SystemClock_Config();

	
  while (1)
  {
		HAL_Delay(LCD_DELAY);
		
		uint8_t but_1 = HAL_GPIO_ReadPin(GPIOC, BUTTON_1_Pin);
		uint8_t but_2 = HAL_GPIO_ReadPin(GPIOC, BUTTON_2_Pin);
		uint8_t but_3 = HAL_GPIO_ReadPin(GPIOC, BUTTON_3_Pin);
		uint8_t but_4 = HAL_GPIO_ReadPin(GPIOC, BUTTON_4_Pin);
		uint8_t but_5 = HAL_GPIO_ReadPin(GPIOC, BUTTON_5_Pin);
		uint8_t reset = GPIO_PIN_RESET;
		
		if(but_1 == reset || but_2 == reset || but_3 == reset || but_4 == reset || but_5 == reset)
		{
			LcdClearDisplay();
			LcdSetCursor(0, 0);
			
			int random = rand();
			LcdWriteStr(ToArray(random));
		} 
  }
}

void LcdWriteByte(uint8_t data)
{
	LcdWriteHalfByte(data >> 4);
	LcdWriteHalfByte(data);
}

void LcdWriteHalfByte(uint8_t data)
{
	HAL_GPIO_WritePin(LCD_PORT, ((data & 0x0F)) << LCD_PIN_OFFSET, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_PORT, LCD_EN_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LCD_PORT, LCD_EN_Pin, GPIO_PIN_RESET); // Data receive on falling edge
	HAL_GPIO_WritePin(LCD_PORT, 0x0F<<LCD_PIN_OFFSET, GPIO_PIN_RESET);
}

void LcdWriteCmd(uint8_t data)
{
	HAL_GPIO_WritePin(LCD_PORT, LCD_RS_Pin, GPIO_PIN_RESET);
	LcdWriteByte(data);
	HAL_GPIO_WritePin(LCD_PORT, LCD_RS_Pin, GPIO_PIN_SET);
}

void LcdWriteData(uint8_t data)
{
	HAL_GPIO_WritePin(LCD_PORT, LCD_RS_Pin, GPIO_PIN_SET);
	LcdWriteByte(data);
	HAL_GPIO_WritePin(LCD_PORT, LCD_RS_Pin, GPIO_PIN_RESET);
}

void LcdClearDisplay()
{
	LcdWriteCmd(LCD_CLEAR_CMD);
}

void LcdSetCursor(uint8_t line, uint8_t pos)
{
	pos |= 0x80;
	if (line == 1) {
		pos += 0x40;
	}
	LcdWriteCmd(pos);
}

void LcdWriteStr(char* str)
{
	while(*str != '\0'){
		LcdWriteData(*str);
		str++;
	}
}

char * ToArray(int number)
{
        int n = log10(number) + 1;
        int i;
      char *numberArray = calloc(n, sizeof(char));
        for ( i = 0; i < n; ++i, number /= 10 )
        {
            numberArray[i] = number % 10;
        }
        return numberArray;
}
		
void DisplayInit(void)
{
	LcdWriteCmd(0x20);
	HAL_Delay(LCD_DELAY);
	LcdWriteCmd(0x28);
	HAL_Delay(LCD_DELAY);
	LcdWriteCmd(0x28);
	HAL_Delay(LCD_DELAY);
	LcdWriteCmd(0x0f);
	HAL_Delay(LCD_DELAY);
	LcdWriteCmd(0x01);
	HAL_Delay(LCD_DELAY);
	LcdWriteCmd(0x06);
	HAL_Delay(LCD_DELAY);
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
	
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOE, LCD_RS_Pin|LCD_RW_Pin|LCD_EN_Pin|LCD_D4_Pin 
                          |LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = LCD_RS_Pin|LCD_RW_Pin|LCD_EN_Pin|LCD_D4_Pin 
                          |LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BUTTON_4_Pin|BUTTON_5_Pin|BUTTON_3_Pin|BUTTON_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BUTTON_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_2_GPIO_Port, &GPIO_InitStruct);

	HAL_GPIO_WritePin(LCD_PORT, LCD_RW_Pin, GPIO_PIN_RESET);
}

void Error_Handler(void)
{
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
