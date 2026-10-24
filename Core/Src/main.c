/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <string.h>
#include <stdio.h>

#include "ring_buffer.h"
#include "stm32f4xx_hal.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

static void uart_puts(const char *s);

typedef struct {
  const char *name;
  const char *usage;
  const char *desc;
  void (*handler)(int argc, char **argv);
} cmd_t;

static void cmd_help_handler(int argc, char **argv);
static void cmd_led_handler(int argc, char **argv);
static void cmd_uptime_handler(int argc, char **argv);
static void cmd_echo_handler(int argc, char **argv);

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define NUM_CMDS (sizeof cmds / sizeof cmds[0])
#define MAX_ARGS 8

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

uint8_t rx_byte;

const cmd_t cmds[] = {
  { "help", "help [command]", "Show this list, or details for one command", cmd_help_handler },
  { "led", "led <on|off|toggle>", "Control the green user LED (LD2)", cmd_led_handler },
  { "uptime", "uptime", "Show time since reset", cmd_uptime_handler },
  { "echo", "echo [text...]", "Print the text back", cmd_echo_handler }
};

uint8_t len = 0;
uint8_t line[RB_SIZE] = {0xAA};
uint8_t c;
bool prev_cr = false;

const char nl[] = "\r\n";
const char prompt[] = "> ";
const char bs[] = "\b \b";

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

  uart_puts(nl);
  uart_puts(prompt);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    while (rb_pop(&c)) {
      if (c == '\r' || c == '\n') {
        if (c == '\n' && prev_cr) { prev_cr = false; continue; }
        prev_cr = (c == '\r');

        uart_puts(nl);

        line[len] = '\0';
        int argc = 0;
        char *argv[MAX_ARGS + 1];
        char *tok = strtok((char *) line, " ");
        while(tok != NULL && argc < MAX_ARGS) {
          argv[argc++] = tok;
          tok = strtok(NULL, " ");
        }
        argv[argc] = NULL;

        if (argc > 0) {
          for (size_t i = 0; i < NUM_CMDS; i++) {
            if (strcmp(argv[0], cmds[i].name) == 0) {
              cmds[i].handler(argc, argv);
              break;
            }
          }
        }

        len = 0;

        uart_puts(prompt);
      } else if (c == 0x7F || c == 0x08) {
        prev_cr = false;
        if (len > 0) len--;
        uart_puts(bs);
      } else if (c >= 0x20 && c <= 0x7E && len < RB_SIZE) {
        prev_cr = false;
        line[len++] = c;
        HAL_UART_Transmit(&huart2, &c, 1, HAL_MAX_DELAY);
      }
    }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

static void uart_puts(const char *s) {
  HAL_UART_Transmit(&huart2, (const uint8_t *) s, strlen(s), HAL_MAX_DELAY);
}

static const char led_help[] =
  "usage: led <on|off|toggle>\r\n"
  "  on        turn the LED on\r\n"
  "  off       turn the LED off\r\n"
  "  toggle    switch the LED to the opposite state\r\n";

static void cmd_help_handler(int argc, char **argv) {
  char buf[96];
  if (argc == 1) {
    uart_puts("Commands:\r\n");
    for (size_t i = 0; i < NUM_CMDS; i++) {
      snprintf(buf, sizeof buf, "  %-21s %s \r\n", cmds[i].usage, cmds[i].desc);
      uart_puts(buf);
    }
    return;
  }

  if (strcmp(argv[1], "led") == 0) {
    uart_puts(led_help);
    return;
  }

  for (size_t i = 0; i < NUM_CMDS; i++) {
    if (strcmp(argv[1], cmds[i].name) == 0) {
      snprintf(buf, sizeof buf, "usage: %s\r\n  %s\r\n", cmds[i].usage, cmds[i].desc);
      uart_puts(buf);
      return;
    }
  }
}

static void cmd_led_handler(int argc, char **argv) {
  if (argc > 2) return;

  if (argc == 1) {
    uart_puts(led_help);
    return;
  }

  if (strcmp(argv[1], "on") == 0) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
  } else if (strcmp(argv[1], "off") == 0) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
  } else if (strcmp(argv[1], "toggle") == 0) {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  }
}

static void cmd_uptime_handler(int argc, char **argv) {
  if (argc > 1) return;

  char uptime[24];
  snprintf(uptime, sizeof uptime, "%lu ticks", HAL_GetTick());
  uart_puts(uptime);
  uart_puts(nl);
}

static void cmd_echo_handler(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    uart_puts(argv[i]);
    uart_puts(" ");
  }
  uart_puts(nl);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART2) {
    rb_push(&rx_byte, 1);
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
  }
}

/* USER CODE END 4 */

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
#ifdef USE_FULL_ASSERT
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
