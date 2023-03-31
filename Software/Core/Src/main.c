/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STACK_SIZE 1000
#define DELAY_1 100

#define Q_LENGTH 10
#define Q_SIZE sizeof(int)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
SemaphoreHandle_t sem ;
TaskHandle_t h_tasktake= NULL;
QueueHandle_t h_Queue = NULL;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch) {
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

/*  void Led_toggle (void * pvParameters) {
	int compteur = 0;
	int duree = (int) pvParameters;
	char* s = pcTaskGetName(xTaskGetCurrentTaskHandle());

	while (1) {
		HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
		printf("%d) Je suis la tache %s et je m'endors pour %d periodes\n\r", compteur, s, duree);
		vTaskDelay(duree);
		compteur++;
	}
}*/

/* Sémaphores
void taskGive(void * unused){
	int my_time=100;
	for(;;){
		printf("Give donne un semaphore\r\n");
		xSemaphoreGive(sem);
		printf("Give a donne un semaphore\r\n");
		vTaskDelay(my_time);
		my_time=my_time+100;
	}
}
void taskTake(void * unused){
	for(;;){
		printf("Take prend un semaphore\r\n");
		BaseType_t error = xSemaphoreTake(sem, 1000);
		if (error == 0)
		{
			printf("Timeout error\r\n");
			NVIC_SystemReset();
//			Error_Handler();
		}
		printf("Take a pris un semaphore = %d\r\n", error);
	}
}
*/

/* Notification
void taskGive(void * unused){
	int my_time=100;
	for(;;){
		printf("Give donne une notification\r\n");
		xTaskNotifyGive(h_tasktake);
		printf("Give a donne une notification\r\n");
		vTaskDelay(my_time);
		my_time=my_time+100;
	}
}
void taskTake(void * unused){
	for(;;){
		printf("Take prend une notification\r\n");
		BaseType_t error = ulTaskNotifyTake(pdTRUE, 1000);
		if (error == 0)
		{
			printf("Timeout error\r\n");
			NVIC_SystemReset();
//			Error_Handler();
		}
		printf("Take a pris une notification = %d\r\n", error);
	}
}
*/

void taskGive(void * unused){
	int my_time=100;
	for(;;){
		printf("Give ecris dans la queue\r\n");
		xQueueSend(h_Queue, (void *) &my_time, portMAX_DELAY);
		printf("Give a ecris dans la queue\r\n");
		vTaskDelay(my_time);
		my_time=my_time+100;
	}
}
void taskTake(void * unused){
	int timer;
	for(;;){
		printf("Take prend dans la queue \r\n");
		BaseType_t error = xQueueReceive(h_Queue, (void *)&timer, 1000);
		if (error == 0)
		{
			printf("Timeout error\r\n");
			NVIC_SystemReset();
//			Error_Handler();
		}
		printf("Take a pris dans la queue = %d\r\n", timer);
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	printf("\r\n===== START =====\r\n");
	/*	 */
	BaseType_t xReturned;
	TaskHandle_t xHandle1 = NULL;

	/* Create the task, storing the handle. */
	/*
	xReturned = xTaskCreate(Led_toggle, "Task1", STACK_SIZE, (void *) DELAY_1,tskIDLE_PRIORITY,	&xHandle1 );
	if (xReturned != pdPASS)
	{
		Error_Handler();
	}

	*/
	sem = xSemaphoreCreateBinary();
	h_Queue = xQueueCreate(Q_LENGTH, Q_SIZE);

	xTaskCreate(taskGive, "Give", 256, NULL, 2, NULL);
	xTaskCreate(taskTake, "Take", 256, NULL, 1, &h_tasktake);

	vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
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
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
