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
#include <string.h>

// defaults for cmdline options
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                18
#define DMA                     10
#define STRIP_TYPE              WS2811_STRIP_GRB

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
        LED_W = 0x00101010,
        LED_R = 0x00200000,
        LED_G = 0x00002000,
        LED_B = 0x00000020
    } ;
    
    LEDControl(uint16_t led_count)
        :_count(led_count)
    {
        memset(&_ledstring, 0, sizeof(_ledstring));

        _ledstring.freq = TARGET_FREQ;
        _ledstring.dmanum = DMA;
        _ledstring.channel[0].gpionum = GPIO_PIN;
        _ledstring.channel[0].count = _count;
        _ledstring.channel[0].invert = 0;
        _ledstring.channel[0].brightness = 255;
        _ledstring.channel[0].strip_type = STRIP_TYPE;

        if (ws2811_init(&_ledstring) != WS2811_SUCCESS)
        {
            LOG(LOG_ERR, "ws2811_init failed");
            exit(1);
        }
    }

    ~LEDControl() = default;
    void setNumLED (uint16_t _ledCount)
    {
        LOG(LOG_INFO, "Setting number of LED to %d", _ledCount);
        _count = _ledCount;
        _ledstring.channel[0].count = _count;
        _render();
    }

    void setIntensity(float intensity)
    {
        if((intensity>=0)&&(intensity<=1)){
            LOG(LOG_INFO, "Setting intensity to %f", intensity);    
            _intensity = (uint8_t)(intensity * 255);
            _ledstring.channel[0].brightness = _intensity;
            _render();
        }
    }

    void setPattern(led_t* leds)
    {
        LOG(LOG_INFO, "Setting pattern");
        // TODO update output 
       

        _render();
    }

    void setNewColor(led_t color)
    {
        LOG(LOG_INFO, "Setting all to color");
        for (int i = 0; i<_count; i++){
            _ledstring.channel[0].leds[i] = color.wrgb;
        }
        _render();
    }

    void setAllOff()
    {
        LOG(LOG_INFO, "Setting all LED off");
        for(int i =0; i<_count; i++){
            _ledstring.channel[0].brightness = 0;
        }
        //_count =  0;
        //_ledstring.channel[0].count = _count;
        _render();
    }

private:
    uint16_t _count;
    uint8_t _intensity = 0;
    ws2811_t _ledstring;

    void _render()
    {
        //LOG(LOG_DEBUG, "Rendering");
        ws2811_render(&_ledstring);
        //LOG(LOG_DEBUG, "Rendered");
        // TODO add error checking
    }

};
