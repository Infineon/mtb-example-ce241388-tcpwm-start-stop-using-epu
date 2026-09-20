/***************************************************************************//**
* \file main.c
* \version 1.0
*
* \brief
* PPCA Core 0 application for the TCPWM Start/Stop using EPU example.
* Configures two EPU T2 units to detect button edge events on BTN1 and BTN2
* signals routed from the PPCA boundary. Routes BTN1 via combiner 0 as the
* PWM1 start trigger and BTN2 via combiner 12 as the PWM1 stop trigger.
* Enters an idle loop once configuration is complete.
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

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* Main function for PPCA Core 0. Configures the EPU to detect rising/falling
* edges on the button input signals (routed via PPCA boundary) and wire them
* as start and stop triggers for PWM1. Once configured, the core stays idle
* because all further operation is event-driven through the EPU.
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
     /* Grant this PPCA core exclusive ownership of the EPU block */
     Cy_PPCA_EPU_EnableExclusiveAccess(EPU_BLK_HW, true);
     /* Enable the EPU so its processing units can route and filter signals */
     Cy_PPCA_EPU_Enable(EPU_BLK_HW);

     /* Configure T2 processing unit 0 to detect the BTN1 (start) edge event */
     Cy_PPCA_EPU_PU_T2_Configure(put2_0_HW, put2_0_INDEX, &put2_0_put2_config);
     /* Configure T2 processing unit 1 to detect the BTN2 (stop) edge event */
     Cy_PPCA_EPU_PU_T2_Configure(put2_1_HW, put2_1_INDEX, &put2_1_put2_config);
     
     /* Enable PUT T2 processing unit 0  */
     Cy_PPCA_EPU_PU_T2_Enable(put2_0_HW, put2_0_INDEX, put2_0_ENABLE_MODE);
     /* Enable PUT T2 processing unit 1  */
     Cy_PPCA_EPU_PU_T2_Enable(put2_1_HW, put2_1_INDEX, put2_1_ENABLE_MODE);

     /* Combiner 0 routes the start event from BTN1 to the PWM1 start input */
     Cy_PPCA_EPU_Combo_Configure(combiner0_HW, combiner0_INDEX, &combiner0_combo_config);
     /* Combiner 12 routes the stop event from BTN2 to the PWM1 stop input */
     Cy_PPCA_EPU_Combo_Configure(combiner12_HW, combiner12_INDEX, &combiner12_combo_config);

     /* Initialize PWM1 – it will start/stop in response to EPU trigger events */
     Cy_TCPWM_PWM_Init(PWM1_HW, PWM1_NUM, &PWM1_config);
     /* Enable PWM1 so it is ready to respond when the start trigger arrives */
     Cy_TCPWM_PWM_Enable(PWM1_HW, PWM1_NUM);

     /* PPCA Core 0 is now idle – all TCPWM control is handled by EPU events */
     for(;;)
     {

     }
}
