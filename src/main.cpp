// Embedded Challenge - Embedded Sentry
// Due 12/20/2024
// Team Members: Sho Ishizaki, Maximilliano Vega, Jaylan Wu

// state 0 - 
// state 1 - 

// Steps:
// 1) show yellow light for ready to setup (state == 0)
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
#include <colorlib.h> //modified Adafruit_Neopixel library 
#include <SPI.h>

uint8_t x_array[100];
uint8_t y_array[100];
uint8_t z_array[100];

// constants
#define timerCounts 150   // Since we are setting 50 Hz, we get 150 timers counts in 3sec
#define NUMPIXELS 10
#define NEOPIN 17

colorlib strip(NUMPIXELS, NEOPIN); //set up LEDs 

// control state = 0 waiting for first press of button (that will be when)
// control state = 1 that is when we read the accel for the first persons gesture
// control state = 2 wait for second button press
// control state = 3 one more movement
// control state = 4 show confirmation
volatile int controlState = 0;

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

void colorNeo(uint16_t r, uint16_t g, uint16_t b){
  strip.clear();
  for(int i = 0; i < NUMPIXELS; i++){
    strip.setPixelColor(i, r, g, b);
  }
  strip.show();
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

  // setup button on PD4
  DDRD &= ~(1<<4); // Clear DDRD4 Bit to configure as input for the button

  strip.begin();
  strip.setBrightness(15);
  strip.show(); // Initialize all pixels to 'off'

   // Initialize Serial and SPI
  Serial.begin(9600);
  SPI.begin();
}

void loop() {
  // control state 0 - waiting on first button press
  if (controlState == 0) {
    // debugging
    Serial.print("Control State: ");
    Serial.println(controlState);

    // show YELLOW LED on neopixels
    colorNeo(255,255,0);
    
    // read button input
    if (PIND & (1<<PIND4)) {
      Serial.println("Button is pressed");
      controlState = 1;     // move to next state
      delay(250);
    }
  }
  // control state 1 -  read accel values 3 seconds
  if (controlState == 1) { 
    Serial.print("Control State: ");
    Serial.println(controlState);
    colorNeo(255, 128, 0); //color orange
    delay(3000);
    controlState = 2;
  }
  // control state 2 - waiting for second button press. 
  if (controlState == 2) { 
    Serial.print("Control State: ");
    Serial.println(controlState);
    colorNeo(0, 0, 255); //color blue
    // read button input
    if (PIND & (1<<PIND4)) {
      Serial.println("Button is pressed");
      controlState = 3;     // move to next state
      delay(250);
    }
  }
  // control state 3 - reading unlock
  if (controlState == 3) {
    Serial.print("Control State: ");
    Serial.println(controlState);
    colorNeo(255, 128, 0); //color orange
    delay(3000);
    controlState = 4;
  }
  if (controlState == 4) {
    Serial.print("Control State: ");
    Serial.println(controlState);
    if(true){
      colorNeo(0,255,0);      
    }
    else{
      colorNeo(255,0,0);
    }
    if (PIND & (1<<PIND4)) {
      Serial.println("Button is pressed");
      controlState = 0;     // move to next state
      delay(250);
    }
  }
}
