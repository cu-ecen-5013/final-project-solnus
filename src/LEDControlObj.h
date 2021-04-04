/******************************************************************************
 * @file LEDControlObj.h
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
#ifndef __LED_CONTROL_H__

#include <stdint.h>


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

    LEDControl(uint16_t led_count)
        :_count(led_count)
    {
        // TODO initialize the LED library
    }

    ~LEDControl() = default;

    void setIntensity(uint8_t intensity)
    {
        _intensity = intensity;
        // TODO: update output
    }

    void setPattern(led_t* leds)
    {
        // TODO update output
    }

private:
    uint16_t _count;
    uint8_t _intensity = 0;

};

#endif
