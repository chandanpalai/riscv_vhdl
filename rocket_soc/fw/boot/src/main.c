/******************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     Boot procedure of copying FW image into SRAM with the debug
 *            signals.
******************************************************************************/

#include <string.h>
#include "axi_maps.h"

static const int FW_IMAGE_SIZE_BYTES = 1 << 18;

void led_set(int output) {
    ((gpio_map *)ADDR_NASTI_SLAVE_GPIO)->led = output;
}

void print_uart(const char *buf, int sz) {
    uart_map *uart = (uart_map *)ADDR_NASTI_SLAVE_UART1;
    for (int i = 0; i < sz; i++) {
        while (uart->status & UART_STATUS_TX_FULL) {}
        uart->data = buf[i];
    }
}

void copy_image() { 
    uint32_t tech;
    uint64_t *fwrom = (uint64_t *)ADDR_NASTI_SLAVE_FWIMAGE;
    uint64_t *sram = (uint64_t *)ADDR_NASTI_SLAVE_SRAM;
    pnp_map *pnp = (pnp_map *)ADDR_NASTI_SLAVE_PNP;

    led_set(0xff);
    /** Speed-up RTL simulation by skipping coping stage: */
    tech = pnp->tech & 0xFF;
    if (tech != TECH_INFERRED) {
        memcpy(sram, fwrom, FW_IMAGE_SIZE_BYTES);
    }
    led_set(0x81);
}

void _init() {
    uint32_t tech;
    pnp_map *pnp = (pnp_map *)ADDR_NASTI_SLAVE_PNP;
    uart_map *uart = (uart_map *)ADDR_NASTI_SLAVE_UART1;
    // Half period of the uart = Fbus / 115200 / 2 = 70 MHz / 115200 / 2:
    uart->scaler = 304;

    print_uart("Boot . . .", 10);
    copy_image();
    print_uart("OK\r\n", 4);

    /** Check ADC detector that RF front-end is connected: */
    tech = (pnp->tech >> 16) & 0xff;
    if (tech != 0xFF) {
        print_uart("ADC clock not found. Enable DIP int_rf.\r\n", 41);
    }
}

/** Not used actually */
int main() {
    while (1) {}

    return 0;
}
