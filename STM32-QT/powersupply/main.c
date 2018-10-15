/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "main.h"
#include "stm32f0xx_hal.h"

/* USER CODE BEGIN Includes */
#define VIN 	(float)24
#define VOUT 	(float)48
#define constan_pwm_duty (float)(1-(VIN/VOUT))*480
#define adcval_200V 	1485
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

COMP_HandleTypeDef hcomp1;

DAC_HandleTypeDef hdac1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint16_t vdd;
uint32_t counter,sample_counter=0,adc_filter=0,ADC_raw=0;
uint32_t voltage_filter_temp=0;
uint32_t voltage_input_filter_temp=0;
uint32_t current_filter_temp=0;
uint32_t voltage_sample;
uint32_t voltage_input_sample;
uint32_t current_sample;
const float Kp = 0.01;
const float Ki = 0.001;
const float Kd = 0.1;

float Set_Point = 550; //this value for 48V output
float iMax = 200;
float iMin = -200;
float i_Temp=0;
float d_Temp=0;

float pidout;
float PWM_Temp = 0;
float pre_val=0;

uint16_t ADC_RAW[3];
uint8_t adcindex=0;
uint8_t count_pid=0;
int32_t iref=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_COMP1_Init(void);
static void MX_DAC1_Init(void);   
uint16_t voltage_filter(uint32_t filterShift,uint16_t filterinput);
uint16_t voltage_input_filter(uint32_t filterShift,uint16_t filterinput);
uint16_t current_filter(uint32_t filterShift,uint16_t filterinput);
float mypid(float SetPoint, float input);
int32_t pidcal_current(int32_t setpointin, int32_t signalin);
int32_t pidcal_voltage(int32_t setpointin, int32_t signalin);


void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                

int __io_putchar(int ch)
{
	uint8_t c[1];
 	 c[0] = ch & 0x00FF;
 	 HAL_UART_Transmit(&huart1, &*c, 1, 10);
 	 return ch;
}

int _write(int file,char *ptr, int len)
{
 int DataIdx;
 for(DataIdx= 0; DataIdx< len; DataIdx++)
 {
 __io_putchar(*ptr++);
 }
return len;
}

/* USER CODE END PFP */
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */
uint32_t adc_filter_reg = 0;


int32_t adcVal;

//#define SETPOINTCUR 100
#define DUTYCONCUR 120
#define WINDOWCUR 2400000

#define SETPOINTVOL 600
#define DUTYCONVOL 0

#define WINDOWVOL 16000000

//#define WINDOWVOL 240000

int32_t isum_CUR = 0;
uint16_t KP_CUR = 700;
uint16_t KI_CUR = 30;

//int32_t isum_VOL = 0;
//uint16_t KP_VOL = 1000;
//uint16_t KI_VOL = 600;

int32_t isum_VOL = 0;
int32_t KP_VOL = 500000;
int32_t KI_VOL = 0;

//#define QUANG

#ifdef QUANG

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	int32_t duty_uki;
	int32_t adctrigger;

	if(htim->Instance==TIM2)
	{
//		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,240);
//		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,100);
//		duty_uki = 100;
//		duty_off = (duty_uki)/2;
//		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,duty_uki);
//		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,duty_off);
		if(voltage_sample<100)
		{
//			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,100);
			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,0);
			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,1);
//			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
		}
		else
		{
			count_pid++;
			if(count_pid>=10)
			{
				count_pid = 0;
				iref = pidcal_voltage(SETPOINTVOL,voltage_sample);
				//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
			}
			duty_uki = pidcal_current(iref,current_sample);
//			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,100);
			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,duty_uki);
			adctrigger = (duty_uki)/2;
//			duty_off = duty_off + duty_uki;
			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,adctrigger);
		}
	}
}
#else

uint16_t loopCounter = 0;

int32_t vSum = 0;
int32_t iSum = 0;

uint16_t iRef = 0;
uint16_t vRef = 450;

uint16_t vKP = 10;
uint16_t vKI = 70;

uint16_t iKP = 40;
uint16_t iKI = 10;

//#define VSHIFT 11
#define ISHIFT 10

#define VWINDOW (1<<18)
#define IWINDOW (240 << ISHIFT)

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	int32_t duty = 0;



	if(htim->Instance==TIM2) //10kHz
	{
		if (loopCounter++ == 20) // 1kHZ
		{
			loopCounter = 0;
			int32_t ev = vRef - voltage_sample;
			vSum += vKI * ev;
			if (vSum > VWINDOW) vSum = VWINDOW;
			else if (vSum < - VWINDOW) vSum = -VWINDOW;
			int32_t out = ((vKP*ev + vSum)*voltage_input_sample) >> 16;

			if (out > 4096)
			{
				iRef = 4096;
				out = 4096;
			}
			else if (out < 0 ) iRef = 0;
			else iRef = out;
		}

		/*current PID here*/

		int32_t ei = iRef - current_sample;

		//if (e == 0) iSum = 0;
		iSum += iKI * ei;

		if (iSum > IWINDOW ) iSum = IWINDOW;
		else if (iSum < -IWINDOW) iSum = -IWINDOW;

		duty = ((iKP*ei + iSum) >> ISHIFT);

		if (duty < 0) duty = 0;
		if (duty > 300) duty = 300;
//		duty = 300;

		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,duty); 		//apply duty
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,duty/2); 	//trigger source for ADC
	}
}

#endif



int32_t pidcal_current(int32_t setpointin, int32_t signalin)
{
	int32_t duty_out=0;
	int32_t duty_uki;

	int32_t e = setpointin - signalin;

	if (e == 0) isum_CUR = 0;
	else
	{
		isum_CUR += KI_CUR*e;
		if (isum_CUR > WINDOWCUR) isum_CUR = WINDOWCUR;
		if (isum_CUR < -WINDOWCUR) isum_CUR = -WINDOWCUR;
	}

	duty_uki = ((KP_CUR * e + isum_CUR)>>12) ;// + DUTYCONCUR;

	if (duty_uki > 430) duty_uki = 430;
	if (duty_uki < 0 ) duty_uki = 0;

	duty_out = duty_uki;

	return duty_out;
}




int32_t pidcal_voltage(int32_t setpointin, int32_t signalin)
{
	int32_t duty_out=0;
	int32_t duty_uki;

	int32_t e = setpointin - signalin;

	if (e == 0) isum_VOL = 0;
	else
	{
		isum_VOL += KI_VOL*e;
		if (isum_VOL > WINDOWVOL) isum_VOL = WINDOWVOL;
		if (isum_VOL < -WINDOWVOL) isum_VOL = -WINDOWVOL;
	}

	duty_uki = ((KP_VOL * e + isum_VOL)>>12)  + DUTYCONVOL;

	if (duty_uki > 4096) duty_uki = 4096;
	if (duty_uki < 0 ) duty_uki = 0;

	duty_out = duty_uki;

	return duty_out;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
	if(__HAL_ADC_GET_FLAG(hadc,ADC_FLAG_EOC))
	{
		ADC_RAW[adcindex]=HAL_ADC_GetValue(hadc);
		adcindex++;

		voltage_sample = voltage_filter(5,ADC_RAW[0]);
		current_sample = current_filter(3,ADC_RAW[1]);
		voltage_input_sample = voltage_input_filter(5,ADC_RAW[2]);
//		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
	}

	if(__HAL_ADC_GET_FLAG(hadc,ADC_FLAG_EOS))
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
		adcindex=0;
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
	uint8_t count=0;
	uint16_t daccount=0;
	uint32_t duty;
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_ADC_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_IT(&hadc);
  /* USER CODE END 2 */
  HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
  MX_COMP1_Init();
  MX_DAC1_Init();
  HAL_COMP_Start(&hcomp1);
  HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,constan_pwm_duty);
  HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_12B_R,4095);

  HAL_TIM_Base_Start_IT(&htim2);
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  while(1);
//  HAL_TIM_Base_Stop_IT(&htim2);
//  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,0);
  while (1)
  {

//	  if(adcVal<100)
  /* USER CODE END WHILE */
//	  printf("%d, adc value = %lu\r\n",count++,counter);
//	  HAL_Delay(10);
  /* USER CODE BEGIN 3 */
//	  counter = __HAL_TIM_GET_COUNTER(&htim2);
//	  HAL_Delay(1);
//	  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,duty_uki);
//	  pre_val = pidout;
//	  daccount=daccount+100;
//	  if(daccount>=0xfff)  daccount = 100;
  }
  /* USER CODE END 3 */

}

uint16_t voltage_filter(uint32_t filterShift,uint16_t filterinput)
{
	uint16_t filter_output;

	voltage_filter_temp = voltage_filter_temp-(voltage_filter_temp>>filterShift)+filterinput;
	filter_output = voltage_filter_temp>>filterShift;

	return filter_output;
}
uint16_t voltage_input_filter(uint32_t filterShift,uint16_t filterinput)
{
	uint16_t filter_output;

	voltage_input_filter_temp = voltage_input_filter_temp-(voltage_input_filter_temp>>filterShift)+filterinput;
	filter_output = voltage_input_filter_temp>>filterShift;

	return filter_output;
}
uint16_t current_filter(uint32_t filterShift,uint16_t filterinput)
{
	uint16_t filter_output;

	current_filter_temp = current_filter_temp-(current_filter_temp>>filterShift)+filterinput;
	filter_output = current_filter_temp>>filterShift;

	return filter_output;
}

float mypid(float SetPoint, float input)
{
	float Err_Value;
	float P_Term;
	float I_Term;
	float D_Term;
	float result;

//	input = (float)ADC_raw;
	Err_Value = SetPoint - input;
	P_Term = Kp * Err_Value;
//	i_Temp = i_Temp + Err_Value;
	i_Temp = i_Temp + Ki*Err_Value;
	if (i_Temp > iMax)
	{
		i_Temp = iMax;
	}
	else if (i_Temp < iMin)
	{
		i_Temp = iMin;
	}
	I_Term = i_Temp;
	D_Term = 0;//Kd * (d_Temp - Err_Value);
	d_Temp = Err_Value;
	result = (P_Term + I_Term + D_Term)+constan_pwm_duty;
	// PWM overflow prevention
	if (result<0)
	{
		result=0;
	}
	else if (result>300)
	{
		result = 300;
	}
	PWM_Temp = result;
	return result;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC init function */
static void MX_ADC_Init(void)
{
	 ADC_ChannelConfTypeDef sConfig;

	    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	    */
	  hadc.Instance = ADC1;
	  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	  hadc.Init.Resolution = ADC_RESOLUTION_12B;
	  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
	  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	  hadc.Init.LowPowerAutoWait = DISABLE;
	  hadc.Init.LowPowerAutoPowerOff = DISABLE;
	  hadc.Init.ContinuousConvMode = DISABLE;
	  hadc.Init.DiscontinuousConvMode = ENABLE;
	  hadc.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_TRGO;
	  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	  hadc.Init.DMAContinuousRequests = DISABLE;
	  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	  if (HAL_ADC_Init(&hadc) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	   /**Configure for the selected ADC regular channel to be converted.
	    */
	  sConfig.Channel = ADC_CHANNEL_13;
	  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	    /**Configure for the selected ADC regular channel to be converted.
	    */
	  sConfig.Channel = ADC_CHANNEL_14;
	  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	    /**Configure for the selected ADC regular channel to be converted.
	    */
	  sConfig.Channel = ADC_CHANNEL_15;
	  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }


}
/* COMP1 init function */
static void MX_COMP1_Init(void)
{

  hcomp1.Instance = COMP1;
  hcomp1.Init.InvertingInput = COMP_INVERTINGINPUT_1_4VREFINT;
  hcomp1.Init.NonInvertingInput = COMP_NONINVERTINGINPUT_DAC1SWITCHCLOSED;
  hcomp1.Init.Output = COMP_OUTPUT_TIM1OCREFCLR;
  hcomp1.Init.OutputPol = COMP_OUTPUTPOL_INVERTED;
  hcomp1.Init.Hysteresis = COMP_HYSTERESIS_NONE;
  hcomp1.Init.Mode = COMP_MODE_HIGHSPEED;
  hcomp1.Init.WindowMode = COMP_WINDOWMODE_DISABLE;
  hcomp1.Init.TriggerMode = COMP_TRIGGERMODE_NONE;
  if (HAL_COMP_Init(&hcomp1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* DAC1 init function */
static void MX_DAC1_Init(void)
{

  DAC_ChannelConfTypeDef sConfig;

    /**DAC Initialization
    */
  hdac1.Instance = DAC;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**DAC channel OUT1 config
    */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}
/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 480;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_OC_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC4REF;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
  sConfigOC.Pulse = 1000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim1);

}
/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 4;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 480;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}
/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LD4_Pin|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD4_Pin LD3_Pin */
  GPIO_InitStruct.Pin = LD4_Pin|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
