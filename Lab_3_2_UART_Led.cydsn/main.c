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
#include <stdio.h>
#include <math.h>

#define LED_ON  0
#define LED_OFF 1

void out7seg(uint16, uint8);
void rst_count(void);

volatile uint16 counter = 0;
uint8 cnt_mode = 0;       // (0 -> 9999) or (0 -> -999)

uint16 indyk[4]         = {0x10FF,0x20FF,0x40FF,0x80FF};
const uint8 KOD7[10]    = {0x03,0x9F,0x25,0x0D,0x99,0x49,0x41,0x1F,0x01,0x09};
const uint16 minus = 0x00FD;      // minus code on a 7 seg indicator

uint8 floating_p_mode = 0;        // 1 - fixed floating point mode, 0 - integer mode
uint8 point_pos = 0;              // fixed point position on a 7 seg indicator

CY_ISR(isr_100Hz)
{   
    // ISR to sequentially light next segment
    static uint8 N_indyk = 0u;
    
    SPIM_WriteTxData(indyk[N_indyk]);
    
    N_indyk++;
    N_indyk &= 3;    
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

int main()
{
    uint16 total_sum = 0;
    uint8 num_count = 0;
    uint8 ch;
    
    SPIM_Start();
    isr_100Hz_StartEx(isr_100Hz);
    CyGlobalIntEnable;

    Pin_D1_Write(LED_OFF);
    Pin_D2_Write(LED_OFF);
    Pin_D3_Write(LED_OFF);
    Pin_D4_Write(LED_OFF);
                    
    UART_Start(); 
    CyDelay(1000);
    
    UART_UartPutString("LAB 3.2\r\n");
    UART_UartPutString("Enter number (0-9999): ");
    
    for(;;)
    {
        ch = UART_UartGetChar();
        
        if (ch == 0) continue;
        
        UART_UartPutChar(ch);
        
        if (cnt_mode == 0)
        {
            if (ch >= '0' && ch <= '9')
            {
                if (num_count < 4)
                {
                    total_sum = total_sum * 10 + (ch - '0');
                    num_count++;
                }
                else
                {
                    UART_UartPutString("\r\nERROR: Max 4 digits\r\n");
                }
            }
            else if (ch == '\r' || ch == '\n' || ch == 0x1B)
            {
                UART_UartPutString("\r\n Display updated: ");
                
                out7seg(total_sum, 0);

                num_count = 0;
                total_sum = 0;
                
                UART_UartPutString("Enter number: ");
            }
            else if (ch == 0x08 || ch == 0x7F)
            {
                if (num_count > 0)
                {
                    total_sum /= 10;
                    num_count--;
                    UART_UartPutString("\b \b");
                }
            }
            else
            {
                UART_UartPutString("\r\nERROR: Invalid character\r\n");
            }
        }
    }
}/* [] END OF FILE */
