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


static const char *TAG = "MFRC522";

#define BUF_SIZE (1024)

#define NFC_UART_TXD  (16)
#define NFC_UART_RXD  (17)
#define NFC_UART_RTS  (18)
#define NFC_UART_CTS  (19)
QueueHandle_t uart2_queue;

#include "MFRC522.h"

//------------------MFRC522 register ---------------
#define         COMMAND_WAIT        0x02
#define         COMMAND_READBLOCK   0x03
#define         COMMAND_WRITEBLOCK  0x04
#define         MFRC522_HEADER      0xAB

#define         STATUS_ERROR        0
#define         STATUS_OK           1

#define         MIFARE_KEYA         0x00
#define         MIFARE_KEYB         0x01

/**
 * Constructor.
 */
MFRC522::MFRC522() {
}

/**
 * Description： Obtiene control del Serial para controlar MFRC522. 
 * Ademas, pone MFRC522 en modo de espera.
 */
void MFRC522::begin(void) {
    uart_port_t uart_num = UART_NUM_2;
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
    wait();
}

/**
 * Description：Pone MFRC522 en modo espera.
 */
void MFRC522::wait() {
    write(COMMAND_WAIT);
}

/**
 * Description：Returns true if detect card in MFRC522.
 */
bool MFRC522::available() {
    return false;
}

/**
 * Description：Read the serial number of the card.
 */
void MFRC522::readCardSerial() {
    for (int i = 0; i < sizeof(cardSerial); ){
        if (available()){
            cardSerial[i] = read();
            i++;
        }
    }
}

/**
 * Description：Returns a pointer to array with card serial.
 */
byte *MFRC522::getCardSerial() {
    return cardSerial;
}

/**
 * Description：Read a block data of the card.
 * Return：Return STATUS_OK if success.
 */
bool MFRC522::getBlock(byte block, byte keyType, byte *key, byte *returnBlock) {    
    byte sendData[8] = {
        block,      // block
        keyType,    // key type
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // key
    };
    byte returnBlockLength;

    for (int i = 0; i < 6; ++i) {
        sendData[2 + i] = key[i];
    }

    return communicate(
        COMMAND_READBLOCK,  // command
        sendData,           // sendData
        0x0A,               // length
        returnBlock,        // returnData
        &returnBlockLength  // returnDataLength
    );
}

/**
 * Description：Write a block data in the card.
 * Return：Return STATUS_OK if success.
 */
bool MFRC522::writeBlock(byte block, byte keyType, byte *key, byte *data) {    
    byte sendData[24] = {
        block,      // block
        keyType,    // key type
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // key
        0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // Data
    };
    byte returnBlock[3];
    byte returnBlockLength;

    for (int i = 0; i < 6; ++i) {
        sendData[2 + i] = key[i];
    }

    for (int i = 0; i < 16; ++i) {
        sendData[8 + i] = data[i];
    }

    byte result = communicate(
        COMMAND_WRITEBLOCK, // command
        sendData,           // sendData
        0x1A,               // length
        returnBlock,        // returnData
        &returnBlockLength  // returnDataLength
    );

    return result;
}

/**
 * Description：Comunication between MFRC522 and ISO14443.
 * Return：Return STATUS_OK if success.
 */
bool MFRC522::communicate(byte command, byte *sendData, byte sendDataLength, byte *returnData, byte *returnDataLength) {
    // Send instruction to MFRC522
    write(MFRC522_HEADER);      // Header
    write(sendDataLength);      // Length (Length + Command + Data)
    write(command);             // Command

    for (int i = 0; i < sendDataLength - 2; i++) {
        write(sendData[i]);     // Data
    }

    // Read response to MFRC522
    while (!available());
    byte header = read();           // Header
    while (!available());
    *returnDataLength = read();     // Length (Length + Command + Data)
    while (!available());
    byte commandResult = read();    // Command result

    for (int i = 0; i < *returnDataLength - 2; i=i) {
        if (available()) {
            returnData[i] = read(); // Data
            i++;
        }
    }

    // Return
    if (command != commandResult) {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

/*
 * Description：Write a byte data into MFRC522.
 */
void MFRC522::write(byte value) {
    int uart_num = UART_NUM_2;
    uart_write_bytes(uart_num, (const char*)value, 1);
}

/*
 * Description：Read a byte data of MFRC522
 * Return：Return the read value.
 */
byte MFRC522::read() {
    uart_port_t uart_num = UART_NUM_2;
    uint8_t *data = (uint8_t *)malloc(1);
    TickType_t ticks_to_wait = portTICK_RATE_MS;
    uart_read_bytes(uart_num,data,1,ticks_to_wait);
    return *data;
}
