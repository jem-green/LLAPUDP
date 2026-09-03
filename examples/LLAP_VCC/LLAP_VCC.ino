//////////////////////////////////////////////////////////////////////////
// xino RF PIR Sensor
//
// Uses the Ciseco LLAPSerial library
// Uses a separated LLAPSerial and Sleeper library
// Example by Jeremy Green
//
// https://code.google.com/p/tinkerit/wiki/SecretVoltmeter
//////////////////////////////////////////////////////////////////////////


#include <LLAPSerial.h>
#include <Sleeper.h>

#define DEVICEID "AC"
#define PIR_PIN 2

#define WAKEC 10
byte battc = 9;

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

  sleeper.sleep(PIR_PIN, RISING, false);         // deep sleep until PIR causes interupt
  battc++;                                    // increase battery count

  delay(450);                                 // give it time to wake up

  LLAP.sendMessage(F("PIRTRIG"));             // the pir trigered send a message

  if (battc >= WAKEC) {                       		// is it time to send a battery reading
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

/**
  saveWakeCounter
  Stores wake counter into EEPROM
*/
 void saveWakeCounter(int wakes) {
  int temp;
  EEPROM.get(EEPROM_WAKE_COUNTER_ADDRESS, temp);
  if (temp != wakes)
  {
    EEPROM.put(EEPROM_WAKE_COUNTER_ADDRESS, wakes);
  }
}

/**
 loadWakeCounter
 Recovers wake counter from EEPROM
*/
int loadWakeCounter() {
  int wakes = 0;
  EEPROM.get(EEPROM_WAKE_COUNTER_ADDRESS, wakes);
  //LLAP.sendInt("WAKES", wakes); // send back error status
  if ((wakes <= 0) || (wakes > 999)) {
    wakes = 10;
  }
  return (wakes);
}
