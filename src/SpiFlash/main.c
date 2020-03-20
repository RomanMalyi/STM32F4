
#include "main.h"
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common

//***
#include "Driver_SPI.h"
//***

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

void Configure_GPIO_7(void);
void Drv_SPI_CS_Enable (void);
void Drv_SPI_CS_Disable (void);

void Enable_Write (void);
uint8_t Read_Status(void);
void Chip_Erase(void);
int8_t Write_Buffer(long startAdd, uint8_t *data, uint8_t len);
void Write_Byte(long add, uint8_t data);
int8_t Read_Buffer (long startAdd, uint8_t *data, uint8_t len);
uint8_t Read_Byte(long add);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
	
	/* Test data buffers */
 uint8_t write_buffer[] = { 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1 };
 uint8_t read_buffer[14]= {0};

int init_result1;
int init_result2;
int init_result3;

uint8_t status_result = 0;
int main(void)
{	
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* Add your application code here
     */
	
	  /* Initialize the SPI driver */
    init_result1=SPIdrv->Initialize(NULL);
    /* Power up the SPI peripheral */
    init_result2=SPIdrv->PowerControl(ARM_POWER_FULL);
    /* Configure the SPI to Master, 8-bit mode @10000 kBits/sec */
    init_result3=SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_UNUSED | ARM_SPI_DATA_BITS(8), 16000000);
		
		if(init_result1==init_result2==init_result3==0)
		{
			Configure_GPIO_7();
			Enable_Write();
			
			status_result = Read_Status();
			
			Chip_Erase();
			HAL_Delay(50);
			
			int8_t write_buffer_status = Write_Buffer(0, write_buffer, sizeof(write_buffer));
			int8_t read_buffer_status = Read_Buffer(0, read_buffer, sizeof(write_buffer));
		}

  /* Infinite loop */
  while (1)
  {
  }
}

void Configure_GPIO_7(void)
{
		GPIO_InitTypeDef GPIO_InitStruct; 
		/*Configure GPIO pin : PD7 */
		GPIO_InitStruct.Pin = GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct); 
}

void Drv_SPI_CS_Enable (void)
{
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET);
}


void Drv_SPI_CS_Disable (void)
{
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET);
}

	//DEVICE OPERATION INSTRUCTIONS
	uint8_t write_enable = 0x06;	
	uint8_t enable_write_status_register = 0x50;
	uint8_t write_status_register = 0x01;
	uint8_t read_status_register = 0x05;
	uint8_t chip_erase = 0x60;
	uint8_t to_program_one_data_byte = 0x02;
	uint8_t read_memory = 0x03;

void Enable_Write (void)
{
		
	uint8_t value = 0x00;	
	
	Drv_SPI_CS_Enable();
	SPIdrv->Send(&write_enable,1);
	Drv_SPI_CS_Disable();
	
	Drv_SPI_CS_Enable();
  SPIdrv->Send(&enable_write_status_register,1);
	Drv_SPI_CS_Disable();
	
	Drv_SPI_CS_Enable();
	SPIdrv->Send(&write_status_register,1);
	SPIdrv->Send(&value,1);
	Drv_SPI_CS_Disable();
}

uint8_t Read_Status(void)
{
	uint8_t status;	
	
	Drv_SPI_CS_Enable();
	SPIdrv->Send(&read_status_register, 1);
	SPIdrv->Receive(&status, 1);	
	Drv_SPI_CS_Disable();
	
	return status;
}

void Chip_Erase(void)
{
	Drv_SPI_CS_Enable();
	SPIdrv->Send(&enable_write_status_register, 1);	
	Drv_SPI_CS_Disable();
	
	Drv_SPI_CS_Enable();
	SPIdrv->Send(&chip_erase, 1);	
	Drv_SPI_CS_Disable();
}

int8_t Write_Buffer(long startAdd, uint8_t *data, uint8_t len)
{
	if ((startAdd < 0) || (startAdd> 0xFFFFFF))
		return -1; 
	if ((len > 0) && ((startAdd + len) <  0xFFFFFF))
	{
		for (int i = 0; i < len; i++)
		{
			Write_Byte(startAdd + i, *(data + i));
		}
		return 0;
	}	
		else
			return -1;
}

void Write_Byte(long add, uint8_t data)
{
	Drv_SPI_CS_Enable();
	SPIdrv->Send(&write_enable,1);
	Drv_SPI_CS_Disable();	
	
	Drv_SPI_CS_Enable();
	SPIdrv->Send(&to_program_one_data_byte,1);
	
	uint8_t add1 = (add>>16)&0xFF;
	uint8_t add2 = (add>>8)&0xFF;
	uint8_t add3 = add&0xFF;
	
	SPIdrv->Send(&add1,1);
	SPIdrv->Send(&add2,1);
	SPIdrv->Send(&add3,1);
	
	SPIdrv->Send(&data,1);
	
	Drv_SPI_CS_Disable();
}

int8_t Read_Buffer (long startAdd, uint8_t *data, uint8_t len)
{
if ((startAdd < 0) || (startAdd > 0xFFFFFF))
		return -1; 
	if ((len > 0) && ((startAdd + len) <  0xFFFFFF))
	{
		for (int i = 0; i < len; i++)
		{
			*(data + i) = Read_Byte(startAdd + i);
		}
		
		return 0;
	}	
		else
			return -1;
}

uint8_t Read_Byte(long add)
{
	uint8_t result = 0;
	
	Drv_SPI_CS_Enable();
	SPIdrv->Send(&read_memory,1);
	
	uint8_t add1 = (add>>16)&0xFF;
	uint8_t add2 = (add>>8)&0xFF;
	uint8_t add3 = add&0xFF;
	
	SPIdrv->Send(&add1,1);
	SPIdrv->Send(&add2,1);
	SPIdrv->Send(&add3,1);
	
	SPIdrv->Receive(&result, 1);		
	
	Drv_SPI_CS_Disable();
	return result;
}
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 25
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
