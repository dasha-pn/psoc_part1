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

int main()
{
    for(;;)
    {
        Pin_Green_Write(~Pin_Green_Read());
        Pin_Blue_Write(~Pin_Blue_Read());

        CyDelay(100);
    }
}

/* [] END OF FILE */
