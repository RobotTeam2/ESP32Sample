#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "soc/uart_struct.h"


static const char *TAG = "uart_example";

void uart_evt_nfc()
{
    int uart_num = UART_NUM_0;
    uart_config_t uart_config = {
       .baud_rate = 115200,
       .data_bits = UART_DATA_8_BITS,
       .parity = UART_PARITY_DISABLE,
       .stop_bits = UART_STOP_BITS_1,
       .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
       .rx_flow_ctrl_thresh = 122,
    };
    //Set UART parameters
    uart_param_config(uart_num, &uart_config);
    //Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    //Install UART driver, and get the queue.
    uart_driver_install(uart_num, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart0_queue, 0);
    //Set UART pins,(-1: default pin, no change.)
    //For UART0, we can just use the default pins.
    //uart_set_pin(uart_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    //Set uart pattern detect function.
    uart_enable_pattern_det_intr(uart_num, '+', 3, 10000, 10, 10);
    //Create a task to handler UART event from ISR
    xTaskCreate(uart_task, "uart_task", 2048, (void*)uart_num, 12, NULL);
    //process data
    uint8_t* data = (uint8_t*) malloc(BUF_SIZE);
    do {
        int len = uart_read_bytes(uart_num, data, BUF_SIZE, 100 / portTICK_RATE_MS);
        if(len > 0) {
            ESP_LOGI(TAG, "uart read : %d", len);
            uart_write_bytes(uart_num, (const char*)data, len);
        }
    } while(1);
}




void nfc_app_main()
{
  uart_evt_nfc();
}
