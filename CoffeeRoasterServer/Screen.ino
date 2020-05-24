#include "icon.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
// from http://colormind.io/
#define COFFEE_BROWN 0xA38B
#define COFFEE_GREEN 0x0652
#define COFFEE_YELLOW 0xE6EF
#define COFFE_RED 0xB1E7

#define ROW_ONE_X 0
#define ROW_ONE_Y 0
#define ROW_TWO_X 0
#define ROW_TWO_Y 85
#define ROW_THREE_X 0
#define ROW_THREE_Y 160
#define ROW_FOUR_X 0
#define ROW_FOUR_Y 240
#define ICON_SIZE 84
#define ICON_TEXT_MARGIN 25
#define TEXT_BLOCK_HEIGHT 84
#define TEXT_BLOCK_WIDTH 120

Adafruit_ILI9341 tft = Adafruit_ILI9341(0, 1);

void setupScreen(int tftCs, int tftDc) {
	tft = Adafruit_ILI9341(tftCs, tftDc);
	tft.begin();
	tft.setRotation(4);

	tft.fillScreen(ILI9341_BLACK);
	tft.setTextSize(3);
	tft.setCursor(0, 0);
}

void setTextSize(int fontsize) {
	tft.setTextSize(fontsize);
}

void displayNewMessage(String message) {
	tft.fillScreen(ILI9341_BLACK);
	tft.setCursor(0, 0);
	tft.println(message);
}

void displayNewMessages(String messages[], int n)
{
	tft.fillScreen(ILI9341_BLACK);
	tft.setCursor(0, 0);

	for (int i = 0; i < n; i++) {
		tft.println(messages[i]);
	}
}

void drawHeater(bool heaterOnStatus) {
	int xOffset = ROW_FOUR_X + ICON_SIZE + ICON_TEXT_MARGIN;
	tft.fillRect(xOffset, ROW_FOUR_Y, TEXT_BLOCK_WIDTH - ICON_TEXT_MARGIN, TEXT_BLOCK_HEIGHT - ICON_TEXT_MARGIN, ILI9341_BLACK);
	tft.setCursor(xOffset, ROW_FOUR_Y + ICON_TEXT_MARGIN);

	if (heaterOnStatus)
	{
		tft.print("On");
	}
	else
	{
		tft.print("Off");
	}
	
}

void drawHomeScreen() {
	tft.fillScreen(ILI9341_BLACK);
	tft.drawBitmap(ROW_ONE_X, ROW_ONE_Y, coffeeBeanIcon, ICON_SIZE, ICON_SIZE, COFFEE_BROWN);
	tft.drawBitmap(ROW_TWO_X, ROW_TWO_Y, pidTemperatureIcon, ICON_SIZE, ICON_SIZE, COFFEE_YELLOW);
	tft.drawBitmap(ROW_THREE_X, ROW_THREE_Y, timeIcon, ICON_SIZE, ICON_SIZE, COFFEE_GREEN);
	tft.drawBitmap(ROW_FOUR_X, ROW_FOUR_Y, heaterOnIcon, ICON_SIZE, ICON_SIZE, COFFE_RED);
	tft.setTextSize(3);
	tft.setTextColor(ILI9341_WHITE);
	int xOffset = ROW_THREE_X + ICON_SIZE + ICON_TEXT_MARGIN;
	tft.setCursor(xOffset, ROW_THREE_Y + ICON_TEXT_MARGIN);
}

void drawSystemScreen() {
	tft.fillScreen(ILI9341_BLACK);
	tft.drawBitmap(ROW_ONE_X, ROW_ONE_Y, wiFiIcon, ICON_SIZE, ICON_SIZE, COFFEE_GREEN);
	tft.drawBitmap(ROW_TWO_X, ROW_TWO_Y, hddIcon, ICON_SIZE, ICON_SIZE, COFFEE_YELLOW);
	tft.setTextSize(2);
	tft.setTextColor(ILI9341_WHITE);
}

void drawTemperatureSection(int temperature) {
	int xOffset = ROW_ONE_X + ICON_SIZE + ICON_TEXT_MARGIN;
	tft.fillRect(xOffset, ROW_ONE_Y, TEXT_BLOCK_WIDTH + ICON_TEXT_MARGIN, TEXT_BLOCK_HEIGHT - ICON_TEXT_MARGIN, ILI9341_BLACK);
	tft.setCursor(xOffset, ROW_ONE_Y + ICON_TEXT_MARGIN);
	String outputText = (String)temperature + "`C";
	tft.print(outputText);
}

void drawTemperatureLimit(int setTemp) {
	int xOffset = ROW_TWO_X + ICON_SIZE + ICON_TEXT_MARGIN;
	tft.fillRect(xOffset, ROW_TWO_Y, TEXT_BLOCK_WIDTH + ICON_TEXT_MARGIN, TEXT_BLOCK_HEIGHT - ICON_TEXT_MARGIN, ILI9341_BLACK);
	tft.setCursor(xOffset, ROW_TWO_Y + ICON_TEXT_MARGIN);
	String outputText = (String)setTemp + "`C";
	//String outputText = (String)dhtTemp + "`C";
	tft.print(outputText);
}

void drawTimeUpdate(int minute, int second) {
	int xOffset = ROW_THREE_X + ICON_SIZE + ICON_TEXT_MARGIN;
	tft.fillRect(xOffset, ROW_THREE_Y, TEXT_BLOCK_WIDTH + TEXT_BLOCK_WIDTH, TEXT_BLOCK_HEIGHT - ICON_TEXT_MARGIN, ILI9341_BLACK);
	tft.setCursor(xOffset, ROW_THREE_Y + ICON_TEXT_MARGIN);
	String outputText = (String)minute + ":" + (String)second;
	tft.print(outputText);
}
