// Code generated by Arduino IoT Cloud, DO NOT EDIT.

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char DEVICE_LOGIN_NAME[]  = "2fb8a57c-0854-481a-8b97-fc69d151a0aa";

const char SSID[]               = SECRET_SSID;    // Network SSID (name)
const char PASS[]               = SECRET_OPTIONAL_PASS;    // Network password (use for WPA, or use as key for WEP)
const char DEVICE_KEY[]  = SECRET_DEVICE_KEY;    // Secret device password

void onDurationSecondsChange();
void onRelayDownChange();
void onRelayUpChange();
void onStartRelayDownChange();
void onStartRelayUpChange();

int duration_seconds;
bool relay_down;
bool relay_up;
bool startRelayDown;
bool startRelayUp;

void initProperties(){

  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(duration_seconds, READWRITE, ON_CHANGE, onDurationSecondsChange);
  ArduinoCloud.addProperty(relay_down, READWRITE, ON_CHANGE, onRelayDownChange);
  ArduinoCloud.addProperty(relay_up, READWRITE, ON_CHANGE, onRelayUpChange);
  ArduinoCloud.addProperty(startRelayDown, READWRITE, ON_CHANGE, onStartRelayDownChange);
  ArduinoCloud.addProperty(startRelayUp, READWRITE, ON_CHANGE, onStartRelayUpChange);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
