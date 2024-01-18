#define PxMATRIX_COLOR_DEPTH   1
#define PxMATRIX_OE_INVERT     1
#define PxMATRIX_DATA_INVERT   1

#include <PxMatrix.h>

#ifndef ESP32
#error "Other platform is not supported"
#endif

// Define output pins
#define P_A    16
#define P_B    17
#define P_OE   22
#define P_LAT  21
#define P_LAT2 5
// SPI pins (for the reference)
const uint8_t SPI_MOSI = 23;
const uint8_t SPI_SCK  = 18;

hw_timer_t* timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

const uint16_t WIDTH = 32 * 6;
const uint8_t HEIGHT = 16 * 2;
PxMATRIX display(WIDTH, HEIGHT, { P_LAT, P_LAT2 }, P_OE, { P_A, P_B });

#include <text_example/FreeSans10.h>

void IRAM_ATTR display_updater() {
    portENTER_CRITICAL_ISR(&timerMux);
    display.display(32);
    portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
    Serial.begin(115200);
    display.begin(4);
    display.setFastUpdate(true);
    display.setFont(&FreeSans10pt8b);

    // Clear buffer
    display.clearDisplay();
    display.fillScreen(0xFF);
    display.showBuffer();

    // Register display scanline task
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 1000, true); // 1 ms
    timerAlarmEnable(timer);

    delay(3000);
}

uint16_t frame = 0;
int8_t pos_x = 0;
int8_t pos_y = 0;
int8_t MAX_X = 19;
int8_t MAX_Y = 13;

void loop() {
    Serial.println(++frame);

    display.clearDisplay();
    display.drawRect(0, 0, 192, 32, 0xFF);
    display.drawRect(1, 1, 192-2, 32-2, 0xFF);
    uint8_t offset_x = pos_x >= 0 ? pos_x : -pos_x;
    uint8_t offset_y = pos_y >= 0 ? pos_y : -pos_y;
    display.setCursor(1 + offset_x, 16 + offset_y);
    display.print("ХАКЕР ЭМБАССИ");

    if(++pos_x == MAX_X)
        pos_x = -pos_x + 1;
    if(++pos_y == MAX_Y)
        pos_y = -pos_y;

    display.showBuffer();
    delay(100);
}