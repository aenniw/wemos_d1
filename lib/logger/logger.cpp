#include "logger.h"

WebSocketsServer Log::webSocket(8181);

void Log::webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght) {
    switch (type) {
        case WStype_ERROR:
            Log::println("[%u] Error!\n", num);
            break;
        case WStype_DISCONNECTED:
            Log::println("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = Log::webSocket.remoteIP(num);
            Log::println("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        }
            break;
        case WStype_TEXT:
            Log::println("[%u] get Text: %s\n", num, payload);
            break;
        case WStype_BIN:
            Log::println("[%u] get binary lenght: %u\n", num, lenght);
            break;
    }
}


void Log::init() {
    Log::webSocket.begin();
    Log::webSocket.onEvent(webSocketEvent);
#ifdef __DEBUG__
    Serial.begin(115200);
    Serial.println();
#endif
}

void Log::print(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char *buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = new char[len + 1];
        if (!buffer) {
            return;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
#ifdef __DEBUG__
    Serial.print(buffer);
#endif
    Log::webSocket.broadcastTXT(buffer);
    if (buffer != temp) {
        delete[] buffer;
    }
}

void Log::println(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char *buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = new char[len + 1];
        if (!buffer) {
            return;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
#ifdef __DEBUG__
    Serial.println(buffer);
#endif
    Log::webSocket.broadcastTXT(buffer);
    if (buffer != temp) {
        delete[] buffer;
    }
}

void Log::println(const String &msg) {
#ifdef __DEBUG__
    Serial.println(msg);
#endif
    Log::webSocket.broadcastTXT(msg.c_str());
}

void Log::cycle_routine() {
    Log::webSocket.loop();
}