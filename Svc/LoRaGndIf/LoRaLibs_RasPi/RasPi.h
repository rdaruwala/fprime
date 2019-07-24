// RasPi.h
//
// Routines for implementing RadioHead on Raspberry Pi
// using BCM2835 library for GPIO
// Contributed by Mike Poublon and used with permission

#ifndef RASPI_h
#define RASPI_h

#include <bcm2835.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef unsigned char byte;

#ifndef NULL
  #define NULL 0
#endif

#ifndef OUTPUT
  #define OUTPUT BCM2835_GPIO_FSEL_OUTP
#endif

#ifndef INPUT
  #define INPUT BCM2835_GPIO_FSEL_INPT
#endif

#ifndef NOT_A_PIN
  #define NOT_A_PIN 0xFF
#endif

// No memcpy_P Raspberry PI
#ifndef memcpy_P
#define memcpy_P memcpy 
#endif

class SPIClass
{
  public:
    static byte transfer(byte _data);
    // SPI Configuration methods
    static void begin(); // Default
    static void begin(uint16_t, uint8_t, uint8_t);
    static void end();
    static void setBitOrder(uint8_t);
    static void setDataMode(uint8_t);
    static void setClockDivider(uint16_t);
};

extern SPIClass SPI;

class SerialSimulator
{
  public:
    #define DEC 10
    #define HEX 16
    #define OCT 8
    #define BIN 2

    // TODO: move these from being inlined
    static void begin();
    static void println(const char* s);
    static void print(const char* s);
    static void print(unsigned int n, int base = DEC);
    static void print(char ch);
    static void println(char ch);
    static void print(unsigned char ch, int base = DEC);
    static void println(unsigned char ch, int base = DEC);
};

extern SerialSimulator Serial;

void RasPiSetup();

void pinMode(unsigned char pin, unsigned char mode);

void digitalWrite(unsigned char pin, unsigned char value);

unsigned char digitalRead(unsigned char pin) ;

unsigned long RH_millis();

void delay (unsigned long delay);

long random(long min, long max);

void printbuffer(uint8_t buff[], int len);

#endif