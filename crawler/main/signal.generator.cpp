/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "sdkconfig.h"



void signal_generator_task(void *pvParameter)
{
    while(1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void signal_generator_app_main()
{
    nvs_flash_init();
    xTaskCreate(&signal_generator_task, "signal_generator_task", 512, NULL, 5, NULL);
}
