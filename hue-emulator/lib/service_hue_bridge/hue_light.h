#ifndef ESP8266_PROJECTS_ROOT_HUE_LIGHT_H
#define ESP8266_PROJECTS_ROOT_HUE_LIGHT_H

#include <hue_commons.h>
#include <service_led_strip.h>

class LedLight : public HueLight {
private:
    LedStripService *ls;
protected:
    uint8_t get_brightness() const override;

public:
    LedLight(LedStripService *l, const char *n, const uint8_t index);

    void set_color_cie(float x, float y) override;

    void set_color_rgb(const uint8_t r, const uint8_t g, const uint8_t b) override;

    void set_color_ct(const uint32_t ct) override;

    void set_state(const bool s) override;

    void set_hue(const uint16_t h) override;

    void set_brightness(const uint8_t b) override;

    void set_saturation(const uint8_t s) override;

    virtual void set_transition(const uint16_t) override;
};

#endif //ESP8266_PROJECTS_ROOT_HUE_LIGHT_H
