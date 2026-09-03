//////////////////////////////////////////////////////////////////////////
// WEMOS UDP PIR Sensor
//
// Uses the Ciseco LLAPSerial library
// Uses a separated LLAPSerial and Sleeper library
// Uses a modified LLAPSerial library to support UDP -> LLAPUDP
// Used the google volt meter https://code.google.com/p/tinkerit/wiki/SecretVoltmeter
//
//////////////////////////////////////////////////////////////////////////


#include <LLAPSerial.h>
#include <Sleeper.h>

/****************************************
 * Define Constants
 ****************************************/

IPAddress host = IPAddress(address);  // Assume its on the the same subnet
byte ip = host[3];
const int port = 10;

// Setup the device

char deviceId[2];           // The device Id
int deviceState = Unknown;  // device state

#define DEVICEID "AC"
#define PIR_PIN 2

#define WAKEC 10
byte battc = 9;

#define PIRBLOCKTIME 20000          // time in ms to block after a triger

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
  }
  Serial.println("Connected to Wifi");

  // Set the remote connection
  LLAP.init(DEVICEID);  //
  LLAP.setRemoteConnection(host, port);
  deviceId[0] = 'P';
  deviceId[1] = 'I';
  LLAP.init(deviceId);  //
  LLAP.sendMessage(F("STARTED"));

  pinMode(PIR_PIN, INPUT);        // PIR Input pin
  digitalWrite(PIR_PIN, LOW);     // no pullup

  LLAP.sendMessage(F("STARTED"));
}

void loop() {
  // print the string when a newline arrives:
  LLAP.UdpEvent();
  if (LLAP.bMsgReceived) {
    Serial.println(LLAP.sMessage); 
    LLAP.bMsgReceived = false;	// if we do not clear the message flag then message processing will be blocked
  }



  pinMode(4, INPUT);                          // sleep the radio

  sleeper.sleep(PIR_PIN, RISING, false);         // deep sleep until PIR causes interupt
  battc++;                                    // increase battery count

  pinMode(4, OUTPUT);                         // wake the radio

  delay(450);                                 // give it time to wake up

  LLAP.sendMessage(F("PIRTRIG"));             // the pir trigered send a message

  if (battc >= WAKEC) {                       // is it time to send a battery reading
    battc = 0;
    LLAP.sendIntWithDP("BATT", int(readVcc()), 3);   // read the battery voltage and send
  }
  pinMode(4, INPUT);                          // sleep the radio again
  sleeper.sleepForaWhile(PIRBLOCKTIME);          // sleep for a little while before we go back to listening for the PIR

}

long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}
