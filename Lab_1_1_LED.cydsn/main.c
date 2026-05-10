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
    
    for(;;)
    {
        Pin_LED_Write(1);
        Pin_Green_Write(0);
        Pin_Blue_Write(1);
        CyDelay(300);

        Pin_Green_Write(1);
        Pin_Blue_Write(0);
        Pin_LED_Write(1);
        CyDelay(300);
    }
}

