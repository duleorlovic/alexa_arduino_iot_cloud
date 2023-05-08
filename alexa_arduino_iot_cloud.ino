#include "arduino_secrets.h"
/* 
  Sketch generated by the Arduino IoT Cloud Thing "StartMachine"
  https://create.arduino.cc/cloud/things/bd393c29-d24e-49a2-8780-90b7152eb8c8 

  Arduino IoT Cloud Variables description

  The following variables are automatically generated and updated when changes are made to the Thing

  bool relay_down;
  bool relay_up;
  bool startRelayDown;
  bool startRelayUp;
  int duration_seconds;

  relay_down also start relayWedgeHolderActive for WEDGE_HOLDER_UP_DURATION with
  relay_up (during that time wedge holder should stop blocking) and than
  relay_down starts and after that. Also because of inertia we should wait
  WEDGE_HOLDER_DOWN_INERTION_DURATION before stopping the gear wheel with
  relayWedgeHolderActive

  Variables which are marked as READ/WRITE in the Cloud Thing will also have functions
  which are called when their values are changed from the Dashboard.
  These functions are generated with the Thing and added at the end of this sketch.
*/

// #define USE_ALEXA

#include "thingProperties.h"
#ifdef USE_ALEXA
#include <Espalexa.h>
#endif

#define ON_FOR_ACTIVE_LOW_OUTPUT 0
#define OFF_FOR_ACTIVE_LOW_OUTPUT 1

// https://lastminuteengineers.com/esp8266-pinout-reference/
// do not use D0, D3, D4
#define RELAY_UP_PIN 14 // D5
#define RELAY_DOWN_PIN 12 // D6
#define RELAY_WEDGE_HOLDER_PIN 5 // D1
// unused relay  4 // D2

// 5V is on RSV reserved pin VV

#define BLINK_DURATION 50
#define WEDGE_HOLDER_UP_DURATION 100
#define WEDGE_HOLDER_DOWN_INERTION_DURATION 5000

#ifdef USE_ALEXA
Espalexa espalexa;
#endif
bool alexaDeviceAdded = false;

unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()

bool relayDownActive = false;
// last time the relay_down was activated ie relayDownActive become true
unsigned long previousRelayDownActivatedMillis = 0;

bool relayUpActive = false;
// last time the relay_up was activated ie relayUpActive become true
unsigned long previousRelayUpActivatedMillis = 0;
// relayWedgeHolderActive become true before previousRelayUpActivatedMillis
// for WEDGE_HOLDER_UP_DURATION and should become false after
// WEDGE_HOLDER_DOWN_INERTION_DURATION after previousRelayUpActivatedMillis +
// duration_seconds
bool relayWedgeHolderActive = false;

int blinkActiveTimes = 0;
unsigned long previousBlinkActivedMillis = 0;

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 
  Serial.println("setup start");
  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // wifi connection event callbacks
  ArduinoIoTPreferredConnection.addCallback(NetworkConnectionEvent::CONNECTED, onNetworkConnect);
  ArduinoIoTPreferredConnection.addCallback(NetworkConnectionEvent::DISCONNECTED, onNetworkDisconnect);
  ArduinoIoTPreferredConnection.addCallback(NetworkConnectionEvent::ERROR, onNetworkError);

  // pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_UP_PIN, OUTPUT);
  pinMode(RELAY_DOWN_PIN, OUTPUT);
  pinMode(RELAY_WEDGE_HOLDER_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN, OFF_FOR_ACTIVE_LOW_OUTPUT);
  digitalWrite(RELAY_UP_PIN, OFF_FOR_ACTIVE_LOW_OUTPUT);
  digitalWrite(RELAY_DOWN_PIN, OFF_FOR_ACTIVE_LOW_OUTPUT);
  digitalWrite(RELAY_WEDGE_HOLDER_PIN, OFF_FOR_ACTIVE_LOW_OUTPUT);

  Serial.println("Setup done");
}

void loop() {
  ArduinoCloud.update();
#ifdef USE_ALEXA
  espalexa.loop();
#endif
  deactivateWedgeHolder();
  deactivateRelayDown();
  deactivateRelayUp();
  deactivateBlink();
}

void deactivateWedgeHolder() {
  currentMillis = millis();   // capture the latest value of millis()
  if (relayWedgeHolderActive) {
    // since in relday_down we manually set previousRelayDownActivatedMillis in
    // future, it will be greater than currentMillis and substraction will be
    // negative
    if (currentMillis > previousRelayDownActivatedMillis &&
        currentMillis - previousRelayDownActivatedMillis > duration_seconds * 1000 + WEDGE_HOLDER_DOWN_INERTION_DURATION) {
      digitalWrite(RELAY_WEDGE_HOLDER_PIN, OFF_FOR_ACTIVE_LOW_OUTPUT);
      relayWedgeHolderActive = false;
      Serial.println("deactivateWedgeHolder.relayWedgeHolderActive = false");
    }
  }
}

void deactivateRelayDown() {
  currentMillis = millis();   // capture the latest value of millis()
  if (relayDownActive) {
    if (currentMillis - previousRelayDownActivatedMillis > duration_seconds * 1000) {
      relay_down = false;
      digitalWrite(RELAY_DOWN_PIN, OFF_FOR_ACTIVE_LOW_OUTPUT);
      relayDownActive = false;
      Serial.println("deactivateRelayDown.relay_down = false");
    }
  }
}

void deactivateRelayUp() {
  currentMillis = millis();   // capture the latest value of millis()
  if (relayUpActive) {
    if (currentMillis - previousRelayUpActivatedMillis > duration_seconds * 1000) {
      relay_up = false;
      digitalWrite(RELAY_UP_PIN, OFF_FOR_ACTIVE_LOW_OUTPUT);
      relayUpActive = false;
      Serial.println("deactivateRelayUp.relay_up = false");
    }
  }
}

// to start blink use
// previousBlinkActivedMillis = millis()
// blinkActiveTimes = 2;
// blinkActiveTimes = 4;
void deactivateBlink() {
  currentMillis = millis();   // capture the latest value of millis()
  if (blinkActiveTimes) {
    if (currentMillis - previousBlinkActivedMillis > BLINK_DURATION) {
      blinkActiveTimes -= 1;
      digitalWrite(LED_BUILTIN, blinkActiveTimes % 2);
      Serial.print(".");
    }
  }
}

void openWedgeHolderWithRelayUp() {
  digitalWrite(RELAY_UP_PIN, ON_FOR_ACTIVE_LOW_OUTPUT);
  digitalWrite(RELAY_WEDGE_HOLDER_PIN, ON_FOR_ACTIVE_LOW_OUTPUT);
  relayWedgeHolderActive = true;
  delay(WEDGE_HOLDER_UP_DURATION);
  // now it should be able to release the wedge holder
  digitalWrite(RELAY_UP_PIN, OFF_FOR_ACTIVE_LOW_OUTPUT);
}

void performRelayDown() {
  Serial.print("performRelayDown.relay_down for ");
  Serial.print(duration_seconds);
  Serial.println(" seconds");
  openWedgeHolderWithRelayUp();
  relay_down = true;
  digitalWrite(RELAY_DOWN_PIN, ON_FOR_ACTIVE_LOW_OUTPUT);
  relayDownActive = true;
  previousRelayDownActivatedMillis = millis();
  // we will shut down in deactivateRelayDown
}

void performRelayUp() {
  Serial.print("performRelayUp.relay_up for ");
  Serial.print(duration_seconds);
  Serial.println(" seconds");
  relay_up = true;
  digitalWrite(RELAY_UP_PIN, ON_FOR_ACTIVE_LOW_OUTPUT);
  relayUpActive = true;
  previousRelayUpActivatedMillis = millis();
  // we will shut down in deactivateRelayUp
}

void onRelayUpChange()  {
  digitalWrite(RELAY_UP_PIN, !relay_up);  // relays is active on low
  blinkActiveTimes = relay_up ? 4 : 2;
  Serial.print("onRelayUpChange relay_up=");
  Serial.println(relay_up);
}

void onRelayDownChange()  {
  if (relay_down == true) {
    previousRelayDownActivatedMillis = millis();
    // this will stop deactivateWedgeHolder for one hour
    previousRelayDownActivatedMillis += 3600000;
    openWedgeHolderWithRelayUp();
    digitalWrite(RELAY_DOWN_PIN, ON_FOR_ACTIVE_LOW_OUTPUT);
  } else {
    previousRelayDownActivatedMillis = millis();
    // this will trigger deactivateWedgeHolder now
    previousRelayDownActivatedMillis -= duration_seconds * 1000 + WEDGE_HOLDER_DOWN_INERTION_DURATION;
    digitalWrite(RELAY_DOWN_PIN, OFF_FOR_ACTIVE_LOW_OUTPUT);
  }
  blinkActiveTimes = relay_up ? 4 : 2;
  Serial.print("onRelayDownChange relay_down=");
  Serial.println(relay_down);
}

void onStartRelayDownChange()  {
  if (startRelayDown) {
    Serial.print("startRelayDown=true ");
    performRelayDown();
  }
}

void onStartRelayUpChange()  {
  if (startRelayUp) {
    Serial.print("startRelayUp=true ");
    performRelayUp();
  }
}

void onDurationSecondsChange()  {
}

void firstDeviceChanged(uint8_t brightness) {
  Serial.print("firstDeviceChanged changed to ");

  if (brightness) {
    blinkActiveTimes = 2;
    Serial.print("ON, brightness ");
    Serial.println(brightness);
    performRelayDown();
  }
  else  {
    blinkActiveTimes = 4;
    Serial.println("OFF");
    performRelayUp();
  }
}

// https://github.com/arduino-libraries/Arduino_ConnectionHandler
void onNetworkConnect() {
  digitalWrite(LED_BUILTIN, ON_FOR_ACTIVE_LOW_OUTPUT);
  Serial.print(">>>> CONNECTED to network. ");
#ifdef USE_ALEXA
  if (!alexaDeviceAdded) {
    alexaDeviceAdded = true;
    Serial.println("espalexa.addDevice Light 1");
    espalexa.addDevice("Light 1", firstDeviceChanged);
    espalexa.begin();
  } else {
    Serial.println("Keep using old device");
  }
#endif
}

void onNetworkDisconnect() {
  digitalWrite(LED_BUILTIN, OFF_FOR_ACTIVE_LOW_OUTPUT);

  Serial.println(">>>> DISCONNECTED from network");
}

void onNetworkError() {
  digitalWrite(LED_BUILTIN, OFF_FOR_ACTIVE_LOW_OUTPUT);

  Serial.println(">>>> ERROR");
}

// https://gist.github.com/ctkjose/09aaed811802ab083f0771a007ac9fcd
// serial_printi("Hello %s\n", "Jose");
// serial_printi("Hello X=%d F=%f\n", 25, 340.36);
void serial_printi(const char *format, ...){
  char ch;
  bool flgInterpolate = false;
  va_list args;
  va_start( args, format );
  for( ; *format ; ++format ){
    ch = *format;
    if(flgInterpolate){
      flgInterpolate = false;
      if((ch=='d') || (ch=='c')){
        Serial.print(va_arg(args, int));
      }else if(ch=='s'){
        Serial.print(va_arg(args, char*));
      }else if(ch=='o'){
        Serial.print(va_arg(args, unsigned int));
      }else if((ch=='f') || (ch=='e') || (ch=='a') || (ch=='g')){
        Serial.print(va_arg(args, double));
      }else{
        Serial.print('%');
        Serial.print(ch);
      }
    }else if(ch=='%'){
      flgInterpolate = true;
    }else{
      Serial.print(ch);
    }
  }

  va_end( args );
}
