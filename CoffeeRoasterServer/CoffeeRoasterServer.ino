/*
 Name:		CoffeeRoasterServerV2.ino
 Created:	28-Mar-20 8:26:28 PM
 Author:	Tomas
*/

#include <ESP8266WiFi.h>
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "icon.h"


#define SSR_OUT 15 //D8
#define TFT_DC 2  //D4
#define TFT_CS 0  //D3

const char* ssid = "chorizoLAN";
const char* password = "cubal1br313";

int lastTemp = 0;
long timer = 0;
int symbol = 0;
volatile bool heaterOn = false;
int setTemp = 160;
int temperature = 0;
int chamberTemperature = 0;
int tempStep = 0;
bool timerEnabled;

struct RoastProfile {
	int stage;
	int secondsStart;
	int secondsEnd;
	int maxTemperature;
};

struct RoastProfile roastProfile;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
	mlx.begin();

	Serial.begin(115200);

	pinMode(SSR_OUT, OUTPUT);

	digitalWrite(SSR_OUT, LOW);

	setupScreen(TFT_CS, TFT_DC);

	setupServer(ssid, password);

	delay(500);

	mlx.begin();
}

void loop() {
	serverLoop();
}

void readStoredProfile(int stageIndex)
{
	StaticJsonDocument<1000> jsonBuffer;
	File file = SPIFFS.open("/newRoast.json", "r");
	DeserializationError root = deserializeJson(jsonBuffer, file.readString());
	if (!root)
	{
		roastProfile.secondsStart = jsonBuffer[(String)stageIndex]["SecondsStart"];
		roastProfile.secondsEnd = jsonBuffer[(String)stageIndex]["SecondsEnd"];
		roastProfile.maxTemperature = jsonBuffer[(String)stageIndex]["Temperature"];
		String message[] = { (String)roastProfile.secondsStart, (String)roastProfile.secondsEnd,(String)roastProfile.maxTemperature };
		displayNewMessages(message, 3);
	};
}

void handleNewProfile(String profileMessage) {
	 StaticJsonDocument<1000> jsonBuffer;
	 DeserializationError root = deserializeJson(jsonBuffer, profileMessage);
	 if (!root) {
	 	File file = SPIFFS.open("/newRoast.json", "w");
	 	file.print(profileMessage);
	 	file.close();
	 	displayNewMessage("Completed saving profile");
	 }
	 else {
	 	displayNewMessage("Invalid profile");
	 }
}


void calculateSetTempPoint()
{
	setTemp = roastProfile.maxTemperature;
}

String getFileListFromServer() {
	String str = "";
	Dir dir = SPIFFS.openDir("/");
	while (dir.next())
	{
		str += dir.fileName();
		str += " / ";
		str += dir.fileSize();
		str += "\n";
	}

	return str;
}

void checkHeater()
{
	String heaterOnStatus = "OFF";
	if (temperature < setTemp && !heaterOn)
	{
		heaterOn = true;
		digitalWrite(SSR_OUT, HIGH);
	}

	if (temperature > setTemp && heaterOn)
	{
		heaterOn = false;
		digitalWrite(SSR_OUT, LOW);
	}

	if (heaterOn)
	{
		heaterOnStatus = "ON";
	}
	drawHeater(heaterOnStatus);
}

String getTemperature() {
	temperature = (int)mlx.readObjectTempC();
	chamberTemperature = (int)mlx.readAmbientTempC();
	if (temperature > 1000) {
		String data = "";
		data += (String)lastTemp;
		data += ",";
		data += "";
		data += ",";
		data += (String)timer;
		return data;
	}
	lastTemp = temperature;
	String data = "";
	data += (String)temperature;
	data += ",";
	data += (String)chamberTemperature;
	data += ",";
	data += (String)timer;
	return data;
}

void updateTemperature()
{
	int currentTemperature = (int)mlx.readObjectTempC();
	temperature = currentTemperature > 1000 ? lastTemp : currentTemperature;

	drawTemperatureSection(temperature);
}

void updateTime()
{
	int seconds = 0;
	int minutes = 0;
	if (timerEnabled) {
		if (timer < 60)
		{
			seconds = timer;
		}
		else if (timer > 60)
		{
			minutes = timer / 60;
			seconds = timer % 60;
		}

		drawTimeUpdate(minutes, seconds);
	}
	else
	{
		timer = 0;
	}
}
