/*
 * Project: nRF905 AVR/Arduino Library/Driver
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/nrf905-avrarduino-librarydriver/
 */

/*
 * Turn the nRF905 on to transmit some data, wait for a reply,
 * turn off and wait for a second.
 * Output power is set to the lowest setting, receive sensitivity is
 * lowered and uses the power up/down feature of the nRF905.
 *
 * 7 -> CE
 * 8 -> PWR
 * 9 -> TXE
 * 2 -> CD
 * 3 -> DR
 * 10 -> CSN
 * 12 -> SO
 * 11 -> SI
 * 13 -> SCK
 */

#include <nRF905.h>
#include <SPI.h>

// Frequency band
// NRF905_BAND_433
// NRF905_BAND_868
// NRF905_BAND_915

#define RXADDR {0xFE, 0x4C, 0xA6, 0xE5} // Address of this device (4 bytes)
#define TXADDR {0x58, 0x6F, 0x2E, 0x10} // Address of device to send to (4 bytes)
#define NRF905_FREQ 433200000UL
#define NRF905_BAND NRF905_BAND_433

#define TIMEOUT 1000 // 1 second ping timeout

void setup()
{
	// Start up
	nRF905_init();
	
	// Set address of this device
	byte addr[] = RXADDR;
	nRF905_setRXAddress(addr);
	
	// Output power
// n means negative, n10 = -10
// NRF905_PWR_n10 (-10dBm = 100uW)
// NRF905_PWR_n2 (-2dBm = 631uW)
// NRF905_PWR_6 (6dBm = 4mW)
// NRF905_PWR_10 (10dBm = 10mW)

	// Max transmit level 10db
	nRF905_setTransmitPower(NRF905_PWR_10);
	
	//set band and channel, see nRF905.h for details
	
	nRF905_setChannel(433MHz band, 0 channel);
	
	// Put into receive mode
	nRF905_receive();

	Serial.begin(9600);
	
	Serial.println(F("Client started"));
}

void loop()
{
	static byte counter;

	// Make data
	char data[NRF905_MAX_PAYLOAD] = {0};
	sprintf(data, "test %hhu", counter);
	counter++;

	// Turn on module
	nRF905_powerUp();
        Serial.println(F("Starting the radio module"));

	unsigned long startTime = millis();

	// Set address of device to send to
	byte addr[] = TXADDR;
	nRF905_setTXAddress(addr);
   
        // Set payload data

	nRF905_setData(data, sizeof(data));

	// Send payload (send fails if other transmissions are going on, keep trying until success)
	while(!nRF905_send());

	// Put into receive mode
	nRF905_receive();

	// Make buffer for reply
	byte buffer[NRF905_MAX_PAYLOAD];
	bool success;

	// Wait for reply with timeout
	unsigned long sendStartTime = millis();
	while(1)
	{
		success = nRF905_getData(buffer, sizeof(buffer));
		if(success)// Got data
			break;

		// Timeout
		if(millis() - sendStartTime > TIMEOUT)
			break;
	}

	// Turn off module
	nRF905_powerDown();

	if(success)
	{
		unsigned int totalTime = millis() - startTime;
		Serial.print(F("Ping time: "));
		Serial.print(totalTime);
		Serial.println(F("ms"));

		// Printout ping contents
		Serial.print(F("Data from server: "));
		Serial.write(buffer, sizeof(buffer));
		Serial.println();
	}
	else
		Serial.println(F("Ping timed out"));

	delay(1000);
}