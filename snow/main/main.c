/**
 * Copyright (c) 2017-2018 Tara Keeling
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "ssd1306.h"
#include "ssd1306_draw.h"
#include "ssd1306_font.h"
#include "ssd1306_default_if.h"

#define USE_I2C_DISPLAY
//#define USE_SPI_DISPLAY

#if defined USE_I2C_DISPLAY
    static const int I2CDisplayAddress = 0x3C;
    static const int I2CDisplayWidth = 128;
    static const int I2CDisplayHeight = 64;
    static const int I2CResetPin = 16;  						// Board WiFi LoRa 32 (V2) https://heltec.org/project/wifi-lora-32/
    static const gpio_num_t GPIO_VEXT 		= GPIO_NUM_21;		// Board WiFi LoRa 32 (V2) https://heltec.org/project/wifi-lora-32/

    struct SSD1306_Device I2CDisplay;
#endif

#if defined USE_SPI_DISPLAY
    static const int SPIDisplayChipSelect = 15;
    static const int SPIDisplayWidth = 128;
    static const int SPIDisplayHeight = 64;
    static const int SPIResetPin = 5;

    struct SSD1306_Device SPIDisplay;
#endif

struct SSD1306_Device* DisplayList[ ] = {
#if defined USE_I2C_DISPLAY
    &I2CDisplay,
#endif
#if defined USE_SPI_DISPLAY
    &SPIDisplay,
#endif
};

const int DisplayCount = sizeof( DisplayList ) / sizeof( DisplayList[ 0 ] );

extern void SnowyFieldTask( void* Param );

bool DefaultBusInit( void ) {
    #if defined USE_I2C_DISPLAY
        assert( SSD1306_I2CMasterInitDefault( ) == true );
        assert( SSD1306_I2CMasterAttachDisplayDefault( &I2CDisplay, I2CDisplayWidth, I2CDisplayHeight, I2CDisplayAddress, I2CResetPin ) == true );
    #endif

    #if defined USE_SPI_DISPLAY
        assert( SSD1306_SPIMasterInitDefault( ) == true );
        assert( SSD1306_SPIMasterAttachDisplayDefault( &SPIDisplay, SPIDisplayWidth, SPIDisplayHeight, SPIDisplayChipSelect, SPIResetPin ) == true );
    #endif

    return true;
}

void app_main( void ) {
    int i = 0;

    printf( "Ready...\n" );


    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */

    gpio_pad_select_gpio(GPIO_VEXT);
    gpio_set_direction(GPIO_VEXT, GPIO_MODE_OUTPUT);



    gpio_set_level(GPIO_VEXT, 0); //LOW means Vext ON



    if ( DefaultBusInit( ) == true ) {
        printf( "BUS Init lookin good...\n" );
        printf( "Drawing.\n" );

        for ( i = 0; i < DisplayCount; i++ ) {
            SSD1306_SetContrast( DisplayList[ i ], 0xFF );

            printf( "Starting demo on display %d\n", i );
            xTaskCreate( SnowyFieldTask, "SnowyFieldTask", 4096, DisplayList[ i ], i, NULL );
        }

        printf( "Done!\n" );

        gpio_set_level(GPIO_VEXT, 1); //HIGH means Vext OFF
    }
}
