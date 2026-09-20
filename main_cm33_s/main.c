/***************************************************************************//**
* \file main.c
* \version 1.0
*
* \brief
* Demonstrates TCPWM start stop using EPU 
*
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

#include "cy_pdl.h"
#include "cycfg.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
********************************************************************************/
/* These are the addresses where the core0 and core1 images are located. */
#define CORE0_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca0_nvm_C_S_START    //0x12030000
#define CORE1_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca1_nvm_C_S_START    //0x12038000
#define PPCA0_IMAGE_SIZE       CYMEM_CM33_0_S_ppca0_code_SIZE
#define PPCA1_IMAGE_SIZE       CYMEM_CM33_0_S_ppca1_code_SIZE

/*******************************************************************************
* Global Variables
********************************************************************************/
/* Debug UART variables */
static cy_stc_scb_uart_context_t    DEBUG_UART_context; /* UART context */
static mtb_hal_uart_t               DEBUG_UART_hal_obj; /* Debug UART HAL object */

const cy_stc_ppca_cnfg_ppcaout_input_selector_t ppca_input_sel1 = {
    .inputSelSrc = PPCAIN_SEL_SRC_0,
    .disSynchronizerStage = true,
};

const cy_stc_ppca_cnfg_ppcaout_input_selector_t ppca_input_sel2 = {
    .inputSelSrc = PPCAIN_SEL_SRC_1,
    .disSynchronizerStage = true,
};


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for the CM33 secure core. It initializes the
* board peripherals, configures the PPCA signal routing for GPIO button inputs,
* boots the two PPCA cores, and then remains idle while the PPCA cores service
* the EPU-triggered TCPWM start/stop events autonomously.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize UART peripheral for debug output */
    Cy_SCB_UART_Init(DEBUG_UART_HW, &DEBUG_UART_config, &DEBUG_UART_context);
    Cy_SCB_UART_Enable(DEBUG_UART_HW);

    /* Setup the HAL UART */
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &DEBUG_UART_hal_config,
                                &DEBUG_UART_context, NULL);

    /* HAL UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize redirecting of low level IO */
    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);

    /* retarget IO init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize PPCA configuration block to route GPIO button signals
       into the PPCA boundary so the EPU inside PPCA can process them */
    Cy_PPCA_CNFG_Init(CNFG_PPCA_INOUT_HW, &CNFG_PPCA_INOUT_config);
    Cy_PPCA_Enable(CNFG_PPCA_INOUT_HW);

    /* Route PPCA output signals to the configured physical LED pins */
    Cy_PPCA_CNFG_PPCA_Output_Selector(CNFG_PPCA_INOUTCNFG_HW, &CNFG_PPCA_INOUT_ppcaOutConfig);

    /* Map USER-BTN1 (SW3) to PPCA input 0 – used as the TCPWM start trigger */
    Cy_PPCA_CNFG_PPCA_Input_Selector(CNFG_PPCA_INOUTCNFG_HW, &ppca_input_sel1, 0);
    /* Map USER-BTN2 (SW4) to PPCA input 1 – used as the TCPWM stop trigger */
    Cy_PPCA_CNFG_PPCA_Input_Selector(CNFG_PPCA_INOUTCNFG_HW, &ppca_input_sel2, 1);

    /* Transmit header to the terminal */
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: TCPWM start/stop using EPU\r\n");
    printf("************************************************************\r\n\n");

    /* Enable global interrupts before booting PPCA cores */
    __enable_irq();

    /* Boot PPCA Core 0 (handles EPU configuration + TCPWM PWM1 for LED1) */
     Cy_System_Init_CPU0((void*)CORE0_IMAGE_ADDRESS,PPCA0_IMAGE_SIZE);
    /* Boot PPCA Core 1 (idle – LED2 is driven by EPU output from Core 0) */
     Cy_System_Init_CPU1((void*)CORE1_IMAGE_ADDRESS,PPCA1_IMAGE_SIZE);

     printf("PPCA CM33 cores Boot called\r\n");
     printf("Press the USER-BTN1 (SW3) to start the PPCA TCPWM\r\n"
     "observe that LED1 (P1.5) and LED2 (P4.1) blinks every second\r\n");
     printf("Press the USER-BTN2 (SW4) to stop the PPCA TCPWM\r\n"
      "observe that LED1 (P1.5) and LED2 (P4.1) stops blinking\r\n");

    /* Main core idle loop – PPCA cores handle button/TCPWM events autonomously */
    for (;;)
    {
        Cy_SysLib_Delay(500);
    }
}
