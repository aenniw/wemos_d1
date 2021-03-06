#ifndef COMMONS_H
#define COMMONS_H

#include <stdlib.h>
#include <stdint.h>

#define CONFIG_GLOBAL_JSON      "/json/config-global.json"

#define MIN(a,b) ((a<b)?a:b)
#define MAX(a,b) ((a>b)?a:b)
#define ABS(a) (((a)>=0)?(a):-(a))

void *checked_free(void *ptr);

class Service {
public:
    virtual void begin() {};
    virtual void cycle_routine() = 0;
};

typedef enum {
    ANY, DIGITAL_IO
} DEVICE_TYPE;

class Device {
public:
    virtual uint8_t get_id() const = 0;

    virtual DEVICE_TYPE get_type() const = 0;

    virtual ~Device() {};
};

#endif //COMMONS_H
