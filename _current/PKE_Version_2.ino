/**************************************************************************
  Using the examples for the OLED screen + the Stepper Motor, this will
  initally ONLY handle an example loop of the arms raising and lowering
  as well as a few animation / screen examples. The EMF Functionality is not
  currently included in this version (as of v1)

  TODO
   - OPTIONAL : Update Wing Positioning for Button Press
   - BUG : Weird retraction issue in initialization run
   - ADD : LEDs Loop for Wing LEDS
   - OPTIONAL : LED's in Case of Meter
    

    
    


COMPLETE
    - Adjust start up order (Complete)
    - Create GB Logo 16x16 (Complete)
    - Integrated 9v battery (Completeish)
    - Create Scanning Animation/Page
    - EMF Functionality
    - HARDWARE = solder currently 'taped' connections for more secure power flow (Complete)
    - ground wire to board for Motor Controller (Complete)
    - Implement Button Controls of Stepper Motor (Complete)
    - HARDWARE = Open Step Motor area for air flow / add open vents - Doesn't Seem Needed (Complete)



  *************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Stepper.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define BUTTON_PIN 25 //Shouldbe GPIO 21
#define DEBOUNCE_TIME 50 //Debounce in Milli

//Variables for Button
int lastSteadyState = LOW;
int lastFlickerableState = LOW;
int currentState;
int bootint = 1;
unsigned long lastDebounceTime = 0;

const int stepsPerRevolution = 2048;

const int  baud_rate = 115200;

// ULN2003 Motor Driver Pins
#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 17

#define PIN_ANTENNA 27
#define CHECK_DELAY 1000
#define lmillis() ((long)millis())
// Initialize Stepper Motor
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

//Initialize Screens
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   64
#define LOGO_WIDTH    64
static const unsigned char PROGMEM logo_bmp[] =
{
	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xe8, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0xfb, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xbd, 0x88, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0xff, 0xc2, 0x40, 0x00, 0x00, 0x00, 0x00, 0x03, 0xbb, 0xd0, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xbb, 0xc4, 0x28, 0x00, 0x00, 0x00, 0x00, 0x03, 0xbb, 0xc9, 0x09, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0xff, 0xc0, 0x90, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xc6, 0x06, 0x40, 0x00, 
	0x00, 0x00, 0x0f, 0xff, 0xe0, 0x60, 0x40, 0x00, 0x00, 0x00, 0x1f, 0xf7, 0xe1, 0x09, 0x10, 0x00, 
	0x00, 0x00, 0x1f, 0xef, 0xf0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xf0, 0x04, 0x64, 0x00, 
	0x00, 0x00, 0x3f, 0xff, 0xf0, 0x00, 0x04, 0x00, 0x00, 0x01, 0x3f, 0xef, 0xf7, 0x02, 0x90, 0x00, 
	0x00, 0x00, 0x3f, 0xcf, 0xf7, 0xf0, 0x12, 0x00, 0x00, 0x02, 0x7f, 0xcf, 0xf7, 0xf9, 0x02, 0x00, 
	0x00, 0x01, 0x1f, 0xef, 0xff, 0xe0, 0xc8, 0x80, 0x00, 0x04, 0x1f, 0xff, 0xef, 0xcc, 0x08, 0x00, 
	0x00, 0x00, 0x8f, 0xff, 0xff, 0x83, 0x23, 0x00, 0x00, 0x02, 0x57, 0xc7, 0xff, 0x30, 0x20, 0x38, 
	0x00, 0x09, 0x3f, 0xff, 0xfc, 0x8c, 0x8c, 0x70, 0x00, 0x00, 0x7f, 0xff, 0xfa, 0x62, 0x01, 0xe0, 
	0x00, 0x06, 0x7f, 0xff, 0xf3, 0x12, 0x5b, 0xc0, 0x00, 0x10, 0xff, 0xff, 0xe8, 0xc9, 0x7f, 0xc0, 
	0x00, 0x01, 0xff, 0xff, 0x96, 0x28, 0xff, 0x80, 0x40, 0x09, 0xff, 0xff, 0x53, 0x25, 0xff, 0xa0, 
	0x3c, 0x0b, 0xff, 0xfe, 0x49, 0x93, 0xff, 0xfe, 0x1e, 0x07, 0xff, 0xfd, 0x3c, 0xcb, 0xff, 0xfe, 
	0x0f, 0x8f, 0xff, 0xf1, 0xa6, 0x1d, 0xff, 0xc0, 0x1f, 0xf7, 0xff, 0xe6, 0x63, 0x3f, 0xff, 0xc0, 
	0x3f, 0xff, 0xff, 0xc9, 0x59, 0x7b, 0xc8, 0xe0, 0x4f, 0xff, 0xf3, 0x99, 0x4d, 0xfb, 0x80, 0x30, 
	0x03, 0xff, 0xe2, 0x26, 0x63, 0xfa, 0xf1, 0x00, 0x0f, 0xff, 0xc0, 0x42, 0x97, 0xff, 0xf0, 0x40, 
	0x0b, 0xff, 0x81, 0x19, 0x8f, 0xff, 0xe6, 0x00, 0x00, 0xba, 0x01, 0x24, 0x7f, 0xff, 0xe0, 0x80, 
	0x00, 0x1c, 0x08, 0xa6, 0x7f, 0xff, 0xe0, 0x80, 0x00, 0x0c, 0x4a, 0x10, 0xff, 0xff, 0xca, 0x20, 
	0x00, 0x04, 0x41, 0x49, 0xff, 0xff, 0xc2, 0x00, 0x00, 0x01, 0x11, 0x27, 0xff, 0xff, 0x90, 0xc0, 
	0x00, 0x00, 0x12, 0x21, 0xff, 0xff, 0x84, 0x00, 0x00, 0x06, 0x40, 0x82, 0x7f, 0xff, 0x09, 0x00, 
	0x00, 0x00, 0x4c, 0x07, 0xc7, 0xfe, 0x20, 0x00, 0x00, 0x01, 0x01, 0x07, 0xff, 0xfc, 0x24, 0x80, 
	0x00, 0x00, 0x30, 0x0f, 0xff, 0xf8, 0x82, 0x00, 0x00, 0x00, 0x86, 0x1f, 0xff, 0xf0, 0x18, 0x00, 
	0x00, 0x00, 0x00, 0x8f, 0xff, 0xc2, 0x40, 0x00, 0x00, 0x00, 0x60, 0x23, 0xff, 0x02, 0x44, 0x00, 
	0x00, 0x00, 0x0d, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x52, 0x00, 0x91, 0x80, 0x00, 
	0x00, 0x00, 0x12, 0x44, 0x92, 0x08, 0x20, 0x00, 0x00, 0x00, 0x01, 0x08, 0x10, 0x62, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0x22, 0x45, 0x04, 0x80, 0x00, 0x00, 0x00, 0x00, 0x81, 0x08, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x34, 0x62, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x02, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

};



void setup() {
  
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(PIN_ANTENNA, INPUT);
    //Step 1: Screen voltage loop
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3v interally (IE over Serial)
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  };

//Step 2: Display Adafruit and Ghostbusters Logo
  //display.display();//Should display Adafruit Industries Logo
  //delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  
  //Draw GB Logo Bitmap
  //testdrawbitmap();
  boot_anim_Heading(baud_rate);


//Step 7 MAIN LOOP: Pre version 2, example animation of main screen with selection info at bottom. Version 2 will have EMF readings instead of example data/loop


  display.clearDisplay();

}

void showReadings(int emfValue) {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.drawRoundRect(0, 6, 126, 56, 3, 1);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("PKE:");
    display.drawLine(16, 16, 16, 62, 1);
    display.drawLine(32, 16, 32, 62, 1);
    display.drawLine(48, 16, 48, 62, 1);
    display.drawLine(64, 16, 64, 62, 1);
    display.drawLine(80, 16, 80, 62, 1);
    display.drawLine(96, 16, 96, 62, 1);
    display.drawLine(112, 16, 112, 62, 1);
    display.drawLine(0, 24, 125, 24, 1);
    display.drawLine(0, 32, 125, 32, 1);
    display.drawLine(0, 40, 125, 40, 1);
    display.drawLine(0, 48, 125, 48, 1);
    display.drawLine(0, 56, 125, 56, 1);
    
    display.setCursor(64, 0);
    display.println("Class: IV");
    

    display.setCursor(24,0);
    display.println(emfValue);
    

    display.display();
}

void loop() {

  currentState = digitalRead(BUTTON_PIN);

  if (currentState != lastFlickerableState) {
    lastDebounceTime = millis();
    lastFlickerableState = currentState;
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    
    if(lastSteadyState == HIGH && currentState == LOW){
     myStepper.setSpeed(8);
      Serial.println(F("ButtonPress 1"));
      myStepper.step(1500);
    } else if(lastSteadyState == LOW && currentState == HIGH){
      myStepper.setSpeed(8);
      Serial.println(F("ButtonPress R"));
      if(bootint == 1){
        bootint = 0;
        myStepper.step(0);
      } else if(bootint == 0){
        myStepper.step(-1500);
      }
      

    }
   lastSteadyState = currentState;
  }
  
  


  //testdrawline();      // Draw many lines
  
  //The Below SHOULD display the "PKE" (Read: EMF) reading on the screen
  
  static int avgValue = 0, emfValue = 0;
    static long nextCheck = 0, emfSum = 0, iterations = 0;

    emfValue = constrain(analogRead(PIN_ANTENNA), 0, 1023);
    emfSum += emfValue;
    iterations++;
    if (lmillis() - nextCheck >= 0) {
        avgValue = emfSum / iterations;
        emfSum = 0;
        iterations = 0;
        showReadings(avgValue);
        nextCheck = lmillis() + CHECK_DELAY;
    }


    //display.drawRoundRect(0, 20, 128, 37, 2, WHITE);
    
    display.fillCircle(16, 24, 4, 1);
    display.fillCircle(32, 24, 4, 1);
    display.fillCircle(48, 24, 4, 1);
    display.fillCircle(64, 40, 4, 1);
    display.fillCircle(80, 56, 4, 1);
    display.fillCircle(96, 40, 4, 1);
    display.fillCircle(112, 24, 4, 1);
    display.fillRect(0, 9, 122, 5, BLACK);
    display.fillRect(0, 9, map(emfValue, 0, 1023, 0, 122), 5, WHITE);
    display.display();

}


// Classes and Internal...stuff...idk.

void testdrawline() {
  int16_t i;

  display.clearDisplay(); // Clear display buffer

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn line
    delay(1);
  }
  for(i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  delay(2000); // Pause for 2 seconds
}


void boot_anim_Heading(int BAUDRATE) {
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  heading_print();
  delay(2000);
  display.setCursor(1,25);
  display.println("Ghostbusters LLC");
  display.setCursor(1,35);
  display.println("Phone: 555-2368");
  display.setCursor(1,45);
  display.println("14 N Moore St");
  display.setCursor(1,55);
  display.println("New York City");
  display.display();
  delay(5000);
  display.clearDisplay();
  heading_print();
  delay(1000);
  display.setCursor(1,9);
  display.println("Initialize: Hardware");
  display.display();
  delay(500);
  display.setCursor(1,20);
  display.println("SERIAL @ 1152K BAUD..");
  //display.println("12345123451234512345");
  display.display();
  delay(500);
  display.setCursor(1,30);
  myStepper.setSpeed(10);//Speed of Stepper Motor
  display.println("WING STEP SPEED @ 100%");
  display.display();
  delay(500);
  display.setCursor(1,40);
  myStepper.step(1023); //Extend Arms 1023 steps from start
  delay(1500);
  
  display.println("WING TEST EXTEND...OK");
  display.display();
  delay(500);
  display.setCursor(1,50);
  myStepper.step(-1023); //retract arms 1023 steps To start
  display.println("WING TEST RETRACT..OK");
  display.display();
  delay(2500);
  display.clearDisplay();
  heading_print();
  delay(1500);
  display.setCursor(1,9);
  display.println("Initialize Software");
  display.display();
  delay(500);
  display.setCursor(1,20);
  display.println("Load: Tobin Guide.zip");
  display.display();
  delay(500);
  display.setCursor(1,30);
  display.println("Load: PKE_RANGE_1.zip");
  display.display();
  delay(500);
  display.setCursor(1,40);
  display.println("Load: FARM_UPDATE.zip");
  display.display();
  delay(500);
  display.setCursor(1,50);
  display.println("Load: CLASS_CHART.zip");
  display.display();
  delay(2500);
  display.clearDisplay();
  delay(500);

  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(1000);
  //display.clearDisplay();
  display.setCursor(1,50);
  display.println("Who Ya Gonna Call??");
  display.display();
  delay(2000);
  display.clearDisplay();

}

void heading_print(){
  display.setCursor(1,0);
  display.println("GhostOS v2.30 (c)1989");
  display.display();
}
