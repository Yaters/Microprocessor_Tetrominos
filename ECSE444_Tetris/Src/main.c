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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
#include "fontlib.h"
#include "graphics.h"
#include "tetris.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define horiz_size 100
#define vert_size 449

#define INPUT_BUFFER_SIZE 5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
typedef enum {
	DOWN, CW, CCW, LEFT, RIGHT, TOGGLEPAUSE, INPUT_ERROR
} game_input_t;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac1_ch1;
DMA_HandleTypeDef hdma_dac1_ch2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim8;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

//Input Storing

game_input_t input_buffer[INPUT_BUFFER_SIZE];
int buffer_pop = 0;
int buffer_push = 0; // pop = push then overflow or empty


int vert_count = 0;
uint8_t** frame_buffer;
uint8_t** true_buffer;
static const char empty[80] =
		"                                                                              \n\0";

char buf[80] =
		"                                                                              \n\0";
char in_buf[2] = "hi";

int paused = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DAC1_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM8_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void swap_buffer();
void clear_buffer();
void init_buffer(uint8_t* tmp_buffer, uint8_t* tmp2_buffer);
void print_str(char* buffer, int x, int y);
void togglePause();
void _print();
void clear();
void hello_world();
void print_gameInput(game_input_t input);

extern uint8_t tetromino_current[];
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void _print() {
	HAL_UART_Transmit(&huart1, buf, 80, 0xFFFF);
}

void clear() {
	sprintf(buf, empty);
//	_print();
}

void hello_world() {
	sprintf(buf, "Hello, World!");
	_print();
}

void print_gameInput(game_input_t input) {
	// here is where we handle input. RIght now this is just an inefficient nested switchcase
	clear();
//	sprintf(buf, input);
	//swap_buffer();
	switch (input) {
	case LEFT:
		sprintf(buf, "Input: Left");
		break;
	case RIGHT:
		sprintf(buf, "Input: Right");
		break;
	case DOWN:
		sprintf(buf, "Input: Down");
		break;
	case CW:
		sprintf(buf, "Input: Clockwise");
		break;
	case CCW:
		sprintf(buf, "Input: Counterclockwise");
		break;
	case TOGGLEPAUSE:
		sprintf(buf, "Input: Toggle Pause/Resume");
	default:
		sprintf(buf, "INPUT ERROR");

	}
	_print();
}

void clear_buffer() {
	for (int i = 0; i < vert_size; i++) {
		for (int j = 0; j < horiz_size; j++) {
			frame_buffer[i][j] = 10;
		}
	}
}

// only used for testing & visual output on CMD
extern void update_screen(Window * window) {
//    system("cls");
//    if (window->curBuff == 0) {
//        for (int row = 0; row < FRAME_HEIGHT; row++) {
//            for (int col = 0; col < FRAME_WIDTH; col++) {
//                printf("%c", window->imgBuff1[row * FRAME_WIDTH + col]);
//            }
//            printf("\n");
//        }
//    }
//    else {
//        for (int row = 0; row < FRAME_HEIGHT; row++) {
//            for (int col = 0; col < FRAME_WIDTH; col++) {
//                printf("%c", window->imgBuff2[row * FRAME_WIDTH + col]);
//            }
//            printf("\n");
//        }
//    }

}

// create window, as well as initializes tetris game
void create_window(Window * window) {
    // initialize window
    window->width = IMAGE_WIDTH;
    window->height = IMAGE_HEIGHT;

    // fill image buffers with default value
    for (int i = 0; i < FRAME_HEIGHT * FRAME_WIDTH; i++) {
        window->imgBuff1[i] = '.';
        window->imgBuff2[i] = '.';
    }
    window->curBuff = 0;

    // initialize tetris game board
    tetris_initialize_game(window);
}

/**
 * @brief Use when the tetris game is playing. (state machine -> game)
 *
 * @param window window that is being used
 * @param event user input
 */
void game_playing(Window* window, int event) {

    switch (event) {
        // move left = 1
        case 1:
            tetris_move_left(window);
        break;

        // move right = 2
        case 2:
            tetris_move_right(window);
        break;

        // rotate clockwise
        case 3:
            tetris_rotate_C_tetromino(window);
        break;

        // rotate counter clockwise
        case 4:
            tetris_rotate_CC_tetromino(window);
        break;

        // drop piece
        case 5:
            tetris_move_down(window);
        break;

        // swap pieces
        case 6:
            tetris_move_down(window);
        break;

        default:
            tetris_move_down(window);
        break;
    }

    // draw game board
    drawRect_color(window, 0, 0, window->width, window->height, 1, 1, '@');
    drawRect(window, BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, 1, 1, window->game.board);
    drawRect(window, BOARD_X + window->game.x, BOARD_Y + window->game.y, 4, 4, 1, 1, tetromino_current);

}

/**
 * @brief Use when the tetris game is paused.
 *
 * @param window window that is being used
 * @param event user input
 */
void game_paused(Window* window, int event) {
    // draw game board
    drawRect_color(window, 0, 0, window->width, window->height, 1, 1, '+');
}

/**
 * @brief Use when the tetris game is paused.
 *
 * @param window window that is being used
 * @param event user input
 */
void game_start(Window* window, int event) {
    // draw game board
    drawRect_color(window, 0, 0, window->width, window->height, 1, 1, '!');
}


// Fill buffers with testing stuff
void init_buffer(uint8_t* tmp_buffer, uint8_t* tmp2_buffer) {
  // Allocate buffers
  // Continuous memory alloc
  frame_buffer = (uint8_t**) malloc(sizeof(uint8_t*) * vert_size);
  true_buffer  = (uint8_t**) malloc(sizeof(uint8_t*) * vert_size);

  // Fill them with data, start with increasing grayscale (and decreasing for back)
  for(int i = 0; i < vert_size; i++) {
	  // Point to place in continuous mem location
	  frame_buffer[i] = tmp_buffer  + i*horiz_size;
	  true_buffer[i] = tmp2_buffer  + i*horiz_size;
	  for(int j = 0; j < horiz_size; j++) {
		  // Back porch Horizontal || Front Porch Horizontal
//		  if (j < 6 || j >= 85) {
		  if (j < 3 || j >= 82) {
			  frame_buffer[i][j] = (uint8_t) 0;
			  true_buffer[i][j] = (uint8_t) 0;
		  }
		  // Back porch Vertical || Front Porch Vertical
		  else if (i < 60 || i >= 410) {
			  true_buffer[i][j] = (uint8_t) 0;
		  }
		  // Color based on x pos
		  else {
			  float y = 350-(i-60) - 175;
			  float x = 4.48718*(j-3) - 175;
			  float rad_head = x*x + y*y;
			  float rad_eyes = (abs(x)-70)*(abs(x)-70) + (y-30)*(y-30);
			  float quad_rad = abs((y+100)+0.01*x*x);
			  if(rad_head > 150*150 && rad_head < 170*170) {
				  true_buffer[i][j] = (uint8_t) 255;
			  } else if (rad_eyes < 20*20) {
				  true_buffer[i][j] = (uint8_t) 255;
			  } else if (quad_rad < 10 && y < -55) {
				  true_buffer[i][j] = (uint8_t) 255;
			  } else {
				  true_buffer[i][j] = (uint8_t) 0;
			  }
			  frame_buffer[i][j] = (uint8_t) (2.8*(i%44)+12*(j%10));
			  //true_buffer[i][j] = (uint8_t) (5.6*(i%22)+6*(j%20));
		  }


	  }
  }
}



void swap_buffer() {
	// Swap pointers
	uint8_t** tmp = true_buffer;
	true_buffer = frame_buffer;
	frame_buffer = tmp;
//	clear_buffer();
	// Change DMA memory address
	hdac1.DMA_Handle1->Instance->CMAR = (uint32_t) true_buffer[0];
}

void print_str(char* buffer, int x, int y) {

	x += 3; // Avoid back porch
	y += 65;

	char cur_char = buffer[0];
	if(cur_char >= 97) cur_char -= 32;
	int i = 0;
	while(cur_char != '\0') {
		char* bitmap = font_map[cur_char - 32]; // 32 = ' '
		for (int h = 0; h < 50; h++) {
			frame_buffer[h+y][x] = 80; // Precursor
			//x += 1;
			for(int w = 0; w < 5; w++) {
				int array_index = (h/10) * 5 + w; // h/5 = floor division, to stretch
				if(bitmap[array_index]) frame_buffer[h+y][1+w+x] = 190;
				else frame_buffer[h+y][1+w+x] = 80;
			}
			frame_buffer[h+y][x+6] = 80;
		}
		x += 7; // 1 pre & postcursor

		i++;
		cur_char = buffer[i];
		//'a' -> 'A' for example
		if(cur_char >= 97) cur_char -= 32;
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
	for (int i = 0; i < INPUT_BUFFER_SIZE; i++) {
	    input_buffer[i] = INPUT_ERROR;
	}
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
  MX_DMA_Init();
  MX_DAC1_Init();

  MX_TIM4_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM8_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  Window window;
  create_window(&window);


  // Fill the frame buffer
  init_buffer(window.imgBuff1, window.imgBuff2);
  HAL_TIM_Base_Start_IT(&htim1);	// start slave first.
  //HAL_Delay(50);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);	// start slave first.
  //HAL_Delay(50);
  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *) true_buffer[0], horiz_size*vert_size, DAC_ALIGN_8B_R);
  //HAL_Delay(50);
  HAL_TIM_Base_Start(&htim4);	// start master timer.

  HAL_UART_Receive_IT(&huart1, (uint8_t*) in_buf, 1);
  //clear_buffer();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){
//	print_str("Welcome", 0, 0);
//	print_str("To", 0, 60);
//	print_str("Tetris!", 0, 120);
//	print_str("-Yanis J.", 0, 180);
	//swap_buffer();
//	HAL_Delay(5000);
      // process button presses (update game state)
	int event = process_user_input(&window);

	if (event > 0) {
	  switch (window.game.state) {
		  case Start:
			  game_start(&window, event);
		  break;
		  case Playing:
			  // update the game state, and draw to frame buffer
			  game_playing(&window, event);
		  break;
		  case Paused:
			  game_paused(&window, event);
		  break;
	  }
	  swap_buffer();

//	  refreshScreen(&window);
//	  // update screen
//	  update_screen(&window);
	}
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */
  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT1 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_T4_TRGO;
  sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_DISABLE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT2 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_T8_TRGO;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */
  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 99;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
  sSlaveConfig.InputTrigger = TIM_TS_ITR3;
  if (HAL_TIM_SlaveConfigSynchro(&htim1, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */
  TIM1->SMCR = TIM_TS_ITR3 | TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1 | TIM_SMCR_SMS_2;
  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 99;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
  sSlaveConfig.InputTrigger = TIM_TS_ITR3;
  if (HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 90;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  // trigger selection TS=001 ITR1 = TIM2, slave mode SMS=0111 external clock mode 1
  TIM2->SMCR = TIM_TS_ITR3 | TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1 | TIM_SMCR_SMS_2;
  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 9999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 25400;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 7;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 2;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM8 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM8_Init(void)
{

  /* USER CODE BEGIN TIM8_Init 0 */

  /* USER CODE END TIM8_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM8_Init 1 */

  /* USER CODE END TIM8_Init 1 */
  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 0;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 10000;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM8_Init 2 */

  /* USER CODE END TIM8_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Vert_Synch_GPIO_Port, Vert_Synch_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : myButton_Pin */
  GPIO_InitStruct.Pin = myButton_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(myButton_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Vert_Synch_Pin */
  GPIO_InitStruct.Pin = Vert_Synch_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Vert_Synch_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
//	if (htim->Instance == TIM1) {
		vert_count = (vert_count + 1) % 449;
		if(vert_count >= 447) {
			HAL_GPIO_WritePin(Vert_Synch_GPIO_Port, Vert_Synch_Pin, GPIO_PIN_RESET);
		} else {
			HAL_GPIO_WritePin(Vert_Synch_GPIO_Port, Vert_Synch_Pin, GPIO_PIN_SET);
		}
//	}
}



void push_input_buffer(game_input_t input) {
	input_buffer[buffer_push] = input;
	buffer_push = (buffer_push + 1) % INPUT_BUFFER_SIZE;
}

game_input_t pop_input_buffer() {
	if(buffer_pop == buffer_push) return INPUT_ERROR;
	game_input_t ret = input_buffer[buffer_pop];
	buffer_pop = (buffer_pop + 1) % INPUT_BUFFER_SIZE;
	return ret;
}



int process_user_input(Window * window) {
    //char c = getchar();
    game_input_t c = pop_input_buffer();
    while(c == INPUT_ERROR) c = pop_input_buffer();

    switch (c) {
        // move left
        case LEFT:
            return 1;
        break;
        // move right
        case RIGHT:
            return 2;
        break;
        case DOWN:
            return 0;
        break;
//        case 'q':
//            return -1;
//        break;
        // rotate CC = 4
        case CCW:
            return 4;
        break;
        // rotate C = 3
        case CW:
            return 3;
        break;
        // pause game
        case TOGGLEPAUSE:
            if (window->game.state == Playing) {
                window->game.state = Paused;
            }
            else {
                window->game.state = Playing;
            }
        break;
        default:
        break;
    }

    // return a value not mapped to any event
    return 10;
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
//	print_inbuf();
	char c = ((huart)->Instance)->RDR;
//	swap_buffer();
	switch (c) {
	case 53: // 5 as in the key "5" (to test if working)
		hello_world();
		break;
	case 97: // a
		push_input_buffer(LEFT);
		break;
	case 100: // d
		push_input_buffer(RIGHT);
		break;
	case 107: // l ' '?
		push_input_buffer(CCW);
		break;
	case 59: // semicolon
		push_input_buffer(CW);
		break;
	case 115: // s
		push_input_buffer(DOWN);
		break;
	case 32: // space
		push_input_buffer(TOGGLEPAUSE);
		//togglePause();
	default:
		break;
	}
	HAL_UART_Receive_IT(&huart1, (uint8_t*) in_buf, 1);
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

