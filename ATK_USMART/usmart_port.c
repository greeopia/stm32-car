/**
 ****************************************************************************************************
 * @file        usmart_port.c
 * @author      ALIENTEK
 * @brief       USMART code
 * @license     Copyright (C) 2012-2024, ALIENTEK
 ****************************************************************************************************
 * @attention
 *
 * platform     : ALIENTEK M100-STM32F103 board
 * website      : www.alientek.com
 * forum        : www.openedv.com/forum.php
 *
 * change logs  £º
 * version      data         notes
 * V1.0         20240409     the first version
 *
 ****************************************************************************************************
 */


#include "usmart.h"
#include "usmart_port.h"
#include "tim.h"


/**
 * @brief      Get the input data stream (string)
 * @note       USMART parses the string returned by the function for information such as the function name and arguments
 * @param      none
 * @retval
 * @arg        0, no data received
 * @arg        Other, stream start address (cannot be 0)
 */
char *usmart_get_input_string(void)
{
    uint8_t len;
    char *pbuf = 0;

    if (g_usart_rx_sta & 0x8000)        /* Complete serial port reception? */
    {
        len = g_usart_rx_sta & 0x3fff;  /* Get the length of the received data */
        g_usart_rx_buf[len] = '\0';     /* Add the terminator at the end. */
        pbuf = (char*)g_usart_rx_buf;
        g_usart_rx_sta = 0;             /* Enable next reception */
    }

    return pbuf;
}

/* If timer scanning is enabled, the following functions need to be defined */
#if USMART_ENTIMX_SCAN == 1

/**
 * @brief   Reset runtime function
 * @note    needs to be modified according to the timer parameters of the MCU to be ported
 * @param   None
 * @retval  None
 */
void usmart_timx_reset_time(void)
{
    __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE); /* Clear the interrupt flag bit */
    __HAL_TIM_SET_AUTORELOAD(&htim4, 0XFFFF);      /* Set the reload value to the maximum */
    __HAL_TIM_SET_COUNTER(&htim4, 0);              /* Clear counter */
    usmart_dev.runtime = 0;
}

/**
 * @brief   gets the runtime time
 * @note    needs to be modified according to the timer parameters of the MCU to be ported
 * @param   none
 * @retval  execution time in 0.1ms. The maximum delay time is 2 times the CNT value *0.1ms
 */
uint32_t usmart_timx_get_time(void)
{
    if (__HAL_TIM_GET_FLAG(&htim4, TIM_FLAG_UPDATE) == SET)  /* During the run, a timer overflow occurs */
    {
        usmart_dev.runtime += 0XFFFF;
    }

    usmart_dev.runtime += __HAL_TIM_GET_COUNTER(&htim4);
    return usmart_dev.runtime;                               /* Return count value */
}


/**
 * @brief   USMART timer interrupt service function
 * @param   None.
 * @retval  None.
 */
void TIM4_IRQHandler(void)
{
    /* overflow trap */
    if(__HAL_TIM_GET_IT_SOURCE(&htim4,TIM_IT_UPDATE)==SET)
    {
        usmart_dev.scan();                                  /* usmart scan */
        __HAL_TIM_SET_COUNTER(&htim4, 0);;                  /* Clear counter */
        __HAL_TIM_SET_AUTORELOAD(&htim4, 100);              /* Restore the original Settings */
    }

    __HAL_TIM_CLEAR_IT(&htim4, TIM_IT_UPDATE);              /* Clear the interrupt flag bit */
}

#endif
