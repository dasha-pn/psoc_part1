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
#include <stdio.h>
#include <stdlib.h>

#define RX_BUF_LEN 20
char rxBuf[RX_BUF_LEN];
uint8 rxIdx = 0;
char TEXT_BUF[32];

void out7seg(uint16);

uint16 indyk[4]      = {0x10FF,0x20FF,0x40FF,0x80FF};
const uint8 KOD7[10] = {0x03,0x9F,0x25,0x0D,0x99,0x49,0x41,0x1F,0x01,0x09};

CY_ISR(isr_100Hz)
{
    static uint8 N_indyk = 0u;
    
    SPIM_WriteTxData(indyk[N_indyk]);
    
    N_indyk++;
    N_indyk &= 3;    
}

void out7seg(uint16 data)
{
    uint8 i;
    
    for (i = 0; i < 4; i++)
    {
        indyk[i] &= 0xFF00;
        indyk[i] |= (uint16)KOD7[data % 10];
        data /= 10;
        if (data == 0) break;
    }
    
    for (i++; i < 4; i++)
    {
        indyk[i] |= 0x00FF;
    }
}

int main(void)
{
    CyGlobalIntEnable;
    
    SPIM_Start();
    isr_100Hz_StartEx(isr_100Hz);
    
    UART_Start();
    
    CyDelay(100);
    
    UART_UartPutString("CY8CKIT-042 UART + 7SEG\r\n");
    UART_UartPutString("Enter integer (0..9999) and press Enter:\r\n");
    
    out7seg(0);

    for(;;)
    {
        while (UART_SpiUartGetRxBufferSize() != 0)
        {
            char c = UART_UartGetChar();

            if (c == '\r' || c == '\n')
            {
                if (rxIdx > 0)
                {
                    rxBuf[rxIdx] = '\0';
                    rxIdx = 0;

                    int value;
                    if (sscanf(rxBuf, "%d", &value) == 1)
                    {
                        if (value < 0 || value > 9999)
                            value = 0;

                        out7seg((uint16)value);

                        sprintf(TEXT_BUF, "\r\nOK: %d\r\n", value);
                        UART_UartPutString(TEXT_BUF);
                    }
                    else
                    {
                        UART_UartPutString("\r\nParse error\r\n");
                    }

                    UART_UartPutString("Enter integer (0..9999):\r\n");
                }
            }
            else
            {
                if (rxIdx < RX_BUF_LEN - 1)
                {
                    rxBuf[rxIdx++] = c;
                    UART_UartPutChar(c);
                }
            }
        }
    }
}

/* [] END OF FILE */
