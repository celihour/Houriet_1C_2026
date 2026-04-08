#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

int8_t convertirBCD(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
    for (int8_t i = digits - 1; i >= 0; i--)
    {
        bcd_number[i] = data % 10;
        data /= 10;
    }
    return 0;
}

void app_main(void)
{
    uint32_t data = 123;
    uint8_t digits = 3;
    uint8_t bcd_number[3];
    convertirBCD(data, digits, bcd_number);

    // Imprimir el resultado
    printf("Número convertido a BCD:\n");
    for (uint8_t i = 0; i < digits; i++)
    {
        printf("bcd_number[%d] = %d\n", i, bcd_number[i]);
    }
}