/***************************************************************************************
  To use this sketch you must install the TTN ESP32 BLE Provisioning APP from gGoogle Play
  https://play.google.com/store/apps/details?id=org.rgot.BLE_TEST
  this program was tested with heltech boards (Heltec Wifi Lora 32 & Heltec Wireless Stick)

  When running press the user button (named PROG on board)  the on board LED_BUILTIN Light 
  and the Bluetooth Low Energy (BLE) is power on.
  On the Android application you can see a new device named "RGOT_xxx" where xxx is the hardware devEui.

  When you tap on this device, you can complete a form with devEUI, AppEui and AppKey.
  On the keyboard press Save (enter) to save the parameter.
  When finish press the back arrow, then the esp32 board reboot and take your new parameters.

*/
#include <EzLoRaWAN_BLE.h>
#include <EzLoRaWAN_CayenneLPP.h>
#include <EzLoRaWAN.h>

// wireless stick pinout
#ifndef LED_BUILTIN
#define LED_BUILTIN 25
#endif // !LED_BUILTIN
#ifndef KEY_BUILTIN
#define  KEY_BUILTIN 0
#endif // !1

#define INTERVAL 30000
EzLoRaWAN ttn;
EzLoRaWAN_BLE ble;
EzLoRaWAN_CayenneLPP lpp;

#ifndef AUTO_PIN_MAP // AUTO_PIN_MAP is set if board is defined in the file target-config.h
#include "board_config.h"
#endif // !AUTO_PINS
void setup() {
	
    Serial.begin(115200);
    Serial.println("Starting");
	ttn.begin();
	if (ttn.restoreKeys()) {// provisioning with restored key from NVS
		Serial.println("devEui : "+ ttn.getDevEui());
		Serial.println("appEui : "+ ttn.getAppEui());
		ttn.join();
		Serial.println("joining");
	}
	else
	{
		Serial.println("No key are provisioned, please press the User Button\n\
then launch the \'TTN ESP32 Prosioning\' Android App \n\
Select the RGOT_... device and provisioning the keys\n\
Quit the android App, then the esp32 restart...	");
	}
	pinMode(KEY_BUILTIN, INPUT);
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
	static bool jonction = false;
	static float nb = 0.0;
	static unsigned previoumillis = 0;
	if (!digitalRead(KEY_BUILTIN) && !ble.getInitialized())
	{
		ttn.stop();
		ble.begin();			
			digitalWrite(LED_BUILTIN, HIGH);	
		while (!digitalRead(KEY_BUILTIN));
			
	}
	
	if (ttn.isJoined())
	{
		if (!jonction)
		{
			ttn.showStatus();
			jonction = true;
		}
		if (millis() - previoumillis > INTERVAL)
		{
			if (ttn.isRunning())
			{
			nb += 0.1;
			lpp.reset();
			lpp.addTemperature(1, nb);
			if (ttn.sendBytes(lpp.getBuffer(), lpp.getSize(), 1, 1)) {
				Serial.printf("envoi reussi : %d %x %02X%02X\n", lpp.getBuffer()[0], lpp.getBuffer()[1], lpp.getBuffer()[2], lpp.getBuffer()[3]);
			}
			}
			
			previoumillis = millis();
		}
	}
	else {
		if (ttn.isRunning())
		{
			Serial.print(".");
			delay(500);
		}
			
		}	   	 	

 }
