#include "Adafruit_ThinkInk.h"

#ifdef ARDUINO_ADAFRUIT_FEATHER_RP2040_THINKINK // detects if compiling for
                                                // Feather RP2040 ThinkInk
#define EPD_DC PIN_EPD_DC       // ThinkInk 24-pin connector DC
#define EPD_CS PIN_EPD_CS       // ThinkInk 24-pin connector CS
#define EPD_BUSY PIN_EPD_BUSY   // ThinkInk 24-pin connector Busy
#define SRAM_CS -1              // use onboard RAM
#define EPD_RESET PIN_EPD_RESET // ThinkInk 24-pin connector Reset
#define EPD_SPI &SPI1           // secondary SPI for ThinkInk
#else
#define EPD_DC 10
#define EPD_CS 9
#define EPD_BUSY 7 // can set to -1 to not use a pin (will wait a fixed delay)
#define SRAM_CS 6
#define EPD_RESET 8  // can set to -1 and share with microcontroller Reset!
#define EPD_SPI &SPI // primary SPI
#endif

// 1.54" Monochrome displays with 200x200 pixels and SSD1681 chipset
ThinkInk_154_Mono_D67 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL343.h>

#define ADXL343_SCK 13
#define ADXL343_MISO 12
#define ADXL343_MOSI 11
#define ADXL343_CS 10

// Rock characteristics
#define THRESHHOLD_UP 4
#define THRESHHOLD_DOWN 3.5

// Refresh characteristics
#define REFRESH_RATE 6000
#define TICK_DELAY 100

// Debugging tools

#define PLOTTING

/* Assign a unique ID to this sensor at the same time */
/* Uncomment following line for default Wire bus      */
Adafruit_ADXL343 accel = Adafruit_ADXL343(12345);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  display.begin(THINKINK_MONO);
  display.clearBuffer();
  display.setTextSize(10);

  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL343 ... check your connections */
    Serial.println("Ooops, no ADXL343 detected ... Check your wiring!");
    while(1);
  }

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL343_RANGE_2_G);

  #ifdef PLOTTING
  Serial.println("X\tThreshhold-up\tThreshhold-down");
  #endif
}

int tick = 0;
int rock_count = 0;
int old_rock_count = rock_count;

char buf[10];


bool up;

void loop() {

  /* Get a new sensor event */
  sensors_event_t event;
  accel.getEvent(&event);

  #ifdef PLOTTING
  Serial.print(event.acceleration.x);Serial.print("\t");
  Serial.print(THRESHHOLD_UP);Serial.print("\t");
  Serial.print(THRESHHOLD_DOWN);Serial.println("");
  #else
  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.println("  ");
  #endif


  // redraw only every few seconds, and only if there has been a new rock
  if (tick++ % (REFRESH_RATE/TICK_DELAY) == 0 && rock_count != old_rock_count) {

    old_rock_count = rock_count;

    // Add the current number to the buffer
    sprintf(buf, "%d", rock_count);
    testdrawtext(
        buf,
        EPD_BLACK);
        
    // Display on the screen what's in the buffer
    display.display();
  
    display.clearBuffer();

  }

  if (up == false && event.acceleration.x >= THRESHHOLD_UP) {
    up = true;
    rock_count++;
  }

  if (up == true && event.acceleration.x <= THRESHHOLD_DOWN) {
    up = false;
  }

  delay(TICK_DELAY);

}

void testdrawtext(const char *text, uint16_t color) {
  display.setCursor(30, 60); // TODO: make it auto center using text size and subtracting half
  display.setTextColor(color);
  display.setTextWrap(true);
  display.print(text);
}
