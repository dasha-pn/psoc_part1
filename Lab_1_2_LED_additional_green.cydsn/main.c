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

    for(;;)
    {
        int duty;

        for(duty = 0; duty <= 100; duty++)
        {
            PWM_GREEN_WriteCompare(100 - duty);
            CyDelay(20);
        }

        for(duty = 100; duty >= 0; duty--)
        {
            PWM_GREEN_WriteCompare(100 - duty);
            CyDelay(20);
        }
    }
}

/* [] END OF FILE */
