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

#define BUF_SIZE (1024)


//------------------MFRC522 register ---------------
#define         COMMAND_WAIT        0x02
#define         COMMAND_READBLOCK   0x03
#define         COMMAND_WRITEBLOCK  0x04
#define         MFRC522_HEADER      0xAB

#define         STATUS_ERROR        0
#define         STATUS_OK           1

#define         MIFARE_KEYA         0x00
#define         MIFARE_KEYB         0x01


#define NFC_UART_TXD  (16)
#define NFC_UART_RXD  (17)
	
#define NFC_UART_RTS  (18)
#define NFC_UART_CTS  (19)


QueueHandle_t uart2_queue;


void uart_task(void *pvParameters)
{
    int uart_num = (int) pvParameters;
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(BUF_SIZE);
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart2_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            printf("uart[%d] event:", uart_num);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.
                in this example, we don't process data in event, but read data outside.*/
                case UART_DATA:
                    uart_get_buffered_data_len(uart_num, &buffered_size);
                    ESP_LOGI(TAG, "data, len: %d; buffered len: %d", event.size, buffered_size);
		    uint8_t *data = (uint8_t *)malloc(event.size);
	            TickType_t ticks_to_wait = portTICK_RATE_MS;
		    len = uart_read_bytes(uart_num,data,event.size,ticks_to_wait);
	            ESP_LOGI(TAG, "data=%02x", data[0]);
		    free(data);
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow\n");
                    //If fifo overflow happened, you should consider adding flow control for your application.
                    //We can read data out out the buffer, or directly flush the rx buffer.
                    uart_flush(uart_num);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full\n");
                    //If buffer full happened, you should consider encreasing your buffer size
                    //We can read data out out the buffer, or directly flush the rx buffer.
                    uart_flush(uart_num);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break\n");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error\n");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error\n");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    ESP_LOGI(TAG, "uart pattern detected\n");
                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d\n", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}


const portTickType xDelay = 500 / portTICK_RATE_MS;

void uart_evt_nfc()
{
    int uart_num = UART_NUM_2;
    uart_config_t uart_config = {
       .baud_rate = 9600,
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
    //Set UART pins,(-1: default pin, no change.)
    //For UART2, we can just use the default pins.
    uart_set_pin(uart_num, NFC_UART_TXD, NFC_UART_RXD, NFC_UART_RTS, NFC_UART_CTS);
    //Install UART driver, and get the queue.
    esp_err_t installed =  uart_driver_install(uart_num, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart2_queue, 0);
    printf("installed : %d\n", installed);
    //Create a task to handler UART event from ISR
    xTaskCreate(uart_task, "uart_task", 2048, (void*)uart_num, 12, NULL);
    //process data
    uint8_t* data = (uint8_t*) malloc(BUF_SIZE);
    data[0] = COMMAND_WAIT;
    do {
       printf("uart write wait : %02x\n", COMMAND_WAIT);
       uart_write_bytes(uart_num, (const char*)data, 1);
       vTaskDelay( xDelay );
    } while(1);
}




void nfc_app_main()
{
  uart_evt_nfc();
}
