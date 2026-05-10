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

    Pin_Blue_Write(1);
    Pin_Green_Write(1);

    PWM_RED_Start();

    for(;;)
    {
        for (int duty = 0; duty <= 100; duty++)
        {
            PWM_RED_WriteCompare(100 - duty);
            CyDelay(20);
        }

        for (int duty = 100; duty >= 0; duty--)
        {
            PWM_RED_WriteCompare(100 - duty);
            CyDelay(20);
        }
    }
}


/* [] END OF FILE */
