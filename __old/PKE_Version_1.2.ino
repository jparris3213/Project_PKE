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
#define LOGO_WIDTH    58
static const unsigned char PROGMEM logo_bmp[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0xff, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x13, 0x6f, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0xfe, 0x22, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x5f, 0xfe, 0x55, 0x40, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0x3f, 0xbe, 0x05, 0x50, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 0x02, 0x20, 0x00, 0x00, 
	0x00, 0x01, 0x7f, 0xff, 0x01, 0x50, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x05, 0x3f, 0x3f, 0xe0, 0x54, 0x00, 0x00, 0x00, 0x02, 0x3f, 0x3f, 0xf8, 0x22, 0x00, 0x00, 
	0x00, 0x05, 0x1f, 0xfe, 0xfc, 0x55, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x00, 0x00, 0x00, 
	0x00, 0x15, 0x7f, 0xff, 0xf5, 0x55, 0x00, 0x00, 0x00, 0x02, 0xff, 0xff, 0xe2, 0x22, 0x00, 0x00, 
	0x18, 0x14, 0xff, 0xff, 0xd5, 0x55, 0x00, 0x00, 0x0c, 0x01, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x03, 0x97, 0xff, 0xfd, 0x55, 0xe5, 0xe0, 0x00, 0x0b, 0xaf, 0xff, 0xfa, 0x22, 0xe3, 0xc0, 0x00, 
	0x1f, 0xef, 0xff, 0xf5, 0x53, 0xff, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0x00, 0x00, 
	0x07, 0xff, 0xff, 0x55, 0x7b, 0xff, 0x00, 0x00, 0x0f, 0xff, 0x8e, 0x22, 0x7d, 0xff, 0xe0, 0x00, 
	0x0f, 0xff, 0x8d, 0x54, 0xfd, 0xff, 0xf0, 0x00, 0x00, 0x7c, 0x00, 0x03, 0xff, 0xdf, 0x00, 0x00, 
	0x00, 0x38, 0x15, 0x5f, 0xff, 0x01, 0x00, 0x00, 0x00, 0x1a, 0x22, 0x1f, 0xf9, 0xe2, 0xc0, 0x00, 
	0x00, 0x00, 0x55, 0x7f, 0xff, 0xe5, 0xc0, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xe0, 0x00, 0x00, 
	0x00, 0x15, 0x54, 0xff, 0xff, 0xf5, 0x00, 0x00, 0x00, 0x02, 0x20, 0x3f, 0xff, 0xe2, 0x00, 0x00, 
	0x00, 0x05, 0x50, 0x7f, 0xff, 0xd5, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfd, 0xff, 0x80, 0x00, 0x00, 
	0x00, 0x05, 0x41, 0xff, 0xff, 0x54, 0x00, 0x00, 0x00, 0x02, 0x23, 0xff, 0xff, 0x20, 0x00, 0x00, 
	0x00, 0x01, 0x5f, 0xff, 0xfe, 0x50, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf8, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0x55, 0xff, 0xfd, 0x50, 0x00, 0x00, 0x00, 0x00, 0x22, 0xff, 0xe2, 0x20, 0x00, 0x00, 
	0x00, 0x00, 0x55, 0x5f, 0x55, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x15, 0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x22, 0x22, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0x55, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
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
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(1,0);
    display.println("Avg PKE");
    display.setCursor(1,56);
    display.println("0-------5---------100");

    display.setCursor(70,0);
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
      myStepper.step(1023);
    } else if(lastSteadyState == LOW && currentState == HIGH){
      myStepper.setSpeed(8);
      Serial.println(F("ButtonPress R"));
      myStepper.step(-1023);

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


    display.drawRoundRect(0, 20, 128, 37, 2, WHITE);
    display.fillRect(5, 23, 60, 23, BLACK);
    display.fillRect(5, 23, map(emfValue, 0, 1023, 0, 58), 20, WHITE);
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
  delay(3000);
  display.setCursor(1,25);
  display.println("Ghostbusters LLC");
  display.setCursor(1,35);
  display.println("Phone: 555-2368");
  display.setCursor(1,45);
  display.println("14 N Moore St");
  display.setCursor(1,55);
  display.println("New York City");
  display.display();
  delay(7000);
  display.clearDisplay();
  heading_print();
  delay(3000);
  display.setCursor(1,9);
  display.println("Initialize: Hardware");
  display.display();
  delay(1000);
  display.setCursor(1,20);
  display.println("SERIAL @ 1152K BAUD..");
  //display.println("12345123451234512345");
  display.display();
  delay(1000);
  display.setCursor(1,30);
  myStepper.setSpeed(8);//Speed of Stepper Motor
  display.println("WING STEP SPEED @ 80%");
  display.display();
  delay(1000);
  display.setCursor(1,40);
  myStepper.step(1023); //Extend Arms 1023 steps from start
  delay(3000);
  
  display.println("WING TEST EXTEND...OK");
  display.display();
  delay(1000);
  display.setCursor(1,50);
  myStepper.step(-1023); //retract arms 1023 steps To start
  display.println("WING TEST RETRACT..OK");
  display.display();
  delay(5000);
  display.clearDisplay();
  heading_print();
  delay(3000);
  display.setCursor(1,9);
  display.println("Initialize Software");
  display.display();
  delay(1000);
  display.setCursor(1,20);
  display.println("Load: Tobin Guide.zip");
  display.display();
  delay(1000);
  display.setCursor(1,30);
  display.println("Load: PKE_RANGE_1.zip");
  display.display();
  delay(1000);
  display.setCursor(1,40);
  display.println("Load: FARM_UPDATE.zip");
  display.display();
  delay(1000);
  display.setCursor(1,50);
  display.println("Load: CLASS_CHART.zip");
  display.display();
  delay(5000);
  display.clearDisplay();
  delay(1000);

  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(1000);
  //display.clearDisplay();
  display.setCursor(1,50);
  display.println("Happy Bustin!!!");
  display.display();
  delay(2000);
  display.clearDisplay();

}

void heading_print(){
  display.setCursor(1,0);
  display.println("GhostOS v2 (c) 1989");
  display.display();
}
