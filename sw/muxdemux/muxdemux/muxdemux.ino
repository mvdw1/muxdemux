// (relevant) settings for ESP c3 super mini
// Board: ESP32C3 Dev Module
// USB CDC on boot: enabled
// CPU frequency: 160 MHz (WiFi)
// Flash Frequency: 80 Mhz
// JTAG adapter: Integrated USB JTAG
// Partition Scheme: Default with spiffs (1.2MB APP/ 1.5MB SPIFFS)



#define SEL_PIN 2
#define OE_PIN 3
#define BTN_PIN 4
#define SEL_PWR_A_PIN 8
#define SEL_PWR_B_PIN 7

#define VERSION "0.0.1" //TODO add e.g. git version 
#define OK_CODE '1'
#define ERR_CODE '2'

void pressedA();
void pressedB();
void pressedC();
void pressedD();
void pressedH();
void pressedS();
void pressedV();

// Begin macro's for serial interface
// Mapping macro with character, function to call, help text
#define CHARMAP(F) \
F('a', pressedA, "Activate output")\
F('b', pressedB, "Connect outputs A <-> B")\
F('c', pressedC, "Connect outputs A <-> C")\
F('d', pressedD, "Deactivate output")\
F('h', pressedH, "Print help")\
F('s', pressedS, "Print current status")\
F('v', pressedV, "Print version info")

// Mapping from CHARMAP to case entries for switch
#define CHARCASE(CHAR, FUNCTION, HELP) \
case CHAR: FUNCTION(); break;
// Mapping from CHARMAP to print help
#define CHARHELP(CHAR, FUNCTION, HELP)\
Serial.print(CHAR);Serial.print(": "); Serial.println(HELP);
// End macro's for serial interface

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting muxdemux");
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(SEL_PIN, OUTPUT);
  pinMode(OE_PIN, OUTPUT);
  pinMode(SEL_PWR_A_PIN, OUTPUT);
  pinMode(SEL_PWR_B_PIN, OUTPUT);
  switchPorts(false, false);
}

bool readButton() {
  return !digitalRead(BTN_PIN);
}

bool enableVoltageBridge(bool enableAB, bool enableAC) {
  if (enableAB && enableAC) {
    Serial.println("Enabling both not supported");
  } else if (enableAB) {
    digitalWrite(SEL_PWR_B_PIN, LOW);
    digitalWrite(SEL_PWR_A_PIN, HIGH);
  } else if (enableAC) {
    digitalWrite(SEL_PWR_A_PIN, LOW);
    digitalWrite(SEL_PWR_B_PIN, HIGH);
  } else {
    digitalWrite(SEL_PWR_A_PIN, LOW);
    digitalWrite(SEL_PWR_B_PIN, LOW);
  }
  return true;
}

bool switchPorts(bool enabled, bool mode) {
  if (!enabled) {
    enableVoltageBridge(false, false);
    digitalWrite(OE_PIN, HIGH);
    digitalWrite(SEL_PIN, LOW);
    return true;
  }
  if (mode) {
    Serial.println("Connecting AB");
    enableVoltageBridge(false, false);
    delay(50);
    digitalWrite(OE_PIN, LOW);
    digitalWrite(SEL_PIN, LOW);
    delay(50);
    enableVoltageBridge(true, false);
  } else {
    Serial.println("Connecting AC");
    enableVoltageBridge(false, false);
    delay(50);
    digitalWrite(OE_PIN, LOW);
    digitalWrite(SEL_PIN, HIGH);
    delay(50);
    enableVoltageBridge(false, true);
  }
  return true;
}
bool enabled = true;
bool state = false;

bool activateOutput() {
Serial.println("activate");
  enabled = true;
  switchPorts(enabled, state);
  return true;
}
bool deactivateOutput() {
  Serial.println("Deactivate");
  enabled = false;
  switchPorts(enabled, state);
  return true;
}
bool selectAB() {
  Serial.println("AB");
  state = true;
  switchPorts(enabled, state);
  return true;
}
bool selectAC() {
  Serial.println("AC");
  state = false;
  switchPorts(enabled, state);
  return true;
}


void pressedA() {
  activateOutput();
  Serial.println(OK_CODE);
}


void pressedD() {
  deactivateOutput();
  Serial.println(OK_CODE);
}

void pressedB() {
  selectAB();
  Serial.println(OK_CODE);
}


void pressedC() {
  selectAC();
  Serial.println(OK_CODE);
}


void pressedH() {
  // Print help
  Serial.println("Command summary:");
  CHARMAP(CHARHELP)
  
  Serial.println(OK_CODE);
}
void pressedS() {
  Serial.println("Current state:");
  if(enabled){
    Serial.println("Output: enabled");
  }else{
    Serial.println("Output: disabled");
  }
  if(state){
    Serial.println("Switch: AB");
  }else{
    Serial.println("Switch: AC");
  }
  
  Serial.println(OK_CODE);
}

void pressedV(){
  Serial.println(VERSION);
  Serial.println(OK_CODE);
}

void checkSerialInput() {
  //static String inputString = "";  // String to hold incoming serial data
  static bool newData = false;
  static char inChar = 0;
  while (Serial.available() > 0) {
    char newChar = (char)Serial.read();

    // Check for new line character
    if (newChar == '\n') {
      newData = true;
    } else {
      inChar = newChar;  // Read a character from serial
    }
  }


  // If a complete newline is received, process the input
  if (newData) {
    // Reset the variables for the next iteration


    switch (inChar) {
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
bool pressed = false;  // To track if the button was pressed

unsigned long debounceDelay = 50;  // Debounce time in milliseconds
unsigned long lastDebounceTime = 0;


void loop() {
  // put your main code here, to run repeatedly:
  delay(10);
  //if (readButton()) {
  //  Serial.println("btn press");
  //}
  checkSerialInput();


  int reading = digitalRead(BTN_PIN);

  // Check for button state change with debouncing
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has been stable for the debounce delay
    if (reading != buttonState) {
      buttonState = reading;

      // If the button is pressed (LOW state due to INPUT_PULLUP)
      if (buttonState == LOW) {
        if (!pressed) {
          if (state) {
            enabled = true;
            pressedC();
          } else {
            enabled = true;
            pressedB();
          }
          pressed = true;
          currentState = !currentState;
        }
      } else {
        // Toggle the output state
        //digitalWrite(OUTPUT_PIN, currentState ? HIGH : LOW);
        pressed = false;  // Reset the pressed state
      }
    }
  }

  lastButtonState = reading;
}


