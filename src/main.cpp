#include <Arduino.h>

const int OE = 13;
const int A = 13;
const int B = 8;
const int R = 12;
const int CLK = 2;
const int SCLK = 4;

#define dw digitalWrite
#define unsugarC(c)                  \
    {                                \
        dw(A, (c & 1));              \
        dw(B, ((c & 1) ^ (c >> 1))); \
    }

void setup()
{
    pinMode(OE, OUTPUT);
    pinMode(A, OUTPUT);
    pinMode(B, OUTPUT);
    pinMode(R, OUTPUT);
    pinMode(CLK, OUTPUT);
    pinMode(SCLK, OUTPUT);
    dw(OE, 0);
}

void reset()
{
    for (int a = 1; a != 4; a <<= 1)
    {
        dw(A, a >> 1);
        for (int b = 1; b != 4; b <<= 1)
        {
            dw(B, b >> 1);
            for (int p = 0; p < 128; p++)
            {
                dw(R, 1);
                dw(CLK, 0);
                dw(CLK, 1);
            }
            dw(SCLK, 0);
            dw(SCLK, 1);
        }
    }
}

int clock = 0;
void loop()
{
    clock += 1;
    clock %= 1 << 11;
    // Write a thing
    for (int c = 0; c < 4; c++)
    {
        unsugarC(c)
        {
            for (int p = 0; p < 128; p++)
            {
                dw(R, ((p << 2) | c) < (clock >> 0));
                dw(CLK, 0);
                dw(CLK, 1);
            }
            dw(SCLK, 0);
            dw(SCLK, 1);
        }
    }
    for (int c = 0; c < 4; c++)
    {
        unsugarC(c)
        {
            for (int p = 0; p < 128; p++)
            {
                dw(R, ((p << 2) | c) < (clock >> 1));
                dw(CLK, 0);
                dw(CLK, 1);
            }
            dw(SCLK, 0);
            dw(SCLK, 1);
        }
    }
    for (int c = 0; c < 4; c++)
    {
        unsugarC(c)
        {
            for (int p = 0; p < 128; p++)
            {
                dw(R, ((p << 2) | c) < (clock >> 4));
                dw(CLK, 0);
                dw(CLK, 1);
            }
            dw(SCLK, 0);
            dw(SCLK, 1);
        }
    }
}