#pragma once
#include <Arduino.h>

#define ASC_TAB 0x09
#define ASC_LF 0x0A
#define ASC_CR 0x0D
#define ASC_HT 0x09
#define ASC_FF 0x0C
#define ASC_ESC 0x1B
#define ASC_GS 0x1D
#define ASC_FS 0x1C
#define ASC_DC2 0x12

#define FONT_B bit(0)
#define FONT_REVERSE bit(1)
#define FONT_FLIP bit(2)
#define FONT_EMPH bit(3)
#define FONT_DOUBLE_H bit(4)
#define FONT_DOUBLE_W bit(5)
#define FONT_DELLINE bit(6)

class Printer : public Print {
    class Reader {
       public:
        Reader(Stream* stream, uint16_t timeout = 500) : _stream(stream), _timeout(timeout) {}
        Reader(const uint8_t* data, bool pgm) : _data(data), _pgm(pgm) {}

        uint8_t read() {
            if (_stream) {
                if (!_stream->available()) {
                    int count = 0;
                    while (!_stream->available()) {
                        if (++count == _timeout) {
                            _online = 0;
                            break;
                        }
                        delay(1);
                    }
                }
                return _online ? _stream->read() : 0;
            } else if (_data) {
                return _pgm ? pgm_read_byte(_data++) : *_data++;
            }
            return 0;
        }

        bool online() {
            return _online;
        }

       private:
        Stream* _stream = nullptr;
        bool _online = 1;
        uint16_t _timeout = 0;
        const uint8_t* _data = nullptr;
        bool _pgm = 0;
    };

   public:
    Printer(Stream& stream) : stream(stream) {}

    size_t write(uint8_t data) {
        _write(data);
        return 1;
    }

    void begin() {
        wake();
        init();
    }

    void font(uint8_t mode) {
        _write(ASC_ESC, '!', mode);
    }
    void setCharTable(uint8_t table) {
        _write(ASC_ESC, 't', table);
    }

    void beginBitmap(uint8_t w, uint8_t h) {
        _write(ASC_DC2, '*', h, w);
    }

    void drawBitmap(const uint8_t* bitmap, uint16_t w, uint16_t h, bool pgm = true) {
        _drawBitmap(Reader(bitmap, pgm), w, h);
    }

    void drawBitmap(Stream* stream, uint16_t w, uint16_t h, uint16_t timeout = 500) {
        _drawBitmap(Reader(stream, timeout), w, h);
    }

    void config(uint8_t dots = 7, uint8_t time = 80, uint8_t interval = 2) {
        _write(ASC_ESC, '7', dots, time, interval);
    }

    void init() {
        _write(ASC_ESC, '@');
    }

    void wake() {
        _write(0xff);
        delay(60);
    }

    void sleep() {
        _write(ASC_ESC, '8', 1, 0);
    }

    void statusBack(bool rts = 1, bool asb = 0) {
        _write(ASC_GS, 'a', (asb << 2) | (rts << 5));
    }

   private:
    Stream& stream;

    void _write(uint8_t b0) {
        stream.write(b0);
    }
    void _write(uint8_t b0, uint8_t b1) {
        stream.write(b0);
        stream.write(b1);
    }
    void _write(uint8_t b0, uint8_t b1, uint8_t b2) {
        stream.write(b0);
        stream.write(b1);
        stream.write(b2);
    }
    void _write(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
        stream.write(b0);
        stream.write(b1);
        stream.write(b2);
        stream.write(b3);
    }
    void _write(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
        stream.write(b0);
        stream.write(b1);
        stream.write(b2);
        stream.write(b3);
        stream.write(b4);
    }

    void _drawBitmap(Reader reader, uint16_t w, uint16_t h) {
        w = (w + 8 - 1) / 8;  // round up

        while (h) {
            uint32_t hh;

            if (h <= 255) {
                hh = h;
                h = 0;
            } else {
                hh = 255;
                h -= 255;
            }

            beginBitmap(w, hh);
            hh *= w;
            while (hh--) {
                _write(reader.read());
                delay(0);
            }
            if (!reader.online()) return;
        }
    }
};