#ifndef ESP8266_PROJECTS_ROOT_FILE_STREAMS_H
#define ESP8266_PROJECTS_ROOT_FILE_STREAMS_H

#include <FS.h>
#include <initializer_list>

class FStream : public Stream {
public:
    virtual int read() {
        byte d = 0;
        if (available()) {
            read(&d, 1);
        }
        return d;
    };

    virtual int peek() {
        if (available())
            return read();
        return 0;
    };

    virtual size_t write(uint8_t) {
        return 0;
    };

    virtual int read(byte *buf, uint16_t nbyte) = 0;

    virtual int available() = 0;

    virtual uint32_t size() const = 0;

    virtual const char *name() const = 0;

    virtual void rewind()=0;

    virtual ~FStream() {};
};

class MultiFileStream : public FStream {
private:
    char **files;
    uint32_t p = 0, files_len = 0;
public:
    MultiFileStream(std::initializer_list<const char *> fs);

    virtual int read(byte *buf, uint16_t nbyte) override;

    int available() override;

    uint32_t size() const override;

    const char *name() const override;

    void rewind() override;

    ~MultiFileStream();

};

#endif //ESP8266_PROJECTS_ROOT_FILE_STREAMS_H
