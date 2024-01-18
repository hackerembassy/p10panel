#include <Arduino.h>
#include <SPI.h>

// const int OE = 7;
// const int A = 13;
// const int B = 8;
// const int R = 12;
// const int CLK = 2;
// const int SCLK = 4;

const int OE = 7;
const int A = 13;
const int B = 12;
const int R = 8;
const int CLK = 2;
const int SCLK = 4;

byte buf[64];
char sprintbuf[128];

#define dw digitalWrite
#define unsugarC(c)                  \
    {                                \
        dw(A, (c & 1));              \
        dw(B, ((c & 1) ^ (c >> 1))); \
    }

int coordToP(int x, int y)
{
    return (((31 - x) >> 3) << 7) | ((y >> 2) << 5) | (((7 - x) & 7) << 2) | (y & 3);
}

void setup()
{
    Serial.begin(38400);
    pinMode(OE, OUTPUT);
    pinMode(A, OUTPUT);
    pinMode(B, OUTPUT);
    pinMode(R, OUTPUT);
    pinMode(CLK, OUTPUT);
    pinMode(SCLK, OUTPUT);
    dw(OE, 1);
    for (int i = 0; i < 64; i++)
        buf[i] = ~0;
    // for (int x = 0; x < 32; x++)
    // {
    //     for (int y = 0; y < 16; y++)
    //     {
    //         double len = sqrt((x - 5) * (x - 5) + (y - 5) * (y - 5));
    //         if (len <= 10)
    //         {
    //             int br = (10.0 - len) * 4;
    //             if (br > 4)
    //                 br = 4;
    //             buf[coordToP(x, y)] = br;
    //         }
    //     }
    // }
}
int clock = 0;

void loop()
{
    clock += 1;
    clock %= 12 * 3 * 7;
#ifdef FRAMETIMES
    long time = micros();
#endif

    Serial.readBytes(buf, 64);
    for (int c = 0; c < 4; c++)
    {
        unsugarC(c);
        {
            for (int p = 0; p < 128; p++)
            {
                int px = ((p << 2) | c);

                int subm = (px / 32);
                int submx = subm / 4;
                int submy = subm % 4;
                int x = 32 - (submx * 8 + (px % 32 / 4));
                int y = submy * 4 + (px % 4);
                
                dw(R, !(buf[(x / 8) + y * 4] >> (7 - x % 8) & 1));
                // dw(R, x != (clock / 3 % 32) && y != (clock / 7 % 16));
                // dw(R, px > clock);
                dw(CLK, 0);
                dw(CLK, 1);
            }
            dw(SCLK, 0);
            dw(SCLK, 1);
        }
    }
#ifdef FRAMETIMES
    long diff = micros() - time;
    if (clock == 0)
    {
        sprintf(sprintbuf, "Frame in %d", diff);
        Serial.println(sprintbuf);
    }
#endif
}