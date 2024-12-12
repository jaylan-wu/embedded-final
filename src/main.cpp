// Embedded Challenge - Embedded Sentry
// Due 12/20/2024
// Team Members: Sho Ishizaki, Maximilliano Vega, Jaylan Wu

// Steps:
// 1) show yellow light for ready to setup (state == 0)
// 2) press button, yellow light turns off
// 3) record 3 seconds (record x, y, and z into 3 arrays of size 100)
// 4) show blue light to indicate unlock 
// 5) record 3 seconds and do logic while taking values
// 6) show red light if failed, green if pass

#include <Arduino.h>
#include <colorlib.h> // modified Adafruit_Neopixel library 
#include <SPI.h>

// constants
#define SPI_CS 4           // Accelerometer SPI CS on PB4
#define TIMER_COUNT 25*3   // 150 counts for 50Hz in 3sec
#define WINDOW_SIZE 31      // moving average filter window size
#define NUM_PIXELS 10      // 10 neopixels on the board
#define NEO_PIN 17         // pin for setting the neopixels

// neopixel LED setup
colorlib strip(NUM_PIXELS, NEO_PIN);

// control state = 0 waiting for first press of button (that will be when)
// control state = 1 that is when we read the accel for the first persons gesture
// control state = 2 wait for second button press
// control state = 3 one more movement
// control state = 4 show confirmation
volatile int controlState = 0;

// timer variables
volatile bool showValues = true;       // flag indicating timer activity
volatile bool showValues2 = true;       // flag indicating timer activity
volatile unsigned int timerCounter = 0;  // counter for timer 1 overflows

// function setups
void accelerometerInit();
void buttonInit();
void neoInit();
void setNeo(uint16_t r, uint16_t g, uint16_t b);
void onButtonPress();
void startRecording();
void recordValues(int16_t *bufX, int16_t *bufY, int16_t *bufZ);
bool validateSequence();
// edit print buffers to run through the list
void printBuffers();
void printBuffers2();

// accelerometer recordings and windows
int16_t X_key[TIMER_COUNT] = {0};
int16_t Y_key[TIMER_COUNT] = {0};
int16_t Z_key[TIMER_COUNT] = {0};
int16_t X_unlock[TIMER_COUNT] = {0};
int16_t Y_unlock[TIMER_COUNT] = {0};
int16_t Z_unlock[TIMER_COUNT] = {0};
int16_t X_window[WINDOW_SIZE] = {0};
int16_t Y_window[WINDOW_SIZE] = {0};
int16_t Z_window[WINDOW_SIZE] = {0};

// -------------- TIMER INTERRUPT -------------- // 
// timer interrupt on comparison
ISR(TIMER1_COMPA_vect) {
  //record the initial key values
  if (controlState == 2) {
    recordValues(X_key, Y_key, Z_key);
    timerCounter++;
  }
  //record the unlocking sequence values seperately
  if (controlState == 5) {
    recordValues(X_unlock, Y_unlock, Z_unlock);
    timerCounter++;
  }

  // stop and reset timer when it hits 75
  if (timerCounter == TIMER_COUNT) {
    Serial.print("Timer ended: ");
    Serial.println(timerCounter);
    controlState++;
    timerCounter = 0;
    TCCR1B &= ~((1 << CS11) + (1 << CS10));   // Reset the buf index
  }
}

void setup() {
  // set CS pin as OUTPUT
  DDRB |= (1 << SPI_CS);

  // Initialize Serial and SPI
  SPI.begin();
  Serial.begin(9600);

  // Initialize all necessary registers
  neoInit();
  buttonInit();
  accelerometerInit();
  Serial.println("All Systems Initialized");
}

void loop() {
  // Serial.println(controlState);
  // control state 0 - waiting on first button press
  if (controlState == 0) {
    // display BLUE on neopixels
    setNeo(0, 0, 255);
    
    // read button input
    onButtonPress();
  }

  // control state 1 -  read accel values 3 seconds
  if (controlState == 1) { 
    // display ORANGE on neopixels
    setNeo(255, 128, 0);
    // start recording values
    startRecording();
    // move to next state
    controlState++;
  }

  // control state 2 -  read accel values 3 seconds
  if (controlState == 2) { 
    // do nothing while waiting on accel values
  }

  // control state 3 - waiting for second button press. 
  if (controlState == 3) { 
    // debugging
    if (showValues) {
      printBuffers();
      showValues = false;
    }

    // display PURPLE on neopixels
    setNeo(127, 0, 255);

    //clear the window
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
      X_window[i] = 0;
      Y_window[i] = 0;
      Z_window[i] = 0;
    }


    // for (int i = 0; i < WINDOW_SIZE; i++)
    // {
    //   Serial.print(X_window[i]);
    //   Serial.print(Y_window[i]);
    //   Serial.print(Z_window[i]);
    // }


    // read button input
    onButtonPress();
  }

  // control state 4 - reading unlock
  if (controlState == 4) {
    // display ORANGE on neopixels
    setNeo(255, 128, 0);
    // simulate recording values
    startRecording();
    
    controlState++;
  }

  // control state 5 -  read accel values 3 seconds
  if (controlState == 5) { 
    // do nothing while waiting on accel values
  }

  // control state 6 - confirm or deny if unlock was correct
  if (controlState == 6) {
    if (showValues2) {
      printBuffers2();
      showValues2 = false;
    }
    // this needs to be changed
    if(validateSequence()){
      setNeo(0,255,0);      
    }
    else{
      setNeo(255,0,0);
    }
    controlState++;
  }

  if (controlState == 7) {
    // read button input
    onButtonPress();
  }
}

// -------------- INITIALIZE ACCELEROMETER -------------- //
// sets up accelerometer by writing to control register 1
void accelerometerInit() {
  PORTB &= ~(1 << SPI_CS);    // pull down the accel SPI CS
  SPI.transfer(0x20);         // LIS3DH control register 1
  SPI.transfer(0b00110111);   // LIS3DH control register 1 settings
  PORTB |= (1 << SPI_CS);     // release the accel SPI CS
  PORTB &= ~(1 << SPI_CS);    // pull down the accel SPI CS
  SPI.transfer(0x23);         // LIS3DH control register 4
  SPI.transfer(0b00001000);   // LIS3DH control register 4 settings
  PORTB |= (1 << SPI_CS);     // release the accel SPI CS
} 

// -------------- INITIALIZE BUTTON -------------- // 
void buttonInit() {
  // setup button on PD4
  // Configure DDRD4 Bit as input for the button
  DDRD &= ~(1 << 4);
}

// -------------- INITIALIZE NEOPIXELS -------------- // 
void neoInit() {
  strip.begin();
  strip.setBrightness(15);
  strip.show();
}

// -------------- COLOR NEOPIXELS -------------- // 
// takes in RGB values and changes to that color
void setNeo(uint16_t r, uint16_t g, uint16_t b) {
  strip.clear();
  for(int i = 0; i < NUM_PIXELS; i++){
    strip.setPixelColor(i, r, g, b);
  }
  strip.show();
}

// -------------- BUTTON PRESS -------------- // 
void onButtonPress() {
  if (PIND & (1<<PIND4)) {
    if (controlState == 7) {
      controlState = 0;
    } else {
      controlState++;     // move to next state
    }
    delay(400);
  }
}

// -------------- START TIMER -------------- // 
// starts Timer 1 with overflow freq at 50Hz to begin recording
void startRecording() {
  // clear global interrupts
  cli();

  // clear timer registers before we use them
  TCCR1A = 0; // Normal Operations, no PWM
  TCCR1B = 0;
  
  // set CTC mode, clear on OCR1A
  TCCR1B |= (1 << WGM12);

  // 64 prescaler
  TCCR1B |= (1 << CS11) | (1 << CS10);

   // calculate the wanted PWM 
   // (F_CPU / 64) * (1 / 25) = 5000
  OCR1A = 5000;
  
  // Enable Timer1 compare match interrupt
  TIMSK1 |= (1 << OCIE1A);

  // enable global interrupts
  sei();

  Serial.println("Timer Started");
}

// -------------- ACCELEROMETER READING -------------- // 
// reads the accelerometer XYZ values
// updates the XYZ accelerometer buffers 
// uses a moving window average filter with window of size 7
void recordValues(int16_t *bufX, int16_t *bufY, int16_t *bufZ) {
  int16_t x;
  int16_t y;
  int16_t z;
  int32_t X_avg = 0;
  int32_t Y_avg = 0;
  int32_t Z_avg = 0;
  //Continuosly read from OUT_X_L buffer (0x28)
  PORTB &= ~(1 << SPI_CS);
  SPI.transfer(0x28 | 0b11000000);
  x = (int16_t) SPI.transfer(0x00); //Low byte
  x += (((int16_t) SPI.transfer(0x00)) << 8); //High byte
  y = (int16_t) SPI.transfer(0x00); //Low byte
  y += (((int16_t) SPI.transfer(0x00)) << 8); //High byte
  z = (int16_t) SPI.transfer(0x00); //Low byte
  z += (((int16_t) SPI.transfer(0x00)) << 8); //High byte
  PORTB |= (1 << SPI_CS);

  //Data in the form of a 10 bit 2's complement left justifed 
  //If MSB is 0, then shift right by 6
  //IF MSB is 1, then shift left by 6 and add -1024 
  x = (x >> 4);
  y = (y >> 4);
  z = (z >> 4);

  for (int i = WINDOW_SIZE-2; i > -1; i--)
  {
    X_window[i+1] = X_window[i];
    Y_window[i+1] = Y_window[i];
    Z_window[i+1] = Z_window[i];
    X_avg+= X_window[i+1];
    Y_avg+= Y_window[i+1];
    Z_avg+= Z_window[i+1];
  }
  X_window[0] = x;
  Y_window[0] = y;
  Z_window[0] = z;
  X_avg = (X_avg + x)/WINDOW_SIZE;
  Y_avg = (Y_avg + y)/WINDOW_SIZE;
  Z_avg = (Z_avg + z)/WINDOW_SIZE;

  // if on first read set value
  // on second read subtract from the value
  bufX[timerCounter] = X_avg;
  bufY[timerCounter] = Y_avg;
  bufZ[timerCounter] = Z_avg;
}

bool validateSequence(){
  Serial.println("Validating Sequence");
  double tolerance = (0.5); // 50% tolerance
  int failureCount = 0; 
  for (int i = 10; i < TIMER_COUNT; i += 1) {
    bool x_valid = abs(((double)X_key[i] - (double)X_unlock[i]) / (double)X_key[i]) < tolerance;
    bool y_valid = abs(((double)Y_key[i] - (double)Y_unlock[i]) / (double)Y_key[i]) < tolerance;
    bool z_valid = abs(((double)Z_key[i] - (double)Z_unlock[i]) / (double)Z_key[i]) < tolerance;


    char buffer[30];
    sprintf(buffer, "X: %4d Y: %4d Z: %4d", x_valid, y_valid, z_valid);
    Serial.println(buffer);

    if (!(x_valid && y_valid) || !(x_valid && z_valid) || !(y_valid && z_valid)) {
      failureCount++;
    }

  }
  Serial.print("Failure Count: ");
  Serial.println(failureCount);
  if (failureCount > 15) {
    return false;
  } else {
    return true;
  }
}


// -------------- PRINT ACCELEROMETER RECORD-------------- // 
void printBuffers() {
  for (int i = 0; i < TIMER_COUNT; i++)
  {
    char buffer[30];
    sprintf(buffer, "X: %4d Y: %4d Z: %4d", X_key[i], Y_key[i], Z_key[i]);
    Serial.println(buffer);
  }
}

// -------------- PRINT ACCELEROMETER RECORD (second set) -------------- // 
void printBuffers2() {
  for (int i = 0; i < TIMER_COUNT; i++)
  {
    char buffer[60];
    sprintf(buffer, "X: %4d Y: %4d Z: %4d SECOND: X: %4d Y: %4d Z: %4d", 
                X_key[i], Y_key[i], Z_key[i],
                X_unlock[i], Y_unlock[i], Z_unlock[i]);
    Serial.println(buffer);
  }
}
