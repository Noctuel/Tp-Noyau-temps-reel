/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include "shell.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TASK_SHELL_STACK_DEPTH 512
#define TASK_LED_STACK_DEPTH 512
#define TASK_SPAM_STACK_DEPTH 512
#define TASK_SHELL_PRIORITY 1
#define TASK_LED_PRIORITY 2
#define TASK_SPAM_PRIORITY 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TaskHandle_t h_task_shell = NULL;
TaskHandle_t h_task_overflow = NULL;
TaskHandle_t h_task_led = NULL;
TaskHandle_t h_task_Spam = NULL;

SemaphoreHandle_t sem ;

typedef struct{
	char string[100] ;
	int nbr;
	int tempo;
} spam_msg_t;


 char pcWriteBuffer[200];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);

	return ch;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) // fonction weak de base
{
	if (huart->Instance == USART1)
	{
		shell_uart_receive_irq_cb();	// C'est la fonction qui donne le sémaphore!
	}
}

//---------------------------------------------------------------------------------------------------------
//Sémaphores
void taskGive(void * unused){
	int my_time=2000;
	for(;;){
		printf("Give donne un semaphore\r\n");
		xSemaphoreGive(sem);
		printf("Give a donne un semaphore\r\n");
		vTaskDelay(my_time);
	}
}
void taskTake(void * unused){
	for(;;){
		printf("Take prend un semaphore\r\n");
		BaseType_t error = xSemaphoreTake(sem, 5000);
		if (error == 0)
		{
			printf("Timeout error\r\n");
			NVIC_SystemReset();
			//			Error_Handler();
		}
		printf("Take a pris un semaphore = %d\r\n", error);
	}
}
//---------------------------------------------------------------------------------------------------------
//Overflow
void overflow(void * unused){
	int grostableau[TASK_SHELL_STACK_DEPTH];

	for (int i = 0 ; i < TASK_SHELL_STACK_DEPTH ; i++)
	{
		grostableau[i] = i;
		printf("%d\r\n", grostableau[i]);
	}
	vTaskDelete(NULL);
}
//---------------------------------------------------------------------------------------------------------
//SHELL
int variable_globale;			// Segment de données (visible dans tous les fichiers)
static int variable_statique;	// Segment de données aussi (visible que dans le fichier)


int fonction(int argc, char ** argv)//parametres passe dans la Pile
{
	int variable_locale;					// Pile (visible dans la fonction)
	static int variable_locale_statique;	// Segment de données (visible dans la fonction)


	printf("Je suis une fonction bidon\r\n");

	printf("argc = %d\r\n", argc);

	for (int i = 0 ; i < argc ; i++)
	{
		printf("arg %d = %s\r\n", i, argv[i]);
	}

	return 0;
}

int addition(int argc, char ** argv)
{
	if (argc == 3)
	{
		int a, b;
		a = atoi(argv[1]);
		b = atoi(argv[2]);
		printf("%d + %d = %d\r\n", a, b, a+b);

		return 0;
	}
	else
	{
		printf("Erreur, pas le bon nombre d'arguments\r\n");
		return -1;
	}
}

void Led_toggle (void * pvParameters) {
	int duree = (int) pvParameters;

	while (1) {
		HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
		vTaskDelay(duree);
	}
}


void Task_spam (void * pvParameters) {
	spam_msg_t my_msg = *(spam_msg_t*)pvParameters;

	printf("Task Spam created\r\n");

	for(int i=0 ; i<=my_msg.nbr ; i++)
	{
		printf("%s\r\n",my_msg.string);
		vTaskDelay(my_msg.tempo);
	}

	vTaskDelete(NULL);
}

int led(int argc, char ** argv){

	int period = atoi(argv[1]);

	if(argc == 2){
		if (period == 0)
		{
			if(h_task_led!=NULL)
			{
				vTaskDelete(h_task_led);
				printf("Task LED destroy\r\n");
				h_task_led = NULL;
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,0);
			}
			else
			{
				printf("Task LED don't exist\r\n");
			}
		}
		else
		{
			if(h_task_led!=NULL)
			{
				vTaskDelete (h_task_led);
				printf("Task LED destroy to be replace\r\n");
			}
			if(xTaskCreate(Led_toggle, "LED", TASK_LED_STACK_DEPTH, (void *)period,TASK_LED_PRIORITY ,	&h_task_led)!= pdPASS)
			{
				printf("Error creating task LED\r\n");
				Error_Handler();
			}
			else
			{
				printf("Task LED created period = %d ms\r\n", period);
			}
		}
	}
	else{
		printf("Erreur, trop d'arguments\r\n");
	}
	return 0;
}

int spam(int argc, char ** argv){


	spam_msg_t my_msg = {.nbr = atoi(argv[2]),.tempo = atoi(argv[3])};
	strcpy(my_msg.string, argv[1]);

	if(argc == 4){
		if (my_msg.nbr == 0 || my_msg.tempo == 0 )
		{
			printf("Task Spam don't exist nbr or tempo are not int or equal to 0s\r\n");
		}

		else
		{
			if(xTaskCreate(Task_spam, "Spam", TASK_SPAM_STACK_DEPTH, (void *)&my_msg,TASK_SPAM_PRIORITY ,&h_task_Spam)!= pdPASS)
			{
				printf("Error creating task Spam\r\n");
				Error_Handler();
			}
		}
	}
	else{
		printf("Erreur, nbr d'arguments 3 => msg nbr tempo\r\n");
	}
	return 0;
}

int stat (int argc, char ** argv){

	if(argc == 1){
		vTaskGetRunTimeStats(pcWriteBuffer);
		printf("%s\r\n",pcWriteBuffer);
		vTaskList(pcWriteBuffer);
		printf("%s\r\n",pcWriteBuffer);
	}
	return 0;
}

void task_shell(void * unused)
{
	shell_init();
	shell_add('f', fonction, "Une fonction inutile");
	shell_add('a', addition, "Effectue une somme");
	shell_add('l', led, "Toggle led");
	shell_add('s', spam, "Spam string nbr");
	shell_add('t', stat, "Stat");
	shell_run();	// boucle infinie
}
//---------------------------------------------------------------------------------------------------------


void configureTimerForRunTimeStats(void)
{
	HAL_TIM_Base_Start(&htim2);
}

unsigned long getRunTimeCounterValue(void)
{
	return __HAL_TIM_GET_COUNTER(&htim2);
}

void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
	printf("je ne m'éxecute pas\r\n");
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
	MX_TIM2_Init();
	/* USER CODE BEGIN 2 */
	DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM6_STOP;

	//	if (xTaskCreate(overflow, "ov", TASK_SHELL_STACK_DEPTH, NULL, TASK_SHELL_PRIORITY, &h_task_overflow) != pdPASS)
	//	{
	//			printf("Error creating task overflow\r\n");
	//		Error_Handler();
	//	}

	if (xTaskCreate(task_shell, "Shell", TASK_SHELL_STACK_DEPTH, NULL, TASK_SHELL_PRIORITY, &h_task_shell) != pdPASS)
	{
		printf("Error creating task shell\r\n");
		Error_Handler();
	}

	sem = xSemaphoreCreateBinary();
	vQueueAddToRegistry(sem, "SemTest");
	xTaskCreate(taskGive, "Give", 256, NULL, 5, NULL);
	xTaskCreate(taskTake, "Take", 256, NULL, 4, NULL);

	// Tache Bidon pour avoir une erreur
	//
	//	while(1){
	//		static int i = 0;
	//		printf("i = %d\r\n", i);
	//		i++;
	//
	//		if (xTaskCreate(task_shell, "Shell", TASK_SHELL_STACK_DEPTH, NULL, TASK_SHELL_PRIORITY, &h_task_shell) != pdPASS)
	//		{
	//			printf("Error creating task shell\r\n");
	//			Error_Handler();
	//		}
	//	}



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
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 432;
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
