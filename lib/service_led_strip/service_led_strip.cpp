#include "service_led_strip.h"

#define ANIMATION_INDEX 0
#define ANIMATION_END 1000

static RgbColor wheel(uint8_t pos) {
    if (pos < 85) {
        return RgbColor(pos * 3, 255 - pos * 3, 0);
    } else if (pos < 170) {
        pos -= 85;
        return RgbColor(255 - pos * 3, 0, pos * 3);
    } else {
        pos -= 170;
        return RgbColor(0, pos * 3, 255 - pos * 3);
    }
}

// TODO: use something more optimized
static HsbColor wheel(const uint8_t pos, const float b) {
    HsbColor color = HsbColor(wheel(pos));
    color.B = b;
    return color;
}

void LedStripService::animation_0(const AnimationParam &param) {
    const uint8_t step = (uint8_t)(param.progress * 255);
    for (uint16_t i = 0; i < get_len(); i++) {
        led_strip->set_pixel(i, wheel((uint8_t)(i + step), color.B));
    }
    if (param.state == AnimationState_Completed) {
        animator->RestartAnimation(ANIMATION_INDEX);
    }
}

void LedStripService::animation_1(const AnimationParam &param) {
    const uint8_t step = (uint8_t)(param.progress * 255);
    for (uint16_t i = 0; i < get_len(); i++) {
        led_strip->set_pixel(i, wheel((uint8_t)((i * 256 / get_len()) + step), color.B));
    }
    if (param.state == AnimationState_Completed) {
        animator->RestartAnimation(ANIMATION_INDEX);
    }
}

LedStripService::LedStripService(const LED_STRIP_TYPE type, const LED_STRIP_TRANSFER_MODE mode, const uint16_t len) {
    animator = new NeoPixelAnimator(ANIMATION_END, 10);
    set_config(type, mode, len);
    set_color(0, 0, 0);
}

LedStripService::~LedStripService() {
    checked_delete(animator);
}

void LedStripService::set_config(const LED_STRIP_TYPE type_, const LED_STRIP_TRANSFER_MODE mode, const uint16_t len) {
    t_mode = mode;
    type = type_;
    checked_delete(led_strip);
    switch (type) {
        case GRB:
            switch (t_mode) {
                case DMA800:
                    led_strip = new PixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod>(len);
                    return;
                case UART800:
                    led_strip = new PixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>(len);
                    return;
            }
        case GRBW:
            switch (t_mode) {
                case DMA800:
                    led_strip = new PixelBus<NeoGrbwFeature, NeoEsp8266Dma800KbpsMethod>(len);
                    return;
                case UART800:
                    led_strip = new PixelBus<NeoGrbwFeature, NeoEsp8266Uart800KbpsMethod>(len);
                    return;
            }
    }
}

void LedStripService::set_color(const uint8_t r, const uint8_t g, const uint8_t b) {
    color = HsbColor(RgbColor(r, g, b));
    if (SINGLE_COLOR != mode) {
        animator->StopAll();
        mode = SINGLE_COLOR;
    }
    led_strip->set_all_pixels(color);
}

uint32_t LedStripService::get_color() const {
    const RgbColor rgb = RgbColor(color);
    return (uint32_t)((0x00FF0000 & rgb.R << 16) | (0x0000FF00 & rgb.G << 8) | rgb.B);
}

void LedStripService::set_brightness(const uint8_t b) {
    color = HsbColor(color.H, color.S, (b % 101) / (float) 100);
    led_strip->set_all_pixels(color);
}

uint8_t LedStripService::get_brightness() const {
    return (uint8_t)(color.B * 100);
}

void LedStripService::set_delay(const uint16_t d) {
    animator->setTimeScale(d);
}

void LedStripService::set_mode(const LED_STRIP_ANIM_MODE m) {
    animator->StopAll();
    switch ((mode = m)) {
        case SINGLE_COLOR:
            led_strip->set_all_pixels(color);
            return;
        case ANIMATION_0:
            animator->StartAnimation(ANIMATION_INDEX, ANIMATION_END, [this](const AnimationParam &param) {
                animation_0(param);
            });
            break;
        case ANIMATION_1:
            animator->StartAnimation(ANIMATION_INDEX, ANIMATION_END, [this](const AnimationParam &param) {
                animation_1(param);
            });
            break;
    }
}

void LedStripService::cycle_routine() {
    // Call at least every 10ms
    if (animator != NULL && animator->IsAnimating())
        animator->UpdateAnimations();
    led_strip->refresh();
}
