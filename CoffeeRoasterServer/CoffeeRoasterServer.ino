/*
 Name:		CoffeeRoasterServerV2.ino
 Created:	28-Mar-20 8:26:28 PM
 Author:	Tomas
*/

#include <ESP8266WiFi.h>
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <Adafruit_MLX90614.h>
#include <math.h>
#include <Wire.h>

#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07

#define SSR_OUT 15 //D8
#define TFT_DC 2  //D4
#define TFT_CS 0  //D3
#define MAX_PROFILE_SIZE 20
#define MAX_ROAST_POINTS 1800
#define MAX_TEMP_READ_ATTEMPTS 5

long timer = 0;
volatile bool heaterOn = false;
int tempReadAttempts = 0;
int temperature = 0;
int chamberTemperature = 0;
int tempStep = 0;
bool timerEnabled;

typedef struct {
	int StagePoint;
	int secondsStart;
	int secondsEnd;
	int maxTemperature;
} RoastProfile;

int roastPoints[MAX_ROAST_POINTS];
int roastPointCounter;
RoastProfile roastProfile[MAX_PROFILE_SIZE];
int roastProfileSize;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
	Serial.begin(115200);

	pinMode(SSR_OUT, OUTPUT);

	digitalWrite(SSR_OUT, LOW);

	setupScreen(TFT_CS, TFT_DC);

	setupServer(ssid, password);

	setupLoadedProfile();

	delay(500);

	mlx.begin();
	displayNewMessage(WiFi.localIP().toString());
}

void loop() {
	serverLoop();
}

bool readStoredProfile()
{
	StaticJsonDocument<2048> jsonBuffer;
	File file = SPIFFS.open("/newRoast.json", "r");
	int profileBufferLength = file.size() + 1;
	char profileBuffer[profileBufferLength]; 
	file.readString().toCharArray(profileBuffer, profileBufferLength);

	DeserializationError root = deserializeJson(jsonBuffer, profileBuffer);
	if (!root)
	{
		JsonArray RoastPoints = jsonBuffer["RoastPoints"];
		roastProfileSize = RoastPoints.size();
		for(int i = 0; i < roastProfileSize; i++)
		{
			const char* stageName = RoastPoints[i]["StageName"];
			int temperature = RoastPoints[i]["Temperature"];
			int StartSeconds = RoastPoints[i]["StartSeconds"];
			int EndSeconds = RoastPoints[i]["EndSeconds"];
			int StagePoint = RoastPoints[i]["StagePoint"];
			roastProfile[i].StagePoint = StagePoint;
			roastProfile[i].secondsStart = StartSeconds;
			roastProfile[i].secondsEnd = EndSeconds;
			roastProfile[i].maxTemperature = temperature;
		}
		file.close();
		return true;
	}
	else
	{
		file.close();
		return false;
	}
}	

String getStoredProfile()
{
	File file = SPIFFS.open("/newRoast.json", "r");
	String profileString = file.readString();
	file.close();
	return profileString;
}

void handleNewProfile(String profileMessage) {
	File file = SPIFFS.open("/newRoast.json", "w");
	file.print(profileMessage);
	file.close();
	displayNewMessage("Completed saving profile");
	profileMessage = "";	
}

void setupLoadedProfile()
{
	setTextSize(2);
	displayNewMessage("Setting up profile");
	float priorTemperature = 0;
	bool priorTemperatureSet = false;
	roastPointCounter = 0;
	// cycle through the rest of the profile segments
	for(int i = 0; i < roastProfileSize; i++)
	{
		if (!priorTemperatureSet) {
			priorTemperatureSet = true;
			priorTemperature = roastProfile[i].maxTemperature;
			for (int j = roastProfile[i].secondsStart; j < roastProfile[i].secondsEnd; j++) {
				roastPoints[roastPointCounter] = priorTemperature;
				roastPointCounter++;
			}
			continue;
		}

		int timeStartSeconds = roastProfile[i].secondsStart;
		int timeEndSeconds = roastProfile[i].secondsEnd;
		int temperaturePoint = roastProfile[i].maxTemperature;
		float temperatureDelta = temperaturePoint - priorTemperature;
		int timeDelta = timeEndSeconds - timeStartSeconds;

		if (timeDelta <= 0)
			continue;

		float temperatureStep = temperatureDelta / timeDelta;
		if (roastPointCounter > MAX_ROAST_POINTS)
		{
			displayNewMessage("Reached maximum profile size of " + (String)MAX_PROFILE_SIZE);
			break;
		}
		for (int j = 0; j < timeDelta; j++ )
		{
			priorTemperature += temperatureStep;
			roastPoints[roastPointCounter] = round(priorTemperature);
			roastPointCounter++;
		}
	}
	int totalProfileTimeMinutues = roastPointCounter / 60;
	displayNewMessage("Profile Setup \nRoastPoints: " + (String)roastProfileSize + "\nTotal Roast time\n\nTotal Minutes:" + (String)totalProfileTimeMinutues + "\nTotal Seconds:" + (String)roastPointCounter);
}

void checkHeater(int setTemp) {
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
	drawHeater(heaterOn);
}

int getTemperature() {
	temperature = (int)mlx.readObjectTempC();
	if (temperature > 1000) {
		if (tempReadAttempts >= MAX_TEMP_READ_ATTEMPTS)
			tempReadAttempts = 0;
			return -1;

		drawTemperatureSection(-99);
		getTemperature();
		tempReadAttempts += 1;
	}
	tempReadAttempts = 0;
	drawTemperatureSection(temperature);
	return temperature;
}

void updateTime() {
	int seconds = 0;
	int minutes = 0;
	
	if (timer < 60)
	{
		seconds = timer;
	}
	else if (timer > 60)
	{
		minutes = timer / 60;
		seconds = timer % 60;
	}
	timer++;
	drawTimeUpdate(minutes, seconds);
}
