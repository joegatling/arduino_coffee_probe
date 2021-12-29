// Temperature Probe 2021 - Trinket M0

#include <Wire.h>
#include <Thermistor.h>
#include <NTC_Thermistor.h>
#include <Adafruit_DotStar.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1327.h>

#include "CircularBuffer.h"

#define REFERENCE_RESISTANCE   47000
#define NOMINAL_RESISTANCE     100000
#define NOMINAL_TEMPERATURE    25
#define B_VALUE                3950
#define ITERATIONS             3

#define GRID_COLOR 0x1
#define TEMP_COLOR SSD1327_WHITE
#define GRAPH_COLOR SSD1327_WHITE

#define GRID_SPACING_X 40 //pixels
#define GRID_SPACING_Y 10  //celsius

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 128 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1327 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000);

// Delay (millis) between polling for temperature
#define POLLING_DELAY 125

// One data point will be stored every time this number of reads
#define DATA_STORE_INTERVAL 16
#define DATA_STORE_SIZE SCREEN_WIDTH

// Range of temperatures to display
#define MIN_TEMP max(storedData.Min() - 5, 45)
#define MAX_TEMP max(storedData.Max() + 5, 110)

#define ERROR_TEMP -100

CircularBuffer storedData = CircularBuffer(DATA_STORE_SIZE);
CircularBuffer filteredData = CircularBuffer(DATA_STORE_INTERVAL);

int storedDataTimer = 0;

Thermistor* thermistor;
Adafruit_DotStar strip = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);

void setup()
{
  // start serial port
  Serial.begin(57600);

  thermistor = new NTC_Thermistor(
    A4,
    REFERENCE_RESISTANCE,
    NOMINAL_RESISTANCE,
    NOMINAL_TEMPERATURE,
    B_VALUE
  );

  Serial.println("Thermistor Initialzied");

  strip.begin();

  if ( ! display.begin(0x3C) ) {
    Serial.println("Unable to initialize OLED");
    while (1) yield();
  }
  else
  {
    Serial.println("Begin");
  }

  display.setRotation(0);
  storedDataTimer = DATA_STORE_INTERVAL + 1;
}

void loop()
{
  display.clearDisplay();

  float temp = 0;
  for (int i = 0; i < ITERATIONS; i++)
  {
    temp += thermistor->readCelsius();
    delay(10);
  }
  temp /= ITERATIONS;

  filteredData.Add(temp);
  float average = filteredData.Average();
 
  Serial.print("temp:");
  Serial.println(temp);
  Serial.print("filtered:");
  Serial.println(average);

  storedDataTimer++;
  filteredData.Add(temp);

  if (storedDataTimer >= DATA_STORE_INTERVAL)
  {
    storedData.Add(temp);
    storedDataTimer = 0;
  }
  else
  {
    storedData.UpdateLast(average);
  }

  updateDisplay();
  
  delay(POLLING_DELAY);
}

void updateDisplay()
{
  if (storedData.Last() > MIN_TEMP)
  {
    String tempString = String(round(storedData.Last()));
    int textSize = 1;

    int width = (tempString.length()) * (6 * textSize);
    int offset = -width - 2;

    drawGridlines(offset);
    drawTemperatureGraph(offset);

    display.setTextSize(textSize);           // Normal 1:1 pixel scale
    display.setTextColor(TEMP_COLOR);        // Draw white text

    int y = constrain(getYCoordinateForTemperature(storedData.Last()) - 4, 0, SCREEN_HEIGHT - (8 * textSize));

    display.setCursor(SCREEN_WIDTH - width, y);
    display.println(tempString);
  }
  else
  {
    drawBouncingText();
  }

  updateLED();
  
  drawDebugData();

  display.display();  
}

void updateLED()
{
  if (storedData.Last() > MIN_TEMP)
  {
    float t =  max(0.0f, min(1.0f, ((float)storedData.Last() - MIN_TEMP) / (float)(MAX_TEMP - MIN_TEMP)));
    int h = (((360 - 220) * t / 360.0f) + (220 / 360.0f)) * 65536;

    strip.setPixelColor(0, strip.ColorHSV(h));
    strip.setBrightness(6);
  }
  else
  {
    if(storedData.Last() < ERROR_TEMP)
    {
      strip.setPixelColor(0, 255, 0, 0);
      strip.setBrightness(255);
    }
    else
    {
      strip.setBrightness(0);
    }
    
    strip.show();
  } 
}

void drawGridlines(int offset)
{
  int index = storedData.GetCount();
  if (index >= DATA_STORE_SIZE)
  {
    index = storedData.GetStartIndex();
  }

  int x = (SCREEN_WIDTH + offset) - index % GRID_SPACING_X;

  while (x >= 0)
  {
    display.drawLine(x, 0, x, SCREEN_HEIGHT, GRID_COLOR);
    x -= GRID_SPACING_X;
  }

  float temp = floor(MAX_TEMP / GRID_SPACING_Y) * GRID_SPACING_Y;
  int y = getYCoordinateForTemperature(temp);

  display.setTextSize(1);
  display.setTextColor(GRID_COLOR+2);        // Draw white text

  while (y < SCREEN_HEIGHT + 10)
  {
    display.setCursor(0, y - 8);    
    display.println((int)temp);
    
    display.drawLine(0, y, SCREEN_WIDTH, y, 0x2);
    temp -= GRID_SPACING_Y;
    y = getYCoordinateForTemperature(temp);
  }

  display.drawLine(SCREEN_WIDTH + offset, 0, SCREEN_WIDTH + offset, SCREEN_HEIGHT, 0x2);
}

void drawTemperatureGraph(int offset)
{
  int startX = SCREEN_WIDTH + offset - storedData.GetCount() - 1;
  int currentY = getYCoordinateForTemperature(storedData.Get(0));

  for (int i = 1; i < storedData.GetCount(); i++)
  {
    int newY = getYCoordinateForTemperature(storedData.Get(i));

    display.drawLine(startX + i - 1 , currentY, startX + i, newY, GRAPH_COLOR);

    currentY = newY;
  }

  display.drawLine(startX + storedData.GetCount() , currentY, startX + storedData.GetCount() + 1, getYCoordinateForTemperature(storedData.Last()), GRAPH_COLOR);
}

void drawBouncingText()
{
  int currentTemp = round(storedData.Last());

  String tempString = String(currentTemp);

  int xOffset = 0;
  int yOffset = 0;

  int textSize = 5;

  if (currentTemp < ERROR_TEMP)
  {
    textSize = 2;
    tempString = "ERROR";
  }

  int width = (tempString.length()) * (6 * textSize);

  int yRange = (SCREEN_HEIGHT - (8 * textSize)) / 2;
  int xRange = (SCREEN_WIDTH - width) / 2;

  xOffset = xRange + sin(millis() / 10000.0f) * xRange;
  yOffset = yRange + sin(millis() / 9100.0f) * yRange;

  display.setTextSize(textSize);           // Normal 1:1 pixel scale
  display.setTextColor(TEMP_COLOR);        // Draw white text

  int y = constrain(getYCoordinateForTemperature(storedData.Last()) - 4, 0, SCREEN_HEIGHT - (8 * textSize));

  display.setCursor(SCREEN_WIDTH - width - xOffset, y - yOffset);
  display.println(tempString);
}

void drawDebugData()
{
  display.setTextSize(1);
  display.setTextColor(SSD1327_WHITE);
  display.setCursor(0,SCREEN_HEIGHT-10);
  display.print(storedData.GetCount());
  display.print(" ");
  display.println((int)millis()/1000);
}

int getYCoordinateForTemperature(float temperature)
{
  float distancePerDegree = (float)SCREEN_HEIGHT / (float)(MAX_TEMP - MIN_TEMP);
  float baseLine = MAX_TEMP * distancePerDegree;

  return (int)(baseLine - temperature * distancePerDegree);
}
