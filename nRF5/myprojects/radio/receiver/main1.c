/**
 * Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/** @file
*
* @defgroup nrf_dev_radio_rx_example_main main.c
* @{
* @ingroup nrf_dev_radio_rx_example
* @brief Radio Receiver example Application main file.
*
* This file contains the source code for a sample application using the NRF_RADIO peripheral.
*
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
//#include "nrf.h"
#include "bsp.h"
#include "nrf_uart.h"

#if 0
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "nrf_delay.h"

#include "radio_config.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "bsp.h"
#include "app_timer.h"
#include "nordic_common.h"
#include "nrf_error.h"
#include "app_error.h"

#include "app_uart.h"
#include "nrf_uart.h"
#endif

//#define ENABLE_LOOPBACK_TEST  /**< if defined, then this example will be a loopback test, which means that TX should be connected to RX to get data loopback. */

#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */

#define UART_HWFC APP_UART_FLOW_CONTROL_ENABLED



#define LED1_ON          	nrf_gpio_pin_clear(17)
#define LED1_OFF          	nrf_gpio_pin_set(17)
#define LED1_toggle         nrf_gpio_pin_toggle(17)

#define LED2_ON          	nrf_gpio_pin_clear(18)
#define LED2_OFF          	nrf_gpio_pin_set(18)
#define LED2_toggle         nrf_gpio_pin_toggle(18)

#define RELAY1_OFF         	nrf_gpio_pin_clear(12)
#define RELAY1_ON        	nrf_gpio_pin_set(12)
#define RELAY1_toggle      	nrf_gpio_pin_toggle(12)

#define RELAY2_OFF         	nrf_gpio_pin_clear(16)
#define RELAY2_ON        	nrf_gpio_pin_set(16)
#define RELAY2_toggle      	nrf_gpio_pin_toggle(16)

#define SW1_init			nrf_gpio_cfg_input(14, BUTTON_PULL)	 
#define SW2_init			nrf_gpio_cfg_input(13, BUTTON_PULL)	 
#define SW1_read        	nrf_gpio_pin_read(14)
#define SW2_read         	nrf_gpio_pin_read(13)



static uint32_t                   packet;              /**< Packet to transmit. */

/**@brief Function for initialization oscillators.
 */
void clock_initialization()
{
    /* Start 16 MHz crystal oscillator */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    /* Wait for the external oscillator to start up */
	nrf_delay_ms(1);
	#if 0
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
    {
        // Do nothing.
    }
	#endif
    /* Start low frequency crystal oscillator for app_timer(used by bsp)*/
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;
	nrf_delay_ms(1);
	#if 0
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
        // Do nothing.
    }
	#endif
}


/**@brief Function for reading packet.
 */
uint32_t read_packet()
{
    uint32_t result = 0;

    NRF_RADIO->EVENTS_READY = 0U;
    // Enable radio and wait for ready
    NRF_RADIO->TASKS_RXEN = 1U;

    while (NRF_RADIO->EVENTS_READY == 0U)
    {
        // wait
    }
    NRF_RADIO->EVENTS_END = 0U;
    // Start listening and wait for address received event
    NRF_RADIO->TASKS_START = 1U;

    // Wait for end of packet or buttons state changed
    while (NRF_RADIO->EVENTS_END == 0U)
    {
        // wait
    }

    if (NRF_RADIO->CRCSTATUS == 1U)
    {
        result = packet;
    }
    NRF_RADIO->EVENTS_DISABLED = 0U;
    // Disable radio
    NRF_RADIO->TASKS_DISABLE = 1U;

    while (NRF_RADIO->EVENTS_DISABLED == 0U)
    {
        // wait
    }
    return result;
}

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

void uart_init()
{
	uint32_t err_code;
	
	
	const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
          NRF_UART_BAUDRATE_115200
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOWEST,
                         err_code);
	 APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for application main entry.
 * @return 0. int return type required by ANSI/ISO standard.
 */
int main(void)
{
	
	//uint8_t sw1_flag=0;
	//uint8_t sw2_flag=0;
	
    clock_initialization();

    //app_timer_init();

	uart_init();
	
						 
	#if 0
    //bsp_init(BSP_INIT_LED, NULL);
	
	SW1_init;
	SW2_init;
	nrf_gpio_cfg_output(17);
	nrf_gpio_cfg_output(16);
	nrf_gpio_cfg_output(12);
	nrf_gpio_cfg_output(18);
	
   

    // Set radio configuration parameters
    radio_configure();
    NRF_RADIO->PACKETPTR = (uint32_t)&packet;
	#endif
    //bsp_indication_set(BSP_INDICATE_USER_STATE_OFF);

	printf("\r\n RF Start...: \r\n");
	
	while (true);
    while (true)
    {
		#if 0
		if(SW1_read==0)
		{
			nrf_delay_ms(50);
			if((SW1_read==0)&&(sw1_flag==0))
			{
				LED1_toggle;
				RELAY1_toggle;
				sw1_flag = 1;
			}
		}
		else
		{
			sw1_flag = 0;
		}
		if(SW2_read==0)
		{
			nrf_delay_ms(50);
			if((SW2_read==0)&&(sw2_flag==0))
			{
				LED2_toggle;
				RELAY2_toggle;
				sw2_flag = 1;
			}
		}
		else
		{
			sw2_flag = 0;
		}
		#endif
		//nrf_gpio_pin_toggle(18);
        //uint32_t received = read_packet();
		read_packet();
		//nrf_gpio_pin_toggle(18);
        //bsp_indication_set(BSP_INDICATE_RCV_OK);       
    }
}

/**
 *@}
 **/
