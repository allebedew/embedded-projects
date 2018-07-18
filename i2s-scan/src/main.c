
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
#include "main.h"
#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "bmp.h"

/* USER CODE BEGIN Includes */

#include "lcd.h"
#include "BMP280.h"
#include <string.h>
#include "mpu.h"
#include "MadgwickAHRS.h"

asm(".global _printf_float");
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

#define EEPROM_DEVICE_ADDRESS 0b10100000
// #define EEPROM_DEVICE_ADDRESS 0b10100000

#define QNH 1020
double temp, press, alt;
int8_t com_rslt;

/* Variables for all measurements */
int16_t acc[3] = {};
int16_t gyro[3] = {};
int16_t mag[3] = {};
uint16_t accScale = 0;
float gyroScale = 0.f;
float magScale = 32760.0f / 4912.0f;
float ax, ay, az;
float gx, gy, gz;
float mx, my, mz;
/* Timestamp & status variables */
int16_t intStatus;
uint32_t lastTime, currentTime, drawTime;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
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
    MX_USART2_UART_Init();
    MX_I2C1_Init();
    /* USER CODE BEGIN 2 */

    LCD_Init();

    while (1) {
		LCD_DrawBMP(0, 0, penguin);
		// LCD_DrawBMP(0, 0, tm2);
	}


    while (1)
    {

        LCD_SetCursor(0, 0);
        LCD_SetTextColor(GREEN, BLACK);
        LCD_Printf("Scanning I2C...\n");
        for (uint8_t i = 0x07; i < 0x78; i++)
        {
            if (i % 8 == 0)
            {
                LCD_Printf("\n");
            }
            else
            {
                LCD_Printf(" ");
            }
            uint8_t ok = HAL_I2C_IsDeviceReady(&hi2c1, i << 1, 10, 10);
            if (ok == 0)
            {
                LCD_SetTextColor(GREEN, BLACK);
            }
            else
            {
                LCD_SetTextColor(LIGHTGRAY, BLACK);
            }
            LCD_Printf("%.2x", i);
        }
    }

    LCD_Printf("\nConnecting to BMP280...\n");
    bmp280_t bmp280;
    bmp280.i2c_handle = &hi2c1;
    bmp280.dev_addr = 0x76;
    com_rslt = BMP280_init(&bmp280);
    HAL_Delay(100);
    com_rslt += BMP280_set_power_mode(BMP280_NORMAL_MODE);
    HAL_Delay(100);
    com_rslt += BMP280_set_work_mode(BMP280_STANDARD_RESOLUTION_MODE);
    HAL_Delay(100);
    com_rslt += BMP280_set_standby_durn(BMP280_STANDBY_TIME_1_MS);
    if (com_rslt != SUCCESS)
    {
        LCD_Printf("Check BMP280 connection!\nProgram terminated");
        return 0;
    }
    LCD_Printf("Connection successful!\n");

    HAL_Delay(500);

    while (1)
    {
        /* Read temperature and pressure */
        LCD_SetCursor(0, 0);

        uint32_t t, p;
        BMP280_read_temperature(&t);
        HAL_Delay(100);
        BMP280_read_pressure(&p);
        /* Calculate current altitude, based on current QNH pressure */
        double alt = BMP280_calculate_altitude(QNH * 100);

        LCD_Printf("Temp : %d C\n", t);
        LCD_Printf("Press: %d Pa\n", p);
        LCD_Printf("Alt  : %d m", (int)alt);

        HAL_Delay(250);
    }

    mpu_i2c_init(&hi2c1);
    if (mpu_init(NULL))
    {
        LCD_Printf("\nMPU9250 Init failed. Program terminated");
        return 0;
    }
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
    mpu_set_sample_rate(500);
    mpu_get_accel_sens(&accScale);
    mpu_get_gyro_sens(&gyroScale);
    LCD_Printf("\nConnection successful!\n\n");

    Madgwick_init();
    lastTime = drawTime = HAL_GetTick();

    while (1)
    {
        /* Get current mpu chip status */
        mpu_get_int_status(&intStatus);
        if (intStatus & MPU_INT_STATUS_DATA_READY)
        {
            /* Get all raw measurements */
            mpu_get_accel_reg(acc, 0);
            mpu_get_gyro_reg(gyro, 0);
            mpu_get_compass_reg(mag, 0);
            /* Convert to real units */
            ax = (float)(acc[0]) / accScale;
            ay = (float)(acc[1]) / accScale;
            az = (float)(acc[2]) / accScale;
            gx = gyro[0] / gyroScale;
            gy = gyro[1] / gyroScale;
            gz = gyro[2] / gyroScale;
            mx = mag[1] / magScale;
            my = mag[0] / magScale;
            mz = -mag[2] / magScale;
            /* Do data processing by Madgwick filter */
            currentTime = HAL_GetTick();
            Madgwick_update(gx, gy, gz, ax, ay, az, mx, my, mz, (currentTime - lastTime) / 1000.0);
            lastTime = currentTime;
        }
        /* Draw data on screen every 20ms */
        if (HAL_GetTick() - drawTime > 20)
        {
            LCD_SetCursor(0, 0);
            LCD_Printf("Accel:\n%7.4f %7.4f %7.4f\n", ax, ay, az);
            LCD_Printf("Gyro:\n%7.4f %7.4f %7.4f\n", gx, gy, gz);
            LCD_Printf("Compass:\n%7.1f %7.1f %7.1f\n", mx, my, mz);
            LCD_Printf("Madgwick:\nP: %5.1f R: %5.1f Y: %5.1f\n", Madgwick_getPitch(), Madgwick_getRoll(), Madgwick_getYaw());
            drawTime = HAL_GetTick();
        }
    }

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {

        /* USER CODE END WHILE */

        // 256 000
        // 64

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

    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 180;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    /**Activate the Over-Drive mode 
    */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    /**Configure the Systick interrupt time 
    */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    /**Configure the Systick 
    */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
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
