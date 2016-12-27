/*! @mainpage
 *
 ****************************************************************************
 * Copyright (C) 2016 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : main.c
 *
 * Usage: GMA303 Motion Algorithm demo
 *
 ****************************************************************************
 * @section License
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 **************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include "nrf_drv_clock.h"
#include "nrf_drv_timer.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "app_error.h"
#include "app_uart.h"
#include "app_timer.h"
#include "boards.h"
#include "nrf.h"
#include "bsp.h"
#include "gma303.h"
#include "app_twi.h"
#include "gSensor_autoNil.h"
#include "motion_main_ctrl.h"
#include "misc_util.h"

#define STOP_NRT_TIMER(m_timer) (nrf_drv_timer_disable(&m_timer);nrf_drv_timer_uninit(&m_timer);)

#define UART_TX_BUF_SIZE            256                  // UART TX buffer size
#define UART_RX_BUF_SIZE            1                    // UART RX buffer size
#define MAX_PENDING_TRANSACTIONS    5                    // TWI (I2C)
#define APP_TIMER_PRESCALER_BSP     0                    // BSP buttons APP timer          
#define APP_TIMER_OP_QUEUE_SIZE_BSP 2                    // BSP buttons APP timer
#define DELAY_MS(ms)	            nrf_delay_ms(ms)
#define SAMPLING_RATE_HZ            MOTION_ALG_DATA_RATE_HZ     //sensor sampling rate
#define ACC_LAYOUT_PATTERN          PAT6                 //accelerometer layout pattern


const nrf_drv_timer_t m_timer_periodic_measure = NRF_DRV_TIMER_INSTANCE(0);
static app_twi_t m_app_twi = APP_TWI_INSTANCE(0);
static uint8_t ui8StartAutoNilFlag = 0;
static uint8_t ui8PeriodicMeasureFlag = 0;
static uint32_t ui32SamplingRateHz = SAMPLING_RATE_HZ;
static float fTimeMs = 0.0f, fDeltaTus = 1000000.0f / SAMPLING_RATE_HZ;
static const char* activityStr[] = {"Stationary", "Walk", "?", "Run"};

static void event_handler_uart(app_uart_evt_t * p_event){

  uint8_t cr;

  switch (p_event->evt_type){

  case APP_UART_DATA_READY: //echo

    while(app_uart_get(&cr) == NRF_SUCCESS){
      printf("%c", cr);
      if(cr == 'y' || cr == 'Y'){
	ui8StartAutoNilFlag = 1;
      }
    }

    break;
  case APP_UART_TX_EMPTY:
    //do nothin
    break;
  case APP_UART_COMMUNICATION_ERROR:
    APP_ERROR_HANDLER(p_event->data.error_communication);
    break;
  case APP_UART_FIFO_ERROR:
    APP_ERROR_HANDLER(p_event->data.error_code);
    break;

  default:
    break;
  }
}

static void event_handler_timer_periodic_measure(nrf_timer_event_t event_type, void* p_context)
{
  //raise the flag
  ui8PeriodicMeasureFlag = 1;

  //update the time
  fTimeMs += fDeltaTus / 1000.0F;
}

static void event_handler_motion_alg(motion_algorithm_t event, int32_t i32Data)
{

  switch(event){
  case MOTION_ALG_PEDO:
    printf("Step:%d\n", i32Data);
    break;
  case MOTION_ALG_CALORIE:
    printf("Calorie:%d\n", i32Data);
    break;
  case MOTION_ALG_ACTIVITY:
    //0: Stationary
    //1: Walk
    //3: Run
    printf("Activity:%d\n", i32Data);
    break;
  case MOTION_ALG_FALL:
    //0: No falldown
    //1: Falldown occur
    printf("Fall:%d\n", i32Data);
    break;
  case MOTION_ALG_SHAKE:
    //1: X+
    //2: X-
    //4: Y+
    //8: Y-
    //16: Z+
    //32: Z-
    printf("Shake:%d\n", i32Data);
    break;
  case MOTION_ALG_RAISE_HAND:
    //0: No hand-raise
    //1: Hand-raise occur
    printf("Raise hand:%d\n", i32Data);
    break;
  case MOTION_ALG_FLIP:
    //0: No flip
    //1: Flip occur
    printf("Flip:%d\n", i32Data);
    break;
  case MOTION_ALG_SEDENTARY:
    //0: No sedentary
    //1: Sedentary occur
    printf("Sedentary:%d\n", i32Data);    
    break;
  case MOTION_ALG_SLEEP_CYCLE:
    //0: MOTION_SLEEP_CYCLE_WAKE
    //1: MOTION_SLEEP_CYCLE_S1
    //2: MOTION_SLEEP_CYCLE_REM
    //3: MOTION_SLEEP_CYCLE_S2
    //4: MOTION_SLEEP_CYCLE_S3
    //5: MOTION_SLEEP_CYCLE_NONE
    printf("Sleep cycle:%d\n", i32Data);
    break;
  default:
    printf("Unknown event:%d\n", i32Data);
    break;
  }

}

void init_lfclk(void){

  uint32_t err_code;

  // Start internal LFCLK XTAL oscillator - it is needed by BSP to handle
  // buttons with the use of APP_TIMER

  err_code = nrf_drv_clock_init(NULL);
  APP_ERROR_CHECK(err_code);
  nrf_drv_clock_lfclk_request();

}

void init_uart(void)
{
  uint32_t err_code;

  app_uart_comm_params_t const comm_params =
    {
      RX_PIN_NUMBER,
      TX_PIN_NUMBER,
      RTS_PIN_NUMBER,
      CTS_PIN_NUMBER,
      APP_UART_FLOW_CONTROL_DISABLED,
      false,
      UART_BAUDRATE_BAUDRATE_Baud115200
    };

  APP_UART_FIFO_INIT(&comm_params,
		     UART_RX_BUF_SIZE,
		     UART_TX_BUF_SIZE,
		     event_handler_uart,
		     APP_IRQ_PRIORITY_LOW,
		     err_code);

  APP_ERROR_CHECK(err_code);
}

/**
 * Initialize two wire interface (I2C)
 */
void init_twi(nrf_twi_frequency_t clk){

  uint32_t err_code;

  nrf_drv_twi_config_t const config = {
    .scl                = ARDUINO_SCL_PIN,
    .sda                = ARDUINO_SDA_PIN,
    .frequency          = clk,
    .interrupt_priority = APP_IRQ_PRIORITY_LOW
  };

  APP_TWI_INIT(&m_app_twi, &config, MAX_PENDING_TRANSACTIONS, err_code);
  APP_ERROR_CHECK(err_code);

}

void init_timer_periodic_measure(uint32_t time_us,
				 nrf_timer_event_handler_t timer_event_handler,
				 void* p_user_data)
{

  uint32_t time_ticks;
  uint32_t err_code = NRF_SUCCESS;
  nrf_drv_timer_config_t m_nrf_timer_config = {
    .frequency = TIMER0_CONFIG_FREQUENCY,
    .mode = TIMER0_CONFIG_MODE,
    .bit_width = TIMER0_CONFIG_BIT_WIDTH,
    .interrupt_priority = TIMER0_CONFIG_IRQ_PRIORITY,
    .p_context = p_user_data
  };
    
  err_code = nrf_drv_timer_init(&m_timer_periodic_measure,
				&m_nrf_timer_config,
				timer_event_handler);
  APP_ERROR_CHECK(err_code);
    
  time_ticks = nrf_drv_timer_us_to_ticks(&m_timer_periodic_measure, time_us);

  nrf_drv_timer_extended_compare(&m_timer_periodic_measure,
				 NRF_TIMER_CC_CHANNEL0,
				 time_ticks,
				 NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
				 true);
    
  nrf_drv_timer_enable(&m_timer_periodic_measure);

}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{

  uint8_t i;
  bus_support_t gma303_bus;
  raw_data_xyzt_t rawData;
  raw_data_xyzt_t offsetData;
  float_xyzt_t gVal;
  uint32_t ui32StepCount = 0, ui32StepCount_pre = 0;
  uint8_t ui8Activity = 0, ui8Activity_pre = 0;
  float fCal = 0.0;

  //Config and initialize LFCLK
  init_lfclk();

  //Config. and initialize UART
  init_uart();

  //Config. and initialize TWI (I2C)
  init_twi(NRF_TWI_FREQ_400K);
	
  /* GMA303 I2C bus setup */
  bus_init_I2C(&gma303_bus, &m_app_twi, GMA303_7BIT_I2C_ADDR);
  gma303_bus_init(&gma303_bus);

  /* GMA303 soft reset */
  gma303_soft_reset();

  /* GMA303 initialization */
  gma303_initialization();

  /* GMA303 Offset AutoNil */
  printf("Place and hold g-sensor in level for offset AutoNil.\r"); 
  printf("Press y when ready.\n");

  do{
    sd_app_evt_wait();
  }while(ui8StartAutoNilFlag == 0);

  //Conduct g-sensor AutoNil, g is along the Z-axis
  gSensorAutoNil(gma303_read_data_xyz,
		 AUTONIL_AUTO + AUTONIL_Z,
		 GMA303_RAW_DATA_SENSITIVITY,
		 &offsetData);

  printf("Offset_XYZ=%d,%d,%d\n", offsetData.u.x, offsetData.u.y, offsetData.u.z);

  // Pedometer Demo
  printf("Motion demo\n\n");

  //Initialize the motion algorithm main control
  motion_alg_init(event_handler_motion_alg);
  
  //Enable the algorithm
  motion_alg_enable(MOTION_ALG_PEDO | 
		    MOTION_ALG_CALORIE | 
		    MOTION_ALG_ACTIVITY | 
		    MOTION_ALG_FALL | 
		    MOTION_ALG_SHAKE | 
		    MOTION_ALG_RAISE_HAND | 
		    MOTION_ALG_FLIP | 
		    MOTION_ALG_SEDENTARY | 
		    MOTION_ALG_SLEEP_CYCLE
		    , 1);

  //set calorie parameters: height(m) and weight(kg)
  motion_calorie_set_param(1.8, 75.0);

  //set shake parameters: threshold(g), peak duration, peak count, time-out, axes enable
  motion_shake_set_param(0.7, 1, 2, 1.5, X_AXIS|Y_AXIS|Z_AXIS);	

  //set sedentary time: monitor time(min), snooze time(min)
  motion_sedentary_set_param(30, 10);

  //init the timer
  init_timer_periodic_measure(fDeltaTus, event_handler_timer_periodic_measure, NULL);

  while(1){
      
    if(ui8PeriodicMeasureFlag){

      ui8PeriodicMeasureFlag = 0;

      // Read XYZT raw data
      gma303_read_data_xyzt(&rawData);

      //offset compensation and code to g
      for(i = 0; i < 3; ++i)
	gVal.v[i] = (float)(rawData.v[i] - offsetData.v[i]) / GMA303_RAW_DATA_SENSITIVITY;

      //Rotate to the Android Coordinate
      coord_rotate_f(ACC_LAYOUT_PATTERN, &gVal);

      //feed to motion process
      motion_alg_process_data(gVal);

    }
    else{

      sd_app_evt_wait();

    }
  }
}
