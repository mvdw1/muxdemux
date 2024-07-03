/*
 * SPDX-License-Identifier: GPL-3.0
 */


// (relevant) settings for ESP c3 super mini
// Board: ESP32C3 Dev Module
// USB CDC on boot: enabled
// CPU frequency: 160 MHz (WiFi)
// Flash Frequency: 80 Mhz
// JTAG adapter: Integrated USB JTAG
// Partition Scheme: Default with spiffs (1.2MB APP/ 1.5MB SPIFFS)

// (relevant) settings for XIAO RP2040
// board: Seed XIAO RP2040
// CPU speed: 133 MHz

// Currently XIAO RP2040 is only tested

#include "pinconfig.h"

#define VERSION "0.0.3" // TODO add e.g. git version
#define OK_CODE '1'
#define ERR_CODE '2'

void pressedA();
void pressedB();
void pressedC();
void pressedD();
void pressedH();
void pressedS();
void pressedV();

#define VERSION_0P3_UPPER 28
#define VERSION_0P4_UPPER 84
#define VERSION_0P5_UPPER 140

enum BOARD_REV
{
  VERSION_0P3,
  VERSION_0P4,
  VERSION_0P5,
  VERSION_UNKNOWN
};

int readVersionInfo()
{
  analogReadResolution(10);
  long adcValue = analogRead(VERSION_PIN_DEFAULT);
  Serial.println(adcValue);
  if (adcValue < VERSION_0P3_UPPER)
  {
    return VERSION_0P3;
  }
  else
  {
    return VERSION_UNKNOWN;
  }
}
const int *PINMAP;
int setupPins()
{
  int version = readVersionInfo();
  if (version == VERSION_0P3)
  {
    PINMAP = &PINMAP_0p3[0];
    readVersionInfo();
    if (PINMAP[BTN_PIN_INDEX] == BTN_PIN_0p3)
    {
      Serial.println("match");
    }
    else
    {
      Serial.println("mismatch");
      Serial.println(PINMAP[BTN_PIN_INDEX]);
      Serial.println(BTN_PIN_0p3);
    }
  }
  else
  {
    while (1)
    {
      readVersionInfo();
      Serial.println("Unsupported HW revision");
      delay(1000);
    }
    // TODO go to error state
  }

  pinMode(PINMAP[BTN_PIN_INDEX], INPUT_PULLUP);
  pinMode(PINMAP[SEL_PIN_INDEX], OUTPUT);
  pinMode(PINMAP[OE_PIN_INDEX], OUTPUT);
  pinMode(PINMAP[SEL_PWR_B_PIN_INDEX], OUTPUT);
  pinMode(PINMAP[SEL_PWR_C_PIN_INDEX], OUTPUT);
  pinMode(PINMAP[STATUS_LED_PIN_INDEX], OUTPUT);
  return 0;
}

// Begin macro's for serial interface
// Mapping macro with character, function to call, help text
#define CHARMAP(F)                            \
  F('a', pressedA, "Activate output")         \
  F('b', pressedB, "Connect outputs A <-> B. Briefly disconnects upon execution")  \
  F('c', pressedC, "Connect outputs A <-> C. Briefly disconnects upon execution") \
  F('d', pressedD, "Deactivate output")       \
  F('h', pressedH, "Print help")              \
  F('s', pressedS, "Print current status")    \
  F('v', pressedV, "Print version info")

// Mapping from CHARMAP to case entries for switch
#define CHARCASE(CHAR, FUNCTION, HELP) \
  case CHAR:                           \
    FUNCTION();                        \
    break;
// Mapping from CHARMAP to print help
#define CHARHELP(CHAR, FUNCTION, HELP) \
  Serial.print(CHAR);                  \
  Serial.print(": ");                  \
  Serial.println(HELP);
// End macro's for serial interface
#define AB false
#define AC true
#define OFF false
#define ON true
bool enabled = false;
bool state = false;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Default: off, a-b
  enabled = OFF;
  state = AC;
  setupPins();
  updateMode();
}



bool voltageABActive = false;
bool voltageACActive = false;

void updateMode()
{
  if(enabled) Serial.println("Mode enabled");
  if(state == AB){
    connectAB();
  }else if(state == AC){
    connectAC();
  }else{
    // Currently with state as type bool this is not possible,
    // Place here for future change datatype
    Serial.println("Invalid state!");
  }
  // update led
  if(enabled == ON){
    digitalWrite(PINMAP[STATUS_LED_PIN_INDEX], HIGH);
  }else{
    digitalWrite(PINMAP[STATUS_LED_PIN_INDEX], LOW);
  }
}

void connectAB(){

  Serial.println("connect ab\n");
  // first power down
  enableVoltageBridge(false, false);
  digitalWrite(PINMAP[OE_PIN_INDEX], HIGH);
  delay(50);
  digitalWrite(PINMAP[SEL_PIN_INDEX], LOW);
  // apply power if needed
  if(enabled == ON)
  {
    enableVoltageBridge(true, false);
    digitalWrite(PINMAP[OE_PIN_INDEX], LOW);
  }
  delay(50);
}

void connectAC(){
  Serial.println("connect ac\n");
  // first power down
  enableVoltageBridge(false, false);
  digitalWrite(PINMAP[OE_PIN_INDEX], HIGH);
  delay(50);
  digitalWrite(PINMAP[SEL_PIN_INDEX], HIGH);
  // apply power if needed
  if(enabled == ON)
  {
    enableVoltageBridge(false, true);
    digitalWrite(PINMAP[OE_PIN_INDEX], LOW);
  }
  delay(50);
}



bool readButton()
{
  return !digitalRead(PINMAP[BTN_PIN_INDEX]);
}


bool enableVoltageBridge(bool enableAB, bool enableAC)
{
  if (enableAB && enableAC)
  {
    Serial.println("Enabling both not supported");
  }
  else if (enableAB)
  {

    Serial.println("Enable voltage AB");
    digitalWrite(PINMAP[SEL_PWR_C_PIN_INDEX], LOW);
    digitalWrite(PINMAP[SEL_PWR_B_PIN_INDEX], HIGH);
    voltageABActive = true;
    voltageACActive = false;
  }
  else if (enableAC)
  {

    Serial.println("Enable voltage AC");
    digitalWrite(PINMAP[SEL_PWR_B_PIN_INDEX], LOW);
    digitalWrite(PINMAP[SEL_PWR_C_PIN_INDEX], HIGH);
    voltageABActive = false;
    voltageACActive = true;
  }
  else
  {
    Serial.println("Disable voltage");
    digitalWrite(PINMAP[SEL_PWR_B_PIN_INDEX], HIGH);
    digitalWrite(PINMAP[SEL_PWR_C_PIN_INDEX], HIGH);
    voltageABActive = false;
    voltageACActive = false;
  }
  return true;
}





void pressedA()
{
  enabled = ON;
  updateMode();
  Serial.println(OK_CODE);
}

void pressedD()
{
  enabled = OFF;
  updateMode();
  Serial.println(OK_CODE);
}

void pressedB()
{
  // Switch to AB
  state = AB;
  updateMode();
}

void pressedC()
{

  // Switch to AC
  state = AC;
  updateMode();
}

void pressedH()
{
  // Print help
  Serial.println("Command summary:");
  CHARMAP(CHARHELP)

  Serial.println(OK_CODE);
}
void pressedS()
{
  Serial.println("Current state:");
  if (enabled==ON)
  {
    Serial.println("Output: enabled");
  }
  else
  {
    Serial.println("Output: disabled");
  }
  if (state==AB)
  {
    Serial.println("Switch: AB");
  }
  else
  {
    Serial.println("Switch: AC");
  }

  Serial.println(OK_CODE);
}

void pressedV()
{
  Serial.println(VERSION);
  Serial.println(OK_CODE);
}

void checkSerialInput()
{
  // static String inputString = "";  // String to hold incoming serial data
  static bool newData = false;
  static char inChar = 0;
  while (Serial.available() > 0)
  {
    char newChar = (char)Serial.read();

    // Check for new line character
    if (newChar == '\n')
    {
      newData = true;
    }
    else
    {
      inChar = newChar; // Read a character from serial
    }
  }

  // If a complete newline is received, process the input
  if (newData)
  {
    // Reset the variables for the next iteration

    switch (inChar)
    {
      CHARMAP(CHARCASE)
    default:
      Serial.println(ERR_CODE);
    }

    inChar = 0;
    newData = false;
  }
}

bool currentState = false;
bool lastButtonState = HIGH;
bool buttonState;
bool pressed = false; // To track if the button was pressed

unsigned long debounceDelay = 50; // Debounce time in milliseconds
unsigned long lastDebounceTime = 0;

bool debouncedBtnRead()
{
  int reading = digitalRead(PINMAP[BTN_PIN_INDEX]);

  // Check for button state change with debouncing
  if (reading != lastButtonState)
  {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    // If the button state has been stable for the debounce delay
    if (reading != buttonState)
    {
      buttonState = reading;

      // If the button is pressed (LOW state due to INPUT_PULLUP)
      if (buttonState == LOW)
      {
        if (!pressed)
        {
          if (state == AB)
          {
            enabled = ON;
            pressedC();
          }
          else
          {
            enabled = ON;
            pressedB();
          }
          pressed = true;
          currentState = !currentState;
        }
      }
      else
      {
        pressed = false; // Reset the pressed state
      }
    }
  }

  lastButtonState = reading;
  return pressed;
}




void loop()
{

  delay(10);
  debouncedBtnRead();
  checkSerialInput();
}
