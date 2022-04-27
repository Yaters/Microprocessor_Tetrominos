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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
#include <tetris.h>

#include "tetristhemequiet.c"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define INPUT_BUFFER_SIZE 5	// max number of user inputs that can be queues


// Sound variables
#define THEME_SOUND_SIZE 101600
#define SHIFT_SOUND_SIZE 0
#define ROT_SOUND_SIZE 0
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

RNG_HandleTypeDef hrng;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim15;

UART_HandleTypeDef huart1;

/* Definitions for frameTask */
osThreadId_t frameTaskHandle;
const osThreadAttr_t frameTask_attributes = {
  .name = "frameTask",
  .stack_size = 500 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for soundTask */
osThreadId_t soundTaskHandle;
const osThreadAttr_t soundTask_attributes = {
  .name = "soundTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* USER CODE BEGIN PV */

//Input Storing

game_input_t input_buffer[INPUT_BUFFER_SIZE];
int buffer_pop = 0;
int buffer_push = 0; // pop = push then overflow or empty

int fall_rate = FALL_INIT;

int vert_count = 0;
char in_buf[2] = "hi";


// Tetris game window variable
Window window;


// Sound data information
uint16_t *tetris_theme = (uint16_t*) _actetristhemequiet;
uint16_t *rotate_sound = (uint16_t*) _actetristhemequiet;
uint16_t *shift_sound = (uint16_t*) _actetristhemequiet;

// Normal playing (not special effect)
uint16_t *full_snd_data;
unsigned long int full_snd_data_size;
unsigned long int full_snd_data_offset;

// Currently playing
uint16_t *snd_wave_data;
unsigned long int snd_data_size;
unsigned long int snd_data_offset;
int playing_snd = 0; // Whether to output to DAC
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DAC1_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM15_Init(void);
static void MX_RNG_Init(void);
void updateGameLogic(void *argument);
void soundController(void *argument);

/* USER CODE BEGIN PFP */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void swap_buffer();
void init_buffer(uint8_t* tmp_buffer, uint8_t* tmp2_buffer);
void draw_str(Window* window, char* buffer, int x, int y);
void togglePause();
void _print();
void clear();
void print_gameInput(game_input_t input);


game_input_t process_user_input(Window * window);
void game_playing(Window* window, game_input_t event);
void game_paused(Window * window, game_input_t event);
void game_start(Window * window, game_input_t event);
void game_ended(Window * window, game_input_t event);
void create_window(Window * window);
void end_application(Window* window);

extern uint8_t tetromino_current[];
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// create window, as well as initializes tetris game
void create_window(Window * window) {
    // initialize window
    window->width = IMAGE_WIDTH;
    window->height = IMAGE_HEIGHT;

    window->frame = (uint8_t**) malloc(sizeof(uint8_t*) * FRAME_HEIGHT);
    window->true  = (uint8_t**) malloc(sizeof(uint8_t*) * FRAME_HEIGHT);

    // Fill image buffers with default value
    for(int i = 0; i < FRAME_HEIGHT; i++) {
		// Point to place in continuous mem location
    	window->frame[i] = window->frameBuff + i*FRAME_WIDTH;
    	window->true[i]  = window->trueBuff  + i*FRAME_WIDTH;
		for (int j = 0; j < FRAME_WIDTH; j++) {
			window->frame[i][j] = 0;
			window->true[i][j] = 0;
		}
    }

    // initialize tetris game board
    tetris_initialize_game(window);
}

/**
 * Stops currently playing track to play a sound effect. Plays sound effect and returns to previous music
 * Size of sfx currently 0 because we have no sfxs
 */
void triggerSoundEffect(uint16_t* effect_data, unsigned long int effect_size) {
	// Stop DAC, change music source, start again
	playing_snd = 0;
	snd_wave_data = effect_data;
	full_snd_data_offset = snd_data_offset; // Record where we were
	snd_data_offset = 0;
	snd_data_size = effect_size;
	playing_snd = 1;
}

/**
 * @brief Use when the tetris game is playing. (state machine -> game)
 *
 * @param window window that is being used
 * @param event user input
 */
void game_playing(Window* window, game_input_t event) {
    if (event == TOGGLEPAUSE) {
    	window->game.state = Paused;
    	game_paused(window, INPUT_ERROR);
    } else {
        switch (event) {
            // move left = 1
            case LEFT:
            	triggerSoundEffect(shift_sound, SHIFT_SOUND_SIZE);
                tetris_move_left(window);
            break;

            // move right = 2
            case RIGHT:
            	triggerSoundEffect(shift_sound, SHIFT_SOUND_SIZE);
                tetris_move_right(window);
            break;

            // rotate clockwise
            case CW:
            	triggerSoundEffect(rotate_sound, ROT_SOUND_SIZE);
                tetris_rotate_C_tetromino(window);
            break;
            // rotate counter clockwise
            case CCW:
            	triggerSoundEffect(rotate_sound, ROT_SOUND_SIZE);
                tetris_rotate_CC_tetromino(window);
            break;

            // drop piece
            case DOWN:
                tetris_move_down(window);
            break;

            // swap pieces
            case INPUT_ERROR:
                //tetris_move_down(window);
            break;

            default:
            	// never triggered
                //tetris_move_down(window);
            break;
        }

        // draw game board if still playing
        if (window->game.state == Playing ) {
            drawRect(window, BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, HORIZ_SCALE, VERT_SCALE, window->game.board);
            drawRect(window, BOARD_X + window->game.x, BOARD_Y + window->game.y, 4, 4, HORIZ_SCALE, VERT_SCALE, tetromino_current);
            // Shouldn't take too much time - though we could move it to finalize
            tetris_draw_scoreboard(window);
            fall_rate = (int) FALL_INIT - sqrt(1000 * window->game.rows_cleared);
            fall_rate = (fall_rate < 1) ? 1 : fall_rate;
        }
    }

}

/**
 * @brief Use when the tetris game is paused.
 *
 * @param window window that is being used
 * @param event user input
 */
void game_paused(Window* window, game_input_t event) {
    if (event == TOGGLEPAUSE) {
    	window->game.state = Playing;
    	// Draw background before anything (in both frames)
    	tetris_drawBackground(window);
    	swap_buffer(window);
    	tetris_drawBackground(window);
    	game_playing(window, INPUT_ERROR);
    } else {
        // draw game board
        drawRect_color(window, 0, 0, window->width, window->height, 4, 10, 100);
        draw_str(window, "Press Space", 2, 10);
        draw_str(window, "To Continue", 2, 70);
    }
}

/**
 * @brief Moves from the starting screen to the playing state
 *
 * @param window window that is being used
 * @param event user input
 */
void game_start(Window* window, game_input_t event) {
    if (event == TOGGLEPAUSE) {
    	window->game.state = Playing;
    	// Draw background before anything (in both frames)
    	tetris_drawBackground(window);
    	swap_buffer(window);
    	tetris_drawBackground(window);
    	swap_buffer(window);
    	game_playing(window, INPUT_ERROR);
    } else {
        // draw game board
        drawRect_color(window, 0, 0, window->width, window->height, 4, 10, 100);
        draw_str(window, "Welcome To", 2, 10);
        draw_str(window, "Tetris!", 20, 110);
        draw_str(window, "Press Space", 2, 210);
        draw_str(window, "To Continue", 2, 270);
    }
}

/**
 * @brief Use when the tetris game is lost.
 *
 * @param window window that is being used
 * @param event user input
 */
void game_ended(Window* window, game_input_t event) {
	if (event == TOGGLEPAUSE) {
		fall_rate = FALL_INIT;
		window->game.state = Start;
		game_start(window, INPUT_ERROR);
	} else {
		tetris_drawEndScreen(window);
	}
}

/**
 * @brief Swap the image buffers.
 *
 * @param window window with the image buffers.
 */
void swap_buffer(Window * window) {
	// Swap pointers
	uint8_t** tmp = window->true;
	window->true = window->frame;
	window->frame = tmp;

	// Change DMA memory address
	hdac1.DMA_Handle1->Instance->CMAR = (uint32_t) window->true[0];
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
  MX_USART1_UART_Init();
  MX_TIM15_Init();
  MX_RNG_Init();
  /* USER CODE BEGIN 2 */
  create_window(&window);

  HAL_TIM_Base_Start_IT(&htim1);	// start slave first.
  HAL_Delay(100);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);	// start slave first.
  HAL_Delay(100);
  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *) window.true[0], FRAME_WIDTH*FRAME_HEIGHT, DAC_ALIGN_8B_R);
  HAL_Delay(100);
  HAL_TIM_Base_Start(&htim4);	// start master timer.
  HAL_Delay(100);
  HAL_UART_Receive_IT(&huart1, (uint8_t*) in_buf, 1);
  HAL_Delay(100);
  HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
  HAL_Delay(100);
  HAL_TIM_Base_Start_IT(&htim15);	// start slave first.


  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of frameTask */
  frameTaskHandle = osThreadNew(updateGameLogic, NULL, &frameTask_attributes);

  /* creation of soundTask */
  soundTaskHandle = osThreadNew(soundController, NULL, &soundTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){

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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK)
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
  RCC_OscInitStruct.PLL.PLLN = 60;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */
  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */
  /* USER CODE END RNG_Init 2 */

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
  htim4.Init.Prescaler = 12;
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
  * @brief TIM15 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM15_Init(void)
{

  /* USER CODE BEGIN TIM15_Init 0 */

  /* USER CODE END TIM15_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM15_Init 1 */

  /* USER CODE END TIM15_Init 1 */
  htim15.Instance = TIM15;
  htim15.Init.Prescaler = 0;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = 14999;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim15) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim15, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM15_Init 2 */

  /* USER CODE END TIM15_Init 2 */

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
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
/**
 * Push user input into the input queue
 */
void push_input_buffer(game_input_t input) {
	// Can't lock the queue, don't need one since this is called by an interrupt
	input_buffer[buffer_push] = input;
	buffer_push = (buffer_push + 1) % INPUT_BUFFER_SIZE;
}

/**
 * Read from the input queue (private function)
 */
game_input_t pop_input_buffer() {
	if(buffer_pop == buffer_push) {
		return INPUT_ERROR;
	}
	game_input_t ret = input_buffer[buffer_pop];
	buffer_pop = (buffer_pop + 1) % INPUT_BUFFER_SIZE;
	return ret;
}


/**
 * Fetch an instruction from input queue (used for game logic)
 */
game_input_t process_user_input(Window * window) {
    game_input_t c = pop_input_buffer();
    while(c == INPUT_ERROR) c = pop_input_buffer();
    return c;
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	switch (((huart)->Instance)->RDR) {
	case 97: // a
		push_input_buffer(LEFT);
		break;
	case 100: // d
		push_input_buffer(RIGHT);
		break;
	case 107: // k
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
	default:
		break;
	}
	HAL_UART_Receive_IT(&huart1, (uint8_t*) in_buf, 1);
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_updateGameLogic */
/**
  * @brief  Function implementing the frameTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_updateGameLogic */
void updateGameLogic(void *argument)
{
  /* USER CODE BEGIN 5 */
	// Start the Game
	game_start(&window, INPUT_ERROR);
	swap_buffer(&window);

	/* Infinite loop */
	for(;;) {
		//osDelay(1);
	    // process button presses (update game state)
		game_input_t event = process_user_input(&window);

		switch (window.game.state) {
		  case Start:
			  // reset the Game
			  game_start(&window, event);
		  break;
		  case Playing:
			  // update the game state, and draw to frame buffer
			  game_playing(&window, event);

		  break;
		  case Paused:
			  // pause the game
			  game_paused(&window, event);
		  break;
		  case Ended:
			  // go to smile/frown face
			  game_ended(&window, event);
		  break;
		}
		swap_buffer(&window);
	}
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_soundController */
/**
* @brief Function implementing the soundTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_soundController */
void soundController(void *argument)
{
  /* USER CODE BEGIN soundController */
	enum consoleState last_state = Start;
	/* Infinite loop */
	for(;;) {
	osDelay(200);
	switch (window.game.state) {
	    // Update sound playing based on State
		case Start:
			playing_snd = 0;
			last_state = Start;
		break;
		case Playing:
			if (!playing_snd && last_state == Start) {
				// For when special effects happen
				full_snd_data = (uint16_t*) tetris_theme;
				full_snd_data_size = THEME_SOUND_SIZE;
				full_snd_data_offset = 0;

				snd_data_offset = 0;
				snd_data_size = THEME_SOUND_SIZE;
				snd_wave_data = (uint16_t*) tetris_theme;
			}
			playing_snd = 1;
			last_state = Playing;
		break;
		case Paused:
			playing_snd = 0;
			last_state = Paused;
		break;
		case Ended:
			playing_snd = 0;
			last_state = Ended;
		break;
		}
	}
  /* USER CODE END soundController */
}

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
  if (htim->Instance == TIM6) {
    if (HAL_GetTick() % fall_rate == 0) {
    	push_input_buffer(DOWN);
    }
  } else if (htim->Instance == TIM1) {
	vert_count = (vert_count + 1) % 449;
	if(vert_count >= 447) {
		Vert_Synch_GPIO_Port->BRR = (uint32_t)Vert_Synch_Pin;
	} else {
		Vert_Synch_GPIO_Port->BSRR = (uint32_t)Vert_Synch_Pin;
	}
  } else if (htim->Instance == TIM15) {
	  // If playing write value to DAC
	  if(playing_snd) {
		snd_data_offset++;
		// If finished our music data
		if (snd_data_offset >= snd_data_size ) {
			snd_data_offset = full_snd_data_offset;
			full_snd_data_offset = 0;
			// Reset to normal sound
			snd_wave_data = full_snd_data;
			snd_data_size = full_snd_data_size;
		}
	    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, snd_wave_data[snd_data_offset] );
	  }

  }
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

