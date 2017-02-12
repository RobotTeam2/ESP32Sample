/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include <cmath>

#define ms(x) (x/portTICK_PERIOD_MS)

#define DUMP_VAR_d(x) \
 printf("%s:%d:%s=<%d>\n",__FILE__,__LINE__,#x,x)

#define DUMP_VAR_f(x) \
 printf("%s:%d:%s=<%f>\n",__FILE__,__LINE__,#x,x)

extern "C" void ble_server_notify(char);

static const int iConstSampleRate = 100;
static const int iConstSampleDelay = 1000/iConstSampleRate;
static const double pi = std::acos(-1);

static char signal(int counter)
{
   double x = 2 * pi * (double)counter/(double)iConstSampleRate;
   double y = sin(x);
   DUMP_VAR_d(counter);
   DUMP_VAR_f(x);
   DUMP_VAR_f(y);
   return y *126;
}

static int gSamplerCounter = 0;

void signal_generator_task(void *pvParameter)
{
    while(true) {
       auto sign = signal(gSamplerCounter++%iConstSampleRate);
       ble_server_notify(sign);
       vTaskDelay(ms(iConstSampleDelay));
    }
}

extern "C" void signal_generator_app_main()
{
    xTaskCreate(&signal_generator_task, "signal_generator_task", 2048, NULL, 5, NULL);
}
