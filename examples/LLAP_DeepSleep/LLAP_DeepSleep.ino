//////////////////////////////////////////////////////////////////////////
// WEMOS UDP deep sleep
//
// Uses the Ciseco LLAPSerial library
// Uses a separated LLAPSerial and Sleeper library
// Uses a modified LLAPSerial library to support UDP -> LLAPUDP
// Persist after a start and following deep
//
//////////////////////////////////////////////////////////////////////////

#include "config.h"
#include <Ethernet.h>
#include <EEPROM.h>
#include <LLAPUDP.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define EEPROM_DEVICEID_ADDRESS 0  // Address where the two byte DEVICEID is stored
#define EEPROM_STATE_ADDRESS 0     // Address where the signle byte Stage is stored

// State model to keep track of where we are

enum State : int {
  LLAPUnknown = 0,
  Starting = 1,
  Started = 2,
  Registering = 3,
  Registered = 4,
  Restarting = 5,
  Initiating = 6,
  Initiated = 7,
  Sleeping = 8,
  Updating = 9,
  Woken = 10,
  Messaging = 11
};

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

void setup() {

  deviceState = Starting;

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

  // Get the device state

  EEPROM.begin(16);  // Alocate Space for persisting data

  deviceState = loadState();
  Serial.print("State ");
  Serial.println(deviceState);

  // Set the remote connection
  LLAP.setRemoteConnection(host, port);
  deviceId[0] = 'D';
  deviceId[1] = 'S';
  loadDeviceId(deviceId);  // Recover the deviceId or defaults to '--'
  LLAP.init(deviceId);  //
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

/**
   saveDeviceId
   Stores DEVICEID into EEPROM
*/

void saveDeviceId(char *id) {
  char temp[2];
  EEPROM.get(EEPROM_DEVICEID_ADDRESS, temp[0]);
  EEPROM.get(EEPROM_DEVICEID_ADDRESS + 1, temp[1]);
  if (id[0] != temp[0]) {
    EEPROM.put(EEPROM_DEVICEID_ADDRESS, id[0]);
  }
  if (id[1] != temp[1]) {
    EEPROM.put(EEPROM_DEVICEID_ADDRESS + 1, id[1]);
  }
}

/**
   loadDeviceId
   Recovers DEVICEID from EEPROM
*/
void loadDeviceId(char *id) {
  EEPROM.get(EEPROM_DEVICEID_ADDRESS, id[0]);
  EEPROM.get(EEPROM_DEVICEID_ADDRESS + 1, id[1]);
  if (strchr("-#@?\\*ABCDEFGHIJKLMNOPQRSTUVWXYZ", id[0]) == 0 || strchr("-#@?\\*ABCDEFGHIJKLMNOPQRSTUVWXYZ", id[1]) == 0) {
    id[0] = '-';
    id[1] = '-';
    saveDeviceId(id);
  }
}

/**
   saveState
   Stores device state into EEPROM
*/
void saveState(int state) {
  int temp;
  EEPROM.get(EEPROM_STATE_ADDRESS, temp);
  Serial.print("Previous state ");
  Serial.println(temp);
  Serial.print("New state ");
  Serial.println(state);
  if (temp != state) {
    //LLAP.sendInt("WAKE!", wakes); // send back error status
    EEPROM.put(EEPROM_STATE_ADDRESS, state);
    Serial.print("Save state ");
    Serial.println(state);
    EEPROM.commit();
  }
}

/**
   loadState
   Recovers device state into EEPROM
*/
int loadState() {
  int state = Unknown;
  EEPROM.get(EEPROM_STATE_ADDRESS, state);
  Serial.print("Load State ");
  Serial.println(state);
  //LLAP.sendInt("WAKE+", wakes); // send back error status
  if ((state < 0) || (state > 10)) {
    state = Unknown;
  }
  return (state);
}
