/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>

int main(void)
{
    CyGlobalIntEnable;

    PWM_GREEN_Start();
    PWM_BLUE_Start();

    for(;;)
    {
        uint16 duty;

        for (duty = 0; duty <= 255; duty++)
        {
            PWM_GREEN_WriteCompare(duty);
            PWM_BLUE_WriteCompare(0);
            CyDelay(10);
        }

        for (duty = 0; duty <= 255; duty++)
        {
            PWM_GREEN_WriteCompare(255 - duty);
            PWM_BLUE_WriteCompare(0);
            CyDelay(10);
        }

        for (duty = 0; duty <= 255; duty++)
        {
            PWM_GREEN_WriteCompare(0);
            PWM_BLUE_WriteCompare(duty);
            CyDelay(10);
        }

        for (duty = 0; duty <= 255; duty++)
        {
            PWM_GREEN_WriteCompare(0);
            PWM_BLUE_WriteCompare(255 - duty);
            CyDelay(10);
        }
    }
}

/* [] END OF FILE */
