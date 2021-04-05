/******************************************************************************
 * @file LEDControl.h
 * @brief LED Control Object
 * 
 * Contains class definition for the control service object, which is used
 * by the LED Control Service to configure the LED output to the desired
 * pattern/intensity/etc.
 * 
 * @author Steve Rizor
 * @date 4/3/2021
 * 
 *****************************************************************************/
#pragma once

#include "logging.h"
#include "rpi_ws281x/ws2811.h"
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_SIZE(stuff)       (sizeof(stuff) / sizeof(stuff[0]))

// defaults for cmdline options
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                18
#define DMA                     10
//#define STRIP_TYPE            WS2811_STRIP_RGB		// WS2812/SK6812RGB integrated chip+leds
#define STRIP_TYPE              WS2811_STRIP_GBR		// WS2812/SK6812RGB integrated chip+leds
//#define STRIP_TYPE            SK6812_STRIP_RGBW		// SK6812RGBW (NOT SK6812RGB)

#define WIDTH                   1
#define HEIGHT                  64


class LEDControl
{
public:
    // LED struct is formatted for easy interop with the rpi_ws2811 library
    typedef struct
    {
        union
        {
            uint32_t wrgb;
            struct
            {
                uint8_t white;
                uint8_t red;
                uint8_t green;
                uint8_t blue;
            };
        };
    } __attribute__ ((packed)) led_t;

    enum led_color_e
    {
        LED_W = 0xFF000000,
        LED_R = 0x00FF0000,
        LED_G = 0x0000FF00,
        LED_B = 0x000000FF
    } ;

    LEDControl(uint16_t led_count)
        :_count(led_count)
  {
        _ledstring.freq = TARGET_FREQ;
        _ledstring.dmanum = DMA;
        _ledstring.channel[0].gpionum = GPIO_PIN;
        _ledstring.channel[0].count = _count;
        _ledstring.channel[0].invert = 0;
        _ledstring.channel[0].brightness = 128;
        _ledstring.channel[0].strip_type = STRIP_TYPE;

        if (ws2811_init(&_ledstring) != WS2811_SUCCESS)
        {
            LOG(LOG_ERR, "ws2811_init failed");
            exit(1);
        }
    }

    ~LEDControl() = default;

    void setIntensity(uint8_t intensity)
    {
        LOG(LOG_INFO, "Setting intensity to %u", intensity);
        _intensity = intensity;
        // TODO: update output
        _ledstring.channel[0].brightness = _intensity;
    }

    void setPattern(led_t* leds)
    {
        LOG(LOG_INFO, "Setting pattern");
        // TODO update output
    }

    void setAll(led_color_e color)
    {
        LOG(LOG_INFO, "Setting all to color");
        // TODO update output
        for (int i = 0; i<_count; i++){
            _ledstring.channel[0].leds[i] = color;
        }
    }

private:
    uint16_t _count;
    uint8_t _intensity = 0;
    ws2811_t _ledstring;

    void _render()
    {
        ws2811_render(&_ledstring);
        // TODO add error checking
    }

};
