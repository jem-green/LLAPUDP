//////////////////////////////////////////////////////////////////////////
// WEMOS  Heartbeat
//
// Uses the Ciseco LLAPSerial library
// Uses a separated LLAPSerial and Sleeper library
// Used Voltage code from https://code.google.com/p/tinkerit/wiki/SecretVoltmeter
// Uses a modified LLAPSerial library to support UDP -> LLAPUDP
// Example by Jeremy Green
//
//////////////////////////////////////////////////////////////////////////

#include <LLAPUDP.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

/****************************************
 * Define Constants
 ****************************************/

const char *ssid = "Wyvern";
const char *password = "arial2italic";
const char *hostName = "192.168.0.22";
const int port = 10;

// Setup the device

#define DEVICEID "HB"	// this is the LLAP device ID

// Instantiate the UDP classes

LLAPUDP LLAP;

void setup() {
  Serial.begin(9600);  //
  Serial.setTimeout(2000);
  // Wait for serial to initialize.

  while (!Serial) {}

  Serial.println("\nStarted");

  WiFi.begin(ssid, password);

  // Wait for connection
  Serial.println("\nConnecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected to Wifi");

  LLAP.init(DEVICEID);  //
  LLAP.setRemoteConnection(hostName, port);
  LLAP.sendMessage(F("STARTED"));
}

void loop() {
  // print the string when a newline arrives:
  LLAP.UdpEvent();
  if (LLAP.bMsgReceived) {
    Serial.println(LLAP.sMessage); 
    LLAP.bMsgReceived = false;  // if we do not clear the message flag then message processing will be blocked
  }

  // every 30 seconds
  static unsigned long lastTime = millis();
  if ((millis() - lastTime) >= 30000) {
    lastTime = millis();
    LLAP.sendMessage(F("HELLO"));  // send a heartbeat
  }

  delay(1000);
}
