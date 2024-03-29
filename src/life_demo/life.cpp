#define PxMATRIX_COLOR_DEPTH   1
#define PxMATRIX_DOUBLE_BUFFER 1
#define PxMATRIX_OE_INVERT     1
#define PxMATRIX_DATA_INVERT   1

#ifdef ESP8266
#define PxMATRIX_SPI_FREQUENCY 16000000L
#endif

#include <PxMatrix.h>

// Define output pins
#if defined(ESP32)
#define P_A    16
#define P_B    17
#define P_OE   22
#define P_LAT  21
#define P_LAT2 5
// SPI pins (for the reference)
const uint8_t SPI_MOSI = 23;
const uint8_t SPI_SCK  = 18;
#elif defined(ESP8266)
#define P_A    9
#define P_B    10
#define P_OE   5
#define P_LAT  4
#define P_LAT2 16
// SPI pins (for the reference)
const uint8_t SPI_MOSI = 13;
const uint8_t SPI_SCK  = 14;
#elif defined(__AVR__)
#define P_A    2
#define P_B    3
#define P_OE   8
#define P_LAT  7
#define P_LAT2 6
// SPI pins (for the reference)
const uint8_t SPI_MOSI = 11;
const uint8_t SPI_SCK  = 13;
#endif

#if defined(ESP32)
hw_timer_t* timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
#endif
#if defined(ESP8266)
#include <Ticker.h>
Ticker ticker;
#endif
#if defined(__AVR__)
#define USE_TIMER_2 true
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#include "TimerInterrupt.h"
#endif

const uint16_t WIDTH = 32 * 6;
const uint8_t HEIGHT = 16 * 2;
PxMATRIX display(WIDTH, HEIGHT, { P_LAT, P_LAT2 }, P_OE, { P_A, P_B });

#ifdef ESP32
void IRAM_ATTR display_updater() {
    portENTER_CRITICAL_ISR(&timerMux);
    display.display(32);
    portEXIT_CRITICAL_ISR(&timerMux);
}
#endif
#if defined(ESP8266)
void display_updater() {
    display.display(32);
}
#endif
#if defined(__AVR__)
void display_updater() {
    display.display(64);
}
#endif

void setup() {
    Serial.begin(115200);
    display.begin(4);
#ifndef __AVR__
    display.setFastUpdate(true);
#endif
    // Clear both buffers
    display.clearDisplay();
    display.showBuffer();
    display.clearDisplay();
    display.showBuffer();

    // Register display scanline task
#ifdef ESP32
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 1000, true); // 1 ms
    timerAlarmEnable(timer);
#endif
#ifdef ESP8266
    ticker.attach(0.005, display_updater); // 5 ms
#endif
#ifdef __AVR__
    ITimer2.init();
    ITimer2.attachInterruptInterval(10, display_updater); // 10 ms
#endif

    delay(1000);
}

// Returns a positive reminder which in range of [0..(b-1)]
int16_t modulo(int16_t a, int16_t b) {
    int16_t m = a % b;
    if(m < 0) m = (b < 0) ? m - b : m + b;
    return m;
}

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(0 [arr]))

const uint16_t lifeCellsThreashold = (HEIGHT >> 3) * (WIDTH >> 3);

// Stall check logic is taken from https://github.com/f0rthsp4ce/ledka/blob/main/main/life.c
// Kudos for F0RTHSP4CE
uint16_t hist[12];
uint8_t  hist_idx = 0;
inline bool is_stalled(uint8_t period) {
    for(uint8_t i = period; i < ARRAY_SIZE(hist); i++)
        if(hist[i] != hist[i - period]) return false;
    return true;
}

bool life_is_stalled(uint16_t sum) {
  return (sum < lifeCellsThreashold) || is_stalled(1) || is_stalled(2) || is_stalled(3);
}

uint16_t frame = 0;
uint16_t wave = WIDTH;

uint8_t cell(int16_t x, int16_t y) {
    return (display.getPixel(x, y) == 0xFF) ? 1 : 0;
}

void loop() {
    Serial.println(++frame);

    uint16_t sum = 0;
    for(int8_t y = 0; y < HEIGHT; ++y) {
        for(int16_t x = 0; x < WIDTH; ++x) {
            int16_t xPrev = x - 1;//modulo(x - 1, WIDTH);
            int16_t xNext = x + 1;//modulo(x + 1, WIDTH);
            int8_t yPrev = y - 1;//modulo(y - 1, HEIGHT);
            int8_t yNext = y + 1;//modulo(y + 1, HEIGHT);
            if(xPrev < 0) xPrev = WIDTH - 1;
            if(yPrev < 0) yPrev = HEIGHT - 1;
            if(xNext >= WIDTH) xNext = 0;
            if(yNext >= HEIGHT) yNext = 0;
            uint8_t neighbours = cell(xPrev, y) + cell(xNext, y) +
                cell(xPrev, yPrev) + cell(x, yPrev) + cell(xNext, yPrev) +
                cell(xPrev, yNext) + cell(x, yNext) + cell(xNext, yNext);
            uint8_t current = cell(x, y);
            sum += current;
            current = (!current && (neighbours == 3)) || (current && (neighbours == 2 || neighbours == 3));
            display.drawPixel(x, y, current ? 0xFF : 0);
        }
    }
    hist[hist_idx] = sum;
    if(++hist_idx == ARRAY_SIZE(hist))
       hist_idx = 0;
    if(wave == WIDTH && life_is_stalled(sum)) {
        wave = 0;
    }
    if(wave < WIDTH) {
        for(uint8_t y = 0; y < HEIGHT; ++y) {
            uint8_t c = rand() % 2;
            display.drawPixel(wave, y, c << 7);
        }
        ++wave;
    }
    display.showBuffer();
}