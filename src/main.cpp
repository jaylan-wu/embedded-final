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
#include <SPI.h>

uint8_t x_array[100];
uint8_t y_array[100];
uint8_t z_array[100];

// constants
#define timerCounts 150   // Since we are setting 50 Hz, we get 150 timers counts in 3sec

// variables for the timer
volatile bool timerActive = false;       // flag indicating timer activity
volatile unsigned int timerCounter = 0;  // counter for timer 1 overflows

// variables for the accelerometer
const uint16_t x_low = 0x28;
const uint16_t x_high = 0x29;
const uint16_t y_low = 0x2A;
const uint16_t y_high = 0x2B;
const uint16_t z_low = 0x2C;
const uint16_t z_high = 0x2D;

// timer interrupt on comparison
ISR(TIMER1_COMPA_vect) {
  if (timerActive) {
    timerCounter++;
    Serial.println(timerCounter);

    if (timerCounter >= timerCounts) {
      timerActive = false;  // Deactivate the timer
      Serial.println("Timer ended!");
    }
  }
}

void setup() {
  // clear global interrupts
  cli();

  // clear timer registers before we use them
  TCCR1A = 0; // Normal Operations, no PWM
  TCCR1B = 0;
  
  // set CTC mode,
  TCCR1B |= (1 << WGM12);

  // 64 prescaler
  TCCR1B |= (1 << CS11) | (1 << CS10);

   // calculate the wanted PWM 
   // (F_CPU / 64) * (1 / 50) = 2500
  OCR1A = 2500;
  
  // Enable Timer1 compare match interrupt
  TIMSK1 |= (1 << OCIE1A);

  // enable global interrupts
  sei();

  // setup accelerometer
  DDRB |= (1 << 4);

   // Initialize Serial and SPI
  Serial.begin(9600);
  SPI.begin();
}

void loop() {

  PORTB &= ~(1 << 4);
  SPI.transfer(0x0F | (1 << 7));
  uint8_t res = SPI.transfer(0x00);
  Serial.println(res, BIN);

  PORTB |= (1 << 4);
  delay(1000);

  // if (!timerActive) {
  //   delay(1000); // Wait for 1 second
  //   timerCounter = 0; // Reset the counter
  //   timerActive = true; // Reactivate the timer
  //   Serial.println("Timer restarted!");
  // }
}
