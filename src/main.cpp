// Embedded Challenge - Embedded Sentry
// Due 12/20/2024
// Team Members: Sho Ishizaki, Maximilliano Vega, Jaylan Wu

// Steps:
// 1) show yellow light for ready to setup
// 2) press button, yellow light turns off
// 3) record 3 seconds (record x, y, and z into 3 arrays of size 100)
// 4) show blue light to indicate unlock 
// 5) record 3 seconds and do logic while taking values
// 6) show red light if failed, green if pass

// What is needed:
// Timer for recording 3 seconds
// 2 buttons - 1 for setup and 1 for unlocking
// LED - 4 colors

#include <Arduino.h>

uint8_t x_array[100];
uint8_t y_array[100];
uint8_t z_array[100];

void setup() {
}

void loop() {
  // put your main code here, to run repeatedly:
}
