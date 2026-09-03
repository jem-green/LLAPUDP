//////////////////////////////////////////////////////////////////////////
// LLAP basic heartbeat
//
// Uses the Ciseco LLAPSerial library
// Uses a separated LLAPSerial and Sleeper library
// Uses a modified LLAPSerial library to support UDP -> LLAPUDP
// Uses the Adafruit DHT library https://github.com/adafruit/DHT-sensor-library
//
//////////////////////////////////////////////////////////////////////////

#include "config.h"
#include <Ethernet.h>
#include <LLAPUDP.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DHT.h>

/****************************************
 * Define Constants
 ****************************************/

IPAddress host = IPAddress(address);  // Assume its on the the same subnet
byte ip = host[3];
const int port = 10;
int status = WL_IDLE_STATUS;

// Setup the device

char deviceId[2];           // The device Id
int deviceState = Unknown;  // device state

// Instantiate the UDP classes

LLAPUDP LLAP;

#define DHTPIN 2     	// what I/O the DHT-22 data pin is connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

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
  }
  Serial.println("Connected to Wifi");

  // Set the remote connection
  LLAP.setRemoteConnection(host, port);
  deviceId[0] = 'D';
  deviceId[1] = 'H';
  LLAP.init(deviceId);  //
  LLAP.sendMessage(F("STARTED"));
}

void loop() {
  // print the string when a newline arrives:
  LLAP.UdpEvent();
  if (LLAP.bMsgReceived) {
    Serial.println(LLAP.sMessage); 
    LLAP.bMsgReceived = false;	// if we do not clear the message flag then message processing will be blocked
  }

  // every 30 seconds
  static unsigned long lastTime = millis();
  if ((millis() - lastTime) >= 30000) {
  
    lastTime = millis();
    int h = dht.readHumidity() * 10;
    int t = dht.readTemperature() * 10;
    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t) || isnan(h)) {
      LLAP.sendMessage(F("ERROR"));
    }
    else {
      LLAP.sendIntWithDP("HUM",h,1);
      //delay(100);
      LLAP.sendIntWithDP("TMP",t,1);
    }
  }
  delay(1000);
}
