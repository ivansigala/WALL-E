/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022, 2024-2025 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "board.h"
#include "app.h"
#include "fsl_lpadc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MAX_ADC 4095

#define JOYSTICK_CNTRX 2000
#define JOYSTICK_CNTRY 1840
#define DEADZONE 20

#define JOYSTICK_LPADC_USER_CHANNEL     1U
#define XVALUE_LPADC_USER_CMDID       1U
#define YVALUE_LPADC_USER_CMDID       2U
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_LpadcConversionCompletedFlag = false;
volatile uint32_t g_LpadcInterruptCounter    = 0U;
lpadc_conv_result_t g_LpadcResultConfigStruct;
const uint32_t g_LpadcFullRange   = 4096U;
const uint32_t g_LpadcResultShift = 3U;


volatile uint32_t ADC_Valx = 0 ;
volatile uint32_t ADC_Valy = 0 ;

volatile uint32_t Xconv = 0;
volatile uint32_t Yconv = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_LPADC_IRQ_HANDLER_FUNC(void)
{
	lpadc_conv_result_t tmpResultStruct;

	    /* Loop while FIFO is not empty */
	    while (LPADC_GetConvResult(DEMO_LPADC_BASE, &tmpResultStruct, 0U))
	    {
	        uint32_t result = (tmpResultStruct.convValue >> g_LpadcResultShift);

	        /* Check Command ID to decide if this is X or Y data */
	        if (tmpResultStruct.commandIdSource == XVALUE_LPADC_USER_CMDID)
	        {
	            ADC_Valx = result;
	        }
	        else if (tmpResultStruct.commandIdSource == YVALUE_LPADC_USER_CMDID)
	        {
	            ADC_Valy = result;
	        }
	    }

	    g_LpadcConversionCompletedFlag = true;
	    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    lpadc_config_t mLpadcConfigStruct;
    lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
    lpadc_conv_command_config_t mLpadcCommandConfigStruct;

    BOARD_InitHardware();

    PRINTF("LPADC Interrupt Example\r\n");

    LPADC_GetDefaultConfig(&mLpadcConfigStruct);
    mLpadcConfigStruct.enableAnalogPreliminary = true;

    /* Set to highest power level here, users need to properly match ADC clock and power level according 
     * to application requirements. For specific correspondence, please refer to the data sheet. */
    mLpadcConfigStruct.powerLevelMode = kLPADC_PowerLevelAlt4;
    mLpadcConfigStruct.referenceVoltageSource = DEMO_LPADC_VREF_SOURCE;
    mLpadcConfigStruct.conversionAverageMode = kLPADC_ConversionAverage128;
    LPADC_Init(DEMO_LPADC_BASE, &mLpadcConfigStruct);
    LPADC_DoOffsetCalibration(DEMO_LPADC_BASE); /* Request offset calibration, automatic update OFSTRIM register. */
    LPADC_DoAutoCalibration(DEMO_LPADC_BASE);

    /*------------------------- ADC A1 -----------------------*/
    /* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
    mLpadcCommandConfigStruct.channelNumber = JOYSTICK_LPADC_USER_CHANNEL;
    mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelSingleEndSideA;
    mLpadcCommandConfigStruct.chainedNextCommandNumber = YVALUE_LPADC_USER_CMDID;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, XVALUE_LPADC_USER_CMDID, &mLpadcCommandConfigStruct);


    /*------------------------- ADC B1 -----------------------*/
    /* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
    mLpadcCommandConfigStruct.channelNumber = JOYSTICK_LPADC_USER_CHANNEL;
    mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelSingleEndSideB;
    mLpadcCommandConfigStruct.chainedNextCommandNumber = 0U;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, YVALUE_LPADC_USER_CMDID, &mLpadcCommandConfigStruct);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
    mLpadcTriggerConfigStruct.targetCommandId       = XVALUE_LPADC_USER_CMDID;     /* CMD15 is executed. */
    mLpadcTriggerConfigStruct.enableHardwareTrigger = false;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */

    /* Enable the watermark interrupt. */
    LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFO0WatermarkInterruptEnable);
    EnableIRQ(DEMO_LPADC_IRQn);

    PRINTF("ADC Full Range: %d\r\n", g_LpadcFullRange);

    /* When the number of datawords stored in the ADC Result FIFO is greater
     * than watermark value(0U), LPADC watermark interrupt would be triggered.
     */
    PRINTF("Please press any key to get user channel's ADC value.\r\n");

    LED_GREEN_INIT(0);
    LED_RED_INIT(0);
    LED_BLUE_INIT(0);
    LED_GREEN_OFF();
    LED_RED_OFF();
    LED_BLUE_OFF();
    while (1)
    {
//        GETCHAR();
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U); /* 1U is trigger0 mask. */
        while (!g_LpadcConversionCompletedFlag)
        {
        }
//        ADC_Valx = ((g_LpadcResultConfigStruct.convValue) >> g_LpadcResultShift);
        if(ADC_Valy > JOYSTICK_CNTRY + DEADZONE){

        	LED_BLUE_OFF();
        	LED_GREEN_OFF();
        	LED_RED_ON();

        }else if (ADC_Valy < JOYSTICK_CNTRY - DEADZONE){

        	LED_BLUE_OFF();
        	LED_GREEN_ON();
        	LED_RED_OFF();

        }else if (ADC_Valx > JOYSTICK_CNTRX + DEADZONE){

        	LED_BLUE_ON();
        	LED_GREEN_ON();
        	LED_RED_ON();

        }else if (ADC_Valx < JOYSTICK_CNTRX - DEADZONE){

        	LED_BLUE_ON();
        	LED_GREEN_OFF();
        	LED_RED_OFF();

        }else{
        	LED_BLUE_OFF();
        	LED_GREEN_OFF();
        	LED_RED_OFF();
        }

        Xconv = 200 - ((100*ADC_Valx)/MAX_ADC);
        Yconv = 200 - ((100*ADC_Valy)/MAX_ADC);

        PRINTF("X: %d | Y: %d\r\n", Xconv, Yconv);

        g_LpadcConversionCompletedFlag = false;
    }
}
