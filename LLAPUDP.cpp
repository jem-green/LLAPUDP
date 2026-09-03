//
// Modified LLAPSerial library to remove the power / sleep elemets of the class
// Modified LLAPSerial to support UDP
//

#include <Ethernet.h>
#include "LLAPUDP.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

WiFiUDP Udp;

void LLAPUDP::init()
{
	sMessage.reserve(10);
	bMsgReceived = false;
	deviceId[0] = '-';
	deviceId[1] = '-';
}

void LLAPUDP::init(char* dID)
{
	init();
	bMsgReceived = false;
	setDeviceId(dID);
	cMessage[12]=0;		// ensure terminated
}

void LLAPUDP::processMessage(){
	Serial.println(cMessage[1]);
	Serial.println(cMessage[2]);
	//if (LLAP.cMessage[0] != 'a') return; //not needed as already checked
	if (cMessage[1] != '?' && cMessage[1] != deviceId[0]) return;
	if (cMessage[2] != '?' && cMessage[2] != deviceId[1]) return;
	// now we have LLAP.cMessage[3] to LLAP.cMessage[11] as the actual message
	sMessage = String(&cMessage[3]); // let the main program deal with it
	bMsgReceived = true;
}

void LLAPUDP::UdpEvent()
{
	if (bMsgReceived) return; //get out if previous message not yet processed
	if (Udp.parsePacket() > 0) {
		if (Udp.available() >= 12) {
			// get the new byte:
			char inChar = (char)Udp.peek();
			Serial.println(inChar);
			if (inChar == 'a') {
				for (byte i = 0; i<12; i++) {
					inChar = (char)Udp.read();
					cMessage[i] = inChar;
					if (i < 11 && Serial.peek() == 'a') {
						// out of synch so abort and pick it up next time round
						return;
					}
				}
				cMessage[12]=0;
				processMessage();
			}
			else
				Udp.read();	// throw away the character
		}
	}
}

void LLAPUDP::sendMessage(String sToSend)
{
    cMessage[0] = 'a';
    cMessage[1] = deviceId[0];
    cMessage[2] = deviceId[1];
    for (byte i = 0; i<9; i++) {
		if (i < sToSend.length())
			cMessage[i+3] = sToSend.charAt(i);
		else
			cMessage[i+3] = '-';
    }
	
	Serial.print(cMessage);
	Udp.beginPacket(sHostName, iPort);
	Udp.write(cMessage);
    	Udp.endPacket();
	Serial.flush();

}

void LLAPUDP::sendMessage(String sToSend, char* valueToSend)
{
    cMessage[0] = 'a';
    cMessage[1] = deviceId[0];
    cMessage[2] = deviceId[1];
    for (byte i = 0; i<9; i++) {
		if (i < sToSend.length())
			cMessage[i+3] = sToSend.charAt(i);
		else if (i < sToSend.length() + strlen(valueToSend))
			cMessage[i+3] = valueToSend[i - sToSend.length()];
		else
			cMessage[i+3] = '-';
    }
	Serial.print(cMessage);
	Udp.beginPacket(sHostName, iPort);
	Udp.write(cMessage);
    	Udp.endPacket();
	Serial.flush();
}

void LLAPUDP::sendInt(String sToSend, int value)
{
    //           111    
	// 0123456789012
	// aAA0123456789
	// aAA-32767----
	// aAA32768-----
	
	char cValue[7];		// long enough for -32767 to 32768 and the trailing zero
	itoa(value, cValue,10);
	byte cValuePtr = 0;

    cMessage[0] = 'a';
    cMessage[1] = deviceId[0];
    cMessage[2] = deviceId[1];
    for (byte i = 0; i<9; i++) {
		if (i < sToSend.length())
			cMessage[i+3] = sToSend.charAt(i);
		else if (cValuePtr < 7 && cValue[cValuePtr] !=0)
			cMessage[i+3] = cValue[cValuePtr++];
		else
			cMessage[i+3] = '-';
    }
	Serial.print(cMessage);
	Udp.beginPacket(sHostName, iPort);
	Udp.write(cMessage);
    Udp.endPacket();
	Serial.flush();
}

void LLAPUDP::sendIntWithDP(String sToSend, int value, byte decimalPlaces)
{
    //           111    
	// 0123456789012
	// aAA0123456789
	// aAA-3276.7---
	// aAA3276.8----
	
	char cValue[8];		// long enough for -3276.7 to 3276.8 and the trailing zero
	byte cValuePtr=0;
	itoa(value, cValue,10);
	char* cp = &cValue[strlen(cValue)];
	*(cp+1) = 0;	// new terminator
	while (decimalPlaces-- && --cp )
	{
		*(cp+1) = *cp;
	}
	*cp = '.';

    cMessage[0] = 'a';
    cMessage[1] = deviceId[0];
    cMessage[2] = deviceId[1];
    for (byte i = 0; i<9; i++) {
		if (i < sToSend.length())
			cMessage[i+3] = sToSend.charAt(i);
		else if (cValuePtr < 8 && cValue[cValuePtr] !=0)
			cMessage[i+3] = cValue[cValuePtr++];
		else
			cMessage[i+3] = '-';
    }
	Serial.print(cMessage);
	Udp.beginPacket(sHostName, iPort);
	Udp.write(cMessage);
    Udp.endPacket();
	Serial.flush();
}

void LLAPUDP::sendIntWithPad(String sToSend, int value, byte length)
{
	//           111
	// 0123456789012
	// aAAcccc-32767
	// aAAcccc000327
	
	char cValue[7];		// long enough for -32767 to 32768 and the trailing zero
	itoa(value, cValue,10);
	byte cValuePtr = 0;

    cMessage[0] = 'a';cMessage[1] = deviceId[0];
    cMessage[2] = deviceId[1];
    for (byte i = 0; i<9; i++) {
		if (i < sToSend.length())
			cMessage[i+3] = sToSend.charAt(i);
		else if (i < sToSend.length() + length - strlen(cValue))		
			cMessage[i+3] = '0';
		else if (cValuePtr < 7 && cValue[cValuePtr] !=0)
			cMessage[i+3] = cValue[cValuePtr++];
		else
			cMessage[i+3] = '-';
    }
	Serial.print(cMessage);
	Udp.beginPacket(sHostName, iPort);
	Udp.write(cMessage);
    Udp.endPacket();
	Serial.flush();
}

void LLAPUDP::sendIntWithTerminator(String sToSend, int value, byte length, char terminator)
{
	//           111    
	// 0123456789012
	// aAA0123456789
	// aAA-32767----
	
	char cValue[7];		// long enough for -32767 to 32768 and the trailing zero
	itoa(value, cValue,10);
	byte cValuePtr = 0;

    cMessage[0] = 'a';
    cMessage[1] = deviceId[0];
    cMessage[2] = deviceId[1];
    for (byte i = 0; i<9; i++) {
		if (i < sToSend.length())
			cMessage[i+3] = sToSend.charAt(i);
		else if (i < sToSend.length() + length - strlen(cValue))		
			cMessage[i+3] = '0';
		else if (cValuePtr < 7 && cValue[cValuePtr] !=0)
			cMessage[i+3] = cValue[cValuePtr++];
		else if (cValue[cValuePtr] == 0)
			cMessage[i+3] = terminator;
		else
			cMessage[i+3] = '-';
    }
	Serial.print(cMessage);
	Udp.beginPacket(sHostName, iPort);
	Udp.write(cMessage);
    Udp.endPacket();
	Serial.flush();
}

void LLAPUDP::setDeviceId(char* cId)
{
    deviceId[0] = cId[0];
    deviceId[1] = cId[1];
}

void LLAPUDP::setRemoteConnection(IPAddress host, int port)
{
	sHostName = host;
	iPort = port;
	Udp.begin(port);
}
