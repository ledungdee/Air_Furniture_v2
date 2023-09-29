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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define Humi_Set 70.0
#include "string.h"
#include "stdio.h"
#include "delay.h"
#include "ST7920_SERIAL.h"
#include "PMS7003_HAL_STM32.h"
#include "DHT22.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void sensor_init(void);
void greeting_Func();
void LCD_disp();
void disp_SensorValue();
void disp_Sys_Info();
void disp_Rec_Act();
void readPMS();
void readDHT();
void check_Btn_Pushed();
void check_Water_Out();
void SLOW_TIM_SET_COMPARE(uint8_t PWM_Target,uint8_t *PWM_INC_From);
void switch_Mode(uint8_t mode);
void humi_Adding_Func();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

UART_HandleTypeDef huart1;
PMS_typedef PMS7003 = {0};
uint32_t PMS7003_Interval = 0;

uint8_t init_Wheel = 0;
char mesg[5] = {0};

uint8_t hum1, hum2, tempC1, tempC2, SUM, CHECK;
float temp_Celsius = 0;
float humidity = 0;
uint8_t hum_integral, hum_decimal, tempC_integral, tempC_decimal;
char string[15];
char value_string[15];
uint32_t DHT_Interval = 0;

		
int32_t btn_Interval = 0xFFFFFA24;  										// -1500: vua khoi dong van nhan nut nhan duoc
uint8_t LED_Pc13_State, LED_Water_Alert_State = 0;
uint8_t is_Btn_Pushed = 0;
uint32_t switch_Mode_Interval = 0;
uint32_t air_Status_Interval = 0;
uint8_t PWM_Value = 0;

//uint8_t PWM_INC_From = 50;
//uint8_t PWM_Change_Wide_Range = 0;



uint8_t count_Hall2_Low = 200;
//uint32_t count_For_Water = 50;
uint8_t count_For_Water_Out = 0;
uint16_t LED_Water_Out_Interval = 0;
uint8_t pump_Status = 0;

uint8_t humi_Adding = 1;
int32_t humi_Adding_Int = 0xFFFF15A0; // humi Adding checking Interval Init = -60s
uint8_t AC_Motor_State = 0;
uint8_t humi_Delay = 0;

typedef enum{
	SENSOR_VALUE = 1,
	SYS_INFO = 2,
	REC_ACT = 3,
}disp_Mode;
disp_Mode dMode = 1;
uint8_t dispMode_Was_Changed = 0;

typedef enum{
	GOOD = 0,
	MEDIUM = 1,
	BAD = 2,
	TERRIBLE = 3,
}air_Status;
air_Status a_Status;

typedef enum{
	AUTO = 0,
	MANUAL = 1,
	TURBO = 2,
	SLEEP = 3,
	ONLY_SENSOR = 4,
}sys_Mode;
sys_Mode sMode = 0;

uint8_t f_Time_Manual = 1;
typedef enum{
	ZERO = 0,
	LOWER = 1,	 // PWM 40% => 2.68V
	LOW = 2,		 // PWM 47% => 3.15V
	MED =3,			 // PWM 54% => 3.54V
	HIGH = 4,		 // PWM 60% => 3.82V
	HIGHER = 5	 // PWM 70% => 4.2V
}fan_Speed;	
fan_Speed fSpeed = 1;


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin == GPIO_PIN_3){
			if(HAL_GetTick() - btn_Interval >= 1000 && (sMode == MANUAL || sMode == TURBO ||sMode == SLEEP)){
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			is_Btn_Pushed = 1;
			btn_Interval = HAL_GetTick();
			if(humi_Adding == 1) humi_Adding = 0;
			else humi_Adding = 1;
		}
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
	HAL_NVIC_ClearPendingIRQ(EXTI3_IRQn);
	}
	
	if(GPIO_Pin == GPIO_PIN_4){
		if(HAL_GetTick() - btn_Interval >= 1000 && sMode == MANUAL){
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			is_Btn_Pushed = 1;
			btn_Interval = HAL_GetTick();
			fSpeed++;
			if(fSpeed > 5){		
				fSpeed = 1;
			}
		}
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
	HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
	}
	
	if(GPIO_Pin == GPIO_PIN_6){
		if(HAL_GetTick() - btn_Interval >= 1000){
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			is_Btn_Pushed = 1;
			btn_Interval = HAL_GetTick();
			sMode ++;
			if(sMode > 4){
				sMode = 0;
			}
		}
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
	HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	}
	
	if(GPIO_Pin == GPIO_PIN_7){
		if(HAL_GetTick() - btn_Interval >= 1000){
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			is_Btn_Pushed = 1;
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_11);
			btn_Interval = HAL_GetTick();
		}
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_7);
	HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	}
	
	if(GPIO_Pin == GPIO_PIN_8){
		if(HAL_GetTick() - btn_Interval >= 1000){
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			is_Btn_Pushed = 1;
			dispMode_Was_Changed = 1;
			dMode++;
			if(dMode > 3){
				dMode = 1;
			}
			btn_Interval = HAL_GetTick();
		}
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_8);
	HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	}
	
	if(GPIO_Pin == GPIO_PIN_12){
		if(HAL_GetTick() - btn_Interval >= 200){
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			is_Btn_Pushed = 1;
			
			
			if(init_Wheel == 0 && AC_Motor_State == 1){
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 0);
				AC_Motor_State = 0;
				init_Wheel = 1;				
			}
			if((humi_Adding == 0 || humi_Delay == 1) && AC_Motor_State == 1){
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 0);
				AC_Motor_State = 0;
			}
			
			
			btn_Interval = HAL_GetTick();
		}
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
	HAL_NVIC_ClearPendingIRQ(	EXTI15_10_IRQn);
	}	
}

void greeting_Func(){
	ST7920_SendString(0,0,"DO AN TOT NGHIEP");
	ST7920_SendString(1,0,"****************");
	ST7920_SendString(2,1,"LE VAN DUNG");
	ST7920_SendString(3,2,"1912950");
	delay_ms(4000);
	ST7920_Clear();
}

void PMS7003_Init(void)
{
	PMS7003.PMS_huart = &huart1; 					// passing uart handler to communicate with sensor
	PMS7003.PMS_MODE = PMS_MODE_PASSIVE;	// choosing the MODE
	PMS_Init(&PMS7003);
}

void readPMS(){
		if(PMS_read(&PMS7003) == PMS_OK){
			PMS7003_Interval = HAL_GetTick();
		}
}

void readDHT(){
	 if (DHT22_Start()){
			hum1 = DHT22_Read(); 
	    hum2 = DHT22_Read(); 
	    tempC1 = DHT22_Read();
	    tempC2 = DHT22_Read(); 
	    SUM = DHT22_Read(); 
	    CHECK = hum1 + hum2 + tempC1 + tempC2;
	    if (CHECK == SUM){
	        if (tempC1>127){
							temp_Celsius = (float)((tempC1<<8)|tempC2)/10*(-1);
	         }
	         else{
							temp_Celsius = (float)((tempC1<<8)|tempC2)/10;
	         }
	         humidity = (float)((hum1<<8)|hum2)/10;
	         hum_integral = humidity;  
	         hum_decimal = humidity*10-hum_integral*10;         
			}
		}
}

void LCD_disp(){
	if (dMode == SENSOR_VALUE){
		disp_SensorValue();
	}
	else if(dMode == SYS_INFO){
		disp_Sys_Info();
	}
	else disp_Rec_Act();
}
void disp_SensorValue(){
			if(dispMode_Was_Changed){
				ST7920_Clear();
				dispMode_Was_Changed = 0;
			}	
			sprintf(string,"H: %d.%d", hum_integral, hum_decimal);
			ST7920_SendString(0,0,string);
	
			if (temp_Celsius < 0){
						tempC_integral = temp_Celsius *(-1);  
						tempC_decimal = temp_Celsius*(-10)-tempC_integral*10; 
						sprintf(string,"T:%d.%d", tempC_integral, tempC_decimal);
						ST7920_SendString(0,5,string);	
			}
			else{
	          tempC_integral = temp_Celsius; 
	          tempC_decimal = temp_Celsius*10-tempC_integral*10; 
	          sprintf(string,"T:%d.%d", tempC_integral, tempC_decimal);
						ST7920_SendString(0,5,string);		
			}  		
	
			ST7920_SendString(1,0,"PM1.0:" );
			sprintf(mesg,"%d  ",PMS7003.PM1_0_atmospheric);
			ST7920_SendString(1,3,mesg);
			ST7920_SendString(1,5,"ug/m3" );
			
			ST7920_SendString(2,0,"PM2.5:" );
			sprintf(mesg,"%d  ",PMS7003.PM2_5_atmospheric);
			ST7920_SendString(2,3,mesg);
			ST7920_SendString(2,5,"ug/m3" );
			
			ST7920_SendString(3,0,"PM10 :" );
			sprintf(mesg,"%d  ",PMS7003.PM10_atmospheric);
			ST7920_SendString(3,3,mesg);
			ST7920_SendString(3,5,"ug/m3" );	
}

void disp_Sys_Info(){
		if(dispMode_Was_Changed){
			ST7920_Clear();;
			dispMode_Was_Changed = 0;
		}
		
		switch (a_Status){
			case MEDIUM:
			{
				strcpy(value_string, " MEDIUM   ");
				break;
			}
			case BAD:
			{
				strcpy(value_string, " BAD      ");				
				break;
			}
			case TERRIBLE:
			{
				strcpy(value_string, " TERRIBLE ");				
				break;
			}
			default:
			{
				strcpy(value_string, " GOOD     ");
				break;
			}
		}
		sprintf(string,"AIR_Q:%s",value_string);
		ST7920_SendString(0,0,string);	


		switch (sMode){
			case AUTO:
			{
				strcpy(value_string, "   AUTO    ");
				break;
			}
			case MANUAL:
			{
				strcpy(value_string, "  MANUAL   ");				
				break;
			}
			case TURBO:
			{
				strcpy(value_string, "  TURBO    ");				
				break;
			}
			case SLEEP:
			{
				strcpy(value_string, "  SLEEP    ");				
				break;
			}			
			default:
			{
				strcpy(value_string, "ONLY_SENSOR");
				break;
			}
		}
		sprintf(string,"MODE:%s",value_string);
		ST7920_SendString(1,0,string);		
		
		switch (fSpeed){
			case ZERO:
			{
				strcpy(value_string, "  ZERO     ");
				break;
			}
			case LOWER:
			{
				strcpy(value_string, "  LOWER    ");				
				break;
			}
			case LOW:
			{
				strcpy(value_string, "   LOW     ");				
				break;
			}
			case MED:
			{
				strcpy(value_string, "  MEDIUM   ");				
				break;
			}
			case HIGH:
			{
				strcpy(value_string, "  HIGH     ");				
				break;
			}
			default:
			{
				strcpy(value_string, "  HIGHEST  ");
				break;
			}
		}
		sprintf(string,"FAN :%s",value_string);
		ST7920_SendString(2,0,string);			

		
		if(humi_Adding == 1)
			strcpy(value_string, " YES  ");
		else  
			strcpy(value_string, " NO   ");
		sprintf(string,"HUMI_COMP:%s",value_string);
		ST7920_SendString(3,0,string);				
}

void disp_Rec_Act(){
			if(dispMode_Was_Changed){
				ST7920_Clear();
				dispMode_Was_Changed = 0;
			}
			ST7920_SendString(0,0,"Disp Mode 3");			
}
/* Func for reading satatus of HALL sensor 1 to GPIO PA12 (Shape: Rectangle): Use for detect the lowest position of wheel humidity compensation 
void stop_Humd_Compensation(){
	if(!HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_12) && !LED_Pc13_State){
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,0);
				LED_Pc13_State = 1;
		}
		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_12) && LED_Pc13_State){
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,1);
				LED_Pc13_State = 0;
		}			
		
}
*/

void check_Btn_Pushed(){
	if(is_Btn_Pushed){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);
		delay_ms(100);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0);
		is_Btn_Pushed = 0;
	}
}


/*
void check_Water_Out(){
	
		if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15) == 0){
			if(count_Hall2_Low < 60){
				count_Hall2_Low ++;
			}
		}
		else count_Hall2_Low = 0;
		
		
		if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15) == 0 && count_Hall2_Low > 50){ 
					if(LED_Water_Alert_State == 1){
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,0);
						LED_Water_Alert_State = 0;
					}
		}
		
		else if(HAL_GetTick() - count_For_Water > 500)
		{
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
			count_For_Water = HAL_GetTick();
			//if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET){ 
			LED_Water_Alert_State = 1;
			//}
		}
}
*/

/* Hall_2 sensor is not stable, it's mean without magnetic, it maybe low or high. It is only low when have magnetic */
void check_Water_Out(){
	
		if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15) == 0){
			if(count_Hall2_Low < 200){		// tang bien dem len vi cam bien khong on dinh
				count_Hall2_Low ++;
			}
		}
		else count_Hall2_Low = 0;				// reset bien dem neu nhan muc 1
		
		if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15) == 0 && count_Hall2_Low > 190){  // xac nhan het nuoc
			count_For_Water_Out ++;
			if(HAL_GetTick() - LED_Water_Out_Interval >= 500 && LED_Water_Alert_State == 0){
				HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
				LED_Water_Out_Interval = HAL_GetTick();
				LED_Water_Alert_State = 1;
			}
			
		}
		else if(LED_Water_Alert_State == 1) {
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,0);
			LED_Water_Alert_State = 0;
		}
}

void update_LED_Status(air_Status i){
	if( i == GOOD || i == MEDIUM){
		HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, 0);
		delay_ms(10);
		HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, 1);
	}
	else{
		HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, 0);
		delay_ms(10);
		HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, 1);	
	}
}


air_Status update_Air_Status(int pm1, int pm25, int pm10){
	if(pm1 <= 12 && pm25 <= 12 && pm10 <= 54){
		return GOOD;
	}
	else if(pm1 >= 56 && pm25 >= 56 && pm10 >= 255) return TERRIBLE;
	
	else if( (pm1>=13 && pm1 < 35 && pm25 < 35 & pm10 < 154) || (pm25 >= 13 && pm25 < 35 && pm1 < 35 && pm10 < 154) ||( pm10 >=55 && pm10 < 154 && pm1 < 35 && pm25 < 35))
		return MEDIUM;
	else return BAD;
	

}
/*
void SLOW_TIM_SET_COMPARE(uint8_t PWM_Target,uint8_t *PWM_INC_From){
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,*PWM_INC_From);
	if (*PWM_INC_From < PWM_Target){
		*PWM_INC_From = (*PWM_INC_From) + 1;
	}
}
*/

fan_Speed update_Fanspeed_Auto(air_Status i){
	if(i == GOOD){
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,47);
		return LOW;
	}
	else if(i == MEDIUM){
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,47);
		return MED;
	}
	else if(i == BAD){
		//PWM_Change_Wide_Range = 1;
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,60);
		//SLOW_TIM_SET_COMPARE(60, &PWM_INC_From);
		return HIGH;
	}
	else{
		//PWM_Change_Wide_Range = 1;
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,70);		
		//SLOW_TIM_SET_COMPARE(70, &PWM_INC_From);
		return HIGHER;
	}
}
/*
	LOWER = 1,	 // PWM 40% => 2.68V
	LOW = 2,		 // PWM 47% => 3.15V
	MED = 3,		 // PWM 54% => 3.54V
	HIGH = 4,		 // PWM 60% => 3.82V
	HIGHER = 5	 // PWM 70% => 4.2V
*/
void update_Fanspeed_Manual(fan_Speed i){
	if(i == LOWER ){
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,40);
	}
	else if(i == LOW){
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,47);
	}
	else if(i == MED){
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,54);	
	}
	else if(i == HIGH){
		//PWM_Change_Wide_Range = 1;
		//SLOW_TIM_SET_COMPARE(60, &PWM_INC_From);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,60);
	}
	else{
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,70);
		//PWM_Change_Wide_Range = 1;
		//SLOW_TIM_SET_COMPARE(70, &PWM_INC_From);
	}
	f_Time_Manual = 0;
}


void switch_Mode(uint8_t mode){
	switch(mode){
		case MANUAL:
		{
			if(f_Time_Manual == 1){
				fSpeed = LOWER;
			}
			update_Fanspeed_Manual(fSpeed);
			if(humi_Adding){
				humi_Adding_Func();		
			}
			break;
		}
		case TURBO:
		{
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,70);
			fSpeed = HIGHER;
			if(humi_Adding){
				humi_Adding_Func();
			}			
			break;
		}
		case SLEEP:
		{
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,40);
			fSpeed = LOWER;
			if(humi_Adding){
				humi_Adding_Func();
			}				
			break;
		}
		case ONLY_SENSOR:
		{
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,0);
		fSpeed = ZERO;
		humi_Adding = 0;
			break;
		}
		
		default:
		{
			
			fSpeed = update_Fanspeed_Auto(a_Status);
			humi_Adding = 1;
			f_Time_Manual = 1;
			humi_Adding_Func();
			break;
		}
	}
}

void humi_Adding_Func(){
	if(humidity <= Humi_Set && AC_Motor_State == 0 && HAL_GetTick() - humi_Adding_Int >= 3000){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 1);
		AC_Motor_State = 1;
		humi_Delay = 0;
		humi_Adding_Int = HAL_GetTick();
	}
	if( (humidity > Humi_Set) && AC_Motor_State == 1 && HAL_GetTick() - humi_Adding_Int >= 3000){	
		humi_Delay = 1;		
		humi_Adding_Int = HAL_GetTick();
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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);		//PWM default on wake up 40%\

	
	PMS7003_Init();
	delay_init();
	ST7920_Init();
	greeting_Func();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	PMS7003_Interval = HAL_GetTick();
	DHT_Interval = HAL_GetTick();
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 1);
	AC_Motor_State =  1;
  while(1){
		if(HAL_GetTick() - PMS7003_Interval >= 1500){
			readPMS();
		}
		
		if(HAL_GetTick() - DHT_Interval >= 1500){
			readDHT();
			DHT_Interval = HAL_GetTick();
		}
		
		LCD_disp();
		check_Water_Out();
		check_Btn_Pushed();
		
		if(HAL_GetTick() - air_Status_Interval >= 3000){
			a_Status = update_Air_Status(PMS7003.PM1_0_atmospheric,PMS7003.PM2_5_atmospheric,PMS7003.PM10_atmospheric);
			update_LED_Status(a_Status);
			air_Status_Interval = HAL_GetTick();
		}		
		
		if(HAL_GetTick() - switch_Mode_Interval >= 5000){
			switch_Mode(sMode);
			switch_Mode_Interval = HAL_GetTick();
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 100;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 40;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
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
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

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

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xffff-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_R_Pin|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|EN_AC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Buzzer_Pin|EN_Valve_Pin|EN_DC_Pin|LED_Water_Alert_Pin
                          |DHT22_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_G_Pin LED_R_Pin PA4 PA5
                           PA6 PA7 EN_AC_Pin */
  GPIO_InitStruct.Pin = LED_G_Pin|LED_R_Pin|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|EN_AC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Buzzer_Pin EN_Valve_Pin EN_DC_Pin LED_Water_Alert_Pin
                           DHT22_Pin */
  GPIO_InitStruct.Pin = Buzzer_Pin|EN_Valve_Pin|EN_DC_Pin|LED_Water_Alert_Pin
                          |DHT22_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : HALL_1_EXTI_Pin */
  GPIO_InitStruct.Pin = HALL_1_EXTI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(HALL_1_EXTI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : HALL_2_Pin */
  GPIO_InitStruct.Pin = HALL_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(HALL_2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Btn_1_Pin Btn_2_Pin Btn_3_Pin Btn_4_Pin
                           Btn_5_Pin */
  GPIO_InitStruct.Pin = Btn_1_Pin|Btn_2_Pin|Btn_3_Pin|Btn_4_Pin
                          |Btn_5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
