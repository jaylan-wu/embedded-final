#include <Arduino.h>
typedef uint16_t neoPixelType; ///< 3rd arg to Adafruit_NeoPixel constructor
#define NEO_GRB ((1 << 6) | (1 << 4) | (0 << 2) | (2)) ///< Transmit as G,R,B
#define NEO_KHZ800 0x0000 ///< 800 KHz data transmission

class colorlib{
public:
    colorlib(uint16_t n, int16_t pin = 6,
                    neoPixelType type = NEO_GRB + NEO_KHZ800);
    colorlib(void);
    ~colorlib();

    void begin(void);
    void show(void);
    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
    void setBrightness(uint8_t);
    void clear(void);
    void updateLength(uint16_t n);
    void updateType(neoPixelType t);
    void setPin(uint16_t p);
    bool canShow(void) {
        uint32_t now = micros();
        if (endTime > now) {
        endTime = now;
        }
        return (now - endTime) >= 300L;
    }

    
    void fill(uint32_t c, uint16_t first, uint16_t count) {
        uint16_t i, end;
    }
protected:
    bool begun;         ///< true if begin() previously called
    uint16_t numLEDs;   ///< Number of RGB LEDs in strip
    uint16_t numBytes;  ///< Size of 'pixels' buffer below
    int16_t pin;        ///< Output pin number (-1 if not yet set)
    uint8_t brightness; ///< Strip brightness 0-255 (stored as +1)
    uint8_t *pixels;    ///< Holds LED color values (3 or 4 bytes each)
    uint8_t rOffset;    ///< Red index within each 3- or 4-byte pixel
    uint8_t gOffset;    ///< Index of green byte
    uint8_t bOffset;    ///< Index of blue byte
    uint8_t wOffset;    ///< Index of white (==rOffset if no white)
    uint32_t endTime;   ///< Latch timing reference
    #ifdef __AVR__
    volatile uint8_t *port; ///< Output PORT register
    uint8_t pinMask;        ///< Output PORT bitmask
    #endif
};
