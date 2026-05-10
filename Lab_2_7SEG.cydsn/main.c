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
#include "project.h"

void out7seg(uint16, uint8);
void rst_count(void);

volatile uint16 counter = 0;
volatile uint8 cnt_mode = 0;       // (0 -> 9999) or (0 -> -999)


uint16 indyk[4]         = {0x10FF,0x20FF,0x40FF,0x80FF};
const uint8 KOD7[10]    = {0x03,0x9F,0x25,0x0D,0x99,0x49,0x41,0x1F,0x01,0x09};
const uint16 minus = 0x00FD;      // minus code on a 7 seg indicator

uint8 floating_p_mode = 0;        // 1 - fixed floating point mode, 0 - integer mode
uint8 point_pos = 1;              // fixed point position on a 7 seg indicator

CY_ISR(isr_100Hz)
{   
    // ISR to sequentially light next segment
    static uint8 N_indyk = 0u;
    
    SPIM_WriteTxData(indyk[N_indyk]);
    
    N_indyk++;
    N_indyk &= 3;    
}

// Added
CY_ISR(isr_CNT_RST)
{   
    // ISR to switch counter to (0 -> -999) mode
    cnt_mode = 0;
    rst_count();
    Pin_B2_ClearInterrupt();
}

CY_ISR(isr_CNT_NEG)
{
    // ISR to switch counter to (0 -> 9999) mode
    cnt_mode = 1;
    rst_count();
    Pin_B3_ClearInterrupt();
}

// Adjusted
void out7seg(uint16 data, uint8 pos)
{   
    uint8 i;
    
    if (cnt_mode == 0) // if (0 -> 9999) mode
    {   
        uint8 cond = (floating_p_mode == 1 && (pos < 4) && (pos > 0)); // floating point mode & pos in range
        
        for (i = 0; i < 4; i++)
        {   
            indyk[i] &= 0xFF00;
            indyk[i] |= (uint16)KOD7[data % 10];
            data /= 10;

            if (cond)
            {   
                if (i == pos)
                {   
                    indyk[i] &= 0xFFFE; // Adding fixed point on a position if condition |^|
                }
            }
            if (data == 0) break;
        }
        
        for (++i; i < 4; i++)
        {   
            if (cond)  // floating point mode & pos in range
            {
                if (i < pos)
                {   
                    indyk[i] &= 0xFF00;
                    indyk[i] |= (uint16)KOD7[0];            // Adding 0 before . if condition |^|
                }
                else if (i == pos)
                {   
                    indyk[i] &= 0xFF00;
                    indyk[i] |= ((uint16)KOD7[0] & 0xFFFE); // Adding 0. on a position if condition |^|
                }
                else
                {
                    indyk[i] |= 0x00FF;                     // Turning off unused segments if condition |^|
                }
            }
            else
            {
                indyk[i] |= 0x00FF;                         // Turning off unused segments if NOT condition |^|
            }    
        }
    }
    else // if (0 -> -999) mode
    {   
        uint8 cond = (floating_p_mode == 1 && (pos < 3) && (pos > 0));
        if (cond)                                           // floating point mode & pos in range
        {   
            for (i = 0; i < 3; i++)
            {
                indyk[i] &= 0xFF00;
                indyk[i] |= (uint16)KOD7[data % 10];
                data /= 10;
                if (i == pos)
                {   
                    indyk[i] &= 0xFFFE;                     // Adding fixed point on a position if condition |^|
                }
                if (data == 0) break;    
            }
            
            uint8 last_ind = i;                             // Last valid segm ind
            
            for (++i; i < 4; i++)
            {   
                if (i < pos)
                {   
                    indyk[i] &= 0xFF00;
                    indyk[i] |= (uint16)KOD7[0];            // Adding 0 before . if condition |^|
                    last_ind = i;
                }
                else if (i == pos)
                {   
                    indyk[i] &= 0xFF00;
                    indyk[i] |= ((uint16)KOD7[0] & 0xFFFE); // Adding 0. on a position if condition |^|
                    last_ind = i;
                }
                else
                {   
                    if (i == (last_ind + 1))
                    {
                        indyk[i] &= 0xFF00;
                        indyk[i] |= minus;                  // Adding '-' sign on the next valid segment
                    }
                    else
                    {
                        indyk[i] |= 0x00FF;                 // Turning off unused segments if condition |^|
                    }
                }
            }
        }
        else 
        {                                                   // integer mode
            for (i = 0; i < 3; i++)
            {
                indyk[i] &= 0xFF00;
                indyk[i] |= (uint16)KOD7[data % 10];
                data /= 10;
                if (data == 0) break;    
            }

            if (counter != 0){
                i++;
                indyk[i] &= 0xFF00;
                indyk[i] |= (minus & 0x00FF);                // Adding '-' sign on the next valid segment
            }

            for (++i; i < 4; i++)
            {   
                indyk[i] |= 0x00FF;                          // Turning off unused segments
            }    
        }    
    }
}

// Added
void rst_count(void){
    uint8 prev_sys_state = CyEnterCriticalSection();
    counter = 0; // reseting counter
    (floating_p_mode) ? out7seg(0, point_pos) : out7seg(0, 0); // if pos is invalid floating_p_mode = 0;
    CyExitCriticalSection(prev_sys_state);
}

int main(void)
{   
    
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    SPIM_Start();
    
    isr_CNT_RST_StartEx(isr_CNT_RST); // assign ISR to the interrupt S2
    isr_CNT_NEG_StartEx(isr_CNT_NEG); // assign ISR to the interrupt S3
    isr_100Hz_StartEx(isr_100Hz);

    for(;;)
    {   
        counter++;
        if (cnt_mode == 0)
        {
            if (counter > 9999)
            {
                rst_count();          // count reset for positive numbers
            }
        }
        else
        {
            if (counter > 999)
            {   
                rst_count();          // count reset for negative numbers
            }
        }
        
        out7seg(counter, point_pos);
        
        CyDelay(3);
        
        if (Pin_B1_Read() == 0)
        {
            CyDelay(700);
        }
    }
}

/* [] END OF FILE */
