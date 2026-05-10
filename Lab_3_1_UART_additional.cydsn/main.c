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
#include <string.h>

#define RX_BUF_LEN 16
char rxBuf[RX_BUF_LEN];
uint8 rxIdx = 0;
char TEXT_BUF[32];

void out7seg(uint16 data);
void out7seg_float(float x);

uint16 indyk[4] = {0x10FF,0x20FF,0x40FF,0x80FF};
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

#define DP_POS_FROM_RIGHT 1

void out7seg_float(float x)
{
    int scaled;
    uint16 u;
    uint8 dp_index;

    if (x < 0.0f)
        x = -x;

    if (x > 999.9f)
        x = 999.9f;

    scaled = (int)(x * 10.0f + 0.5f);
    if (scaled < 0)
        scaled = 0;
    if (scaled > 9999)
        scaled = 9999;

    u = (uint16)scaled;

    out7seg(u);

    dp_index = DP_POS_FROM_RIGHT;
    if (dp_index < 4)
    {
        indyk[dp_index] &= ~(uint16)0x01u;
    }
}

uint8 parse_float_1digit(const char *s, float *out)
{
    int sign = 1;
    int intPart = 0;
    int fracDigit = 0;
    uint8 hasInt = 0;
    uint8 hasFrac = 0;
    uint8 i = 0;

    while (s[i] == ' ' || s[i] == '\t') i++;

    if (s[i] == '+' || s[i] == '-')
    {
        if (s[i] == '-') sign = -1;
        i++;
    }

    while (s[i] >= '0' && s[i] <= '9')
    {
        hasInt = 1;
        intPart = intPart * 10 + (s[i] - '0');
        i++;
    }

    if (s[i] == '.' || s[i] == ',')
    {
        i++;
        if (s[i] >= '0' && s[i] <= '9')
        {
            fracDigit = s[i] - '0';
            hasFrac = 1;
            i++;
        }
    }

    if (!hasInt && !hasFrac)
    {
        return 0;
    }

    *out = (float)sign * ( (float)intPart + (float)fracDigit / 10.0f );
    return 1;
}


int main(void)
{
    CyGlobalIntEnable;
    
    SPIM_Start();
    isr_100Hz_StartEx(isr_100Hz);
    
    UART_Start();
    
    CyDelay(100);
    
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

                    if (strchr(rxBuf, '.') != NULL || strchr(rxBuf, ',') != NULL)
                    {
                        float fvalue;

                        if (parse_float_1digit(rxBuf, &fvalue))
                        {
                            out7seg_float(fvalue);

                            {
                                int scaled_print = (int)(fvalue * 10.0f + 0.5f);
                                int ip = scaled_print / 10;
                                int fp = scaled_print % 10;
                                sprintf(TEXT_BUF, "\r\nOK float: %d.%1d\r\n", ip, fp);
                                UART_UartPutString(TEXT_BUF);
                            }
                        }
                        else
                        {
                            UART_UartPutString("\r\nParse error (float)\r\n");
                        }
                    }
                    else
                    {
                        int value;
                        if (sscanf(rxBuf, "%d", &value) == 1)
                        {
                            if (value < 0 || value > 9999)
                                value = 0;

                            out7seg((uint16)value);

                            sprintf(TEXT_BUF, "\r\nOK int: %d\r\n", value);
                            UART_UartPutString(TEXT_BUF);
                        }
                        else
                        {
                            UART_UartPutString("\r\nParse error (int)\r\n");
                        }
                    }

                    UART_UartPutString("Enter next value:\r\n");
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
